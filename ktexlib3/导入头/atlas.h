#pragma once


#if defined(KTEXLIB3_EXPORTS) 
#if defined(_MSC_VER)
#define	KTEXLIB3_EXPORT __declspec(dllexport)
#else
#define KTEXLIB3_EXPORT __declspec(dllimport)
#endif
#else
#define KTEXLIB3_EXPORT
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
	}
}