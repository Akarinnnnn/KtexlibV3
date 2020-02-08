#include "pch.h"
#include "导入头/atlas.h"
#include <wil/com.h>
#include <wil/result.h>
#include "导入头/texfile.h"
#include <d2d1.h>
#include <KleiAnim/Binary.hpp>
#include <pugixml.hpp>

using wil::com_ptr;
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
bool Intersects(D2D1_RECT_F& a, D2D1_RECT_F& b)
{
	//if (
	//	a.x - b.x >= a.w - b.w ||//两个w - x同号
	//	a.y - b.y >= a.h - b.h ||//同理
	//	a.w + b.w >= a.x + b.x ||//异号
	//	a.h + b.h >= a.y + b.y//同理
	//	)return true;
	if (
		b.left <= a.right ||
		b.bottom >= a.top ||
		b.right >= a.left ||
		b.top >= a.bottom
		)
		return true;
	return false;
}

bool Intersects(ktexlib::atlasv3::boundry_box& a, ktexlib::atlasv3::boundry_box& b)
{
	if (
		b.x >= a.x + a.w ||
		b.x + b.w <= a.x ||
		b.y + b.h <= a.y ||
		b.y >= a.y + a.w
		)
		return false;
	return true;
}

IWICBitmapFrameDecode* ktexlib::atlasv3::FileLoadWIC(std::filesystem::path& filepath)
{
	HRESULT hr;
	com_ptr<IWICStream> stream;
	hr = wicobj::factory->CreateStream(&stream);
	THROW_IF_FAILED_MSG(hr, "create stream");

	hr = stream->InitializeFromFilename(filepath.c_str(), GENERIC_READ);
	THROW_IF_FAILED_MSG(hr, "init wic stream, filepath = %s", filepath.string().c_str());

	com_ptr<IWICBitmapDecoder> decoder;
	hr = wicobj::factory->CreateDecoderFromStream(stream.get(), nullptr,
		WICDecodeMetadataCacheOnLoad, &decoder);
	THROW_IF_FAILED_MSG(hr, "create decoder");

	IWICBitmapFrameDecode* frame;
	hr = decoder->GetFrame(0, &frame);
	THROW_IF_FAILED_MSG(hr, "get frame");

	return frame;
}



void Draw(ID2D1RenderTarget* rt, ID2D1Bitmap* bmp, D2D_RECT_F& rect, size_t& i)
{
	rt->DrawBitmap(
		bmp, rect, 1,
		D2D1_BITMAP_INTERPOLATION_MODE_LINEAR);
	i++;
}

ktexlib::atlasv3::boundry_box d2drect2bbox(D2D_RECT_F& rect)
{
	return ktexlib::atlasv3::boundry_box{
	(uint32_t)(rect.right - rect.left),(uint32_t)(rect.bottom - rect.top),
	rect.left,rect.top
	};
}

bool check_and_draw2(ID2D1RenderTarget* rt, ID2D1Bitmap* bmp, std::vector<ktexlib::atlasv3::boundry_box>& bboxes,
	D2D1_RECT_F& current)
{
	auto curbbox = d2drect2bbox(current);
	bool dodraw = true;

	for (auto& bbox : bboxes)
	{
		if (!Intersects(bbox, curbbox))
		{
			rt->DrawBitmap(bmp, current);
			bboxes.push_back(curbbox);
			return true;
		}
	}

	return false;
}

D2D1_RECT_F bbox2dxrect(ktexlib::atlasv3::boundry_box& lastbbox)
{
	D2D1_RECT_F lastrect;
	lastrect.top = lastbbox.y;
	lastrect.bottom = lastbbox.y + lastbbox.h;
	lastrect.left = lastbbox.x;
	lastrect.right = lastbbox.w;
	return lastrect;
}

/// <summary>
/// 拼图函数，bboxes参数传入一个vector来接收拼图结果
/// </summary>
/// <param name="images"></param>
/// <param name="name_bbox">接收拼图结果</param>
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
	hr = wicobj::factory->CreateBitmap(
		2048, 2048,
		GUID_WICPixelFormat32bppRGBA,
		WICBitmapNoCache, &ret);
	hr_oom("create WIC bitmap");

	com_ptr<ID2D1RenderTarget> rt;
	D2DFactory->CreateWicBitmapRenderTarget(ret,
		D2D1::RenderTargetProperties(
			D2D1_RENDER_TARGET_TYPE_DEFAULT,
			D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
			0.0f, 0.0f,
			D2D1_RENDER_TARGET_USAGE_FORCE_BITMAP_REMOTING),
		&rt
	);
	hrthrow("create render target");
	std::ofstream log("MergeImage.log", std::ios::app | std::ios::out);
	//D2D1_RECT_F last;
	rt->BeginDraw();

	D2D_RECT_F lastrect;
	for (auto* img : images)
	{
		boundry_box bbox{};
		img->GetSize(&bbox.w, &bbox.h);

		com_ptr<ID2D1Bitmap> d2dbmp;
		hr = rt->CreateBitmapFromWicBitmap(img, &d2dbmp);
		THROW_IF_FAILED_MSG(hr, "d2d bitmap form wic");
		try {
			while (true)
			{
				D2D1_RECT_F current_rect = D2D1::RectF();
				if (!bboxes.empty())
				{
					auto& lastbbox = bboxes.back();
					current_rect = bbox2dxrect(lastbbox);
				}
				auto size = d2dbmp->GetSize();
				char dir = 0;//0,1,2,3 上 下 左 右
				size_t insert_index = 0;
				switch (dir)
				{
				case 0:
					current_rect.bottom += size.height;
					current_rect.top += size.height += (current_rect.bottom - current_rect.top);
					//last = cur
					if (!check_and_draw2(rt.get(), d2dbmp.get(), bboxes, current_rect))
						dir = 1;
					break;
				case 1:
					current_rect.bottom -= size.height;
					current_rect.top -= size.height -= (current_rect.bottom - current_rect.top);
					if (!check_and_draw2(rt.get(), d2dbmp.get(), bboxes, current_rect))
						dir = 2;
					break;
				case 2:
					current_rect.left -= size.width;
					current_rect.right -= size.width;
					if (!check_and_draw2(rt.get(), d2dbmp.get(), bboxes, current_rect))
						dir = 3;
					break;
				case 3:
					current_rect.left += size.width;
					current_rect.right += size.width;
					if (!check_and_draw2(rt.get(), d2dbmp.get(), bboxes, current_rect))
						dir = 0;
					break;
				default:
					dir = 0;
					current_rect = bbox2dxrect(bboxes.at(insert_index));
					insert_index++;
					break;
				}
			}
		}
		catch (const std::out_of_range&)
		{
			log << "INSERT IMAGE FAILED at image" << bboxes.size() + 1;
			break;
		}
	}
	rt->EndDraw();
	return ret;
}

/// <summary>
/// 拼图函数，bboxes参数传入一个vector来接收拼图结果
/// </summary>
/// <param name="folder">存放待拼png文件的文件夹</param>
/// <param name="name_bbox">接收拼图结果</param>
/// <returns></returns>
/// <example>
/// <code>
/// std::vector{boundry_box} out_bboxes;
/// IWICBitmap* image = MergeImages(folder, out_bboxes);
/// </code>
/// </example>
/// <exception cref="wil::ResultException"/>
/// <exception cref="std::bad_alloc"/>
/// <created>Fa鸽,2020/1/31</created>
/// <changed>Fa鸽,2020/1/31</changed>
IWICBitmap* ktexlib::atlasv3::MergeImages(std::filesystem::path folder, std::vector<boundry_box>& bboxes)
{
	using namespace std;
	using namespace filesystem;
	vector<path> pathes;
	for (auto& ent : directory_iterator(folder))
	{
		if (ent.is_regular_file())
		{
			auto& path = ent.path();
			if (path.extension() == L".png")
				pathes.push_back(path);
		}
	}
	return MergeImages(pathes, bboxes);
}

/// <summary>
/// 拼图函数，bboxes参数传入一个vector来接收拼图结果
/// </summary>
/// <param name="image_pathes"></param>
/// <param name="name_bbox">接收拼图结果</param>
/// <returns></returns>
/// <example>
/// <code>
/// std::vector{boundry_box} out_bboxes;
/// IWICBitmap* image = MergeImages(pathes, out_bboxes);
/// </code>
/// </example>
/// <exception cref="wil::ResultException"/>
/// <exception cref="std::bad_alloc"/>
/// <created>Fa鸽,2020/1/31</created>
/// <changed>Fa鸽,2020/1/31</changed>
IWICBitmap* ktexlib::atlasv3::MergeImages(std::vector<std::filesystem::path>& image_pathes, std::vector<boundry_box>& bboxes)
{
	std::vector<IWICBitmapSource*> imgs;
	for (auto& imgpath : image_pathes)
		imgs.push_back(FileLoadWIC(imgpath));

	auto ret = MergeImages(imgs, bboxes);

	for (auto* img : imgs)
		img->Release();

	return ret;
}


IWICBitmapSource* LoadWICImage(std::filesystem::path& filepath)
{
	using namespace ktexlib;
	HRESULT hr = S_OK;
	IWICBitmapSource* image;
	if (filepath.extension() == L".tex")
	{
		auto ktex = v3::load_ktex(filepath.c_str());
		auto rgba = v3detail::decompress(ktex[0], ktex.info.pixelFormat);

		if (rgba.pitch == 0) rgba.pitch = rgba.width * 4;
		v3detail::filp(rgba.data._Unchecked_begin(), rgba.data._Unchecked_end(), rgba.pitch);

		IWICBitmap* bitmapimage;
		hr = wicobj::factory->CreateBitmapFromMemory(
			rgba.width, rgba.height,
			GUID_WICPixelFormat32bppRGBA, WICBitmapCacheOnLoad,
			rgba.data.size(), rgba.data.data(),
			&bitmapimage);
		THROW_IF_FAILED_MSG(hr, "create wic image");

		image = bitmapimage;
	}
	else//文件
	{
		image = atlasv3::FileLoadWIC(filepath);
	}
	return image;
}

/// <summary>
/// 切图函数
/// </summary>
/// <param name="filepath">文件路径，可以是tex或一张普通图片</param>
/// <param name="name_bbox">边界盒（宽、高、x y坐标）</param>
/// <exception cref="wil::ResultException">可作为std::exception处理</exception>
/// <returns>WIC图像</returns>
/// <created>Fa鸽,2020/1/31</created>
/// <changed>Fa鸽,2020/1/31</changed>
std::vector<IWICBitmap*> ktexlib::atlasv3::CutImage(std::filesystem::path filepath, std::vector<ktexlib::atlasv3::boundry_box>& bboxes)
{
	
	wil::com_ptr<IWICBitmapSource> image;

	image = LoadWICImage(filepath);

	return CutImage(image.get(),bboxes);
}

std::vector<IWICBitmap*> ktexlib::atlasv3::CutImage(IWICBitmapSource* image, std::vector<ktexlib::atlasv3::boundry_box>& bboxes)
{
	std::vector<IWICBitmap*> ret_val;

	ret_val.reserve(bboxes.size());
	for (auto& bbox : bboxes)
	{
		IWICBitmap* single;
		auto hr = wicobj::factory->CreateBitmapFromSourceRect(image, bbox.x, bbox.y, bbox.w, bbox.h, &single);
		THROW_IF_FAILED_MSG(hr, "new form bbox(sourcerect)");
		ret_val.push_back(single);
	}
	return ret_val;
}

uint32_t 四舍五入(float x)
{
	return (uint32_t)x + 0.5;
}

void  frame2xywh(pugi::xml_node_iterator& frame, float& x, float& y, uint32_t& w, uint32_t& h)
{
	x = frame->attribute("x").as_float();
	y = frame->attribute("y").as_float();
	w = frame->attribute("w").as_float();
	h = frame->attribute("h").as_float();
}


[[nodiscard]] std::vector<std::string> LoadAtlas(pugi::xml_document& doc, uint32_t w0, uint32_t h0, std::vector<ktexlib::atlasv3::boundry_box>& bboxes)
{
	using ktexlib::atlasv3::boundry_box;
	std::vector<std::string> names;
	pugi::xpath_query elements("Atlas/Elements/Element");
	bboxes.clear();

	for (auto& _element : doc.select_nodes(elements))
	{
		auto elem = _element.node();
		float u1 = elem.attribute("u1").as_float(),
				u2 = elem.attribute("u2").as_float(),
				v1 = elem.attribute("v1").as_float(),
				v2 = elem.attribute("v2").as_float();
		bboxes.push_back(
			boundry_box{
				四舍五入((u2 - u1) * w0),
				四舍五入((v2 - v1) * h0),
				u1 * w0,v1 * w0
			});
		names.push_back(elem.attribute("name").as_string());
	}
	return names;
}

[[nodiscard]] std::vector<std::string> LoadBuild(KleiAnim::Binary::BuildReader build, std::vector<ktexlib::atlasv3::boundry_box>& bboxes)
{
	std::vector<std::string> names;
	
	for (auto& sym : build)
	{
		auto framecur = sym.frames.begin();
		auto frameend = sym.frames.end();

		ktexlib::atlasv3::boundry_box bbox{
			framecur->w,framecur->h,
			framecur->x,framecur->y
		};

		bboxes.push_back(bbox);
		names.push_back(build.de_hash(sym.name_hash));

		if (++framecur != frameend)
		{
			for (uint32_t i = 1; framecur != frameend; ++framecur)
			{
				bboxes.push_back(ktexlib::atlasv3::boundry_box {
					(uint32_t)framecur->w,(uint32_t)framecur->h,
					framecur->x,framecur->y
				});
				names.push_back(build.de_hash(sym.name_hash) + "_" + std::to_string(i));
			}
		}
	}

	return names;
}

/// <summary>
/// 切图函数
/// </summary>
/// <param name="filepath">tex文件或图片</param>
/// <param name="atlas_or_build">以下三者之一：atlas文件、build的bin文件、build的xml文件</param>
/// <returns>含图片名和图片的std::pair</returns>
/// <exception cref="std::invalid_argument">filepath不是以下三者之一：atlas文件、build的bin文件、build的xml文件</exception>
/// <created>Fa鸽,2020/2/4</created>
/// <changed>Fa鸽,2020/2/4</changed>
std::vector<std::pair<std::string,IWICBitmap*>> ktexlib::atlasv3::CutImage(std::filesystem::path filepath, std::filesystem::path atlas_or_build)
{
	std::vector<std::pair<std::string, IWICBitmap*>> ret;
	uint32_t w0 = 0, h0 = 0;
	std::vector<std::string> names;
	std::vector<boundry_box> bboxes;
	com_ptr<IWICBitmapSource> image = LoadWICImage(filepath);
	image->GetSize(&w0, &h0);

	if (filepath.extension() == L".xml")
	{
		using namespace std::string_literals;
		pugi::xml_document doc;
		doc.load_file(filepath.c_str());
		auto* name = doc.children().begin()->name();

		if (name == "Atlas"s)
			names = LoadAtlas(doc, w0, h0, bboxes);
		
		if (name == "Build"s)
		{
			for (auto& _sym :doc.select_nodes("Build/Symbol"))
			{
				auto sym = _sym.node();
				auto framecur = sym.children().begin();
				auto frameend = sym.children().end();
				boundry_box bbox;
				frame2xywh(framecur, bbox.x, bbox.y, bbox.w, bbox.h);

				names.push_back(sym.attribute(name).as_string());
				bboxes.push_back(bbox);

				if (++framecur != frameend)
				{
					for (uint32_t i = 1; framecur != frameend; ++framecur)
					{
						frame2xywh(framecur, bbox.x, bbox.y, bbox.w, bbox.h);
						bboxes.push_back(bbox);
						names.push_back(sym.attribute("name").as_string() + "_"s + std::to_string(i));
					}
				}
			}
		}
		auto ret_img = CutImage(image.get(), bboxes);

		for (size_t i = 0; i < ret_img.size(); i++)
		{
			try { ret.push_back(std::make_pair(names[i], ret_img[i])); }
			catch (const std::overflow_error&) { }
		}

		return ret;
	}

	if (filepath.extension() == L".bin")
	{
		KleiAnim::Binary::BuildReader build(filepath);
		names = LoadBuild(build, bboxes);

		auto ret_img = CutImage(image.get(), bboxes);

		for (size_t i = 0; i < ret_img.size(); i++)
		{
			try { ret.push_back(std::make_pair(names[i], ret_img[i])); }
			catch (const std::overflow_error&) { break; }
		}

		return ret;
	}
	throw std::invalid_argument("\"stlas_or_build\" should be atlas or build file");
}


