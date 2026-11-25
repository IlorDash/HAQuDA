#pragma once

#include "Arduino.h"

#include <atomic>

#include "HAQuDA_DisplayShown.h"
#include "HAQuDA_WebPageDisplayManip_Translator.h"

class HAQuDA_WebPageDisplayManip {
  public:
	HAQuDA_WebPageDisplayManip();

	// ShowTask only
	void Publish(const display_shown &shown);

	// The page reports the user's value at once, not when ShowTask gets to it
	void SetBrightness(uint8_t brightPerCent);

	// Returns 0 when the JSON does not fit WEB_DISPLAY_JSON_BUF_LEN bytes. async_tcp task only.
	size_t BuildJson(char *buf, size_t size);

  private:
	portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
	display_shown picture;
	std::atomic<uint8_t> brightPercent{0};
};
