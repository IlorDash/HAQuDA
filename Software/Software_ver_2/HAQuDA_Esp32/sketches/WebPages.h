#pragma once

const String indexPage = "﻿<!DOCTYPE html>"
						 "<html>"
						 "<head>"
						 "<title>HAQuDA WEB SERVER</title>"
						 "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
						 "<style>"
						 "html {"
						 "font-family: Arial, Helvetica, sans-serif;"
						 "display: inline-block;"
						 "text-align: center;"
						 "}"
						 "h1 {"
						 "font-size: 1.8rem;"
						 "color: white;"
						 "}"
						 "p {"
						 "font-size: 1.4rem;"
						 "}"
						 ".topnav {"
						 "overflow: hidden;"
						 "background-color: #0A1128;"
						 "}"
						 "body {"
						 "margin: 0;"
						 "}"
						 ".content {"
						 "padding: 5%;"
						 "}"
						 "</style>"
						 "</head>"
						 "<body>"
						 "<div class=\"topnav\">"
						 "<h1>HAQuDA WEB SERVER</h1>"
						 "</div>"
						 "<div>"
						 "<h2>"
						 "Hello, world!</h1>"
						 "<p class=\"text\">"
						 "<a href=\"wifi_manager\">Add WiFi credentials</a>"
						 "</p>"
						 "</div>"
						 "</body>"
						 "</html>";

const String WiFiManagerPage = "﻿<!DOCTYPE html>"
							   "<html>"
							   "<head>"
							   "<title>ESP Wi-Fi Manager</title>"
							   "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
							   "<style>"
							   "html {"
							   "font-family: Arial, Helvetica, sans-serif;"
							   "display: inline-block;"
							   "text-align: center;"
							   "}"
							   "h1 {"
							   "font-size: 1.8rem;"
							   "color: white;"
							   "}"
							   "p {"
							   "font-size: 1.4rem;"
							   "}"
							   ".topnav {"
							   "overflow: hidden;"
							   "background-color: #0A1128;"
							   "}"
							   "body {"
							   "margin: 0;"
							   "}"
							   ".content {"
							   "padding: 5%;"
							   "}"
							   "</style>"
							   "</head>"
							   "<body>"
							   "<div class=\"topnav\">"
							   "<h1>ESP Wi-Fi Manager</h1>"
							   "</div>"
							   "<div class=\"content\">"
							   "<div class=\"card-grid\">"
							   "<div class=\"card\">"
							   "<form action=\"/add_wifi_creds\" method=\"POST\">"
							   "<p>"
							   "<label for=\"ssid\">SSID</label>"
							   "<input type=\"text\" id=\"ssid\" name=\"ssid\"><br>"
							   "<label for=\"pass\">Password</label>"
							   "<input type=\"text\" id=\"pass\" name=\"pass\"><br>"
							   "<label for=\"ip\">IP Address</label>"
							   "<input type=\"text\" id=\"static_ip\" name=\"static_ip\" value=\"192.168.1.200\"><br>"
							   "<label for=\"gateway\">Gateway Address</label>"
							   "<input type=\"text\" id=\"gateway\" name=\"gateway\" value=\"192.168.1.1\"><br>"
							   "<input type=\"submit\" value=\"Submit\">"
							   "</p>"
							   "</form>"
							   "</div>"
							   "</div>"
							   "</div>"
							   "</body>"
							   "</html>";
							   