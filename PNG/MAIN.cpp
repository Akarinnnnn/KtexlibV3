// PNG.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include <iostream>
#include "../ktexlib3/导入头/texfile.h"
#include "arg_parser.h"
#include <wincodec.h>
#include "MAIN.h"
#include <wil/com.h>
#include <DirectXTex.h>
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
	using namespace ktexlib;
	v3::init_COM_as_mthread();
	std::filesystem::path path = parser.GetString(L"path");
	std::wstring outpath = parser.GetString(L"out");
	if (outpath[0] == L'\0') outpath = path.replace_extension(L".tex");
	wcout << path << L" -> " << outpath << std::endl;

	auto ktex = v3detail::load_and_compress(path, v3detail::PixelFormat::dxt5, false);

	ktex.SaveFile(outpath);
}

void ktex2png(ArgumentParser& parser)
{
	ktexlib::v3::init_COM_as_mthread();

	auto ktex = ktexlib::v3::load_ktex(parser.GetString(L"path").c_str());
	ktexlib::v3detail::RgbaImage out = ktexlib::v3detail::decompress(ktex[0], ktex.info.pixelFormat);

	bool iswic2 = false;
	auto wicfactory = DirectX::GetWICFactory(iswic2);

	wil::com_ptr<IWICBitmap> wic_bmp;
	THROW_IF_FAILED_MSG(wicfactory->CreateBitmapFromMemory(
		out.width, out.height, GUID_WICPixelFormat32bppRGBA,
		out.pitch, out.data.size(), out.data.data(),
		&wic_bmp), "create wic bmp");

	wil::com_ptr<IWICStream> stream;
	THROW_IF_FAILED_MSG(wicfactory->CreateStream(&stream), "Create wic stream");

	auto outpath = parser.GetString(L"out");
	if (outpath[0] == L'\0')
		outpath = std::filesystem::path(parser.GetString(L"path")).replace_extension(L".png");

	THROW_IF_FAILED_MSG(stream->InitializeFromFilename(outpath.c_str(), GENERIC_WRITE), "init out stream");

	wil::com_ptr<IWICBitmapEncoder> encoder;
	THROW_IF_FAILED_MSG(wicfactory->CreateEncoder(GUID_ContainerFormatPng, nullptr, &encoder), "create encoder");
	THROW_IF_FAILED_MSG(encoder->Initialize(stream.get(), WICBitmapEncoderNoCache), "init encoder");

	wil::com_ptr<IWICBitmapFrameEncode> encode;
	THROW_IF_FAILED_MSG(encoder->CreateNewFrame(&encode, nullptr), "create frame");
	THROW_IF_FAILED_MSG(encode->Initialize(nullptr), "init frame");

	THROW_IF_FAILED_MSG(encode->WriteSource(wic_bmp.get(), nullptr), "write frame");
	THROW_IF_FAILED_MSG(encode->Commit(), "frame commit");
	THROW_IF_FAILED_MSG(encoder->Commit(), "encoder commit");
	
}
#if 1
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
			catch (const std::exception & e)
			{
				std::cout << e.what();
				CoUninitialize();
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
			CoUninitialize();
			return 0;
		}
		catch (const wil::ResultException & e)
		{
			using namespace std;
			cout << "err\n\tHRESULT = " << hex << e.GetErrorCode() << " MSG:" << dec;
			wcout << e.GetFailureInfo().pszMessage << endl;
			CoUninitialize();
			return 2;
		}
		catch (const std::exception & e)
		{
			std::cout << e.what();
			CoUninitialize();
			return 2;
		}
	}

	if (mode == L"topng" || mode == L"d")
	{
		try
		{
			ktex2png(parser);
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
			CoUninitialize();
			return 2;
		}
	}

	CoUninitialize();
	return 999;
}
#else
int main()
{
	uint8_t a[5 * 2] = { 1,1,1,1,1,2,2,2,2,2 };
	ktexlib::v3detail::filp(a, a + 10, 5);
	return 0;
}
#endif // 0
