#pragma once

#include <ESPAsyncWebServer.h>
#include "WebPages.h"

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
		request->send(200, "text/html", indexPage);
	}
};