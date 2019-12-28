/***************************
* Encoding: GB2312
***************************/
#pragma once
//已经相当于祖传代码了，年代久远
#include <string>

#include <filesystem>
#include <exception>
#include "ktexcommon.h"
//多线程控制台输出
#ifdef MULTI_THREAD_KTEXCONOUTPUT
#include <mutex>
#endif


namespace ktexlib
{
	namespace KTEXFileOperation
	{
		class KTEX
		{
		public:

			bool operator==(KTEX& rhs)
			{
				return this == (&rhs);
			}

			__API void PushRGBA(RGBAv2 RGBA_array);
			__API void PushRGBA(RGBAv2 RGBA_array, unsigned int pitch);
			//RGBAv2 -> KTEX
			__API void Convert();
			__API void LoadKTEX(std::filesystem::path filepath);
			__API mipmapv2 GetMipmapByPitch(unsigned int pitch);
			__API mipmapv2 GetMipmap(unsigned int pitch);

			//KTEX -> RGBAv2
			__API RGBAv2 GetImageFromMipmap(unsigned int pitch);

			//KTEX -> RGBAv2
			__API RGBAv2 GetImageArray(unsigned int pitch);
			__API void clear();
			__API KTEX();
			__API ~KTEX();
			//__API friend void ktexlib::KTEXFileOperation::KTEX2PNG(KTEX target);
			__API void operator+=(RGBAv2 src);
			__API RGBAv2* operator[](int i);

			KTEXInfo Info;
			std::wstring output;
		private:
			mipmaps mipmaps;
			KTEXHeader Header;
			imgs RGBA_vectors;

			friend class RgbaItereator;
			friend class MipmapIterator;
		};

		class RgbaItereator
		{
		public:
			RgbaItereator(ktexlib::KTEXFileOperation::KTEX& ktex) : images(ktex.RGBA_vectors) {}
			imgs::iterator begin()
			{
				return images.begin();
			}
			imgs::iterator end()
			{
				return images.end();
			}
		private:
			imgs& images;
		};
		class MipmapIterator
		{
		public:
			MipmapIterator(ktexlib::KTEXFileOperation::KTEX& ktex) : mips(ktex.mipmaps) {}
			mipmaps::iterator begin()
			{
				return mips.begin();
			}
			mipmaps::iterator end() 
			{
				return mips.end();
			}
		private:
			mipmaps& mips;
		};

		__API KTEX operator+(KTEX dest, RGBAv2 src);
	}
}