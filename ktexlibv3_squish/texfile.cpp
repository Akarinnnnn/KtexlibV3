#include "pch.h"
#include <iostream>
#include "导入头/texfile.h"
#include "thirdparty/inc/squish.h"

#define TO_PCHAR(x) reinterpret_cast<char*>(x)
#define TO_CONST_PCHAR(x) reinterpret_cast<const char*>(x)


using std::cout, std::endl;
namespace wicobj
{
	wil::com_ptr<IWICImagingFactory> factory;
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

inline ktexlib::v3detail::Mipmap::Mipmap(const Mipmap& other) : Mipmap(other.width, other.height, other.pitch, data) {}

inline ktexlib::v3detail::Mipmap::Mipmap(Mipmap&& xval) noexcept :Mipmap(xval.width, xval.height, xval.pitch, std::move(xval.data)) {}

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
		ret.info.textureType = (ktexlib::v3detail::TextureType)	((v & 0x00001E00) >> 9);
		ret.info.pixelFormat = (ktexlib::v3detail::PixelFormat)	((v & 0x000001F0) >> 4);
		ret.info.platform = (ktexlib::v3detail::Platform)		(v & 0x0000000F);
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

//form DirectXTex
uint32_t ComputePitchBC23(uint32_t w)
{
	auto nbw = std::max<uint32_t>(1u, (uint64_t(w) + 3u) / 4u);
	return nbw * 16u;
}
uint32_t ComputePitchBC1(uint32_t w)
{
	uint64_t nbw = std::max(1u, (w + 3u) / 4u);
	return nbw * 8u;
}
Mipmap ktexlib::v3detail::convert(const RgbaImage& image, PixelFormat fmt, bool pararral)
{
	switch (fmt)
	{
	case ktexlib::v3detail::PixelFormat::dxt1:
	{
		auto size = squish::GetStorageRequirements(image.width, image.height, squish::kDxt1);
		Mipmap ret{ 
			image.width,image.height,
			ComputePitchBC1(image.width),
			std::vector<uint8_t>(size) };
		squish::CompressImage(image.data.data(), image.width, image.height,
			ret.data.data(), squish::kDxt1);
		break;
	}
	case ktexlib::v3detail::PixelFormat::dxt3:
	{
		auto size = squish::GetStorageRequirements(image.width, image.height, squish::kDxt1);
		Mipmap ret{
			image.width,image.height,
			ComputePitchBC23(image.width),
			std::vector<uint8_t>(size) };
		squish::CompressImage(image.data.data(), image.width, image.height,
			ret.data.data(), squish::kDxt3);
		break;
	}
	case ktexlib::v3detail::PixelFormat::dxt5:
	{
		auto size = squish::GetStorageRequirements(image.width, image.height, squish::kDxt1);
		Mipmap ret{
			image.width,image.height,
			ComputePitchBC23(image.width),
			std::vector<uint8_t>(size) };
		squish::CompressImage(image.data.data(), image.width, image.height,
			ret.data.data(), squish::kDxt5);
		break;
	}
	case ktexlib::v3detail::PixelFormat::rgba:
		return image;
		break;
	case ktexlib::v3detail::PixelFormat::r8g8b8:
	{
		auto pitch = image.width * 3;
		if (pitch & 4)
		{
			pitch &= 4;
			pitch + 4;
		}

		Mipmap ret{
			image.width,image.height,
			pitch,
			std::vector<uint8_t>(pitch * image.height)
		};
		for (size_t i = 0; i < image.data.size(); i++)
		{

		}

		break;
		}
	default:
		break;
	}
}