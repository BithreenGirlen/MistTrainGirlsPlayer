
#include "sfml_spine_player.h"

#include <SFML/Audio.hpp>

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


std::shared_ptr<spine::SkeletonData> readSkeletonJsonData(const spine::String& filename, spine::Atlas* atlas, float scale) 
{
	spine::SkeletonJson json(atlas);
	json.setScale(scale);
	auto skeletonData = json.readSkeletonDataFile(filename);
	if (!skeletonData) 
	{
		/*必要であれば<windows.h>をインクルードしてMessageBoxを使う*/
		//printf("%s\n", json.getError().buffer());
		return nullptr;
	}
	return std::shared_ptr<spine::SkeletonData>(skeletonData);
}

std::shared_ptr<spine::SkeletonData> readSkeletonBinaryData(const char* filename, spine::Atlas* atlas, float scale) 
{
	spine::SkeletonBinary binary(atlas);
	binary.setScale(scale);
	auto skeletonData = binary.readSkeletonDataFile(filename);
	if (!skeletonData) 
	{
		//printf("%s\n", binary.getError().buffer());
		return nullptr;
	}
	return std::shared_ptr<spine::SkeletonData>(skeletonData);
}

CSfmlSpinePlayer::CSfmlSpinePlayer()
{

}

CSfmlSpinePlayer::~CSfmlSpinePlayer()
{

}

bool CSfmlSpinePlayer::SetSpines(const std::string& strFolderPath, const std::vector<std::string>& names)
{
	Clear();

	for (const std::string& name : names)
	{
		std::string strAtlasPath = strFolderPath + "\\" + name + ".atlas";
		std::string strSkeletonPath = strFolderPath + "\\" + name + ".skel";

		m_atlases.emplace_back(std::make_unique<spine::Atlas>(strAtlasPath.c_str(), &m_textureLoader));

		std::shared_ptr<spine::SkeletonData> skeltonData = readSkeletonBinaryData(strSkeletonPath.c_str(), m_atlases.back().get(), 1.0f);
		if (skeltonData == nullptr)return false;

		m_skeletonData.emplace_back(skeltonData);
	}

	if (m_skeletonData.empty())return false;

	/*背景に合わせる*/
	m_fMaxWidth = m_skeletonData.at(0).get()->getWidth();
	m_fMaxHeight = m_skeletonData.at(0).get()->getHeight();

	for (size_t i = 0; i < m_skeletonData.size(); ++i)
	{
		m_drawables.emplace_back(std::make_shared<spine::SkeletonDrawable>(m_skeletonData.at(i).get()));

		spine::SkeletonDrawable* drawable = m_drawables.back().get();
		drawable->timeScale = 1.0f;
		drawable->skeleton->setPosition(m_fMaxWidth / 2, m_fMaxHeight / 2);
		drawable->skeleton->updateWorldTransform();

		auto& animations = m_skeletonData.at(i).get()->getAnimations();
		for (size_t ii = 0; ii < animations.size(); ++ii)
		{
			std::string strAnimationName = animations[ii]->getName().buffer();
			auto iter = std::find(m_animationNames.begin(), m_animationNames.end(), strAnimationName);
			if (iter == m_animationNames.cend())m_animationNames.push_back(strAnimationName);
		}

		if (animations.size())
		{
			drawable->state->setAnimation(0, m_animationNames.at(0).c_str(), true);
		}
	}

	size_t nPos = strFolderPath.find_last_of("\\/");
	if (nPos != std::string::npos)
	{
		m_FolderName = strFolderPath.substr(nPos + 1);
	}

	return m_animationNames.size() > 0;
}
/*音声ファイル設定*/
void CSfmlSpinePlayer::SetAudios(std::vector<std::string>& filePaths)
{
	m_audio_files = filePaths;
}
/*ウィンドウ表示*/
int CSfmlSpinePlayer::Display()
{
	size_t nAnimationIndex = 0;

	float fTimeScale = 1.0f;
	float fSkeletonScale = 1.0f;

	sf::Vector2i iMouseStartPos;
	sf::Vector2i iOffset;

	size_t nAudioIndex = 0;
	sf::SoundBuffer soundBuffer;
	sf::Sound sound;
	if (!m_audio_files.empty())
	{
		soundBuffer.loadFromFile(m_audio_files.at(0));
		sound.setBuffer(soundBuffer);
		sound.setVolume(50.f);
		sound.play();
	}

	int iRet = 0;
	sf::RenderWindow window(sf::VideoMode(static_cast<unsigned int>(m_fMaxWidth), static_cast<unsigned int>(m_fMaxHeight)), "MistTrainGirls spine player", sf::Style::None);
	window.setPosition(sf::Vector2i(0, 0));
	window.setFramerateLimit(0);
	sf::Event event;
	sf::Clock deltaClock;
	while (window.isOpen())
	{
		while (window.pollEvent(event))
		{
			switch (event.type)
			{
			case sf::Event::Closed:
				window.close();
				break;
			case sf::Event::MouseButtonPressed:
				if (event.mouseButton.button == sf::Mouse::Left)
				{
					iMouseStartPos.x = event.mouseButton.x;
					iMouseStartPos.y = event.mouseButton.y;
				}
				break;
			case sf::Event::MouseButtonReleased:
				if (event.mouseButton.button == sf::Mouse::Left)
				{
					int iX = iMouseStartPos.x - event.mouseButton.x;
					int iY = iMouseStartPos.y - event.mouseButton.y;

					if (iX == 0 && iY == 0)
					{
						/*場面移行*/
						++nAnimationIndex;
						if (nAnimationIndex > m_animationNames.size() - 1)nAnimationIndex = 0;
						for (size_t i = 0; i < m_drawables.size(); ++i)
						{
							m_drawables.at(i).get()->state->setAnimation(0, m_animationNames.at(nAnimationIndex).c_str(), true);
						}
					}
					else
					{
						/*視点移動*/
						iOffset.x += iX;
						iOffset.y += iY;
						for (size_t i = 0; i < m_drawables.size(); ++i)
						{
							m_drawables.at(i).get()->skeleton->setPosition((m_fMaxWidth - iOffset.x) / 2, (m_fMaxHeight - iOffset.y) / 2);
						}
					}
				}
				if (event.mouseButton.button == sf::Mouse::Middle)
				{
					/*速度・拡縮・視点初期化*/
					fTimeScale = 1.0f;
					fSkeletonScale = 1.0f;
					iOffset = sf::Vector2i{};
					for (size_t i = 0; i < m_drawables.size(); ++i)
					{
						m_drawables.at(i).get()->timeScale = fTimeScale;
						m_drawables.at(i).get()->skeleton->setScaleX(fSkeletonScale);
						m_drawables.at(i).get()->skeleton->setScaleY(fSkeletonScale);
						m_drawables.at(i).get()->skeleton->setPosition(m_fMaxWidth / 2, m_fMaxHeight / 2);
						m_drawables.at(i).get()->skeleton->updateWorldTransform();
					}
					window.setSize(sf::Vector2u(static_cast<unsigned int>(m_fMaxWidth), static_cast<unsigned int>(m_fMaxHeight)));
				}
				break;
			case sf::Event::MouseWheelScrolled:
				if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
				{
					/*速度変更*/
					if (event.mouseWheelScroll.delta < 0)
					{
						fTimeScale += 0.05f;
					}
					else
					{
						fTimeScale -= 0.05f;
						if (fTimeScale < 0.f)fTimeScale = 0.f;
					}
					for (size_t i = 0; i < m_drawables.size(); ++i)
					{
						m_drawables.at(i).get()->timeScale = fTimeScale;
					}
				}
				else if (sf::Mouse::isButtonPressed(sf::Mouse::Right))
				{
					/*音声送り・戻し*/
					if (!m_audio_files.empty())
					{
						if (event.mouseWheelScroll.delta < 0)
						{
							++nAudioIndex;
							if (nAudioIndex >= m_audio_files.size())nAudioIndex = 0;
						}
						else
						{
							--nAudioIndex;
							if (nAudioIndex >= m_audio_files.size())nAudioIndex = m_audio_files.size() - 1;
						}
						soundBuffer.loadFromFile(m_audio_files.at(nAudioIndex));
						sound.setBuffer(soundBuffer);
						sound.play();
					}
				}
				else
				{
					/*拡縮変更*/
					if (event.mouseWheelScroll.delta < 0)
					{
						fSkeletonScale += 0.025f;
					}
					else
					{
						fSkeletonScale -= 0.025f;
						if (fSkeletonScale < 0.49f)fSkeletonScale = 0.5f;
					}
					for (size_t i = 0; i < m_drawables.size(); ++i)
					{
						m_drawables.at(i).get()->skeleton->setScaleX(fSkeletonScale > 0.99f ? fSkeletonScale : 1.f);
						m_drawables.at(i).get()->skeleton->setScaleY(fSkeletonScale > 0.99f ? fSkeletonScale : 1.f);
					}
					unsigned int uiWindowWidthMax = static_cast<unsigned int>(m_fMaxWidth * (fSkeletonScale - 0.025f));
					unsigned int uiWindowHeightMax = static_cast<unsigned int>(m_fMaxWidth * (fSkeletonScale - 0.025f));
					if (uiWindowWidthMax < sf::VideoMode::getDesktopMode().width || uiWindowHeightMax < sf::VideoMode::getDesktopMode().height)
					{
						window.setSize(sf::Vector2u(static_cast<unsigned int>(m_fMaxWidth* fSkeletonScale), static_cast<unsigned int>(m_fMaxHeight* fSkeletonScale)));
					}
				}
				break;
			case sf::Event::KeyReleased:
				if (event.key.code == sf::Keyboard::Key::Escape)
				{
					window.close();
				}
				if (event.key.code == sf::Keyboard::Key::Up)
				{
					iRet = 2;
					window.close();
				}
				if (event.key.code == sf::Keyboard::Key::Down)
				{
					iRet = 1;
					window.close();
				}
				//if (event.key.code == sf::Keyboard::Key::Insert)
				//{
				//	sf::Texture texture;
				//	texture.create(window.getSize().x, window.getSize().y);
				//	texture.update(window);
				//	sf::Image image = texture.copyToImage();
				//	spine:: Animation *animation = m_skeletonData.at(0).get()->findAnimation(m_animationNames.at(nAnimationIndex).c_str());
				//	std::string strElapsed;
				//	if (animation != nullptr)
				//	{
				//		strElapsed = std::to_string(animation->getDuration());
				//	}
				//	std::string strFileName = m_FolderName + "_" + m_animationNames.at(nAnimationIndex) + "_" + strElapsed + ".png";
				//	image.saveToFile(strFileName);
				//}
				break;
			}
		}

		float delta = deltaClock.getElapsedTime().asSeconds();
		deltaClock.restart();

		window.clear();
		for (size_t i = 0; i < m_drawables.size(); ++i)
		{
			m_drawables.at(i).get()->update(delta);
			window.draw(*m_drawables.at(i).get(), sf::RenderStates(sf::BlendAlpha));
		}

		window.display();

		if (!m_audio_files.empty())
		{
			if (sound.getStatus() == sf::SoundSource::Stopped)
			{
				if (nAudioIndex < m_audio_files.size() - 1)
				{
					++nAudioIndex;
					soundBuffer.loadFromFile(m_audio_files.at(nAudioIndex));
					sound.setBuffer(soundBuffer);
					sound.play();
				}
			}
		}
	}
	return iRet;
}
/*消去*/
void CSfmlSpinePlayer::Clear()
{
	m_atlases.clear();
	m_skeletonData.clear();
	m_animationNames.clear();
}
