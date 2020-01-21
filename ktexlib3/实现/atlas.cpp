#include "../pch.h"
#include "../导入头/atlas.h"
#include <wil/com.h>
#include <wil/result.h>
#include "../导入头/texfile.h"

static wil::com_ptr<IWICImagingFactory> WICFactory;

bool ktexlib::atlasv3::相交(boundry_box& a, boundry_box& b)
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

std::vector<IWICBitmap*> ktexlib::atlasv3::切图(std::filesystem::path ktex_path, std::vector<ktexlib::atlasv3::boundry_box>& bboxes)
{
	std::vector<IWICBitmap*> ret_val;
	HRESULT hr = S_OK;
	if (!WICFactory)
	{
		hr = CoCreateInstance(CLSID_WICImagingFactory1, nullptr, CLSCTX_INPROC, IID_IWICImagingFactory, (LPVOID*)&WICFactory);
		THROW_IF_FAILED_MSG(hr, "WIC Factory");
	}



	//IWICStream* in_pic
	WICFactory->CreateBitmap()
	for (auto& bbox : bboxes)
	{

	}
}
