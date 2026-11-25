#pragma once

#include <cstdint>

#include "HAQuDA_DisplayShown.h"
#include "HAQuDA_WebPageDisplayManip_Translator.h"

// The WiFi lost picture has no error word and arrives as a red static effect, as the page expects it
void HAQuDA_WebPictureBuild(const display_shown &shown, uint8_t brightPercent, uint32_t nowMs, web_display_state &out);
