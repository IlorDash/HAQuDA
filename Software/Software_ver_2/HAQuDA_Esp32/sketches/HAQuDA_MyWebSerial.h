#pragma once

#include "WebSerial.h"

class HAQuDA_MyWebSerial : public WebSerialClass {

	static bool WebSerialOpened; // flag for tracking when WebSerial page opened, after what we have to send all msgs

  public:
	HAQuDA_MyWebSerial();

	void begin(AsyncWebServer *server, const char *url = "/webserial");

	bool GetWebSerialOpened();
	void ResetWebSerialOpened();

	~HAQuDA_MyWebSerial();

  private:
};

extern HAQuDA_MyWebSerial MyWebSerial;