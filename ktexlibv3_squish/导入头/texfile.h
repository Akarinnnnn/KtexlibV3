#pragma once

#if defined(KTEXLIBV3SQUISH_EXPORTS)
#if defined(_MSC_VER)	
#define	KTEXLIB3_EXPORT __declspec(dllexport)
#else
#define KTEXLIB3_EXPORT
#endif
#else
#if  defined(_MSC_VER)
#define KTEXLIB3_EXPORT __declspec(dllimport)
#else
#define KTEXLIB3_EXPORT
#endif
#endif 

#include <vector>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <combaseapi.h>
#include <memory>

namespace ktexlib
{
	template<typename iter_beg,typename iter_end>
	struct range
	{
		iter_beg _begin;
		iter_end _end;

		range(iter_beg begin, iter_end end):_begin(begin),_end(end) {}

		iter_beg begin() { return _begin; }
		iter_end end() { return _end; }
	};

	namespace v3detail
	{
		/// <summary>
		/// 材质的像素格式
		/// </summary>
		enum class PixelFormat :unsigned short
		{
			/// <summary>bc1</summary>
			dxt1 = 0,
			/// <summary>bc2</summary>
			dxt3 = 1,
			/// <summary>最常用,bc3</summary>
			dxt5 = 2,
			/// <summary>高保真，大小很大</summary>
			rgba = 4,
			/// <summary>滤镜用的</summary>
			r8g8b8 = 7,
		};

		/// <summary>
		/// 材质对应平台
		/// </summary>
		enum class Platform : unsigned char
		{
			/// <summary>OpenGL，适合饥荒PC版，可以不生成mipmap</summary>
			opengl = 12,
			/// <summary>XBox360，要求未知</summary>
			xb360 = 11,
			/// <summary>PS3，要求未知</summary>
			ps3 = 10,
			/// <summary>适合任意平台，必须生成mipmap</summary>
			universal = 0
		};

		/// <summary>
		/// 材质种类
		/// </summary>
		enum class TextureType : unsigned char
		{
			/// <summary>1D</summary>
			d1 = 1,
			/// <summary>2D</summary>
			d2 = 2,
			/// <summary>3D</summary>
			d3 = 3,
			/// <summary>Cubemap，适合2D游戏</summary>
			cubemap = 4
		};

		struct KTEXLIB3_EXPORT Mipmap
		{
			Mipmap(uint32_t w, uint32_t h, uint32_t pitch);
			Mipmap(const Mipmap& other);
			Mipmap(Mipmap&& xval) noexcept;

			Mipmap& operator=(const Mipmap& other)
			{
				new(this)Mipmap(other);
				return *this;
			}
			Mipmap(const uint32_t w, const uint32_t h, const uint32_t pitch, const std::vector<unsigned char>& data);
			Mipmap(const uint32_t w, const uint32_t h, const uint32_t pitch, std::vector<unsigned char>&& data) noexcept;

			uint16_t width, height, pitch = 0;//pitch是一行的数据长度，以字节为单位
#pragma warning(push)
#pragma warning (disable:4251)
			std::vector<unsigned char> data;
#pragma warning(pop)

		};

		struct RgbaImage : Mipmap
		{
			RgbaImage() :Mipmap{0,0,0}
			{

			}
		};

		struct KTEXInfo
		{
			uint8_t flags = 3ui8;
			TextureType textureType = TextureType::d2;
			PixelFormat pixelFormat = PixelFormat::dxt5;
			Platform platform = Platform::universal;
		};

		class KTEXLIB3_EXPORT Ktex
		{
		public:
			Ktex() = default;

			void AddMipmap(const Mipmap& mipmap);
			void AddMipmap(Mipmap&& mipmap);

			using const_iterator = std::vector<Mipmap>::const_iterator;
			using iterator = const_iterator;
			iterator begin();
			iterator end();

			const Mipmap& operator[] (size_t i);

			KTEXInfo info;

			void SaveFile(std::filesystem::path path);
		private:
#pragma warning(push)
#pragma warning (disable:4251)
			std::vector<Mipmap> mips;
#pragma warning(pop)
		};

		/// <summary>
		/// 转换
		/// </summary>
		/// <param name="image"></param>
		/// <param name="pararral">true多线程压缩，false单线程</param>
		/// <param name="fmt">目的像素格式</param>
		/// <returns></returns>
		/// <created>Fa鸽,2019/12/28</created>
		/// <changed>Fa鸽,2019/12/28</changed>
		KTEXLIB3_EXPORT Mipmap convert(const RgbaImage& image, PixelFormat fmt = PixelFormat::dxt5);

		/// <summary>
		/// 解压mipmap
		/// </summary>
		/// <param name="Mipmap"></param>
		/// <param name="fmt">mipmap像素格式</param>
		/// <returns></returns>
		/// <created>Fa鸽,2020/1/21</created>
		/// <changed>Fa鸽,2020/1/21</changed>
		KTEXLIB3_EXPORT RgbaImage decompress(const Mipmap& Mipmap, PixelFormat fmt);

		KTEXLIB3_EXPORT Ktex load_and_compress(std::filesystem::path path, PixelFormat fmt = PixelFormat::dxt5, bool gen_mips = false);

		KTEXLIB3_EXPORT void filp(uint8_t * begin, uint8_t * end, size_t pitch);
	}

	namespace v3
	{
		/// <summary>
		/// 加载COM
		/// </summary>
		/// <returns>HRESULT</returns>
		/// <created>Fa鸽,2020/1/18</created>
		/// <changed>Fa鸽,2020/1/18</changed>
		extern "C" KTEXLIB3_EXPORT HRESULT Initalize();

		/// <summary>
		/// 指定UTF16文件名，转换到同一目录
		/// </summary>
		/// <param name="path"></param>
		/// <returns></returns>
		/// <created>Fa鸽,2019/11/23</created>
		/// <changed>Fa鸽,2019/11/23</changed>
		KTEXLIB3_EXPORT bool gen_bc3universal(const char16_t * path, const char16_t * outpath = nullptr);

		/// <summary>
		/// 指定UTF16文件名，转换到同一目录
		/// </summary>
		/// <param name="path"></param>
		/// <returns></returns>
		/// <created>Fa鸽,2019/11/23</created>
		/// <changed>Fa鸽,2019/11/23</changed>
		KTEXLIB3_EXPORT bool gen_bc3universal(_In_ const wchar_t * path, _In_opt_ const wchar_t * outpath = nullptr);

		/// <summary>
		/// 加载一个tex文件
		/// </summary>
		/// <param name="path">生存期由调用方管理</param>
		/// <returns></returns>
		/// <exception cref="std::invalid_argument">不是ktex</exception>
		/// <created>Fa鸽,2019/11/23</created>
		/// <changed>Fa鸽,2019/11/23</changed>
		KTEXLIB3_EXPORT ktexlib::v3detail::Ktex load_ktex(const wchar_t* path);

		/// <summary>
		/// 加载一个tex文件
		/// </summary>
		/// <param name="file">生存期由调用方管理</param>
		/// <exception cref="std::invalid_argument">不是ktex</exception>
		/// <returns></returns>
		/// <created>Fa鸽,2019/11/23</created>
		/// <changed>Fa鸽,2019/11/23</changed>
		KTEXLIB3_EXPORT ktexlib::v3detail::Ktex load_ktex(std::ifstream& file);

#pragma warning(push)
#pragma warning (disable:4275)
		class KTEXLIB3_EXPORT invalid_mipschain : public std::logic_error
#pragma warning(pop)
		{
		public:
			invalid_mipschain(const char* message = "mipmap链各节点间大小不合规"):std::logic_error(message) {}
		};
	}

}