

#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <locale.h>

#include "win_dialogue.h"
#include "win_filesystem.h"
#include "win_text.h"
#include "win_image.h"

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "winmm.lib")

#ifdef  _DEBUG
#pragma comment(lib, "sfml-system-d.lib")
#pragma comment(lib, "sfml-graphics-d.lib")
#pragma comment(lib, "sfml-window-d.lib")
#pragma comment(lib, "sfml-audio-d.lib")
#else
#pragma comment(lib, "sfml-system.lib")
#pragma comment(lib, "sfml-graphics.lib")
#pragma comment(lib, "sfml-window.lib")
#pragma comment(lib, "sfml-audio.lib")
#endif // _DEBUG

#include "sfml_main_window.h"

namespace mstr
{
	static void GetSpineNameList(const std::wstring& wstrFolderPath, std::vector<std::string>& names)
	{
		std::vector<std::wstring> filePaths;
		win_filesystem::CreateFilePathList(wstrFolderPath, L".atlas", filePaths);

		for (const std::wstring& filePath : filePaths)
		{
			size_t nPos1 = filePath.find_last_of(L"\\/");
			if (nPos1 == std::wstring::npos)continue;
			++nPos1;

			size_t nPos2 = filePath.find(L'.', nPos1);
			if (nPos1 == std::wstring::npos)continue;

			int fileNameLength = static_cast<int>(nPos2 - nPos1);
			std::string utf8FileName = win_text::NarrowUtf8(&filePath[nPos1], fileNameLength);
			names.push_back(std::move(utf8FileName));
		}
	}

	static void GetAudioFileList(const std::wstring& wstrFolderPath, std::vector<std::string>& audioFilePaths)
	{
		std::vector<std::wstring> filePaths;
		win_filesystem::CreateFilePathList(wstrFolderPath, L".mp3", filePaths);

		for (const std::wstring& filePath : filePaths)
		{
			audioFilePaths.push_back(win_text::NarrowUtf8(filePath));
		}
	}

	static unsigned long GetDisplayRefreshRate()
	{
		DEVMODE devMode{};
		::EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &devMode);
		return devMode.dmDisplayFrequency;
	}

	/// @brief PNGファイルが存在しなかったらwebpファイルを探索し、WICを通じて読み込む
	static void SpineTextureLoaderCallback(void* pUserDatum, const char* textureFilePath, size_t filePathLength, void* pOutImage)
	{
		if (pOutImage == nullptr)return;

		wchar_t pathBuffer[512]{};
		static constexpr int bufferCapacity = std::size(pathBuffer) - 1;
		int pathBufferLength = win_text::WidenUtf8InBuffer(textureFilePath, static_cast<int>(filePathLength), pathBuffer, bufferCapacity);
		bool bRet = win_filesystem::DoesFileExist(pathBuffer);
		if (bRet)return; /* PNGファイル有り */

		const wchar_t* pPos = pathBuffer + pathBufferLength;
		for (; pPos != pathBuffer; --pPos)
		{
			if (*pPos == L'.')break;
		}
		size_t fileNameLength = pPos - pathBuffer;

		static constexpr wchar_t webpExt[] = L".webp";
		static constexpr size_t webpExtLength = std::size(webpExt) - 1;
		if (fileNameLength + webpExtLength >= static_cast<size_t>(bufferCapacity)) return;
		::wmemcpy(&pathBuffer[fileNameLength], webpExt, webpExtLength);
		fileNameLength += webpExtLength;
		pathBuffer[fileNameLength] = L'\0';
		bRet = win_filesystem::DoesFileExist(pathBuffer);
		if (!bRet)return; /* webpもなし */

		/*
		* SFML, or stb_image to be precise, does not support webp decoding.
		* On the other hand, integrating libwebp is bothersome both in coding and in license requirement.
		* So rely on OS-specific feature here.
		*/
		win_image::SImageFrame imageFrame;
		bRet = win_image::LoadImageToMemory(pathBuffer, &imageFrame);
		if (!bRet)return;

		/* BGRA32 => RGBA32 */
		uint32_t* pPixels = reinterpret_cast<uint32_t*>(imageFrame.pixels.data());
		const size_t nCount = imageFrame.pixels.size() / 4;
		for (size_t i = 0; i < nCount; ++i)
		{
			auto& p = pPixels[i];
			p = ((p & 0x000000ff) << 16) | ((p & 0x00ff0000) >> 16) | ((p & 0xff00ff00));
		}

		sf::Image* pImage = static_cast<sf::Image*>(pOutImage);
		pImage->create(imageFrame.width, imageFrame.height, imageFrame.pixels.data());
	}

	/// @brief Pripritise using dedicated GPU because multiple textures are used in a single frame drawing
	extern "C"
	{
		_declspec(selectany) _declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
		_declspec(selectany) _declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
	}
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	::setlocale(LC_ALL, ".utf8");
	CSfmlMainWindow mainWindow(L"MistTrainGirls spine player");

	std::wstring wstrPickedFolder = win_dialogue::SelectFolder(nullptr, mainWindow.getWindow()->getSystemHandle());
	if (!wstrPickedFolder.empty())
	{
		mainWindow.setFont("C:\\Windows\\Fonts\\yumindb.ttf", true, true);
		mainWindow.getWindow()->setFramerateLimit(mstr::GetDisplayRefreshRate());
		mainWindow.getSpinePlayer()->setTextureLoadCallback(&mstr::SpineTextureLoaderCallback, nullptr);

		std::vector<std::wstring> folderPaths;
		size_t nFolderIndex = 0;
		win_filesystem::GetFilePathListAndIndex(wstrPickedFolder, nullptr, folderPaths, &nFolderIndex);
		for (;;)
		{
			const std::wstring& wstrFolderPath = folderPaths[nFolderIndex];

			std::vector<std::string> names;
			mstr::GetSpineNameList(wstrFolderPath, names);
			bool bRet = mainWindow.setSpines(win_text::NarrowUtf8(wstrFolderPath), names);
			if (!bRet)break;

			std::vector<std::string> audioFilePaths;
			mstr::GetAudioFileList(wstrFolderPath, audioFilePaths);
			mainWindow.setVoices(audioFilePaths);

			int iRet = mainWindow.display();
			if (iRet == 1)
			{
				++nFolderIndex;
				if (nFolderIndex > folderPaths.size() - 1)nFolderIndex = 0;
			}
			else if (iRet == 2)
			{
				--nFolderIndex;
				if (nFolderIndex > folderPaths.size() - 1)nFolderIndex = folderPaths.size() - 1;
			}
			else
			{
				break;
			}
		}
	}

	return 0;
}
