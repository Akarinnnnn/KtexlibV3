// pch.cpp: 与预编译标头对应的源文件

#include "pch.h"
#include <process.h>

// 当使用预编译的头时，需要使用此源文件，编译才能成功。
void ErrorMsgbox(const wchar_t* format, const HRESULT hr, int exitcode)
{
	wchar_t message[64]{ 0 };
	wsprintfW(message, format, hr);
	MessageBoxW(nullptr, message, L"什么情况？？？", MB_ICONERROR | MB_OK);
	if(exitcode != 0)
		exit(exitcode);
}