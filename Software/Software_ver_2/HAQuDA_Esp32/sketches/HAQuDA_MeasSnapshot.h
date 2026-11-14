#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>

// Parameters the lamp averages and shows. The order is the order of the slots.
enum class snapshot_param : uint8_t { temp, humid, eco2, tvoc, pm25, count };

constexpr size_t SNAPSHOT_PARAM_NUM = static_cast<size_t>(snapshot_param::count);

// Latest averaged value of each parameter with the colour the lamp scale gives it. loop() writes it
// after every sensor poll, the web server task reads it, so every slot is a std::atomic and no lock is needed.
// A value and its colour are separate atomics, so the pair is not read as one unit: Publish() stores the colour first,
// hence a reader can see the previous value with the new colour for one poll.
class HAQuDA_MeasSnapshot {
  public:
	HAQuDA_MeasSnapshot();

	// Stores the value and the colour 0xRRGGBB of one parameter. NaN marks a failed reading. loop only.
	void Publish(snapshot_param param, float value, uint32_t rgb);

	// Copies the value and colour of one parameter; returns false when param is out of range or nothing
	// valid was published yet (value is then NaN). Any task.
	bool Get(snapshot_param param, float &value, uint32_t &rgb) const;

  private:
	std::atomic<float> values[SNAPSHOT_PARAM_NUM];
	std::atomic<uint32_t> colors[SNAPSHOT_PARAM_NUM];
};
