﻿// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include <combaseapi.h>
#include <wincodec.h>
#include <wil/com.h>



BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
	HRESULT hr = 0;
	wchar_t message[64]{ 0 };
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
	{

   	}
		break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
		//CoUninitialize();
        break;
    }
    return TRUE;
}

