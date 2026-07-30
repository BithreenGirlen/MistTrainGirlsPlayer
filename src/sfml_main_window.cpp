

#include <SFML/Audio.hpp>

#include "sfml_main_window.h"

CSfmlMainWindow::CSfmlMainWindow(const wchar_t* swzWindowName)
{
	m_window = std::make_unique<sf::RenderWindow>(sf::VideoMode(1280, 720), swzWindowName, sf::Style::None);

	m_window->setPosition(sf::Vector2i(0, 0));

	m_sfmlSpinePlayer = std::make_unique<CSfmlSpinePlayer>(m_window.get());
}

CSfmlMainWindow::~CSfmlMainWindow()
{

}

bool CSfmlMainWindow::SetSpines(const std::string& folderPath, const std::vector<std::string>& names)
{
	std::vector<std::string> atlasPaths;
	atlasPaths.resize(names.size());
	std::vector<std::string> skelPaths;
	skelPaths.resize(names.size());
	for (size_t i = 0; i < names.size(); ++i)
	{
		atlasPaths[i] = folderPath + "\\" + names[i] + ".atlas";
		skelPaths[i] = folderPath + "\\" + names[i] + ".skel";
	}
	return m_sfmlSpinePlayer->LoadSpineFromFile(atlasPaths, skelPaths, true);
}

void CSfmlMainWindow::SetVoices(std::vector<std::string>& filePaths)
{
	m_audio_files = std::move(filePaths);
	m_nAudioIndex = 0;
}
/*‘‘ÌÝ’è*/
bool CSfmlMainWindow::SetFont(const std::string& filePath, bool bBold, bool bItalic)
{
	bool bRet = m_trackFont.loadFromFile(filePath);
#ifdef _WIN32
	if (!bRet)
	{
		bRet = m_trackFont.loadFromFile("C:\\Windows\\Fonts\\arialnb.ttf");
		if (!bRet)return false;
	}
#else
	if (!bRet)return false;
#endif
	constexpr float fOutLineThickness = 2.4f;

	/*Audio track indicator*/
	m_trackText.setFont(m_trackFont);
	m_trackText.setFillColor(sf::Color::Black);
	m_trackText.setStyle((bBold ? sf::Text::Style::Bold : 0) | (bItalic ? sf::Text::Style::Italic : 0));
	m_trackText.setOutlineThickness(fOutLineThickness);
	m_trackText.setOutlineColor(sf::Color::White);

	return true;
}

int CSfmlMainWindow::Display()
{
	ResetScale();

	sf::SoundBuffer soundBuffer;
	sf::Sound sound;
	if (!m_audio_files.empty())
	{
		soundBuffer.loadFromFile(m_audio_files[0]);
		sound.setBuffer(soundBuffer);
		sound.setVolume(50.f);
		sound.play();
	}

	/*C³‚ª–Ê“|‚È‚Ì‚Å‚±‚±‚Å‹Lq*/
	const auto UpdateTrackIndicator = [this]()
		-> void
		{
			if (m_nAudioIndex >= m_audio_files.size())
			{
				m_trackText.setString("");
				return;
			}

			std::string str = std::to_string(m_nAudioIndex + 1) + "/" + std::to_string(m_audio_files.size());
			m_trackText.setString(str);
		};

	const auto StepOnTrack = [this, &soundBuffer, &sound, &UpdateTrackIndicator](bool bForward)
		-> void
		{
			if (!m_audio_files.empty())
			{
				if (bForward)
				{
					++m_nAudioIndex;
					if (m_nAudioIndex >= m_audio_files.size())m_nAudioIndex = 0;
				}
				else
				{
					--m_nAudioIndex;
					if (m_nAudioIndex >= m_audio_files.size())m_nAudioIndex = m_audio_files.size() - 1;
				}
				soundBuffer.loadFromFile(m_audio_files.at(m_nAudioIndex));
				sound.setBuffer(soundBuffer);
				sound.play();

				UpdateTrackIndicator();
			}
		};

	UpdateTrackIndicator();

	sf::Vector2i iMouseStartPos;

	bool bOnWindowMove = false;
	bool bLeftDowned = false;
	bool bLeftCombinated = false;

	m_spineClock.restart();
	while (m_window->isOpen())
	{
		sf::Event event;
		while (m_window->pollEvent(event))
		{
			switch (event.type)
			{
			case sf::Event::Closed:
				m_window->close();
				break;
			case sf::Event::MouseButtonPressed:
				if (event.mouseButton.button == sf::Mouse::Left)
				{
					iMouseStartPos.x = event.mouseButton.x;
					iMouseStartPos.y = event.mouseButton.y;

					bLeftDowned = true;
				}
				break;
			case sf::Event::MouseButtonReleased:
				if (event.mouseButton.button == sf::Mouse::Left)
				{
					if (bLeftCombinated)
					{
						bLeftCombinated = false;
						bLeftDowned = false;
						break;
					}

					if (bOnWindowMove || sf::Mouse::isButtonPressed(sf::Mouse::Right))
					{
						bOnWindowMove ^= true;
						break;
					}

					int iX = iMouseStartPos.x - event.mouseButton.x;
					int iY = iMouseStartPos.y - event.mouseButton.y;

					if (bLeftDowned && iX == 0 && iY == 0)
					{
						m_sfmlSpinePlayer->ShiftAnimation();
					}

					bLeftDowned = false;
				}
				if (event.mouseButton.button == sf::Mouse::Middle)
				{
					ResetScale();
				}
				break;
			case sf::Event::MouseMoved:
				if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
				{
					if (bLeftDowned)
					{
						int iX = iMouseStartPos.x - event.mouseMove.x;
						int iY = iMouseStartPos.y - event.mouseMove.y;
						m_sfmlSpinePlayer->MoveViewPoint(iX, iY);

						iMouseStartPos.x = event.mouseMove.x;
						iMouseStartPos.y = event.mouseMove.y;

						bLeftCombinated = true;
					}
				}
				break;
			case sf::Event::MouseWheelScrolled:
				if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
				{
					m_sfmlSpinePlayer->RescaleTime(event.mouseWheelScroll.delta < 0);
					bLeftCombinated = true;
				}
				else if (sf::Mouse::isButtonPressed(sf::Mouse::Right))
				{
					/*‰¹º‘—‚èE–ß‚µ*/
					StepOnTrack(event.mouseWheelScroll.delta < 0);
				}
				else
				{
					m_sfmlSpinePlayer->RescaleSkeleton(event.mouseWheelScroll.delta < 0);
					if (!sf::Keyboard::isKeyPressed(sf::Keyboard::LControl))
					{
						m_sfmlSpinePlayer->RescaleCanvas(event.mouseWheelScroll.delta < 0);
						ResizeWindow();
					}
				}
				break;
			case sf::Event::KeyPressed:
				switch (event.key.code)
				{
				case sf::Keyboard::Key::Left:
					StepOnTrack(false);
					break;
				case sf::Keyboard::Key::Right:
					StepOnTrack(true);
					break;
				}
				break;
			case sf::Event::KeyReleased:
				switch (event.key.code)
				{
				case sf::Keyboard::Key::A:
					m_sfmlSpinePlayer->TogglePma();
					break;
				case sf::Keyboard::Key::B:
					m_sfmlSpinePlayer->ToggleBlendModeAdoption();
					break;
				case sf::Keyboard::Key::C:
					ToggleTextColour();
					break;
				case sf::Keyboard::Key::T:
					ToggleTextVisibility();
					break;
				case sf::Keyboard::Key::Escape:
					m_window->close();
					break;
				case sf::Keyboard::Key::Up:
					return 2;
				case sf::Keyboard::Key::Down:
					return 1;
				default:
					break;
				}
				break;
			}
		}

		float fDelta = m_spineClock.getElapsedTime().asSeconds();
		m_sfmlSpinePlayer->Update(fDelta);
		m_spineClock.restart();

		m_window->clear(sf::Color(0, 0, 0, 0));

		m_sfmlSpinePlayer->Redraw();
		if (!m_bTrackHidden)
		{
			m_window->draw(m_trackText);
		}

		m_window->display();

		if (!m_audio_files.empty())
		{
			if (sound.getStatus() == sf::SoundSource::Stopped)
			{
				if (m_nAudioIndex < m_audio_files.size() - 1)
				{
					StepOnTrack(true);
				}
			}
		}

		if (bOnWindowMove)
		{
			int iPosX = sf::Mouse::getPosition().x - m_window->getSize().x / 2;
			int iPosY = sf::Mouse::getPosition().y - m_window->getSize().y / 2;
			m_window->setPosition(sf::Vector2i(iPosX, iPosY));
		}
	}

	return 0;
}

void CSfmlMainWindow::ResizeWindow()
{
	if (m_sfmlSpinePlayer.get() != nullptr)
	{
		sf::Vector2f fBaseSize = m_sfmlSpinePlayer->GetBaseSize();
		float fScale = m_sfmlSpinePlayer->GetCanvasScale();

		unsigned int uiWindowWidthMax = static_cast<unsigned int>(fBaseSize.x * (fScale - CSfmlSpinePlayer::kfScalePortion));
		unsigned int uiWindowHeightMax = static_cast<unsigned int>(fBaseSize.y * (fScale - CSfmlSpinePlayer::kfScalePortion));
		if (uiWindowWidthMax < sf::VideoMode::getDesktopMode().width || uiWindowHeightMax < sf::VideoMode::getDesktopMode().height)
		{
			m_window->setSize(sf::Vector2u(static_cast<unsigned int>(fBaseSize.x * fScale), static_cast<unsigned int>(fBaseSize.y * fScale)));
			m_window->setView(sf::View((fBaseSize * fScale) / 2.f, fBaseSize * fScale));
		}
	}
}

void CSfmlMainWindow::ResetScale()
{
	m_sfmlSpinePlayer->ResetScale();
	ResizeWindow();
	m_sfmlSpinePlayer->SetZoom(1.05f);
}

void CSfmlMainWindow::ToggleTextColour()
{
	m_trackText.setFillColor(m_trackText.getFillColor() == sf::Color::Black ? sf::Color::White : sf::Color::Black);
	m_trackText.setOutlineColor(m_trackText.getFillColor() == sf::Color::Black ? sf::Color::White : sf::Color::Black);
}

void CSfmlMainWindow::ToggleTextVisibility()
{
	m_bTrackHidden ^= true;
}
