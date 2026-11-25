#include "HAQuDA_WebPageDisplayManip.h"
#include "HAQuDA_WebPageDisplayState.h"

namespace {
	class CriticalLock {
	  public:
		explicit CriticalLock(portMUX_TYPE &muxToLock) : mux(muxToLock) {
			portENTER_CRITICAL(&mux);
		}
		~CriticalLock() {
			portEXIT_CRITICAL(&mux);
		}
		CriticalLock(const CriticalLock &) = delete;
		CriticalLock &operator=(const CriticalLock &) = delete;

	  private:
		portMUX_TYPE &mux;
	};
} // namespace

HAQuDA_WebPageDisplayManip::HAQuDA_WebPageDisplayManip() : picture{} {
	picture.item = DISPLAY_ITEM_NONE;
	picture.anim = HAQuDA_MeasParams(anim_id::none);
}

void HAQuDA_WebPageDisplayManip::Publish(const display_shown &shown) {
	CriticalLock lock(mux);
	picture = shown;
}

void HAQuDA_WebPageDisplayManip::SetBrightness(uint8_t brightPerCent) {
	brightPercent.store(brightPerCent);
}

size_t HAQuDA_WebPageDisplayManip::BuildJson(char *buf, size_t size) {
	display_shown copy;
	{
		CriticalLock lock(mux);
		copy = picture;
	}
	web_display_state state;
	HAQuDA_WebPictureBuild(copy, brightPercent.load(), millis(), state);
	return HAQuDA_TranslateDisplayToJson(state, buf, size);
}
