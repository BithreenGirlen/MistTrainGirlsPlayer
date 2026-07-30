#ifndef SFML_MAIN_WINDOW_H_
#define SFML_MAIN_WINDOW_H_

#include <memory>

#include "sfml-spine/sfml_spine_player.h"

class CSfmlMainWindow
{
public:
	CSfmlMainWindow(const wchar_t* swzWindowName = nullptr);
	~CSfmlMainWindow();

	bool SetSpines(const std::string& folderPath, const std::vector<std::string>& names);
	void SetVoices(std::vector<std::string>& filePaths);
	bool SetFont(const std::string& filePath, bool bBold = true, bool bItalic = true);

	int Display();

	sf::RenderWindow* GetWindow() const { return m_window.get(); }
private:
	std::unique_ptr<sf::RenderWindow> m_window;

	std::unique_ptr<CSfmlSpinePlayer> m_sfmlSpinePlayer;
	sf::Clock m_spineClock;

	void ResizeWindow();
	void ResetScale();

	std::vector<std::string> m_audio_files;
	size_t m_nAudioIndex = 0;

	sf::Font m_trackFont;
	sf::Text m_trackText;
	bool m_bTrackHidden = false;

	void ToggleTextColour();
	void ToggleTextVisibility();
};

#endif // !SFML_MAIN_WINDOW_H_
