
#if defined(_WIN32)
	#include <Windows.h> /* To retrieve monitor information. */
#endif

#include <SFML/Audio.hpp>

#include "sfml_main_window.h"

CSfmlMainWindow::CSfmlMainWindow(const wchar_t* windowName)
{
	m_window = std::make_unique<sf::RenderWindow>(sf::VideoMode(1280, 720), windowName, sf::Style::None);
	alignWindowToTheTopLeftOfMonitor();

	m_sfmlSpinePlayer = std::make_unique<CSfmlSpinePlayer>();
}

CSfmlMainWindow::~CSfmlMainWindow()
{

}

bool CSfmlMainWindow::setSpines(const std::string& folderPath, const std::vector<std::string>& names)
{
	std::vector<std::string> atlasPaths;
	atlasPaths.resize(names.size());
	std::vector<std::string> skelPaths;
	skelPaths.resize(names.size());
	for (size_t i = 0; i < names.size(); ++i)
	{
		atlasPaths[i].assign(folderPath).append("\\").append(names[i]).append(".atlas");
		skelPaths[i].assign(folderPath).append("\\").append(names[i]).append(".skel");
	}
	return m_sfmlSpinePlayer->loadSpineFromFile(atlasPaths, skelPaths, true);
}

void CSfmlMainWindow::setVoices(std::vector<std::string>& filePaths)
{
	m_audioFiles = std::move(filePaths);
	m_nAudioIndex = 0;
}
/*書体設定*/
bool CSfmlMainWindow::setFont(const std::string& filePath, bool bold, bool italic)
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
	static constexpr float fOutLineThickness = 2.4f;

	/* Audio track indicator */
	m_trackText.setFont(m_trackFont);
	m_trackText.setFillColor(sf::Color::Black);
	m_trackText.setStyle((bold ? sf::Text::Style::Bold : 0) | (italic ? sf::Text::Style::Italic : 0));
	m_trackText.setOutlineThickness(fOutLineThickness);
	m_trackText.setOutlineColor(sf::Color::White);

	return true;
}

int CSfmlMainWindow::display()
{
	resetSpinePlayerScale();

	sf::SoundBuffer soundBuffer;
	sf::Sound sound;
	if (!m_audioFiles.empty())
	{
		soundBuffer.loadFromFile(m_audioFiles[0]);
		sound.setBuffer(soundBuffer);
		sound.setVolume(50.f);
		sound.play();
	}

	/*修正が面倒なのでここで記述*/
	const auto UpdateTrackIndicator = [this]()
		-> void
		{
			if (m_nAudioIndex >= m_audioFiles.size())
			{
				m_trackText.setString("");
				return;
			}

			std::string str = std::to_string(m_nAudioIndex + 1).append("/").append(std::to_string(m_audioFiles.size()));
			m_trackText.setString(str);
		};

	const auto StepOnTrack = [this, &soundBuffer, &sound, &UpdateTrackIndicator](bool bForward)
		-> void
		{
			if (!m_audioFiles.empty())
			{
				if (bForward)
				{
					++m_nAudioIndex;
					if (m_nAudioIndex >= m_audioFiles.size())m_nAudioIndex = 0;
				}
				else
				{
					--m_nAudioIndex;
					if (m_nAudioIndex >= m_audioFiles.size())m_nAudioIndex = m_audioFiles.size() - 1;
				}
				soundBuffer.loadFromFile(m_audioFiles.at(m_nAudioIndex));
				sound.setBuffer(soundBuffer);
				sound.play();

				UpdateTrackIndicator();
			}
		};

	UpdateTrackIndicator();

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
					m_mouseState.lastMousePos.x = event.mouseButton.x;
					m_mouseState.lastMousePos.y = event.mouseButton.y;

					m_mouseState.wasLeftPressed = true;
				}
				break;
			case sf::Event::MouseButtonReleased:
				if (event.mouseButton.button == sf::Mouse::Left)
				{
					if (m_mouseState.wasLeftCombined || m_mouseState.wasLeftDragged)
					{
						m_mouseState.wasLeftCombined = false;
						m_mouseState.wasLeftPressed = false;
						m_mouseState.wasLeftDragged = false;

						break;
					}

					if (m_windowState.toBeMoved || sf::Mouse::isButtonPressed(sf::Mouse::Right))
					{
						m_windowState.toBeMoved ^= true;
						break;
					}

					const int iX = m_mouseState.lastMousePos.x - event.mouseButton.x;
					const int iY = m_mouseState.lastMousePos.y - event.mouseButton.y;

					if (m_mouseState.wasLeftPressed && iX == 0 && iY == 0)
					{
						m_sfmlSpinePlayer->shiftAnimation();
					}

					m_mouseState.wasLeftPressed = false;
				}
				if (event.mouseButton.button == sf::Mouse::Middle)
				{
					resetSpinePlayerScale();
				}
				break;
			case sf::Event::MouseMoved:
				if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
				{
					if (!m_mouseState.wasLeftCombined)
					{
						if (m_mouseState.wasLeftDragged)
						{
							int iX = m_mouseState.lastMousePos.x - event.mouseMove.x;
							int iY = m_mouseState.lastMousePos.y - event.mouseMove.y;
							m_sfmlSpinePlayer->addOffset(iX, iY);
						}

						m_mouseState.lastMousePos.x = event.mouseMove.x;
						m_mouseState.lastMousePos.y = event.mouseMove.y;

						m_mouseState.wasLeftDragged = true;
					}
				}
				break;
			case sf::Event::MouseWheelScrolled:
				if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
				{
					static constexpr float kTimeScaleDelta = 0.05f;
					const float scrollSign = event.mouseWheelScroll.delta < 0 ? 1.f : -1.f;

					float timeScale = m_sfmlSpinePlayer->getTimeScale() + kTimeScaleDelta * scrollSign;
					timeScale = (std::max)(timeScale, 0.f);
					m_sfmlSpinePlayer->setTimeScale(timeScale);

					m_mouseState.wasLeftCombined = true;
				}
				else if (sf::Mouse::isButtonPressed(sf::Mouse::Right))
				{
					/*音声送り・戻し*/
					StepOnTrack(event.mouseWheelScroll.delta < 0);
				}
				else
				{
					static constexpr float kMinScale = 0.15f;
					const float scrollSign = event.mouseWheelScroll.delta < 0 ? 1.f : -1.f;

					float skeletonScale = m_sfmlSpinePlayer->getSkeletonScale() + kScaleDelta * scrollSign;
					skeletonScale = (std::max)(kMinScale, skeletonScale);
					m_sfmlSpinePlayer->setSkeletonScale(skeletonScale);

					if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl))
					{
						float canvasScale = m_sfmlSpinePlayer->getCanvasScale() + kScaleDelta * scrollSign;
						canvasScale = (std::max)(kMinScale, canvasScale);
						m_sfmlSpinePlayer->setCanvasScale(canvasScale);

						resizeWindow();
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
					m_sfmlSpinePlayer->togglePma();
					break;
				case sf::Keyboard::Key::B:
					m_sfmlSpinePlayer->toggleBlendModeAdoption();
					break;
				case sf::Keyboard::Key::C:
					toggleTextColour();
					break;
				case sf::Keyboard::Key::F:
					if (!m_windowState.toBeMoved)
					{
						m_windowStyle.toFitToMonitorHeight ^= true;
						alignWindowToTheTopLeftOfMonitor();
						resetSpinePlayerScale();
					}
					break;
				case sf::Keyboard::Key::T:
					toggleTextVisibility();
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
		m_sfmlSpinePlayer->update(fDelta);
		m_spineClock.restart();

		m_window->clear(sf::Color(0, 0, 0, 0));

		m_sfmlSpinePlayer->redraw(m_window.get());
		if (!m_audioState.isTrackHidden)
		{
			m_window->draw(m_trackText);
		}

		m_window->display();

		if (!m_audioFiles.empty())
		{
			if (sound.getStatus() == sf::SoundSource::Stopped)
			{
				if (m_nAudioIndex < m_audioFiles.size() - 1)
				{
					StepOnTrack(true);
				}
			}
		}

		if (m_windowState.toBeMoved)
		{
			int iPosX = sf::Mouse::getPosition().x - m_window->getSize().x / 2;
			int iPosY = sf::Mouse::getPosition().y - m_window->getSize().y / 2;
			m_window->setPosition(sf::Vector2i(iPosX, iPosY));
		}
	}

	return 0;
}

void CSfmlMainWindow::resizeWindow()
{
	if (m_sfmlSpinePlayer.get() != nullptr)
	{
		const sf::Vector2f fBaseSize = m_sfmlSpinePlayer->getBaseSize();
		const float fScale = m_sfmlSpinePlayer->getCanvasScale();

		const unsigned int maxWindowWidth = static_cast<unsigned int>(fBaseSize.x * (fScale - kScaleDelta));
		const unsigned int maxWindowHeight = static_cast<unsigned int>(fBaseSize.y * (fScale - kScaleDelta));

		const sf::Vector2u monitorSize = getMonitorInfo().size;

		if (maxWindowWidth < monitorSize.x || maxWindowHeight < monitorSize.y)
		{
			const unsigned int windowWidth = static_cast<unsigned int>(fBaseSize.x * fScale);
			const unsigned int windowHeight = static_cast<unsigned int>(fBaseSize.y * fScale);

			m_window->setSize(sf::Vector2u(windowWidth, windowHeight));
			m_window->setView(sf::View((fBaseSize * fScale) / 2.f, fBaseSize * fScale));
		}
	}
}

void CSfmlMainWindow::alignWindowToTheTopLeftOfMonitor()
{
	m_window->setPosition(getMonitorInfo().position);
}

CSfmlMainWindow::MonitorInfo CSfmlMainWindow::getMonitorInfo()
{
	/* SFML provides information only on primary monitor. */
#if !defined _WIN32
	const sf::VideoMode monitorSize = sf::VideoMode::getDesktopMode();
	return { {}, {monitorSize.width, monitorSize.height} };
#else
	MONITORINFO monitorInfo{ sizeof(MONITORINFO) };
	HMONITOR hMonitor = ::MonitorFromWindow(m_window->getSystemHandle(), MONITOR_DEFAULTTONEAREST);
	if (hMonitor != nullptr)
	{
		::GetMonitorInfoW(hMonitor, &monitorInfo);
	}

	const int x = static_cast<int>(monitorInfo.rcMonitor.left);
	const int y = static_cast<int>(monitorInfo.rcMonitor.top);

	const unsigned int width = static_cast<unsigned int>(monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left);
	const unsigned int height = static_cast<unsigned int>(monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top);

	return { {x, y}, {width, height} };
#endif
}

void CSfmlMainWindow::resetSpinePlayerScale()
{
	m_sfmlSpinePlayer->resetScale();

	const sf::Vector2f fBaseSize = m_sfmlSpinePlayer->getBaseSize();
	const float skeletonScale = m_sfmlSpinePlayer->getSkeletonScale();

	const sf::Vector2u monitorSize = getMonitorInfo().size;
	const bool isLandscape = monitorSize.x > monitorSize.y;
	const bool toFitToWidth = isLandscape ^ m_windowStyle.toFitToMonitorHeight;
	const float scale = (toFitToWidth ? monitorSize.x : monitorSize.y) / (toFitToWidth ? fBaseSize.x : fBaseSize.y);

	m_sfmlSpinePlayer->setSkeletonScale(scale + 0.05f);
	m_sfmlSpinePlayer->setCanvasScale(scale);

	resizeWindow();
}

void CSfmlMainWindow::toggleTextColour()
{
	m_trackText.setFillColor(m_trackText.getFillColor() == sf::Color::Black ? sf::Color::White : sf::Color::Black);
	m_trackText.setOutlineColor(m_trackText.getFillColor() == sf::Color::Black ? sf::Color::White : sf::Color::Black);
}

void CSfmlMainWindow::toggleTextVisibility()
{
	m_audioState.isTrackHidden ^= true;
}
