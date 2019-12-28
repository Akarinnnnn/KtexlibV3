/***************************
* Encoding: GB2312
***************************/
#pragma once
#ifndef KTEX_COMMON
#define KTEX_COMMON

#if KTEXLIBDYMANIC_EXPORTS
#define __API __declspec(dllexport)
#else
#define __API __declspec(dllimport)
#endif

#include <filesystem>
#include <vector>
#include <exception>
#include <cstdio>

namespace ktexlib
{
	typedef std::vector<unsigned char> uc_vector;
	typedef std::vector<uc_vector> datavec;
	namespace Atlas
	{
		struct b_box//boundry box
		{
			unsigned short w;	//width
			unsigned short h;	//height
			double x;			//x offset
			double y;			//y offset
		};
	}
	namespace KTEXFileOperation
	{
		enum class  platfrm//platform
		{
			opengl = 12,//mainly PC
			xb360 = 11,
			ps3 = 10,
			unk = 0
		};
		enum class pixfrm //pixel form
		{
			ARGB = 4,
			DXT1 = 0,
			DXT3 = 1,
			DXT5 = 2,//BC3
			unk = 7
		};
		enum class textyp //texture type
		{
			d1 = 1,//1d
			d2 = 2,//2d
			d3 = 3,//3d
			cube = 4//cubemap
		};
		class __API ktex_exception :public std::exception
		{
		public:
			~ktex_exception()
			{
				delete msg;
			}

			ktex_exception(const char* msg, int code) noexcept
			{
				this->_code = code;
				size_t msgsize = strlen(msg) + 36;
				this->msg = new char[msgsize];
				sprintf_s(this->msg, msgsize, "ktex_exception:\n\t%s\n\tCode:0x%08X", msg, code);
			}
			
			virtual const char * what() const noexcept override
			{
				return this->msg;
			}

			int code() const noexcept
			{
				return _code;
			}
		protected:
			char* msg;
		private:
			int _code;
		};

		struct KTEXHeader
		{
			//CC4
			unsigned int cc4 = 0x5845544B;
			//第一数据块
			unsigned int firstblock = 0;
			//0xFFF 12bit, flags 2bit, mipscount 5bit, textype 4bit
			//pixelformat 5bit, platform 4bit
		};
		struct KTEXInfo
		{
			unsigned char flags = 0;
			unsigned short mipscount = 0;
			textyp texturetype = textyp::d1;
			pixfrm pixelformat = pixfrm::DXT5;
			platfrm platform = platfrm::opengl;
		};

		struct RGBAv2
		{
			unsigned short width = 0;
			unsigned short height = 0;
			unsigned short pitch = 0;
			uc_vector data;
		};

		struct mipmapv2
		{
			mipmapv2() = default;
			mipmapv2(const mipmapv2 & rhs)
			{
				memcpy_s(this, 6, &rhs, 6);
				this->size = rhs.size;
				this->data = new unsigned char[size];
				memcpy_s(data, size, rhs.data, size);
			}
			mipmapv2(const mipmapv2 && rhs) noexcept
			{
				memcpy_s(this, sizeof(mipmapv2), &rhs, sizeof(mipmapv2));
			}

			unsigned short width = 0;
			unsigned short height = 0;
			unsigned short pitch = 0;
			unsigned int size = 0;
			unsigned char* data = nullptr;
			__API ~mipmapv2();
		};

		typedef std::vector<mipmapv2> mipmaps;
		typedef std::vector<RGBAv2> imgs;

	}
}
#endif