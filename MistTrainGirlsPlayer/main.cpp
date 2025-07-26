

#include "framework.h"
#include "resource.h"

#include <locale.h>

#include "win_dialogue.h"
#include "win_filesystem.h"
#include "win_text.h"

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
		win_filesystem::CreateFilePathList(wstrFolderPath.c_str(), L".atlas", filePaths);

		for (const std::wstring& filePath : filePaths)
		{
			size_t nPos = filePath.find_last_of(L"\\/");
			if (nPos == std::wstring::npos)continue;

			std::wstring wstrFileName = filePath.substr(nPos + 1);
			nPos = wstrFileName.rfind(L'.');
			if (nPos == std::wstring::npos)continue;

			std::wstring wstrName = wstrFileName.substr(0, nPos);
			names.push_back(win_text::NarrowUtf8(wstrName));
		}
	}

	static void GetAudioFileList(const std::wstring& wstrFolderPath, std::vector<std::string>& audioFilePaths)
	{
		std::vector<std::wstring> filePaths;
		win_filesystem::CreateFilePathList(wstrFolderPath.c_str(), L".mp3", filePaths);

		for (const std::wstring& filePath : filePaths)
		{
			audioFilePaths.push_back(win_text::NarrowUtf8(filePath));
		}
	}

	static unsigned long GetDisplayRefreshRate()
	{
		DEVMODE sDevMode{};
		::EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &sDevMode);
		return sDevMode.dmDisplayFrequency;
	}

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
	
	std::wstring wstrPickedFolder = win_dialogue::SelectWorkFolder(mainWindow.GetWindow()->getSystemHandle());
	if (!wstrPickedFolder.empty())
	{
		mainWindow.SetFont("C:\\Windows\\Fonts\\yumindb.ttf", true, true);
		mainWindow.GetWindow()->setFramerateLimit(mstr::GetDisplayRefreshRate());

		std::vector<std::wstring> folders;
		size_t nFolderIndex = 0;
		win_filesystem::GetFilePathListAndIndex(wstrPickedFolder, nullptr, folders, &nFolderIndex);
		for (;;)
		{
			std::wstring wstrFolderPath = folders[nFolderIndex];

			std::vector<std::string> names;
			mstr::GetSpineNameList(wstrFolderPath, names);
			bool bRet = mainWindow.SetSpines(win_text::NarrowUtf8(wstrFolderPath), names);
			if (!bRet)break;

			std::vector<std::string> audioFilePaths;
			mstr::GetAudioFileList(wstrFolderPath, audioFilePaths);
			mainWindow.SetVoices(audioFilePaths);

			int iRet = mainWindow.Display();
			if (iRet == 1)
			{
				++nFolderIndex;
				if (nFolderIndex > folders.size() - 1)nFolderIndex = 0;
			}
			else if (iRet == 2)
			{
				--nFolderIndex;
				if (nFolderIndex > folders.size() - 1)nFolderIndex = folders.size() - 1;
			}
			else
			{
				break;
			}
		}
	}

	return 0;
}
