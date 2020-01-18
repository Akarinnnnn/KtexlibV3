#pragma once
//#include "framework.h"
#include <Windows.h>
#include <locale>
#define WIDEIOS_EXPORT 
namespace fa_util
{
	namespace win_wideios
	{
		extern WIDEIOS_EXPORT void set_stdin();
		extern WIDEIOS_EXPORT void set_stdout();
		extern WIDEIOS_EXPORT void set_stderr();


		class WIDEIOS_EXPORT wiconstream
		{
		friend void set_stdin();
		public:
			~wiconstream() noexcept;
			const OVERLAPPED& get_overlapped() const;
			wiconstream() :handle(nullptr), overlapped() {}
			DWORD read(wchar_t* buff, DWORD size);
		protected:
			wiconstream(HANDLE handle);
			HANDLE handle;
			OVERLAPPED overlapped;
		};

		class WIDEIOS_EXPORT woconstream
		{
			friend void set_stderr();
			friend void set_stdout();
		public:
			woconstream():handle(nullptr){}
			~woconstream() noexcept;

			const OVERLAPPED& get_overlapped() const;

		#pragma region 无符号整数
			woconstream& operator<<(unsigned int i);
			woconstream& operator<<(unsigned long long i);
			woconstream& operator<<(unsigned short i);
			woconstream& operator<<(unsigned long i);
		#pragma endregion
		#pragma region 有符号整数
			woconstream& operator<<(int i);
			woconstream& operator<<(long long i);
			woconstream& operator<<(short i);
			woconstream& operator<<(long i);
		#pragma endregion
		#pragma region 常见浮点
			woconstream& operator<<(double d);
			woconstream& operator<<(float f);
		#pragma endregion
		#pragma region 宽字符串
			woconstream& operator<<(const wchar_t* wstring);
			woconstream& operator<<(const std::wstring& wstring);

			woconstream& operator<<(const char16_t* u16string);
			woconstream& operator<<(const std::u16string& u16string);
		#pragma endregion
		#pragma region 其他 指针，nullptr，布尔值，控制符等
			woconstream& operator<<(bool flag);

			template<typename = void>
			woconstream & operator<<(nullptr_t) 
			{
				*this << L"nullptr";
			}

			woconstream& operator<<(const void* const ptr);

		#pragma endregion
			DWORD write(const wchar_t* wstr, DWORD size);

			//void imbue(std::locale&& loc);
		protected:
			woconstream(HANDLE handle);
			OVERLAPPED overlapped{};
			HANDLE handle;
			std::locale loc;
		};

		extern WIDEIOS_EXPORT woconstream wcout;
		extern WIDEIOS_EXPORT woconstream wcerr;
		extern WIDEIOS_EXPORT wiconstream wcin;



	}

}