#pragma once

#include "WebSerial.h"

class HAQuDA_MyWebSerial : public WebSerialClass {

	static bool WebSerialOpened; // flag for tracking when WebSerial page opened, after what we have to send all msgs

	AsyncWebServer *_server;
	AsyncWebSocket *_ws;
	RecvMsgHandler _RecvFunc = NULL;

  public:
	HAQuDA_MyWebSerial();

	void begin(AsyncWebServer *server, const char *url = "/webserial");

	bool GetWebSerialOpened();
	void ResetWebSerialOpened();

	~HAQuDA_MyWebSerial();

  private:
};

extern HAQuDA_MyWebSerial MyWebSerial;