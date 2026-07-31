#ifndef SFML_MAIN_WINDOW_H_
#define SFML_MAIN_WINDOW_H_

#include <memory>

#include "sfml-spine/sfml_spine_player.h"

class CSfmlMainWindow
{
public:
	CSfmlMainWindow(const wchar_t* windowName = nullptr);
	~CSfmlMainWindow();

	bool setSpines(const std::string& folderPath, const std::vector<std::string>& names);
	void setVoices(std::vector<std::string>& filePaths);
	bool setFont(const std::string& filePath, bool bold = true, bool italic = true);

	int display();

	sf::RenderWindow* getWindow() const { return m_window.get(); }
	CSfmlSpinePlayer* getSpinePlayer() const { return m_sfmlSpinePlayer.get(); }
private:
	static constexpr float kScaleDelta = 0.025f;

	struct MonitorInfo
	{
		sf::Vector2i position;
		sf::Vector2u size;
	};

	struct MouseState
	{
		bool wasLeftPressed = false;
		bool wasLeftCombined = false;
		bool wasLeftDragged = false;

		bool wasRightCombined = false;

		sf::Vector2i lastMousePos{};
	};

	struct WindowState
	{
		bool toBeMoved = false;
	};

	struct WindowStyle
	{
		bool toFitToMonitorHeight = false;
	};

	struct AudioState
	{
		bool isTrackHidden = false;
	};

	std::unique_ptr<sf::RenderWindow> m_window;

	MouseState m_mouseState;
	WindowState m_windowState;
	WindowStyle m_windowStyle;
	AudioState m_audioState;

	std::unique_ptr<CSfmlSpinePlayer> m_sfmlSpinePlayer;
	sf::Clock m_spineClock;

	void resizeWindow();
	void alignWindowToTheTopLeftOfMonitor();
	MonitorInfo getMonitorInfo();

	void resetSpinePlayerScale();

	std::vector<std::string> m_audioFiles;
	size_t m_nAudioIndex = 0;

	sf::Font m_trackFont;
	sf::Text m_trackText;

	void toggleTextColour();
	void toggleTextVisibility();
};

#endif // !SFML_MAIN_WINDOW_H_
