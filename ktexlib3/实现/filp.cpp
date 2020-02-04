#include "../pch.h"

#include "../导入头/texfile.h"
#include <thread>
//#include <DirectXMath.h>
using std::thread;
void ktexlib::v3detail::filp(uint8_t * begin, uint8_t * end, size_t pitch)
{
	if ((end - begin) % pitch != 0) 
		throw std::invalid_argument("(end-begin) % pitch != 0");

	for (size_t cur = 0; cur < (end - begin) / 2; cur += pitch)
	{
		auto line2 = end - cur - pitch;
		auto line1 = begin + cur;
		for (size_t i = 0; i < pitch; i++)
		{
			line1[i] ^= line2[i];//xor交换流
			line2[i] ^= line1[i];
			line1[i] ^= line2[i];
		}
	}
}
