

#include "sfml_spine_player.h"

CSfmlSpinePlayer::CSfmlSpinePlayer()
{

}

CSfmlSpinePlayer::~CSfmlSpinePlayer()
{

}


void CSfmlSpinePlayer::redraw(sf::RenderTarget* pRenderTarget)
{
	if (pRenderTarget != nullptr)
	{
		const auto& targetSize = pRenderTarget->getSize();
		float fX = (m_fBaseSize.x * m_fSkeletonScale - targetSize.x) / 2;
		float fY = (m_fBaseSize.y * m_fSkeletonScale - targetSize.y) / 2;

		sf::RenderStates renderState;
		renderState.blendMode = sf::BlendAlpha;
		renderState.transform.translate(-fX, -fY).scale(m_fSkeletonScale, m_fSkeletonScale);

		if (!m_isDrawOrderReversed)
		{
			for (size_t i = 0; i < m_drawables.size(); ++i)
			{
				pRenderTarget->draw(*m_drawables[i], renderState);
			}
		}
		else
		{
			for(long long i = m_drawables.size() - 1;i >= 0;--i)
			{
				pRenderTarget->draw(*m_drawables[i], renderState);
			}
		}
	}
}

sf::FloatRect CSfmlSpinePlayer::getCurrentBoundingOfSlot(const char* slotName, size_t nameLength) const
{
	bool found = false;
	for (const auto& drawable : m_drawables)
	{
		const auto& rect = drawable->getBoundingBoxOfSlot(slotName, nameLength, &found);
		if (found)
		{
			return rect;
		}
	}

	return {};
}
/*標準尺度算出*/
void CSfmlSpinePlayer::workOutDefaultScale()
{
	m_fDefaultScale = 1.f;

	unsigned int uiSkeletonWidth = static_cast<unsigned int>(m_fBaseSize.x);
	unsigned int uiSkeletonHeight = static_cast<unsigned int>(m_fBaseSize.y);

	unsigned int uiDesktopWidth = sf::VideoMode::getDesktopMode().width;
	unsigned int uiDesktopHeight = sf::VideoMode::getDesktopMode().height;

	if (uiSkeletonWidth > uiDesktopWidth || uiSkeletonHeight > uiDesktopHeight)
	{
		float fScaleX = static_cast<float>(uiDesktopWidth) / uiSkeletonWidth;
		float fScaleY = static_cast<float>(uiDesktopHeight) / uiSkeletonHeight;

		m_fDefaultScale = fScaleX > fScaleY ? fScaleY : fScaleX;
	}
}

void CSfmlSpinePlayer::workOutDefaultSizeAndOffset()
{
	static constexpr float kMaxDimension = 16384.f;

	float fMinX = FLT_MAX;
	float fMinY = FLT_MAX;
	float fWidth = -FLT_MAX;
	float fHeight = -FLT_MAX;

	for (const auto& pDrawable : m_drawables)
	{
		const auto& rect = pDrawable->getBoundingBox();
		fMinX = (std::min)(fMinX, rect.left);
		fMinY = (std::min)(fMinY, rect.top);

		if (::isless(rect.width, kMaxDimension) && ::isless(rect.height, kMaxDimension))
		{
			fWidth = (std::max)(fWidth, rect.width);
			fHeight = (std::max)(fHeight, rect.height);
		}
	}

	if (::isless(fMinX, FLT_MAX) && ::isless(fMinY, FLT_MAX))
	{
		m_fDefaultOffset.x = fMinX;
		m_fDefaultOffset.y = fMinY;
	}

	if (::isgreater(fWidth, -FLT_MAX) && ::isgreater(fHeight, -FLT_MAX))
	{
		m_fBaseSize.x = fWidth;
		m_fBaseSize.y = fHeight;
	}
}
