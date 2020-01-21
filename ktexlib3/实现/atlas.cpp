#include "../pch.h"
#include "../����ͷ/atlas.h"
#include <wil/com.h>
#include <wil/result.h>
#include "../����ͷ/texfile.h"

static wil::com_ptr<IWICImagingFactory> WICFactory;

bool ktexlib::atlasv3::�ཻ(boundry_box& a, boundry_box& b)
{
	//if (
	//	a.x - b.x >= a.w - b.w ||//����w - xͬ��
	//	a.y - b.y >= a.h - b.h ||//ͬ��
	//	a.w + b.w >= a.x + b.x ||//���
	//	a.h + b.h >= a.y + b.y//ͬ��
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

std::vector<IWICBitmap*> ktexlib::atlasv3::��ͼ(std::filesystem::path ktex_path, std::vector<ktexlib::atlasv3::boundry_box>& bboxes)
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
