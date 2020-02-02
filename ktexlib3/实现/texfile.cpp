#include "../pch.h"
#include "../导入头/texfile.h"
#include <DirectXTex.h>
#include <memory>
#include <wincodec.h>
#include <wil/com.h>
#include <wil/result.h>
#include <iostream>
#include <locale>
#include <fstream>

#define TO_PCHAR(x) reinterpret_cast<char*>(x)
#define TO_CONST_PCHAR(x) reinterpret_cast<const char*>(x)

using std::cout, std::endl;
namespace wicobj
{
	extern wil::com_ptr<IWICImagingFactory> factory;
}

using namespace ktexlib::v3detail;
constexpr inline bool check_4n(unsigned int val)
{
	return !(val & 0xFFFFFFF8u);
}

constexpr inline unsigned int next_4n(unsigned int val)
{
	return (val & 0xFFFFFFF8u) + 4;
}

DXGI_FORMAT dxfmt_from_pixf(PixelFormat fmt);

HRESULT ktexlib::v3::init_COM_as_mthread()
{
	HRESULT hr = CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);
	if (FAILED(hr) && hr != 0x80010106)
	{
		ErrorMsgbox(L"加载COM失败(hresult = %#08X)，即将退出", hr, 10);
	}
	if (FAILED(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_ALL, IID_IWICImagingFactory2, (LPVOID*)&wicobj::factory)))
	{
		hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_ALL, IID_IWICImagingFactory, (LPVOID*)&wicobj::factory);
		if (FAILED(hr))
		{
			ErrorMsgbox(L"创建WIC工厂失败(hresult = %#08X)，即将退出\n这个问题常见于精简版系统", hr, 11);
		}
	};
	return hr;
}

void ktexlib::v3::gen_bc3universal(const char8_t * filename, const char8_t * output)
{
	//cout << std::hex;
	char16_t path[MAX_PATH]{ 0 };
	char16_t out[MAX_PATH]{ 0 };
	{
		mbstate_t mb;
		const char8_t* from_next = nullptr;
		char16_t* to_next = nullptr;
		std::use_facet<std::codecvt<char16_t, char8_t, mbstate_t>>(std::locale("C")).in(mb,
			filename, filename + strlen(reinterpret_cast<const char*>(filename)), from_next,
			path, path + MAX_PATH, to_next
		);
		std::use_facet<std::codecvt<char16_t, char8_t, mbstate_t>>(std::locale("C")).in(mb,
			output, output + strlen(reinterpret_cast<const char*>(output)), 
			from_next, out, out + MAX_PATH, to_next);
	}

	gen_bc3universal((const wchar_t*)path,(const wchar_t*)out);

}

bool ktexlib::v3::gen_bc3universal(const char16_t* path, const char16_t* outpath)
{
	return gen_bc3universal((const wchar_t*)path,(const wchar_t*)outpath);
}

#pragma warning(push)
#pragma warning (disable:28251)
bool ktexlib::v3::gen_bc3universal(const wchar_t* path, const wchar_t* outpath)
#pragma warning(pop)
{
	auto output = load_and_compress(path, v3detail::PixelFormat::dxt5, true);

	std::filesystem::path outfile;
	if (outpath != nullptr) outfile = outpath;
	else
	{
		outfile = path;
		outfile.replace_extension(L".tex");
	}
	output.SaveFile(outfile);
	return true;

}

ktexlib::v3detail::Ktex ktexlib::v3::load_ktex(const char8_t* path)
{
	std::ifstream file;
	file.open(path, std::ios::binary);
	return load_ktex(file);
}

ktexlib::v3detail::Ktex ktexlib::v3::load_ktex(const wchar_t* path)
{
	std::ifstream file;
	file.open(path, std::ios::binary);
	return load_ktex(file);
}

ktexlib::v3detail::Ktex ktexlib::v3::load_ktex(std::ifstream& file)
{
	using namespace ktexlib::v3detail;
	
	if (!file.is_open())
		throw std::system_error(std::make_error_code((std::errc)errno), "文件未打开");

	Ktex ret;
	uint16_t mipscount = 0;
	{
		uint32_t v = 0;

		file.read(TO_PCHAR(&v), 4);
		if (v != 0x5845544BU) throw std::invalid_argument("不是有效的ktex文件");

		file.read(TO_PCHAR(&v), 4);
		ret.info.flags = (v & 0x000C0000) >> 18;
		ret.info.textureType	= (ktexlib::v3detail::TextureType)	((v & 0x00001E00) >> 9);
		ret.info.pixelFormat	= (ktexlib::v3detail::PixelFormat)	((v & 0x000001F0) >> 4);
		ret.info.platform		= (ktexlib::v3detail::Platform)		(v & 0x0000000F);
		mipscount = (v & 0x0003E000) >> 13;
		//ret.mips.reserve(mipscount);
	}

	{
		Mipmap mip{ 0,0,0 };
		unsigned int mipdatasize = 0;
		for (uint16_t i = 0; i < mipscount; i++)
		{
			file.read(TO_PCHAR(&mip.width), 6);
			file.read(TO_PCHAR(&mipdatasize), 4);

			mip.data.resize(mipdatasize);
			file.read(TO_PCHAR(mip.data.data()), mipdatasize);

			ret.AddMipmap(std::move(mip));
		}
	}

	return ret;
}

Ktex ktexlib::v3detail::load_and_compress(std::filesystem::path path, PixelFormat fmt, bool gen_mips, bool pararral)
{
	Ktex ret_val;
	DirectX::ScratchImage out_images;
	{
		DirectX::ScratchImage src;
		DirectX::ScratchImage mipmap_chain;

		THROW_IF_FAILED(DirectX::LoadFromWICFile(path.c_str(), DirectX::WIC_FLAGS_NONE, nullptr, src));

		{
			auto* img0 = src.GetImage(0, 0, 0);
			v3detail::filp(img0->pixels, img0->pixels + img0->slicePitch, img0->rowPitch);
		}

		if (gen_mips)
			THROW_IF_FAILED(DirectX::GenerateMipMaps(src.GetImages(), src.GetImageCount(),
				src.GetMetadata(), DirectX::TEX_FILTER_DEFAULT, 0, mipmap_chain));
			

		THROW_IF_FAILED(DirectX::Compress(mipmap_chain.GetImages(), mipmap_chain.GetImageCount(), mipmap_chain.GetMetadata(),
			dxfmt_from_pixf(fmt),
			DirectX::TEX_COMPRESS_PARALLEL, 1.0f, out_images));
	}

	auto dxmipmaps = out_images.GetImages();
	size_t mipscount = out_images.GetImageCount();

	ktexlib::v3detail::Ktex ret;
	ret.info.pixelFormat = v3detail::PixelFormat::dxt5;
	ret.info.platform = v3detail::Platform::universal;
	ret.info.textureType = v3detail::TextureType::d2;

	for (size_t i = 0; i < mipscount; i++)
	{
		const DirectX::Image& dx = dxmipmaps[i];
		ret.AddMipmap(
			Mipmap{ (uint16_t)dx.width, (uint16_t)dx.height, (uint16_t)dx.rowPitch, std::vector<uint8_t>(dx.pixels, dx.pixels + dx.slicePitch) }
		);
	}
	
	return ret;
}


void ktexlib::v3detail::Ktex::AddMipmap(const Mipmap& mipmap)
{
	mips.push_back(mipmap);
}

void ktexlib::v3detail::Ktex::AddMipmap(Mipmap&& mipmap)
{
	mips.push_back(std::move(mipmap));
}

void ktexlib::v3detail::Ktex::SaveFile(std::filesystem::path path)
{
	//验证mipmap链
	{
		uint16_t width = mips[0].width, height = mips[0].height;
		if (mips.size() > 31) throw std::overflow_error("mipmap超过31个");
		for (auto it = mips.cbegin() + 1; it != mips.cend(); it++)
		{
			if (it->width != (width >>= 1) || it->height != (height >>= 1))
				throw v3::invalid_mipschain();
		}
		//if (width != height != 1)throw v3::invalid_mipschain("mipmap链最终节点大小应为1x1");
	}

	std::ofstream file;
	//file.exceptions(std::fstream::)
	using std::ios;
	file.open(path, ios::trunc | ios::binary | ios::out);
	if (!file.is_open())
		throw std::exception("打开用作写入的文件失败");

	//CC4
	file.write("KTEX", 4);
	
	//文件头+mipmaps数量
	{
		uint32_t v = 0xFFF00000;
		v |= info.flags << 18;
		v |= mips.size() << (size_t)13;
		v |= (uint32_t)info.textureType << 9;
		v |= (uint32_t)info.pixelFormat << 4;
		v |= (uint32_t)info.platform;
		file.write(TO_PCHAR(&v), 4);
	}

	//mipmap链
	{
		uint32_t mipsize = 0;
		for (Mipmap& mip : mips)
		{
#pragma warning(push)
#pragma warning (disable:4267)
			mipsize = mip.data.size();
#pragma warning(pop)
			file.write(TO_PCHAR(&mip.width), 6);//打包写入
			file.write(TO_PCHAR(&mipsize), 4);
			file.write(TO_PCHAR(mip.data.data()), mip.data.size());
		}
	}
}

Ktex::iterator ktexlib::v3detail::Ktex::begin()
{
	return mips.begin();
}

Ktex::iterator ktexlib::v3detail::Ktex::end()
{
	return mips.end();
}

const Mipmap& ktexlib::v3detail::Ktex::operator[] (size_t i)
{
	return mips[i];
}

inline ktexlib::v3detail::Mipmap::Mipmap(uint32_t w, uint32_t h, uint32_t pitch) :width(w), height(h), pitch(pitch), data() {}

inline ktexlib::v3detail::Mipmap::Mipmap(const Mipmap& other) :Mipmap(other.width, other.height, other.pitch, data) {}

inline ktexlib::v3detail::Mipmap::Mipmap(Mipmap&& xval) noexcept :Mipmap(xval.width, xval.height, xval.pitch, std::move(xval.data))  {}

inline ktexlib::v3detail::Mipmap::Mipmap(const uint32_t w, const uint32_t h, const uint32_t pitch, const std::vector<unsigned char>& data) :
	width(w), height(h), pitch(pitch)
{
	this->data = data;
}

inline ktexlib::v3detail::Mipmap::Mipmap(const uint32_t w, const uint32_t h, const uint32_t pitch, std::vector<unsigned char>&& data) noexcept :
	width(w), height(h), pitch(pitch)
{
	this->data = std::move(data);
}
