#include "HAQuDA_WebServer.h"
#include "WebPages.h"

HAQuDA_WebServer::HAQuDA_WebServer() {
}

void HAQuDA_WebServer::beginWebServer() {
	server.begin(80);
	myFS_WiFi->Start();
}

void HAQuDA_WebServer::loop() {
	server.handleClient();
}

void HAQuDA_WebServer::getNewWiFiCreds() {
	TWiFiCreds *newWiFiCreds;
	if (server.hasArg("ssid") && server.hasArg("password")) {

		newWiFiCreds->ssid = server.arg("ssid");
		newWiFiCreds->password = server.arg("password");
		// log_i("Saving WiFi net with SSID = %s\r\n", ssidNew.c_str());

		if (myFS_WiFi->AppendFile(FILE_NAME_WIFI_NET, (uint8_t *)newWiFiCreds, sizeof(newWiFiCreds))) {
			String response_success = "<h1>Success</h1>";
			response_success += "<h2>Device will restart in 3 seconds</h2>";
			server.send(200, "text/html", response_success);
			delay(3000);
			ESP.restart();
		} else {
			String response_error = "<h1>Error writing new WiFi creds in file system</h1>";
			response_error += "<h2><a href='/'>Go back</a>to try again";
			server.send(200, "text/html", response_error);
		}

	} else {
		// If one of the creds is missing, go back to form page
		String response_error = "<h1>Error check WiFi creds</h1>";
		response_error += "<h2><a href='/'>Go back</a>to try again";
		server.send(200, "text/html", response_error);
	}
}

void HAQuDA_WebServer::handle_NotFound() {
	server.send(404, "text/plain", "Not found test");
}

void HAQuDA_WebServer::WebServer_init() {
	server.on("/", HTTPMethod::HTTP_GET, [this] { server.send(200, "text/html", WiFiCredsPage); });
	server.on("/", HTTPMethod::HTTP_POST, [this] { getNewWiFiCreds(); });
	server.onNotFound([this] { handle_NotFound(); });
}
