#ifndef SFML_SPINE_PLAYER_H_
#define SFML_SPINE_PLAYER_H_

#include <memory>

#include "spine-sfml.h"

class CSfmlSpinePlayer
{
public:
	CSfmlSpinePlayer();
	~CSfmlSpinePlayer();
	bool SetSpines(const std::string &strFolderPath, const std::vector<std::string>& names);
	void SetAudios(std::vector<std::string>& filePaths);
	bool SetFont(const std::string& strFilePath, bool bBold, bool bItalic);
	int Display();
private:
	CSfmlTextureLoader m_textureLoader;

	std::vector<std::unique_ptr<spine::Atlas>> m_atlases;
	std::vector<std::shared_ptr<spine::SkeletonData>> m_skeletonData;
	std::vector<std::shared_ptr<CSfmlSpineDrawable>> m_drawables;

	std::vector<std::string> m_animationNames;

	float m_fMaxWidth = 0.f;
	float m_fMaxHeight = 0.f;

	std::vector<std::string> m_audio_files;
	size_t m_nAudioIndex = 0;

	sf::Font m_trackFont;
	sf::Text m_trackText;
	bool m_bTrackHidden = false;
	void SwitchTextColor();

	void Clear();
};

#endif // SFML_SPINE_PLAYER_H_
