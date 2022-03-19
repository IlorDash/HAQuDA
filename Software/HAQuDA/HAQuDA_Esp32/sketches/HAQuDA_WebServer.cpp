#include "HAQuDA_WebServer.h"
#include "WebPages.h"

HAQuDA_WebServer webServer;

HAQuDA_WebServer::HAQuDA_WebServer() {
	server.begin(80);
}

void HAQuDA_WebServer::setWiFiCreds() {
	if (server.hasArg("ssid") && server.hasArg("password")) {
		String ssidNew = server.arg("ssid");
		String passNew = server.arg("password");
		log_i("Saving WiFi net with SSID = %s\r\n", ssidNew.c_str());
		WiFiCredsFound = true;

		
		
		
		server.send(200, "text/html", WiFiCredsPage);
		// server.stop();
	} else {
		// If one of the creds is missing, go back to form page
		server.send(200, "text/html", WiFiCredsPage);
	}
		
}

void HAQuDA_WebServer::handle_NotFound() {
	server.send(404, "text/plain", "Not found");
}

void HAQuDA_WebServer::WebServer_init() {
	server.on("/", [this] { setWiFiCreds(); });
	server.onNotFound([this] { handle_NotFound(); });
}
