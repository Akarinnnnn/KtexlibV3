//#include "pch.h"
#include "wstdout.hpp"
#include "wil/result.h"
#include <stdexcept>
#include <string>
#include <cstdio>
#include <cassert>
using namespace fa_util::win_wideios;

#undef max
#define PRINT_NUM_INTO_BUFF(x) 	wchar_t buff[24]{ 0 };\
	swprintf_s(buff, 24, x, i);\
	*this << buff;\
	return *this;


template <typename T,typename to_check = std::int32_t>
constexpr void overflow_check(T value)
{
	using namespace std::string_literals;
	if (value >= (T)std::numeric_limits<to_check>::max())
		throw std::overflow_error("算术上溢 "s + std::to_string(value));
}

//#include <cstdarg>


fa_util::win_wideios::woconstream fa_util::win_wideios::wcout{};
fa_util::win_wideios::wiconstream fa_util::win_wideios::wcin{};
fa_util::win_wideios::woconstream fa_util::win_wideios::wcerr{};

void fa_util::win_wideios::set_stdin()
{
	HANDLE stdh = GetStdHandle(STD_INPUT_HANDLE);
	wcin = wiconstream(stdh);
	wcout << L"[win_wideios]已设置输出流";
}

void fa_util::win_wideios::set_stdout()
{

	HANDLE stdh = GetStdHandle(STD_OUTPUT_HANDLE);
	wcout = woconstream(stdh);
	wcout << L"[win_wideios]已设置输入流";
}

void fa_util::win_wideios::set_stderr()
{
	HANDLE stdh = GetStdHandle(STD_ERROR_HANDLE);
	wcerr = woconstream(stdh);
	wcerr << L"[win_wideios]已设置错误流";
	//std::cin
}

woconstream& fa_util::win_wideios::woconstream::operator<<(bool flag)
{
	if (flag) *this << L"true";
	else *this << L"false";
	return *this;
}

woconstream& woconstream::operator<<(const void* const ptr)
{
	return *this << reinterpret_cast<uintptr_t>(ptr);
}

//void fa_util::win_wideios::win_wostream::imbue(std::locale&& loc)
//{
//	this->loc = loc;
//}

fa_util::win_wideios::woconstream::woconstream(HANDLE handle): handle(handle)
{
	//std::endl(std::cout);
}

DWORD fa_util::win_wideios::woconstream::write(const wchar_t* wstr, DWORD size)
{
	DWORD writed = 0;
	THROW_IF_WIN32_BOOL_FALSE_MSG(WriteFile(handle, wstr, size * sizeof(wchar_t), &writed, &overlapped), "写入失败，写入了%u字节", writed);
	return writed;
}


fa_util::win_wideios::wiconstream::~wiconstream() noexcept
{
	CloseHandle(handle);
}

const OVERLAPPED& fa_util::win_wideios::wiconstream::get_overlapped() const
{
	return overlapped;
}

fa_util::win_wideios::wiconstream::wiconstream(HANDLE handle):handle(handle),overlapped()
{
}

DWORD fa_util::win_wideios::wiconstream::read(wchar_t* buff, DWORD size)
{
	DWORD readed = 0;
	if (handle == nullptr) throw std::logic_error("this->handle == nullptr");
	THROW_IF_WIN32_BOOL_FALSE(ReadConsoleW(this->handle, buff, size, &readed, nullptr));
	return readed;
}

fa_util::win_wideios::woconstream::~woconstream() noexcept
{
	CloseHandle(handle);
}

const OVERLAPPED& fa_util::win_wideios::woconstream::get_overlapped() const
{
	return overlapped;
}

woconstream& fa_util::win_wideios::woconstream::operator<<(unsigned int i)
{
	PRINT_NUM_INTO_BUFF(L"%X")
}

woconstream& fa_util::win_wideios::woconstream::operator<<(unsigned long long i)
{
	PRINT_NUM_INTO_BUFF(L"%llX")
}

woconstream& fa_util::win_wideios::woconstream::operator<<(unsigned short i)
{
	PRINT_NUM_INTO_BUFF(L"%hX")
}

woconstream& fa_util::win_wideios::woconstream::operator<<(unsigned long i)
{
	PRINT_NUM_INTO_BUFF(L"%lX")
}

woconstream& fa_util::win_wideios::woconstream::operator<<(const wchar_t* wstring)
{
	size_t len = wcslen(wstring);
	overflow_check<DWORD>(len);
	write(wstring, (DWORD)len);
	return *this;
}

woconstream& fa_util::win_wideios::woconstream::operator<<(const std::wstring& wstring)
{
	size_t len = wstring.size();
	overflow_check<DWORD>(len);
	write(wstring.data(), (DWORD)len);
	return *this;
}

woconstream& fa_util::win_wideios::woconstream::operator<<(const char16_t* u16string)
{
	size_t len = wcslen((const wchar_t*)u16string);
	overflow_check<DWORD>(len);
	write((const wchar_t*)u16string, (DWORD)len);
	return *this;
}

woconstream& fa_util::win_wideios::woconstream::operator<<(const std::u16string& u16string)
{
	size_t len = u16string.size();
	overflow_check<DWORD>(len);
	write((const wchar_t*)u16string.data(), (DWORD)len);
	return *this;
}

woconstream& fa_util::win_wideios::woconstream::operator<<(int i)
{
	PRINT_NUM_INTO_BUFF(L"%d")
}

woconstream& fa_util::win_wideios::woconstream::operator<<(long long i)
{
	PRINT_NUM_INTO_BUFF(L"%lld")
}

woconstream& fa_util::win_wideios::woconstream::operator<<(short i)
{
	PRINT_NUM_INTO_BUFF(L"%hd")
}

woconstream& fa_util::win_wideios::woconstream::operator<<(long i)
{
	PRINT_NUM_INTO_BUFF(L"%ld")
}

woconstream& fa_util::win_wideios::woconstream::operator<<(double i)
{
	PRINT_NUM_INTO_BUFF(L"%f");
}

woconstream& fa_util::win_wideios::woconstream::operator<<(float f)
{
	double i = f;
	PRINT_NUM_INTO_BUFF(L"%f");
}
