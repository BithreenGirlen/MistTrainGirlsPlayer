

#include "framework.h"

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

    std::wstring wstrPickedFolder = win_dialogue::SelectWorkFolder(nullptr);
    if (!wstrPickedFolder.empty())
    {
        CSfmlSpinePlayer SfmlPlayer;
        SfmlPlayer.SetFont("C:\\Windows\\Fonts\\yumindb.ttf", true, true);

        std::vector<std::wstring> folders;
        size_t nFolderIndex = 0;
        win_filesystem::GetFolderListAndIndex(wstrPickedFolder, folders, &nFolderIndex);
        for (;;)
        {
            std::wstring wstrFolderPath = folders.at(nFolderIndex);

            std::vector<std::string> names;
            GetSpineNameList(wstrFolderPath, names);

            std::vector<std::string> audioFilePaths;
            GetAudioFileList(wstrFolderPath, audioFilePaths);

            bool bRet = SfmlPlayer.SetSpines(win_text::NarrowUtf8(wstrFolderPath), names);
            if (!bRet)break;

            SfmlPlayer.SetAudios(audioFilePaths);

            int iRet = SfmlPlayer.Display();
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
