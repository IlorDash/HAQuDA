#pragma once

#include <cstdint>

// warming_up: no sample taken because the ZH03B was powered recently - not a failure
enum class pm_meas_result : uint8_t { ok, warming_up, failed };

// True when the PM result must raise the "failed to get PM" error. A sample skipped during the warm-up is expected, not a failure. Any task.
constexpr bool HAQuDA_ShouldReportPMFailure(pm_meas_result result) {
	return result == pm_meas_result::failed;
}
