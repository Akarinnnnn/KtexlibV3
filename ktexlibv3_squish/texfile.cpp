#include "pch.h"
#include <iostream>
#include "导入头/texfile.h"
#include "thirdparty/inc/squish.h"

#define TO_PCHAR(x) reinterpret_cast<char*>(x)
#define TO_CONST_PCHAR(x) reinterpret_cast<const char*>(x)


using std::cout, std::endl;

using namespace ktexlib::v3detail;

constexpr inline bool check_4n(unsigned int val)
{
	return !(val & 0xFFFFFFF8u);
}

constexpr inline unsigned int next_4n(unsigned int val)
{
	return (val & 0xFFFFFFF8u) + 4;
}

constexpr bool check_2pow(unsigned int x)
{
	return (x & (x - 1)) == 0;
}

constexpr unsigned int next_2pow(unsigned int x)
{
	int n = x - 1;
	n |= n >> 1;
	n |= n >> 2;
	n |= n >> 4;
	n |= n >> 8;
	n |= n >> 16;
	return n + 1;
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

ktexlib::v3detail::Ktex ktexlib::v3::load_ktex(const wchar_t* path)
{
	std::ifstream file;
	file.open(path);

	if (!file.is_open())
	{
		auto errc = std::make_error_code((std::errc)errno);
		throw std::system_error(errc, "open ktex file");
	}

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
Mipmap ktexlib::v3detail::convert(const RgbaImage& image, PixelFormat fmt)
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
		if (pitch & 3)
		{
			pitch &= (~3u);
			pitch + 4;
		}

		Mipmap ret{
			image.width,image.height,
			pitch,
			std::vector<uint8_t>(pitch * image.height)
		};
		for (size_t i = 0; i < image.data.size(); i++)
		{
			if ((bool)(i & 3) && i != 0)
				ret.data.push_back(image.data[i]);
			else
				continue;
		}
		break;
		}
	default:
		throw std::invalid_argument("fmt");
		break;
	}
}
ktexlib::v3detail::RgbaImage ktexlib::v3detail::decompress(const Mipmap& mip, PixelFormat fmt)
{
	RgbaImage rgba;
	rgba.width = mip.width;
	rgba.height = mip.height;
	rgba.pitch = rgba.width * 4u;
	rgba.data.resize(rgba.pitch * rgba.height);
	switch (fmt)
	{
	case ktexlib::v3detail::PixelFormat::dxt1:
		squish::DecompressImage(rgba.data.data(), rgba.width, rgba.height,
			mip.data.data(), squish::kDxt1);
		break;
	case ktexlib::v3detail::PixelFormat::dxt3:
		squish::DecompressImage(rgba.data.data(), rgba.width, rgba.height,
			mip.data.data(), squish::kDxt3);
		break;
	case ktexlib::v3detail::PixelFormat::dxt5:
		squish::DecompressImage(rgba.data.data(), rgba.width, rgba.height,
			mip.data.data(), squish::kDxt5);
		break;
	case ktexlib::v3detail::PixelFormat::rgba:
		rgba.data = mip.data;
		break;
	case ktexlib::v3detail::PixelFormat::r8g8b8:
	{
		for (size_t i = 0; i < mip.data.size(); i += 3)
		{
			const auto* rgb = reinterpret_cast<const uint32_t*>(mip.data.data() + i);
			uint32_t pixel = 0x00FFFFFF & *rgb; // uint32_t [ a | g | b | r ]

			for (uint8_t i = 0; i < 4; i++)
				rgba.data.push_back(*reinterpret_cast<uint8_t*>(&pixel + i));
		}
		break;
	}
	default:
		throw std::invalid_argument("fmt");
		break;
	}
	return rgba;
}

std::vector<wil::com_ptr<IWICBitmapSource>> gen_mips(IWICBitmapSource* img0)
{
	using namespace ::comobj;
	using wil::com_ptr;

	UINT w = 0, h = 0;
	HRESULT hr = S_OK;

	img0->GetSize(&w, &h);
	std::vector<wil::com_ptr<IWICBitmapSource>> ret;
	ret.reserve(12);
	com_ptr<IWICBitmapSource> img;
	if (check_2pow(w) && check_2pow(h) && w == h)
	{
		img = img0;
	}
	else
	{
		auto edgelen = std::max(next_2pow(w), next_2pow(h));
		w = edgelen, h = edgelen;
		IWICBitmap* outimg;
		com_ptr<ID2D1Bitmap> d2d;
		com_ptr<ID2D1RenderTarget> rt;
		auto rtprops = D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_HARDWARE,
			D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
			0.0f, 0.0f
		);
		auto drawrect = D2D1::RectF(0, 0, w, h);
		hr = wicfactory->CreateBitmap(edgelen, edgelen, GUID_WICPixelFormat32bppBGRA, WICBitmapNoCache, &outimg);
		THROW_IF_FAILED_MSG(hr, "wic bitmap");

		hr = D2DFactory->CreateWicBitmapRenderTarget(outimg, rtprops, &rt);
		THROW_IF_FAILED_MSG(hr, "d2d rt");

		hr = rt->CreateBitmapFromWicBitmap(img0, &d2d);
		THROW_IF_FAILED_MSG(hr, "d2d bitmap");

		rt->DrawBitmap(d2d.get(), drawrect);
		
		img.attach(outimg);
	}

	{
		com_ptr<IWICFormatConverter> fmtconv;
		hr = wicfactory->CreateFormatConverter(&fmtconv);
		THROW_IF_FAILED_MSG(hr, "create wic format converter");

		hr = fmtconv->Initialize(img.get(), GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone, nullptr, 1.0, WICBitmapPaletteTypeMedianCut);
		THROW_IF_FAILED_MSG(hr, "init wic format converter");

		img = fmtconv;
	}

	for (UINT edgelen = w; edgelen; edgelen >> 1)
	{
		com_ptr<IWICBitmapScaler> scaler;
		wicfactory->CreateBitmapScaler(&scaler);

		hr = scaler->Initialize(img.get(), edgelen, edgelen, WICBitmapInterpolationModeLinear);
		THROW_IF_FAILED_MSG(hr, "init wic scaler");

		ret.push_back(std::move(scaler));
	}

	return ret;
}

Ktex ktexlib::v3detail::load_and_compress(std::filesystem::path path, PixelFormat fmt, bool gen_mips)
{
	using wil::com_ptr;
	Ktex ret;

	com_ptr<IWICBitmapFrameDecode> img;

	if (gen_mips)
	{

	}
	else
	{

	}
}
