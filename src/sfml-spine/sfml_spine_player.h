#ifndef SFML_SPINE_PLAYER_H_
#define SFML_SPINE_PLAYER_H_

#include "spine_player.h"

class CSfmlSpinePlayer : public CSpinePlayer
{
public:
	CSfmlSpinePlayer();
	virtual ~CSfmlSpinePlayer();

	void redraw(sf::RenderTarget* pRenderTarget);

	sf::FloatRect getCurrentBoundingOfSlot(const char* slotName, size_t nameLength) const;
	template<size_t nameSize>
	sf::FloatRect getCurrentBoundingOfSlot(const char(&slotName)[nameSize]) const
	{
		return getCurrentBoundingOfSlot(slotName, nameSize - 1);
	}
private:
	void workOutDefaultScale() override;
	void workOutDefaultSizeAndOffset() override;
};

#endif // !SFML_SPINE_PLAYER_H_
