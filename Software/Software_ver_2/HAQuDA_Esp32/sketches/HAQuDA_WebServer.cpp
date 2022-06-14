#include "HAQuDA_WebServer.h"
#include "WebPages.h"

const char *set_WiFi_creds_params_order[WIFI_CREDS_PARAMS_NUM] = {"ssid", "pass", "ip", "gateway"};

HAQuDA_WebServer::HAQuDA_WebServer() {
}

void HAQuDA_WebServer::beginWebServer() {
	// myFS_WiFi->Start();
	WebServer_init();
	server.begin();
}

bool HAQuDA_WebServer::foundNameInParamsArrOrder(String name) {
	for (int i = 0; i < WIFI_CREDS_PARAMS_NUM; i++) {
		if (name == set_WiFi_creds_params_order[i]) {
			return true;
		}
	}
	return false;
}

bool HAQuDA_WebServer::checkCorrectParams(AsyncWebServerRequest *request) {
	int params_len = request->params();
	if (params_len != WIFI_CREDS_PARAMS_NUM) {
		return false;
	}
	for (int i = 0; i < params_len; i++) {
		AsyncWebParameter *p = request->getParam(i);
		if (!(p->isPost())) {
			return false;
		}
		if (!foundNameInParamsArrOrder(p->name())) {
			return false;
		}
	}
	return true;
}

int HAQuDA_WebServer::getParamVal(AsyncWebServerRequest *request, String param) {
	for (int i = 0; i < WIFI_CREDS_PARAMS_NUM; i++) {
		AsyncWebParameter *p = request->getParam(i);
		if (p->name() == param) {
			return p->value().toInt();
		}
	}
	return -1;
}

void HAQuDA_WebServer::handle_NewWiFiCreds(AsyncWebServerRequest *request) {
	TWiFiCreds *newWiFiCreds;

	if (checkCorrectParams(request)) {

		newWiFiCreds->ssid = getParamVal(request, set_WiFi_creds_params_order[0]);
		newWiFiCreds->password = getParamVal(request, set_WiFi_creds_params_order[1]);
		// log_i("Saving WiFi net with SSID = %s\r\n", ssidNew.c_str());

		if (myFS_WiFi->AppendFile(FILE_NAME_WIFI_NET, (uint8_t *)newWiFiCreds, sizeof(newWiFiCreds))) {
			String response_success = "<h1>Success</h1>";
			response_success += "<h2>Device will restart in 3 seconds</h2>";
			request->send(200, "text/html", response_success);
			delay(3000);
			ESP.restart();
		} else {
			String response_error = "<h1>Error writing new WiFi creds in file system</h1>";
			response_error += "<h2><a href='/'>Go back</a>to try again";
			request->send(200, "text/html", response_error);
		}

	} else {
		// If one of the creds is missing, go back to form page
		String response_error = "<h1>Error check WiFi creds</h1>";
		response_error += "<h2><a href='/'>Go back</a>to try again";
		request->send(200, "text/html", response_error);
	}
}

void HAQuDA_WebServer::handle_NotFound(AsyncWebServerRequest *request) {
	request->send(404);
}

void HAQuDA_WebServer::WebServer_init() {
	// Route for root / web page
	server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) { request->send(SPIFFS, "/index.html", "text/html"); });
	server.serveStatic("/", SPIFFS, "/");

	server.on("/add_wifi_creds", HTTP_GET, [](AsyncWebServerRequest *request) { request->send(SPIFFS, "/wifi_manager.html", "text/html"); });

	server.on("/wifi_manager", HTTP_POST, std::bind(&HAQuDA_WebServer::handle_NewWiFiCreds, this, std::placeholders::_1));
	server.onNotFound(std::bind(&HAQuDA_WebServer::handle_NotFound, this, std::placeholders::_1));
}

HAQuDA_WebServer::~HAQuDA_WebServer() {
}