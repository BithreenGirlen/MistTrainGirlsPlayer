

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

void GetFolderList(const std::wstring& wstrFolderPath, std::vector<std::wstring>& folders, size_t *nIndex)
{
    std::wstring wstrParent;
    std::wstring wstrCurrent;

    size_t nPos = wstrFolderPath.find_last_of(L"\\/");
    if (nPos != std::wstring::npos)
    {
        wstrParent = wstrFolderPath.substr(0, nPos);
        wstrCurrent = wstrFolderPath.substr(nPos + 1);
    }

    win_filesystem::CreateFilePathList(wstrParent.c_str(), nullptr, folders);

    auto iter = std::find(folders.begin(), folders.end(), wstrFolderPath);
    if (iter != folders.end())
    {
        *nIndex = std::distance(folders.begin(), iter);
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
        std::vector<std::wstring> folders;
        size_t nFolderIndex = 0;
        GetFolderList(wstrPickedFolder, folders, &nFolderIndex);
        for (;;)
        {
            std::wstring wstrFolderPath = folders.at(nFolderIndex);

            std::vector<std::string> names;
            GetSpineNameList(wstrFolderPath, names);

            std::vector<std::string> audioFilePaths;
            GetAudioFileList(wstrFolderPath, audioFilePaths);

            CSfmlSpinePlayer SfmlPlayer;
            bool bRet = SfmlPlayer.SetSpines(win_text::NarrowUtf8(wstrFolderPath), names);
            if (bRet)
            {
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
            else
            {
                break;
            }
        }
    }

    return 0;
}
