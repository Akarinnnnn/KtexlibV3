#pragma once
#include <filesystem>
#include <wincodec.h>

#if defined(KTEXLIB3_EXPORTS)
#if defined(_MSC_VER)	
#define	KTEXLIB3_EXPORT __declspec(dllexport)
#else
#define KTEXLIB3_EXPORT
#endif
#else
#if defined(_MSC_VER)
#define KTEXLIB3_EXPORT __declspec(dllimport)
#else
#define KTEXLIB3_EXPORT
#endif
#endif 

#ifdef KTEXLIB3_LINKWIC
#pragma comment(lib,"windowscodecs.lib")
#endif

namespace ktexlib
{
	namespace atlasv3
	{
		struct boundry_box
		{
			unsigned int w = 0, h = 0;
			float x = 0.0f, y = 0.0f;
		};


		std::vector<IWICBitmap*> KTEXLIB3_EXPORT CutImage(std::filesystem::path filepath, std::vector<boundry_box>& bboxes);
		IWICBitmapFrameDecode* KTEXLIB3_EXPORT LoadWICImage(std::filesystem::path& filepath);
		
		IWICBitmap* KTEXLIB3_EXPORT MergeImages(std::vector<IWICBitmapSource*>& images, std::vector<boundry_box>& bboxes);
	}
}