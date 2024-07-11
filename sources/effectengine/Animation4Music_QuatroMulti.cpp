/* Animation4Music_QuatroMulti.cpp
*
*  MIT License
*
*  Copyright (c) 2020-2023 awawa-dev
*
*  Project homesite: http://ambilightled.com
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  The above copyright notice and this permission notice shall be included in all
*  copies or substantial portions of the Software.

*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
*  SOFTWARE.
 */

#include <effectengine/Animation4Music_QuatroMulti.h>
#include <base/SoundCapture.h>

Animation4Music_QuatroMulti::Animation4Music_QuatroMulti() :
	AnimationBaseMusic(AMUSIC_QUATROMULTI),
	_internalIndex(0),
	_oldMulti(0)
{

};

EffectDefinition Animation4Music_QuatroMulti::getDefinition()
{
	EffectDefinition ed(true, EffectFactory<Animation4Music_QuatroMulti>);
	ed.name = AMUSIC_QUATROMULTI;
	return ed;
}

void Animation4Music_QuatroMulti::Init(
	QImage& ambilightImage,
	int ambilightLatchTime
)
{
	SetSleepTime(15);
}

bool Animation4Music_QuatroMulti::Play(QPainter* painter)
{
	return false;
}

bool Animation4Music_QuatroMulti::hasOwnImage()
{
	return true;
};

bool Animation4Music_QuatroMulti::getImage(Image<ColorRgb>& newImage)
{
	bool newData = false;
	auto r = _soundCapture->hasResult(this, _internalIndex, &newData, NULL, NULL, &_oldMulti);

	if (r == nullptr || !newData)
		return false;

	QColor selected;
	uint32_t maxSingle, average;

	newImage.clear();

	if (!r->GetStats(average, maxSingle, selected))
		return false;

	r->RestoreFullLum(selected);

	int value = r->getValue3Step(_oldMulti);

	{
		int hm = (newImage.height() / 2);
		int h = std::min((hm * value) / 255, 255);
		int h1 = std::max(hm - h, 0);
		int h2 = std::min(hm + h, (int)newImage.height() - 1);

		if (h2 > h1)
		{
			int width = newImage.width() * 0.04;

			newImage.gradientVBox(0, h1, width, h2, selected.red(), selected.green(), selected.blue());
			newImage.gradientVBox(newImage.width() - 1 - width, h1, newImage.width() - 1, h2, selected.red(), selected.green(), selected.blue());
		}
	}
	{
		int wm = (newImage.width() / 2);
		int w = std::min((wm * value) / 255, 255);
		int w1 = std::max(wm - w, 0);
		int w2 = std::min(wm + w, (int)newImage.width() - 1);

		if (w2 > w1)
		{
			int height = newImage.height() * 0.067;

			newImage.gradientHBox(w1, 0, w2, height, selected.red(), selected.green(), selected.blue());
			newImage.gradientHBox(w1, newImage.height() - 1 - height, w2, newImage.height() - 1, selected.red(), selected.green(), selected.blue());
		}
	}

	return true;
};







