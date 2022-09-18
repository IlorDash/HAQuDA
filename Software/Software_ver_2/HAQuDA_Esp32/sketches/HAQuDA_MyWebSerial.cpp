#include "HAQuDA_MyWebSerial.h"

bool HAQuDA_MyWebSerial::WebSerialOpened = false;

HAQuDA_MyWebSerial ::HAQuDA_MyWebSerial() {
}

void HAQuDA_MyWebSerial::begin(AsyncWebServer *server, const char *url) {
	_server = server;
	_ws = new AsyncWebSocket("/webserialws");

	_server->on(url, HTTP_GET, [](AsyncWebServerRequest *request) {
		// Send Webpage
		WebSerialOpened = true;
		AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", WEBSERIAL_HTML, WEBSERIAL_HTML_SIZE);
		response->addHeader("Content-Encoding", "gzip");
		request->send(response);
	});

	_ws->onEvent([&](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) -> void {
		if (type == WS_EVT_CONNECT) {
#if defined(DEBUG)
			DEBUG_WEB_SERIAL("Client connection received");
#endif
		} else if (type == WS_EVT_DISCONNECT) {
#if defined(DEBUG)
			DEBUG_WEB_SERIAL("Client disconnected");
#endif
		} else if (type == WS_EVT_DATA) {
#if defined(DEBUG)
			DEBUG_WEB_SERIAL("Received Websocket Data");
#endif
			if (_RecvFunc != NULL) {
				_RecvFunc(data, len);
			}
		}
	});

	_server->addHandler(_ws);

#if defined(WEBSERIAL_DEBUG)
	DEBUG_WEB_SERIAL("Attached AsyncWebServer along with Websockets");
#endif
}

bool HAQuDA_MyWebSerial::GetWebSerialOpened() {
	return WebSerialOpened;
}
void HAQuDA_MyWebSerial::ResetWebSerialOpened() {
	WebSerialOpened = false;
}

HAQuDA_MyWebSerial ::~HAQuDA_MyWebSerial() {
}

HAQuDA_MyWebSerial MyWebSerial;