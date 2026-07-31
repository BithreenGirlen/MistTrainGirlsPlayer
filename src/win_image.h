#ifndef WIN_IMAGE_H_
#define WIN_IMAGE_H_

#include <vector>

namespace win_image
{
	struct SImageFrame
	{
		unsigned int width = 0;
		unsigned int height = 0;
		unsigned int stride = 0;
		std::vector<unsigned char> pixels;
	};

	enum class ERotation : uint8_t
	{
		None = 0, Deg90, Deg180, Deg270
	};

	enum class EAlpha : uint8_t
	{
		Normal = 0, Premultiplied
	};

	bool LoadImageToMemory(const wchar_t* filePath, SImageFrame* pImageFrame, float fScale = 1.f, ERotation rotation = ERotation::None, EAlpha alpha = EAlpha::Normal);
	/// @brief Pass `IWICBitmap**` to the second argument.
	bool LoadImageToWicBitmap(const wchar_t* filePath, void** pWicBitmap, float fScale = 1.f, ERotation rotation = ERotation::None, EAlpha alpha = EAlpha::Normal);

	bool SkimImageSize(const wchar_t* filePath, unsigned int* width, unsigned int* height);

	bool SaveImageAsPng(const wchar_t* filePath, unsigned int width, unsigned int height, unsigned int stride, unsigned char* pixels, bool hasAlpha);
	bool SaveImageAsJpg(const wchar_t* filePath, unsigned int width, unsigned int height, unsigned int stride, unsigned char* pixels, bool hasAlpha);

	class CWicGifEncoder
	{
	public:
		CWicGifEncoder();
		~CWicGifEncoder();

		bool initialise(const wchar_t* filePath);
		bool hasBeenInitialised() const;
		/// @brief Assume that pixel format is RGBA32
		bool commitFrame(unsigned int width, unsigned int height, unsigned int stride, unsigned char* pixels, bool hasAlpha, float delayInSeconds);

		bool finalise();
	private:
		class Impl;
		Impl* m_impl = nullptr;
	};
}
#endif // !WIN_IMAGE_H_
