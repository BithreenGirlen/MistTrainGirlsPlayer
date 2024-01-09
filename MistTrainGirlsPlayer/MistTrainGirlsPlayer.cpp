

#include "framework.h"
#include "MistTrainGirlsPlayer.h"

#include "win_dialogue.h"
#include "win_filesystem.h"
#include "win_text.h"

#include "sfml_spine_player.h"


void GetSpineNameList(const std::wstring &wstrFolderPath, std::vector<std::string>& names)
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

void GetAudioFileList(const std::wstring& wstrFolderPath, std::vector<std::string>& audioFilePaths)
{
    std::vector<std::wstring> filePaths;
    win_filesystem::CreateFilePathList(wstrFolderPath.c_str(), L".mp3", filePaths);

    for (const std::wstring& filePath : filePaths)
    {
        audioFilePaths.push_back(win_text::NarrowUtf8(filePath));
    }
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    std::wstring wstrFolderPath = win_dialogue::SelectWorkFolder(nullptr);
    if (!wstrFolderPath.empty())
    {
        std::vector<std::string> names;
        GetSpineNameList(wstrFolderPath, names);

        std::vector<std::string> audioFilePaths;
        GetAudioFileList(wstrFolderPath, audioFilePaths);

        CSfmlSpinePlayer SfmlPlayer;
        bool bRet = SfmlPlayer.SetSpines(win_text::NarrowUtf8(wstrFolderPath), names);
        if (bRet)
        {
            SfmlPlayer.SetAudios(audioFilePaths);

            SfmlPlayer.Display();
        }
    }

    return 0;
}
