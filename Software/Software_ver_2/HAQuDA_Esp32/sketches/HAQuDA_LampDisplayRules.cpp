#include "HAQuDA_LampDisplayRules.h"

meas_type HAQuDA_ToMeasType(lamp_param param) {
	switch (param) {
		case lamp_param::temp:
			return temp;
		case lamp_param::humid:
			return humid;
		case lamp_param::eco2:
			return eCO2;
		case lamp_param::tvoc:
			return TVOC;
		case lamp_param::pm25:
			return PM2_5;
		case lamp_param::total:
		default:
			return total;
	}
}
