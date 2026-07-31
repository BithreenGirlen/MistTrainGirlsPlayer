

#include "sfml_spine.h"

namespace spine
{
	SpineExtension* getDefaultExtension()
	{
		return new DefaultSpineExtension();
	}
}

static sf::BlendMode g_sfmlBlendModeNormalPma = sf::BlendMode(sf::BlendMode::One, sf::BlendMode::OneMinusSrcAlpha);
static sf::BlendMode g_sfmlBlendModeAddPma = sf::BlendMode(sf::BlendMode::One, sf::BlendMode::One);
static sf::BlendMode g_sfmlBlendModeScreen = sf::BlendMode(sf::BlendMode::One, sf::BlendMode::OneMinusSrcColor);
static sf::BlendMode g_sfmlBlendModeMultiply = sf::BlendMode
(
	sf::BlendMode::Factor::DstColor,
	sf::BlendMode::Factor::OneMinusSrcAlpha,
	sf::BlendMode::Equation::Add,
	sf::BlendMode::Factor::One,
	sf::BlendMode::Factor::OneMinusSrcAlpha,
	sf::BlendMode::Equation::Add
);

CSfmlSpineDrawable::CSfmlSpineDrawable(spine::SkeletonData* pSkeletonData, spine::AnimationStateData* pAnimationStateData)
{
	spine::Bone::setYDown(true);

	/*sf::VertexArray seems not to have reserve-like method.*/
	m_sfmlVertices.setPrimitiveType(sf::PrimitiveType::Triangles);

	m_skeleton = new spine::Skeleton(pSkeletonData);

	if (pAnimationStateData == nullptr)
	{
		pAnimationStateData = new spine::AnimationStateData(pSkeletonData);
		m_hasOwnAnimationStateData = true;
	}

	m_animationState = new spine::AnimationState(pAnimationStateData);

	m_quadIndices.add(0);
	m_quadIndices.add(1);
	m_quadIndices.add(2);
	m_quadIndices.add(2);
	m_quadIndices.add(3);
	m_quadIndices.add(0);
}

CSfmlSpineDrawable::~CSfmlSpineDrawable()
{
	if (m_animationState != nullptr)
	{
		if (m_hasOwnAnimationStateData)
		{
			delete m_animationState->getData();
		}

		delete m_animationState;
	}
	if (m_skeleton != nullptr)
	{
		delete m_skeleton;
	}
}

spine::Skeleton* CSfmlSpineDrawable::skeleton() const noexcept
{
	return m_skeleton;
}

spine::AnimationState* CSfmlSpineDrawable::animationState() const noexcept
{
	return m_animationState;
}

void CSfmlSpineDrawable::premultiplyAlpha(bool premultiplied) noexcept
{
	m_isAlphaPremultiplied = premultiplied;
}

bool CSfmlSpineDrawable::isAlphaPremultiplied() const noexcept
{
	return m_isAlphaPremultiplied;
}

void CSfmlSpineDrawable::forceBlendModeNormal(bool toForce) noexcept
{
	m_isToForceBlendModeNormal = toForce;
}

bool CSfmlSpineDrawable::isBlendModeNormalForced() const noexcept
{
	return m_isToForceBlendModeNormal;
}

void CSfmlSpineDrawable::setPause(bool paused) noexcept
{
	m_isPaused = paused;
}

bool CSfmlSpineDrawable::isPaused() const noexcept
{
	return m_isPaused;
}

void CSfmlSpineDrawable::setVisibility(bool visible) noexcept
{
	m_isVisible = visible;
}

bool CSfmlSpineDrawable::isVisible() const noexcept
{
	return m_isVisible;
}

void CSfmlSpineDrawable::update(float fDelta)
{
	if (m_skeleton != nullptr && m_animationState != nullptr)
	{
		if (!m_isPaused)m_animationState->update(fDelta);
		m_animationState->apply(*m_skeleton);

		/* Spine 4.1 does not have "Skeleton::update()" */
#if !defined(SPINE_41)
		if (!m_isPaused)m_skeleton->update(fDelta);
#endif

#if defined(SPINE_42)
		m_skeleton->updateWorldTransform(spine::Physics::Physics_Update);
#else
		m_skeleton->updateWorldTransform();
#endif
	}
}

void CSfmlSpineDrawable::draw(sf::RenderTarget& renderTarget, sf::RenderStates renderStates) const
{
	if (!m_isVisible)return;
	if (m_skeleton == nullptr || m_animationState == nullptr)return;
	if (m_skeleton->getColor().a == 0) return;

	for (unsigned i = 0; i < m_skeleton->getSlots().size(); ++i)
	{
		spine::Slot& slot = *m_skeleton->getDrawOrder()[i];
		spine::Attachment* pAttachment = slot.getAttachment();

		if (pAttachment == nullptr || slot.getColor().a == 0 || !slot.getBone().isActive())
		{
			m_clipper.clipEnd(slot);
			continue;
		}

		if (isSlotToBeLeftOut(slot.getData().getName()))
		{
			m_clipper.clipEnd(slot);
			continue;
		}

		spine::Vector<float>* pVertices = &m_worldVertices;
		spine::Vector<float>* pAttachmentUvs = nullptr;
		spine::Vector<unsigned short>* pIndices = nullptr;

		spine::Color* pAttachmentColor = nullptr;

		sf::Texture* pSfmlTexture = nullptr;

		if (pAttachment->getRTTI().isExactly(spine::RegionAttachment::rtti))
		{
			spine::RegionAttachment* pRegionAttachment = (spine::RegionAttachment*)pAttachment;
			pAttachmentColor = &pRegionAttachment->getColor();

			if (pAttachmentColor->a == 0)
			{
				m_clipper.clipEnd(slot);
				continue;
			}

			m_worldVertices.setSize(8, 0);
#if defined (SPINE_41) || defined (SPINE_42)
			pRegionAttachment->computeWorldVertices(slot, m_worldVertices, 0, 2);
#else
			pRegionAttachment->computeWorldVertices(slot.getBone(), m_worldVertices, 0, 2);
#endif
			pAttachmentUvs = &pRegionAttachment->getUVs();
			pIndices = &m_quadIndices;

#if defined (SPINE_41) || defined (SPINE_42)
			spine::AtlasRegion* pAtlasRegion = static_cast<spine::AtlasRegion*>(pRegionAttachment->getRegion());

			m_isAlphaPremultiplied = pAtlasRegion->page->pma;
			pSfmlTexture = reinterpret_cast<sf::Texture*>(pAtlasRegion->rendererObject);
#else
			spine::AtlasRegion* pAtlasRegion = static_cast<spine::AtlasRegion*>(pRegionAttachment->getRendererObject());
#ifdef SPINE_40
			m_isAlphaPremultiplied = pAtlasRegion->page->pma;
#endif
			pSfmlTexture = reinterpret_cast<sf::Texture*>(pAtlasRegion->page->getRendererObject());
#endif
		}
		else if (pAttachment->getRTTI().isExactly(spine::MeshAttachment::rtti))
		{
			spine::MeshAttachment* pMeshAttachment = (spine::MeshAttachment*)pAttachment;
			pAttachmentColor = &pMeshAttachment->getColor();

			if (pAttachmentColor->a == 0)
			{
				m_clipper.clipEnd(slot);
				continue;
			}

			m_worldVertices.setSize(pMeshAttachment->getWorldVerticesLength(), 0);
			pMeshAttachment->computeWorldVertices(slot, 0, pMeshAttachment->getWorldVerticesLength(), m_worldVertices, 0, 2);

			pAttachmentUvs = &pMeshAttachment->getUVs();
			pIndices = &pMeshAttachment->getTriangles();

#if defined (SPINE_41) || defined (SPINE_42)
			spine::AtlasRegion* pAtlasRegion = static_cast<spine::AtlasRegion*>(pMeshAttachment->getRegion());

			m_isAlphaPremultiplied = pAtlasRegion->page->pma;
			pSfmlTexture = reinterpret_cast<sf::Texture*>(pAtlasRegion->rendererObject);
#else
			spine::AtlasRegion* pAtlasRegion = static_cast<spine::AtlasRegion*>(pMeshAttachment->getRendererObject());
#ifdef SPINE_40
			m_isAlphaPremultiplied = pAtlasRegion->page->pma;
#endif
			pSfmlTexture = reinterpret_cast<sf::Texture*>(pAtlasRegion->page->getRendererObject());
#endif
		}
		else if (pAttachment->getRTTI().isExactly(spine::ClippingAttachment::rtti))
		{
			spine::ClippingAttachment* clip = (spine::ClippingAttachment*)slot.getAttachment();
			m_clipper.clipStart(slot, clip);
			continue;
		}
		else continue;

		if (m_clipper.isClipping())
		{
			m_clipper.clipTriangles(m_worldVertices, *pIndices, *pAttachmentUvs, 2);
			pVertices = &m_clipper.getClippedVertices();
			pAttachmentUvs = &m_clipper.getClippedUVs();
			pIndices = &m_clipper.getClippedTriangles();
		}

		const spine::Color& skeletonColor = m_skeleton->getColor();
		const spine::Color& slotColor = slot.getColor();
		const spine::Color tint
		(
			skeletonColor.r * slotColor.r * pAttachmentColor->r,
			skeletonColor.g * slotColor.g * pAttachmentColor->g,
			skeletonColor.b * slotColor.b * pAttachmentColor->b,
			skeletonColor.a * slotColor.a * pAttachmentColor->a
		);
		const sf::Vector2u& textureSize = pSfmlTexture->getSize();

		m_sfmlVertices.clear();
		/*
		* The two tasks are required because SFML does not support indexed drawing.
		* 1. Map index to vertex when adding.
		* 2. Multiply alpha to colours if necessary.
		*/
		for (int ii = 0; ii < pIndices->size(); ++ii)
		{
			sf::Vertex sfmlVertex;

			sfmlVertex.position.x = (*pVertices)[(*pIndices)[ii] * 2LL];
			sfmlVertex.position.y = (*pVertices)[(*pIndices)[ii] * 2LL + 1];

			sfmlVertex.color.r = (sf::Uint8)(tint.r * 255.f * (m_isAlphaPremultiplied ? tint.a : 1.f));
			sfmlVertex.color.g = (sf::Uint8)(tint.g * 255.f * (m_isAlphaPremultiplied ? tint.a : 1.f));
			sfmlVertex.color.b = (sf::Uint8)(tint.b * 255.f * (m_isAlphaPremultiplied ? tint.a : 1.f));
			sfmlVertex.color.a = (sf::Uint8)(tint.a * 255.f);

			sfmlVertex.texCoords.x = (*pAttachmentUvs)[(*pIndices)[ii] * 2LL] * textureSize.x;
			sfmlVertex.texCoords.y = (*pAttachmentUvs)[(*pIndices)[ii] * 2LL + 1] * textureSize.y;

			m_sfmlVertices.append(sfmlVertex);
		}

		sf::BlendMode sfmlBlendMode;
		/*
		* Some slots specify BlendMode_Multiply though, BlendMode_Normal seems better;
		* But in order not to overwrite the slot data, copy to temporary variable when forcing.
		*/
		spine::BlendMode spineBlnedMode = m_isToForceBlendModeNormal ? spine::BlendMode_Normal : slot.getData().getBlendMode();
		switch (spineBlnedMode)
		{
		case spine::BlendMode_Additive:
			sfmlBlendMode = m_isAlphaPremultiplied ? g_sfmlBlendModeAddPma : sf::BlendAdd;
			break;
		case spine::BlendMode_Multiply:
			sfmlBlendMode = g_sfmlBlendModeMultiply;
			break;
		case spine::BlendMode_Screen:
			sfmlBlendMode = g_sfmlBlendModeScreen;
			break;
		default:
			sfmlBlendMode = m_isAlphaPremultiplied ? g_sfmlBlendModeNormalPma : sf::BlendAlpha;
			break;
		}

		renderStates.blendMode = sfmlBlendMode;
		renderStates.texture = pSfmlTexture;
		renderTarget.draw(m_sfmlVertices, renderStates);
		m_clipper.clipEnd(slot);
	}
	m_clipper.clipEnd();
}

void CSfmlSpineDrawable::setLeaveOutList(spine::Vector<spine::String>& list)
{
	/*There are some slots having mask or nuisance effect; exclude them from rendering.*/
	m_leaveOutList.clearAndAddAll(list);
}

sf::FloatRect CSfmlSpineDrawable::getBoundingBox() const
{
	sf::FloatRect boundingBox{};

	if (m_skeleton != nullptr)
	{
		spine::Vector<float> tempVertices;
		m_skeleton->getBounds(boundingBox.left, boundingBox.top, boundingBox.width, boundingBox.height, tempVertices);
	}

	return boundingBox;
}

sf::FloatRect CSfmlSpineDrawable::getBoundingBoxOfSlot(const char* slotName, size_t nameLength, bool* found) const
{
	float fMinX = FLT_MAX;
	float fMinY = FLT_MAX;
	float fMaxX = -FLT_MAX;
	float fMaxY = -FLT_MAX;

	if (m_skeleton != nullptr)
	{
		for (size_t i = 0; i < m_skeleton->getSlots().size(); ++i)
		{
			spine::Slot& slot = *m_skeleton->getDrawOrder()[i];
			const spine::String& slotDataName = slot.getData().getName();
			if (nameLength != slotDataName.length())continue;

			if (::memcmp(slotDataName.buffer(), slotName, slotDataName.length()) == 0)
			{
				spine::Attachment* pAttachment = slot.getAttachment();
				if (pAttachment != nullptr)
				{
					spine::Vector<float> tempVertices;
					if (pAttachment->getRTTI().isExactly(spine::RegionAttachment::rtti))
					{
						spine::RegionAttachment* pRegionAttachment = static_cast<spine::RegionAttachment*>(pAttachment);

						tempVertices.setSize(8, 0);
#if defined (SPINE_41) || defined (SPINE_42)
						pRegionAttachment->computeWorldVertices(slot, tempVertices, 0, 2);
#else
						pRegionAttachment->computeWorldVertices(slot.getBone(), tempVertices, 0, 2);
#endif
					}
					else if (pAttachment->getRTTI().isExactly(spine::MeshAttachment::rtti))
					{
						spine::MeshAttachment* pMeshAttachment = static_cast<spine::MeshAttachment*>(pAttachment);
						tempVertices.setSize(pMeshAttachment->getWorldVerticesLength(), 0);
						pMeshAttachment->computeWorldVertices(slot, 0, pMeshAttachment->getWorldVerticesLength(), tempVertices, 0, 2);
					}
					else
					{
						continue;
					}

					for (size_t ii = 0; ii < tempVertices.size(); ii += 2)
					{
						float fX = tempVertices[ii];
						float fY = tempVertices[ii + 1LL];

						fMinX = fMinX < fX ? fMinX : fX;
						fMinY = fMinY < fY ? fMinY : fY;
						fMaxX = fMaxX > fX ? fMaxX : fX;
						fMaxY = fMaxY > fY ? fMaxY : fY;
					}

					if (found != nullptr)*found = true;
					break;
				}
			}
		}
	}

	return sf::FloatRect{ fMinX, fMinY, fMaxX - fMinX, fMaxY - fMinY };
}

bool CSfmlSpineDrawable::isSlotToBeLeftOut(const spine::String& slotName) const
{
	/*The comparison method depends on what should be excluded; the precise matching or partial one.*/
	if (m_pLeaveOutCallback != nullptr)
	{
		return m_pLeaveOutCallback(slotName.buffer(), slotName.length());
	}
	else
	{
		return m_leaveOutList.contains(slotName);
	}

	return false;
}

void CSfmlTextureLoader::load(spine::AtlasPage& atlasPage, const spine::String& textureFilePath)
{
	sf::Image sfImage;
	if (!sfImage.loadFromFile(textureFilePath.buffer()))
	{
		/* SFML does not have logger. */
		return;
	}

#if defined(SPINE_40) || defined(SPINE_41) || defined (SPINE_42)
	bool toConvertToPma = !atlasPage.pma && !m_wasAlreadyConvertedToPma;
	if (toConvertToPma)
	{
		atlasPage.pma = true;
	}
#else
	bool toConvertToPma = m_toConvertToPma;
#endif
	if (toConvertToPma)
	{
		sf::Vector2u imageDimention = sfImage.getSize();
		for (unsigned int y = 0; y < imageDimention.y; ++y)
		{
			for (unsigned int x = 0; x < imageDimention.x; ++x)
			{
				sf::Color pixel = sfImage.getPixel(x, y);

				const float alphaFactor = static_cast<const float>(pixel.a / 255.f);
				pixel.r = static_cast<uint8_t>(pixel.r * alphaFactor);
				pixel.g = static_cast<uint8_t>(pixel.g * alphaFactor);
				pixel.b = static_cast<uint8_t>(pixel.b * alphaFactor);

				sfImage.setPixel(x, y, pixel);
			}
		}
	}

	sf::Texture* pTexture = new (std::nothrow) sf::Texture();
	if (pTexture == nullptr)
	{
		return;
	}
	if (!pTexture->loadFromImage(sfImage))
	{
		delete pTexture;
		return;
	}

	if (atlasPage.magFilter == spine::TextureFilter_Linear)
	{
		pTexture->setSmooth(true);
	}
	if (atlasPage.uWrap == spine::TextureWrap_Repeat && atlasPage.vWrap == spine::TextureWrap_Repeat)
	{
		pTexture->setRepeated(true);
	}
	pTexture->generateMipmap();

	/* This will collapse UVs. */
#if 0
	if (atlasPage.width == 0 || atlasPage.height == 0)
	{
		sf::Vector2u size = pTexture->getSize();
		atlasPage.width = size.x;
		atlasPage.height = size.y;
	}
#endif

#if defined (SPINE_41) || defined (SPINE_42)
	atlasPage.texture = pTexture;
#else
	atlasPage.setRendererObject(pTexture);
#endif
}

void CSfmlTextureLoader::unload(void* texture)
{
	delete static_cast<sf::Texture*>(texture);
}

void CSfmlTextureLoader::enableConversionToPma(bool toEnable)
{
#if defined(SPINE_40) || defined(SPINE_41) || defined (SPINE_42)
	m_wasAlreadyConvertedToPma = !toEnable;
#else
	m_toConvertToPma = toEnable;
#endif
}

bool CSfmlTextureLoader::isConversionToPmaEnabled() const noexcept
{
#if defined(SPINE_40) || defined(SPINE_41) || defined (SPINE_42)
	return !m_wasAlreadyConvertedToPma;
#else
	return m_toConvertToPma;
#endif
}
