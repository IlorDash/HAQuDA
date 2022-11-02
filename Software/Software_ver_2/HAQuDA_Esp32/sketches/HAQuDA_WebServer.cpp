#include "HAQuDA_WebServer.h"
#include "WebPages.h"

bool HAQuDA_WebServer::WebServerStarted = false;

void HAQuDA_WebServer::beginWebServer() {
	WebServerResponds_init();
	server.addHandler(new HAQuDA_CaptiveRequestHandler()).setFilter(ON_AP_FILTER); // only when requested from AP
	server.begin();
	MyWebSerial.begin(&server);
	MyWebSerial.msgCallback(HAQuDA_WebServer::recvMsg);
	WebServerStarted = true;
}

/* Message callback of MyWebSerial */
void HAQuDA_WebServer::recvMsg(uint8_t *data, size_t len) {
	MyWebSerial.println("Received Data...");
	String d = "";
	for (int i = 0; i < len; i++) {
		d += char(data[i]);
	}
	MyWebSerial.println(d);
}

void HAQuDA_WebServer::WebSerialPrint(const char *str) {
	MyWebSerial.println(str);
	MyWebSerial.println("");
}

void HAQuDA_WebServer::WebSerialPrintStoredLogs() {
	MyWebSerial.println("foo");
	int logsNum = HAQuDA_Logger::GetStored_LogsNum();
	char logText[WEB_SERIAL_MAX_STR_LEN] = {0};
//	for (int i = 0; i < logsNum; i++) {
//		HAQuDA_Logger::GetStored_Log(i, logText);
//		MyWebSerial.println(logText);
//	}
}

bool HAQuDA_WebServer::GetWebServerStarted() {
	return WebServerStarted;
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

	saveNewWiFiCredsReturnMsgs saveNewWiFiCredsMsg = SaveNew_WiFiCreds(newWiFiCreds);

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
			vTaskDelay(3000);
			ESP.restart();
			break;
		}
		case saved_new_WiFi_creds: {
			String response_success = "<h1>Success</h1>";
			response_success += "<h2>Saved new WiFi credentials</h2>";
			response_success += "<h2>Device will restart in 3 seconds</h2>";
			request->send(200, "text/html", response_success);
			vTaskDelay(SHOW_REBOOT_MSG_DELAY);
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
	int WiFiCredsNum = GetStored_WiFiCredsNum();
	if (WiFiCredsNum == 0) {
		request->send(200, "text/html", "WiFi credentials are empty");
	} else {
		HAQuDA_FileStorage::ReadFile(FILE_NAME_WIFI_NET, (uint8_t *)bar, sizeof(TWiFiCreds));
		//		char buff[WIFI_SSID_LEN + WIFI_PASS_LEN] = {""};
		//		String answer = "";
		//		for (int i = 0; i < WiFiCredsNum; i++) {
		//			HAQuDA_FileStorage::ReadFileFrom(FILE_NAME_WIFI_NET, sizeof(TWiFiCreds) * i, (uint8_t *)buff, sizeof(TWiFiCreds));
		//			answer += buff;
		//		}
		request->send(200, "text/html", "answer");
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

bool HAQuDA_WebServer::checkStoredWiFiCredsCntLimit() {
	int fSize = HAQuDA_FileStorage::FileSize(FILE_NAME_WIFI_NET);
	uint8_t storedWiFiCredsCnt = fSize / sizeof(TWiFiCreds);
	if (storedWiFiCredsCnt >= MAX_WIFI_CREDS_NUM) {
		return false;
	}
	return true;
}

bool HAQuDA_WebServer::getWiFiCredsWriteIndex(uint16_t *index, const TWiFiCreds newWiFiCreds) {
	*index = 0;
	size_t fileSize = HAQuDA_FileStorage::FileSize(FILE_NAME_WIFI_NET);
	if (fileSize == -1) {
		return false;
	}
	uint8_t storedWiFiCredsCnt = fileSize / sizeof(TWiFiCreds);
	TWiFiCreds readWiFiCreds;
	bool newWiFiCredsExists = false;

	while (((*index) < storedWiFiCredsCnt) && !newWiFiCredsExists) {
		size_t fileReadRes = HAQuDA_FileStorage::ReadFile(FILE_NAME_WIFI_NET, (uint8_t *)&readWiFiCreds, sizeof(TWiFiCreds));
		if (fileReadRes != sizeof(TWiFiCreds)) {
			return false;
		}
		if (readWiFiCreds.ssid == newWiFiCreds.ssid) {
			newWiFiCredsExists = true;
		} else {
			(*index)++;
		}
	}
	return true;
}

saveNewWiFiCredsReturnMsgs HAQuDA_WebServer::SaveNew_WiFiCreds(TWiFiCreds newWiFiCreds) {

	if (!checkStoredWiFiCredsCntLimit()) {
		return too_many_WiFi;
	}

	bool newWiFiCredsExists = false;
	int index = 0;
	int fSize = HAQuDA_FileStorage::FileSize(FILE_NAME_WIFI_NET);
	if (fSize <= 0) {
		bool writeCredsRes = HAQuDA_FileStorage::WriteFile(FILE_NAME_WIFI_NET, (uint8_t *)&newWiFiCreds, sizeof(TWiFiCreds));
		if (!writeCredsRes) {
			return error_saving_new_WiFi_creds;
		}
	} else {
		uint16_t index;
		bool getIndexRes = getWiFiCredsWriteIndex(&index, newWiFiCreds);
		if (!getIndexRes) {
			return error_reading_stored_WiFi_creds;
		}

		bool writeCredsRes = HAQuDA_FileStorage::WriteFile(FILE_NAME_WIFI_NET, index * sizeof(TWiFiCreds), (uint8_t *)&newWiFiCreds, sizeof(TWiFiCreds));
		if (!writeCredsRes) {
			return error_saving_new_WiFi_creds;
		}
	}

	return saved_new_WiFi_creds;
}

TWiFiCreds HAQuDA_WebServer::GetStored_WiFi(int num) {
	TWiFiCreds WiFiCreds;

	uint8_t pos = num * sizeof(TWiFiCreds);

	HAQuDA_FileStorage::ReadFile(FILE_NAME_WIFI_NET, pos, (uint8_t *)&WiFiCreds, sizeof(TWiFiCreds));
	return WiFiCreds;
}

int HAQuDA_WebServer::GetStored_WiFiCredsNum() {
	return HAQuDA_FileStorage::FileSize(FILE_NAME_WIFI_NET) / sizeof(TWiFiCreds);
}

const String HAQuDA_FileStorage::Read_WiFiCreds() {
	//	char buff[MAX_SHOW_WIFI_CREDS_BUFF_LEN];
	//	char *header = "Reading file : /WiFiNetworks\r\n";
	//	strncpy(buff, header, strlen(header));
	//
	//	int WiFiCredsNum = GetStored_WiFiCredsNum();
	//	TWiFiCreds readWiFiCred;
	//	for (int i = 0; i < WiFiCredsNum; i++) {
	//	}
	//
	//	return String(buff);
}

HAQuDA_WebServer::~HAQuDA_WebServer() {
}