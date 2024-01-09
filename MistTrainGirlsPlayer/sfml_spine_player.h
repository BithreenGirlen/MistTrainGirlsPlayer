#ifndef SFML_SPINE_PLAYER_H_
#define SFML_SPINE_PLAYER_H_

#include <memory>

#include <SFML/Graphics.hpp>

#include "deps/spine-sfml-3.8/spine-sfml.h"

class CSfmlSpinePlayer
{
public:
	CSfmlSpinePlayer();
	~CSfmlSpinePlayer();
	bool SetSpines(const std::string &strFolderPath, const std::vector<std::string>& names);
	void SetAudios(std::vector<std::string>& filePaths);
	int Display();
private:
	spine::SFMLTextureLoader m_textureLoader;

	std::vector<std::unique_ptr<spine::Atlas>> m_atlases;
	std::vector<std::shared_ptr<spine::SkeletonData>> m_skeletonData;

	std::vector<std::string> m_animationNames;
	std::vector<std::shared_ptr<spine::SkeletonDrawable>> m_drawables;

	float m_fMaxWidth = 0.f;
	float m_fMaxHeight = 0.f;

	std::vector<std::string> m_audio_files;
	std::string m_FolderName;

	void Clear();
};

#endif // SFML_SPINE_PLAYER_H_
