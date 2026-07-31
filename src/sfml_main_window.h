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
private:
	static constexpr float kScaleDelta = 0.025f;

	std::unique_ptr<sf::RenderWindow> m_window;

	std::unique_ptr<CSfmlSpinePlayer> m_sfmlSpinePlayer;
	sf::Clock m_spineClock;

	void resizeWindow();
	void resetScale();

	std::vector<std::string> m_audio_files;
	size_t m_nAudioIndex = 0;

	sf::Font m_trackFont;
	sf::Text m_trackText;
	bool m_bTrackHidden = false;

	void toggleTextColour();
	void toggleTextVisibility();
};

#endif // !SFML_MAIN_WINDOW_H_
