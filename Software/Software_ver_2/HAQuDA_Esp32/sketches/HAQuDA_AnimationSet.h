#pragma once

#include "HAQuDA_EffectAnimations.h"
#include "HAQuDA_MeasAnimations.h"

class HAQuDA_AnimationSet {
  public:
	explicit HAQuDA_AnimationSet(HAQuDA_ILEDs &leds);

	HAQuDA_Animation *Get(anim_id id);
	HAQuDA_RandomPixelsAnim &RandomPixels();

  private:
	HAQuDA_SnakeAnim snake;
	HAQuDA_GrowingAnim growing;
	HAQuDA_RandomPixelsAnim randomPixels;
	HAQuDA_FadingAnim fading;
	HAQuDA_StaticColorAnim staticColor;
	HAQuDA_MeasStandardAnim measStandard;
	HAQuDA_MeasMultiAnim measMulti;
	HAQuDA_MeasNightAnim measNight;
	HAQuDA_MeasTotalAnim measTotal;
};
