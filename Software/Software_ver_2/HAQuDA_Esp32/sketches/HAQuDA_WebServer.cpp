#include "HAQuDA_WebServer.h"
#include "WebPages.h"

void HAQuDA_WebServer::beginWebServer() {
	WebServerResponds_init();
	server.begin();
}

void HAQuDA_WebServer::handle_NewWiFiCreds(AsyncWebServerRequest *request) {

	int params_cnt = request->params();
	TWiFiCreds newWiFiCreds;
	for (int i = 0; i < params_cnt; i++) {
		AsyncWebParameter *p = request->getParam(i);
		if (p->isPost()) {
			if (p->name() == SSID_PARAM_NAME) {
				// newWiFiCreds.ssid = p->value().c_str();
				strcpy(newWiFiCreds.ssid, p->value().c_str());
			} else if (p->name() == PASS_PARAM_NAME) {
				// newWiFiCreds.password = p->value().c_str();
				strcpy(newWiFiCreds.pass, p->value().c_str());
			}
		}
	}

	if (!strlen(newWiFiCreds.pass) || !strlen(newWiFiCreds.ssid)) {
		// If one of the creds is missing, go back to form page
		String response_error = "<h1>Error check WiFi creds</h1>";
		response_error += "<h2><a href='/'>Go back</a>to try again";
		request->send(200, "text/html", response_error);
		return;
	}

	log_i("Saving WiFi net with SSID = %s\r\n", newWiFiCreds.ssid);

	saveNewWiFiCredsReturnMsgs saveNewWiFiCredsMsg = HAQuDA_FileStorage::SaveNew_WiFiCreds(newWiFiCreds);

	switch (saveNewWiFiCredsMsg) {
		case too_many_WiFi: {
			String response_error = "<h1>Error writing new WiFi creds in file system</h1>";
			response_error += "<h2>Too many WiFi credentials</h2>";
			response_error += "<h2><a href='/'>Go back</a>to try again";
			request->send(200, "text/html", response_error);
			break;
		}
		case error_reading_stored_WiFi_creds: {
			String response_error = "<h1>Error writing new WiFi creds in file system</h1>";
			response_error += "<h2>Error reading stored WiFi credentials</h2>";
			response_error += "<h2><a href='/'>Go back</a>to try again";
			request->send(500, "text/html", response_error);
			break;
		}
		case error_saving_new_WiFi_creds: {
			String response_error = "<h1>Error writing new WiFi creds in file system</h1>";
			response_error += "<h2>Error saving stored WiFi credentials</h2>";
			response_error += "<h2><a href='/'>Go back</a>to try again";
			request->send(500, "text/html", response_error);
			break;
		}
		case re_writed_WiFi_creds: {
			String response_success = "<h1>Success</h1>";
			response_success += "<h2>Re-writed WiFi credentials</h2>";
			response_success += "<h2>Device will restart in 3 seconds</h2>";
			request->send(200, "text/html", response_success);
			delay(3000);
			ESP.restart();
			break;
		}
		case saved_new_WiFi_creds: {
			String response_success = "<h1>Success</h1>";
			response_success += "<h2>Saved new WiFi credentials</h2>";
			response_success += "<h2>Device will restart in 3 seconds</h2>";
			request->send(200, "text/html", response_success);
			delay(3000);
			ESP.restart();
			break;
		}
		default:
			break;
	}
}

void HAQuDA_WebServer::delete_WiFiCreds(AsyncWebServerRequest *request) {
	bool res = HAQuDA_FileStorage::DeleteFile(FILE_NAME_WIFI_NET);
	if (res) {
		request->send(200, "text/html", "Deleted all WiFi credentials");
	} else {
		request->send(500, "text/html", "Error while deleting WiFi credentials");
	}
}

void HAQuDA_WebServer::show_WiFiCreds(AsyncWebServerRequest *request) {
	HAQuDA_FileStorage::ReadFileInSerial(FILE_NAME_WIFI_NET);
	char *bar;
	int WiFiCredsNum = HAQuDA_FileStorage::GetStored_WiFiCredsNum();
	if (WiFiCredsNum == 0) {
		request->send(200, "text/html", "WiFi credentials are empty");
	} else {
		HAQuDA_FileStorage::ReadFileFrom(FILE_NAME_WIFI_NET, 0, (uint8_t *)bar, sizeof(TWiFiCreds));
		//		char buff[WIFI_SSID_LEN + WIFI_PASS_LEN] = {""};
		//		String answer = "";
		//		for (int i = 0; i < WiFiCredsNum; i++) {
		//			HAQuDA_FileStorage::ReadFileFrom(FILE_NAME_WIFI_NET, sizeof(TWiFiCreds) * i, (uint8_t *)buff, sizeof(TWiFiCreds));
		//			answer += buff;
		//		}
		request->send(200, "text/html", "answer");
	}
}

void HAQuDA_WebServer::loop() {
	if (newWiFiCredsFound) {
	}
}

void HAQuDA_WebServer::handle_NotFound(AsyncWebServerRequest *request) {
	request->send(404);
}

void HAQuDA_WebServer::WebServerResponds_init() {
	// Route for root / web page
	server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) { request->send(200, "text/html", indexPage); });

	server.on("/wifi_manager", HTTP_GET, [](AsyncWebServerRequest *request) { request->send(200, "text/html", WiFiManagerPage); });

	server.on("/delete_wifi_creds", HTTP_POST, HAQuDA_WebServer::delete_WiFiCreds);
	server.on("/show_wifi_creds", HTTP_POST, HAQuDA_WebServer::show_WiFiCreds);
	server.on("/add_wifi_creds", HTTP_POST, HAQuDA_WebServer::handle_NewWiFiCreds);
	server.onNotFound(HAQuDA_WebServer::handle_NotFound);
}

HAQuDA_WebServer::~HAQuDA_WebServer() {
}