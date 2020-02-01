#include "../pch.h"
#include "../导入头/atlas.h"
#include <wil/com.h>
#include <wil/result.h>
#include "../导入头/texfile.h"
#include <d2d1.h>

using wil::com_ptr;
static wil::com_ptr<IWICImagingFactory> WICFactory;
static wil::com_ptr<ID2D1Factory> D2DFactory;
#define hr_oom(msg)if (hr == E_OUTOFMEMORY) throw std::bad_alloc();\
	THROW_IF_FAILED_MSG(hr,msg)
#define hrthrow(msg) THROW_IF_FAILED_MSG(hr,msg)
/// <summary>
/// 相交
/// </summary>
/// <param name="a"></param>
/// <param name="b"></param>
/// <returns></returns>
/// <created>Fa鸽,2020/1/31</created>
/// <changed>Fa鸽,2020/1/31</changed>
bool ktexlib::atlasv3::Intersects(boundry_box& a, boundry_box& b)
{
	//if (
	//	a.x - b.x >= a.w - b.w ||//两个w - x同号
	//	a.y - b.y >= a.h - b.h ||//同理
	//	a.w + b.w >= a.x + b.x ||//异号
	//	a.h + b.h >= a.y + b.y//同理
	//	)return true;
	if (
		b.x >= a.x + a.w ||
		b.x + b.w <= a.x ||
		b.y + b.h <= a.y ||
		b.y >= a.y + a.w
		)
		return false;
	return true;
}

IWICBitmapFrameDecode* ktexlib::atlasv3::LoadWICImage(std::filesystem::path& filepath)
{
	HRESULT hr;
	com_ptr<IWICStream> stream;
	hr = WICFactory->CreateStream(&stream);
	THROW_IF_FAILED_MSG(hr, "create stream");

	hr = stream->InitializeFromFilename(filepath.c_str(), GENERIC_READ);
	THROW_IF_FAILED_MSG(hr, "init wic stream, filepath = %s", filepath.string().c_str());

	com_ptr<IWICBitmapDecoder> decoder;
	hr = WICFactory->CreateDecoderFromStream(stream.get(), nullptr,
		WICDecodeMetadataCacheOnLoad, &decoder);
	THROW_IF_FAILED_MSG(hr, "create decoder");

	com_ptr<IWICBitmapFrameDecode> frame;
	hr = decoder->GetFrame(0, &frame);
	THROW_IF_FAILED_MSG(hr, "get frame");
}

/// <summary>
/// 拼图函数，bboxes参数传入一个vector来接收拼图结果
/// </summary>
/// <param name="images"></param>
/// <param name="bboxes">接收拼图结果</param>
/// <returns></returns>
/// <example>
/// <code>
/// std::vector{boundry_box} out_bboxes;
/// IWICBitmap* image = MergeImages(images, out_bboxes);
/// </code>
/// </example>
/// <exception cref="wil::ResultException"/>
/// <exception cref="std::bad_alloc"/>
/// <created>Fa鸽,2020/1/31</created>
/// <changed>Fa鸽,2020/1/31</changed>
IWICBitmap* ktexlib::atlasv3::MergeImages(std::vector<IWICBitmapSource*>& images, std::vector<boundry_box>& bboxes)
{
	HRESULT hr = S_OK;
	bboxes.clear();

	auto pred = [](IWICBitmapSource* a, IWICBitmapSource* b)
	{
		uint32_t w1 = 0, w2 = 0, h1 = 0, h2 = 0;
		a->GetSize(&w1, &h1);
		b->GetSize(&w2, &h2);

		return w2 * h2 < w1 * h1;
	};
	std::sort(images.begin(), images.end(), pred);

	if (!D2DFactory)
	{
		hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, &D2DFactory);
		hrthrow("Direct2D 1.0 Factory");
	}

	IWICBitmap* ret;
	hr = WICFactory->CreateBitmap(
		2048, 2048,
		GUID_WICPixelFormat32bppRGBA,
		WICBitmapNoCache, &ret);
	hr_oom("create WIC bitmap");

	com_ptr<ID2D1RenderTarget> rt;
	D2DFactory->CreateWicBitmapRenderTarget(ret,
		D2D1::RenderTargetProperties(
			D2D1_RENDER_TARGET_TYPE_DEFAULT, 
			D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
			0.0f, 0.0.f,
			D2D1_RENDER_TARGET_USAGE_FORCE_BITMAP_REMOTING),
		&rt
	);
	hrthrow("create render target");

	D2D1_RECT_F last;
	char dir = 0;//0,1,2,3 上 下 左 右
	for (size_t i = 0; i < images.size(); i++)
	{
		auto img = images.at(i);
		boundry_box bbox{};
		img->GetSize(&bbox.w, &bbox.h);

		ID2D1Bitmap* d2dbmp;
		hr = rt->CreateBitmapFromWicBitmap(img, &d2dbmp);
		THROW_IF_FAILED_MSG(hr, "d2d bitmap form wic, i = %u", i);

		switch (dir)
		{
		case 0:{
			dir = 1;

		}break;

		case 1:{
			dir = 2;
		}break;

		case 2 :{
			dir = 3;
		}break;

		case 3:{
			dir = 0;
		}break;

		default:
			abort();
		}
	}
	//hr = ret->
}


/// <summary>
/// 切图函数
/// </summary>
/// <param name="filepath">文件路径，可以是tex或一张普通图片</param>
/// <param name="bboxes">边界盒（宽、高、x y坐标）</param>
/// <exception cref="wil::ResultException">可作为std::exception处理</exception>
/// <returns>WIC图像</returns>
/// <created>Fa鸽,2020/1/31</created>
/// <changed>Fa鸽,2020/1/31</changed>
std::vector<IWICBitmap*> ktexlib::atlasv3::CutImage(std::filesystem::path filepath, std::vector<ktexlib::atlasv3::boundry_box>& bboxes)
{
	std::vector<IWICBitmap*> ret_val;
	wil::com_ptr<IWICBitmapSource> image;

	ret_val.reserve(bboxes.size());

	HRESULT hr = S_OK;
	if (!WICFactory)
	{
		hr = CoCreateInstance(CLSID_WICImagingFactory1, nullptr, CLSCTX_INPROC, IID_IWICImagingFactory, (LPVOID*)&WICFactory);
		THROW_IF_FAILED_MSG(hr, "WIC Factory");
	}

	if (filepath.extension() == L".tex")
	{
		auto ktex = v3::load_ktex(filepath.c_str());
		auto rgba = v3detail::decompress(ktex.Mipmaps[0], ktex.info.pixelFormat);

		if (rgba.pitch == 0) rgba.pitch = rgba.width * 4;
		v3detail::filp(rgba.data._Unchecked_begin(), rgba.data._Unchecked_end(), rgba.pitch);

		IWICBitmap* bitmapimage;
		hr = WICFactory->CreateBitmapFromMemory(
				rgba.width, rgba.height,
				GUID_WICPixelFormat32bppRGBA, WICBitmapCacheOnLoad,
				rgba.data.size(), rgba.data.data(),
				&bitmapimage);
		THROW_IF_FAILED_MSG(hr, "create wic image");
		
		image = bitmapimage;
	}
	else//文件
	{
		image = LoadWICImage(filepath);
	}

	for(auto& bbox : bboxes)
	{
		IWICBitmap* single;
		hr = WICFactory->CreateBitmapFromSourceRect(image.get(), bbox.x, bbox.y, bbox.w, bbox.h, &single);
		THROW_IF_FAILED_MSG(hr, "new form bbox(sourcerect)");
		ret_val.push_back(single);
	}

	return ret_val;
}

