#include "HAQuDA_WebServer.h"

HAQuDA_WebServer webServer;

HAQuDA_WebServer::HAQuDA_WebServer() {
	server.begin(80);
}

void HAQuDA_WebServer::setWiFiCreds(String ssidLocal, String passLocal) {
	if (server.hasArg("ssid") && server.hasArg("password")) {
		ssidLocal = server.arg("ssid");
		passLocal = server.arg("password");
		WiFiCredsFound = true;
		server.send(200, "text/html", WiFiCredsPage);
		// server.stop();
	} else {
		// If one of the creds is missing, go back to form page
		server.send(200, "text/html", WiFiCredsPage);
	}
}

void HAQuDA_WebServer::WebServer_init() {
	server.on("/", setWiFiCreds);
	server.onNotFound(handle_NotFound);
}


void HAQuDA_WebServer::handle_NotFound() {
	server.send(404, "text/plain", "Not found");
}