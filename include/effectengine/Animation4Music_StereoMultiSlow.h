#pragma once

#include <effectengine/AnimationBaseMusic.h>

#define AMUSIC_STEREOMULTISLOW "Music: stereo for LED strip (MULTI COLOR SLOW)"

class Animation4Music_StereoMultiSlow : public AnimationBaseMusic
{
public:

	Animation4Music_StereoMultiSlow();

	void Init(
		QImage& ambilightImage,
		int ambilightLatchTime) override;

	bool Play(QPainter* painter) override;

	static EffectDefinition getDefinition();

	bool hasOwnImage() override;
	bool getImage(Image<ColorRgb>& image) override;
private:
	uint32_t _internalIndex;
	int		 _oldMulti;
};
