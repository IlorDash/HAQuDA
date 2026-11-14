#pragma once

#include <ESPAsyncWebServer.h>

// Sends the gzipped dashboard page. async_tcp task only (route and captive handler callbacks). Defined in HAQuDA_WebServer.cpp, the only file that holds the
// page data.
void HAQuDA_SendIndexPage(AsyncWebServerRequest *request);

class HAQuDA_CaptiveRequestHandler : public AsyncWebHandler {
  public:
	HAQuDA_CaptiveRequestHandler() {
	}
	virtual ~HAQuDA_CaptiveRequestHandler() {
	}

	bool canHandle(AsyncWebServerRequest *request) {
		// request->addInterestingHeader("ANY");
		return true;
	}

	void handleRequest(AsyncWebServerRequest *request) {
		HAQuDA_SendIndexPage(request);
	}
};
