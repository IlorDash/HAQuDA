#pragma once

#include "HAQuDA_EffectAnimations.h"
#include "HAQuDA_MeasAnimations.h"

// One instance of every animation over one LED seam: the lookup the display service uses to turn an anim_id into the object that
// draws it. Adding an animation = a class, an anim_id value, a member and a case in Get().
class HAQuDA_AnimationSet {
  public:
	explicit HAQuDA_AnimationSet(HAQuDA_ILEDs &leds);

	// The animation for id, nullptr for anim_id::none or an invalid value. ShowTask only.
	HAQuDA_Animation *Get(anim_id id);
	// The random pixels animation, for its seed. ShowTask only.
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
