#pragma once

#include <effectengine/AnimationBaseMusic.h>

#define AMUSIC_WAVESPULSE "Music: pulse waves for LED strip (MULTI COLOR)"

class Animation4Music_WavesPulse : public AnimationBaseMusic
{
public:

	Animation4Music_WavesPulse();

	void Init(
		QImage& ambilightImage,
		int ambilightLatchTime) override;

	bool Play(QPainter* painter) override;

	static EffectDefinition getDefinition();

	bool hasOwnImage() override;
	bool getImage(Image<ColorRgb>& image) override;
private:
	uint32_t _internalIndex;
	int 	 _oldMulti;
	QList<QColor> _buffer;
};
