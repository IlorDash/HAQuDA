#include "HAQuDA_AnimationSet.h"

HAQuDA_AnimationSet::HAQuDA_AnimationSet(HAQuDA_ILEDs &leds)
	: snake(leds), growing(leds), randomPixels(leds), fading(leds), staticColor(leds), measStandard(leds), measMulti(leds), measNight(leds), measTotal(leds) {
}

HAQuDA_Animation *HAQuDA_AnimationSet::Get(anim_id id) {
	switch (id) {
		case anim_id::snake:
			return &snake;
		case anim_id::growing:
			return &growing;
		case anim_id::randomPixels:
			return &randomPixels;
		case anim_id::fading:
			return &fading;
		case anim_id::staticColor:
			return &staticColor;
		case anim_id::measStandard:
			return &measStandard;
		case anim_id::measMulti:
			return &measMulti;
		case anim_id::measNight:
			return &measNight;
		case anim_id::measTotal:
			return &measTotal;
		default:
			return nullptr;
	}
}

HAQuDA_RandomPixelsAnim &HAQuDA_AnimationSet::RandomPixels() {
	return randomPixels;
}
