#include "HAQuDA_MeasSnapshot.h"

#include <cmath>
#include <limits>

HAQuDA_MeasSnapshot::HAQuDA_MeasSnapshot() {
	for (size_t i = 0; i < SNAPSHOT_PARAM_NUM; i++) {
		values[i].store(std::numeric_limits<float>::quiet_NaN());
		colors[i].store(0);
	}
}

void HAQuDA_MeasSnapshot::Publish(snapshot_param param, float value, uint32_t rgb) {
	size_t index = static_cast<size_t>(param);
	if (index >= SNAPSHOT_PARAM_NUM) {
		return;
	}
	colors[index].store(rgb);
	values[index].store(value);
}

bool HAQuDA_MeasSnapshot::Get(snapshot_param param, float &value, uint32_t &rgb) const {
	size_t index = static_cast<size_t>(param);
	if (index >= SNAPSHOT_PARAM_NUM) {
		value = std::numeric_limits<float>::quiet_NaN();
		rgb = 0;
		return false;
	}
	value = values[index].load();
	rgb = colors[index].load();
	return !std::isnan(value);
}
