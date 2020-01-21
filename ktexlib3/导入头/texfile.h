#pragma once

#if defined(KTEXLIB3_EXPORTS)
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

	template<typename T,typename array_t>
	struct KTEXLIB3_EXPORT ro_container_property
	{
		ro_container_property(const array_t& arr):arr(arr) {}

		const T& operator() (size_t i)
		{
			return (arr)[i];
		}

		const T& operator[] (size_t i)
		{
			return (arr)[i];
		}

		const size_t size()
		{
			return arr.size();
		}

		auto begin() { return cbegin(); }
		auto end() { return cend(); }
		auto cbegin() { return arr.cbegin(); }
		auto cend() { return arr.cend(); }
	private:
		const array_t& arr;
	};

	template<typename T>
	struct KTEXLIB3_EXPORT ro_property
	{
		ro_property(T& val) :val(val)
		{

		}

		operator T()
		{
			return val;
		}
		T operator() ()
		{
			return val;
		}
		T& val;
	};

	namespace v3detail
	{
		/// <summary>
		/// ���ʵ����ظ�ʽ
		/// </summary>
		enum class PixelFormat :unsigned short
		{
			/// <summary>δ֪��ʽ</summary>
			unknown = 0,
			/// <summary>bc1</summary>
			dxt1 = 1,
			/// <summary>bc2</summary>
			dxt3 = 2,
			/// <summary>���,bc3</summary>
			dxt5 = 3,
			/// <summary>�߱��棬��С�ܴ�</summary>
			rgba = 4,
			/// <summary>�˾��õ�</summary>
			r8g8b8 = 7,
		};

		/// <summary>
		/// ���ʶ�Ӧƽ̨
		/// </summary>
		enum class Platform : unsigned char
		{
			/// <summary>OpenGL���ʺϼ���PC�棬���Բ�����mipmap</summary>
			opengl = 12,
			/// <summary>XBox360��Ҫ��δ֪</summary>
			xb360 = 11,
			/// <summary>PS3��Ҫ��δ֪</summary>
			ps3 = 10,
			/// <summary>�ʺ�����ƽ̨����������mipmap</summary>
			universal = 0
		};

		/// <summary>
		/// ��������
		/// </summary>
		enum class TextureType : unsigned char
		{
			/// <summary>1D</summary>
			d1 = 1,
			/// <summary>2D</summary>
			d2 = 2,
			/// <summary>3D</summary>
			d3 = 3,
			/// <summary>Cubemap���ʺ�2D��Ϸ</summary>
			cubemap = 4
		};

		struct KTEXLIB3_EXPORT Mipmap
		{
			Mipmap(uint32_t w, uint32_t h, uint32_t pitch);
			Mipmap(const Mipmap& other);
			Mipmap(Mipmap&& xval) noexcept;

			Mipmap(const uint32_t w, const uint32_t h, const uint32_t pitch, const std::vector<unsigned char>& data);
			Mipmap(const uint32_t w, const uint32_t h, const uint32_t pitch, std::vector<unsigned char>&& data) noexcept;

			uint32_t width, height, pitch = 0;//pitch��һ�е����ݳ��ȣ����ֽ�Ϊ��λ
			std::vector<unsigned char> data;
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
			Ktex() :Mipmaps(this->mips) {}

			template<typename mips_iterator>
			Ktex(mips_iterator begin, mips_iterator end): mips(begin, end),Mipmaps(this->mips) {}

			void AddMipmap(const Mipmap& mipmap);
			void AddMipmap(Mipmap&& mipmap);

			ro_container_property<Mipmap, std::vector<Mipmap>> Mipmaps;
			
			KTEXInfo info;

			void SaveFile(std::filesystem::path path);
		private:
			std::vector<Mipmap> mips;

		};

		/// <summary>
		/// ת��
		/// </summary>
		/// <param name="image"></param>
		/// <param name="pararral">true���߳�ѹ����false���߳�</param>
		/// <param name="fmt">Ŀ�����ظ�ʽ</param>
		/// <returns></returns>
		/// <created>Fa��,2019/12/28</created>
		/// <changed>Fa��,2019/12/28</changed>
		KTEXLIB3_EXPORT Mipmap convert(const RgbaImage& image, bool pararral = true, PixelFormat fmt = PixelFormat::dxt5);

		/// <summary>
		/// ��ѹmipmap
		/// </summary>
		/// <param name="Mipmap"></param>
		/// <param name="fmt">mipmap���ظ�ʽ</param>
		/// <returns></returns>
		/// <created>Fa��,2020/1/21</created>
		/// <changed>Fa��,2020/1/21</changed>
		KTEXLIB3_EXPORT RgbaImage decompress(const Mipmap& Mipmap, PixelFormat fmt);
	}

	namespace v3
	{
		/// <summary>
		/// ����COM
		/// </summary>
		/// <returns>HRESULT</returns>
		/// <created>Fa��,2020/1/18</created>
		/// <changed>Fa��,2020/1/18</changed>
		extern "C" KTEXLIB3_EXPORT HRESULT init_COM_as_mthread();

		/// <summary>
		/// ָ���ļ�����ת����ͬһĿ¼
		/// </summary>
		/// <param name="filename">�ļ���</param>
		/// <created>Fa��,2019/11/1</created>
		/// <changed>Fa��,2019/11/15</changed>
		extern "C" KTEXLIB3_EXPORT void gen_bc3universal(const char8_t* pngpath, const char8_t * output = nullptr);

		/// <summary>
		/// ָ��UTF16�ļ�����ת����ͬһĿ¼
		/// </summary>
		/// <param name="path"></param>
		/// <returns></returns>
		/// <created>Fa��,2019/11/23</created>
		/// <changed>Fa��,2019/11/23</changed>
		KTEXLIB3_EXPORT bool gen_bc3universal(const char16_t * path, const char16_t * outpath = nullptr);

		/// <summary>
		/// ָ�����ַ��ļ�����ת����ͬһĿ¼
		/// </summary>
		/// <param name="path"></param>
		/// <returns></returns>
		/// <created>Fa��,2019/11/23</created>
		/// <changed>Fa��,2019/11/23</changed>
		KTEXLIB3_EXPORT bool gen_bc3universal(const wchar_t * path, const wchar_t * outpath = nullptr);

		/// <summary>
		/// ����һ��tex�ļ�
		/// </summary>
		/// <param name="path">�������ɵ��÷�����</param>
		/// <returns></returns>
		/// <exception cref="std::invalid_argument">����ktex</exception>
		/// <created>Fa��,2019/11/23</created>
		/// <changed>Fa��,2019/11/23</changed>
		KTEXLIB3_EXPORT ktexlib::v3detail::Ktex load_ktex(const char8_t* path);

		/// <summary>
		/// ����һ��tex�ļ�
		/// </summary>
		/// <param name="path">�������ɵ��÷�����</param>
		/// <returns></returns>
		/// <exception cref="std::invalid_argument">����ktex</exception>
		/// <created>Fa��,2019/11/23</created>
		/// <changed>Fa��,2019/11/23</changed>
		KTEXLIB3_EXPORT ktexlib::v3detail::Ktex load_ktex(const wchar_t* path);

		/// <summary>
		/// ����һ��tex�ļ�
		/// </summary>
		/// <param name="file">�������ɵ��÷�����</param>
		/// <exception cref="std::invalid_argument">����ktex</exception>
		/// <returns></returns>
		/// <created>Fa��,2019/11/23</created>
		/// <changed>Fa��,2019/11/23</changed>
		KTEXLIB3_EXPORT ktexlib::v3detail::Ktex load_ktex(std::ifstream& file);

		class KTEXLIB3_EXPORT invalid_mipschain: public std::logic_error
		{
		public:
			invalid_mipschain(const char* message = "mipmap�����ڵ���С���Ϲ�"):std::logic_error(message) {}
		};
	}

}