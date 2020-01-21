#include "../pch.h"
#include <directxtex.h>
#include "../导入头/texfile.h"
#include "wil/result.h"
//#include <wincodecsdk.h>
//#include <cmath>

using namespace ktexlib::v3detail;
using DirectX::Image;

DXGI_FORMAT dxfmt_from_pixf(PixelFormat fmt)
{
	
	switch (fmt)
	{
	case ktexlib::v3detail::PixelFormat::dxt1:
		return DXGI_FORMAT_BC1_UNORM;
		break;
	case ktexlib::v3detail::PixelFormat::dxt3:
		return DXGI_FORMAT_BC3_UNORM;
		break;
	case ktexlib::v3detail::PixelFormat::dxt5:
		return DXGI_FORMAT_BC3_UNORM;
		break;
	default:
		throw std::invalid_argument("所选格式不在可选范围内");
		break;
	}

}

Mipmap ktexlib::v3detail::convert(const RgbaImage& image, PixelFormat fmt, bool multithread)
{
	DXGI_FORMAT dxgi_fmt;
	size_t row_pitch = 0;
	size_t slice_pitch = 0;

	dxgi_fmt = dxfmt_from_pixf(fmt);
	if(fmt == ktexlib::v3detail::PixelFormat::r8g8b8)
	{
		Mipmap retmip{ image.width,image.height,image.pitch };
		auto size = image.data.size();
		retmip.data.reserve(size / 4 * 3);
		for (size_t i = 0; i < size; i++)
			if (!(i & 3u))
				retmip.data.push_back(image.data[i]);
		return retmip;
	}
	if (fmt == PixelFormat::rgba) return image;

	if (image.pitch == 0) DirectX::ComputePitch(dxgi_fmt, image.width, image.height, row_pitch, slice_pitch);

	DirectX::ScratchImage out;
	DirectX::ScratchImage in;

	{
		in.Initialize2D(DXGI_FORMAT_R8G8B8A8_UNORM, image.width, image.height, 0, 1);
		uint8_t* target = in.GetImage(0, 0, 0)->pixels;
		for (const uint8_t* ln = image.data.data(); ln != image.data.data() + image.data.size(); ln += ((size_t)image.width * (size_t)4))
		{
			auto copyresult = memcpy_s(target,
				image.width, ln, (size_t)image.width * (size_t)4);
			if (copyresult) throw std::system_error(std::make_error_code(std::errc(copyresult)),"复制像素数据失败");
		}
	}

	if(multithread)
	{
		THROW_IF_FAILED_MSG(DirectX::Compress(*in.GetImage(0, 0, 0), dxgi_fmt, DirectX::TEX_COMPRESS_PARALLEL, 1, out),"压缩失败");
	}
	else
	{
		THROW_IF_FAILED_MSG(DirectX::Compress(*in.GetImage(0, 0, 0), dxgi_fmt, 0, 1, out), "压缩失败");
	}

	Mipmap ret_val
	{
		image.width,image.height,image.pitch,
		std::vector<unsigned char>
		(out.GetPixels(),out.GetPixels() + out.GetPixelsSize())
	};
	return ret_val;
}

RgbaImage ktexlib::v3detail::decompress(const Mipmap& mip,PixelFormat mipfmt)
{
	RgbaImage ret_val;

	DirectX::Image in{ mip.width,mip.height,dxfmt_from_pixf(mipfmt),mip.pitch,mip.data.size(),
		const_cast<uint8_t*>(mip.data.data()) };
	DirectX::ScratchImage out;
	DirectX::Decompress(in, DXGI_FORMAT_R8G8B8A8_UNORM, out);

	auto* img = out.GetImage(0, 0, 0);
	ret_val.width = mip.width;
	ret_val.height = mip.height;
	ret_val.pitch = img->rowPitch;
	ret_val.data.reserve(img->slicePitch);
	ret_val.data.assign(img->pixels, img->pixels + img->slicePitch);

	return ret_val;
}