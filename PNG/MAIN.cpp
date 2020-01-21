// PNG.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

//#include "wstdout.hpp"
#include <iostream>
#include "../ktexlib3/导入头/texfile.h"
#include "arg_parser.h"
#include <wincodec.h>
#include "MAIN.h"
#include <wil/result.h>
using std::wcout;

wchar_t helpmsg[] = 
L"帮助消息\n"
"直接转换：mypng_v2 [输入图片路径] [输出tex路径]\n"
"其他开关：/mode，用于选择其他工作模式\n"
"                /path，输入\n"
"                /out，输出tex路径"
"";


void gen_bc3_singlemip(ArgumentParser& parser)
{
	ktexlib::v3::init_COM_as_mthread();
	std::filesystem::path path = parser.GetString(L"path");
	auto outpath = parser.GetString(L"out");
	HRESULT hr = S_OK;
	IWICImagingFactory* factory;
	IWICStream* stream;
	IWICBitmapDecoder* decoder;
	IWICBitmapFrameDecode* decoded;
	if (FAILED(hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, (LPVOID*)&factory)))
	{
		MessageBoxW(nullptr, L"创建wic工厂失败", L"¿", MB_OK);
		THROW_HR_MSG(hr, "WIC factory");
	}

	hr = factory->CreateStream(&stream);
	THROW_IF_FAILED_MSG(hr, "create stream");

	hr = stream->InitializeFromFilename(path.c_str(), GENERIC_READ);
	THROW_IF_FAILED_MSG(hr, "init stream");

	hr = factory->CreateDecoderFromFilename(path.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &decoder);
	THROW_IF_FAILED_MSG(hr, "create decoder");

	hr = decoder->Initialize(stream, WICDecodeMetadataCacheOnLoad);
	THROW_IF_FAILED_MSG(hr, "decode");

	hr = decoder->GetFrame(0, &decoded);
	THROW_IF_FAILED_MSG(hr, "get frame");

	{
		using namespace ktexlib::v3detail;
		RgbaImage img;
		decoded->GetSize(&img.width, &img.height);
		img.pitch = ((img.width | 0xFFFFFFFC) + 4);
		img.data.resize((size_t)img.pitch * (size_t)img.height * (size_t)4);
		hr = decoded->CopyPixels(nullptr, img.pitch, img.pitch * img.height * 4, img.data.data());
		THROW_IF_FAILED_MSG(hr, "Copy Pixels");
		auto mip = ktexlib::v3detail::convert(img);
		Ktex out;
		out.AddMipmap(std::move(mip));
		out.SaveFile(outpath);
	}
}

int wmain(int argc, wchar_t** argv)
{
#pragma warning(push)
#pragma warning (disable:26444)
	wcout.imbue(std::locale(""));
#pragma warning(pop)
	if (argc == 3)
	{
		if (argv[1][0] != '/')
		{
			try
			{
				ktexlib::v3::init_COM_as_mthread();
				wcout << argv[1] << L" -> " << argv[2] << L'\n' <<
					ktexlib::v3::gen_bc3universal(argv[1], argv[2]) << L"\n\n";
			}
			catch (const std::exception& e)
			{
				std::cout << e.what();
				return 1;
			}
			return 0;
		}
	}
	ArgumentParser parser;
	parser.SetHelpMessage(helpmsg);

	parser.AddString(L"path");
	parser.AddString(L"mode");
	parser.AddString(L"out");
	
	parser.Parse(argc, argv);
	auto mode = parser.GetString(L"mode");
	
	if (mode == L"bc3-singlemip")
	{
		try
		{
			gen_bc3_singlemip(parser);
			return 0;
		}
		catch (const wil::ResultException& e)
		{
			using namespace std;
			cout << "err\n\tHRESULT = " << hex << e.GetErrorCode() << " MSG:" << dec;
			wcout << e.GetFailureInfo().pszMessage << endl;
			return 2;
		}
		catch (const std::exception & e)
		{
			std::cout << e.what();
			return 2;
		}
	}
	return 999;
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
