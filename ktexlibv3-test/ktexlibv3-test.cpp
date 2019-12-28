#include "pch.h"
#include "CppUnitTest.h"
#include "../ktexlib3/导入头/texfile.h"
#include "ktexlib2-dbgx64/TEXFileOperation.h"
using ktexlib::KTEXFileOperation::platfrm;
using ktexlib::KTEXFileOperation::pixfrm;

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
namespace tf = Microsoft::VisualStudio::CppUnitTestFramework;
template <> std::wstring tf::ToString<uint16_t>(const unsigned short& a) { RETURN_WIDE_STRING(a); }
template <> std::wstring tf::ToString<platfrm> (const platfrm & v)
{
	switch (v)
	{
	case platfrm::opengl:
		return L"OpenGL";
	default:
		break;
	}
}
namespace ktexlib
{
	TEST_CLASS(V3Test)
	{
	public:
		
		TEST_METHOD(GenerateTest)
		{
			using namespace ktexlib::v3;

			Assert::IsTrue(ktexlib::v3::gen_bc3universal(L"test.jpg"));
			
			ktexlib::KTEXFileOperation::KTEX reader;
			reader.LoadKTEX(L"test.tex");

			Assert::AreEqual(11ui16, reader.Info.mipscount, L"MIPSCOUNT != 11");
			Assert::AreEqual(platfrm::unk, reader.Info.platform);
			Assert::AreEqual(pixfrm::DXT5, reader.Info.pixelformat);
		}
	};
}
