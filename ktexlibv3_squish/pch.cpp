// pch.cpp: 与预编译标头对应的源文件

#include "pch.h"
#include <process.h>
#include "导入头/texfile.h"
// 当使用预编译的头时，需要使用此源文件，编译才能成功。
void ErrorMsgbox(const wchar_t* format, const HRESULT hr, int exitcode)
{
	wchar_t message[64]{ 0 };
	wsprintfW(message, format, hr);
	MessageBoxW(nullptr, message, L"什么情况？？？", MB_ICONERROR | MB_OK);
	if(exitcode != 0)
		exit(exitcode);
}

HRESULT ktexlib::v3::Initalize()
{
	HRESULT hr = CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);
	if (FAILED(hr) && hr != 0x80010106)
	{
		ErrorMsgbox(L"加载COM失败(hresult = %#08X)，即将退出", hr, 10);
	}
	HRESULT hr2 = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_ALL, IID_IWICImagingFactory, (LPVOID*)&comobj::wicfactory);
	if (FAILED(hr2))
		ErrorMsgbox(L"创建WIC工厂失败(hresult = %#08X)，即将退出\n这个问题常见于精简版系统", hr, 11);

	hr2 = D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, &comobj::D2DFactory);
	if (FAILED(hr2))
		ErrorMsgbox(L"创建D2D工厂失败(hresult = %#08X)，即将退出\n这个问题常见于精简版系统", hr, 12);

	return hr;
}


namespace comobj
{
	wil::com_ptr<IWICImagingFactory> wicfactory;
	wil::com_ptr<ID2D1Factory> D2DFactory;
}