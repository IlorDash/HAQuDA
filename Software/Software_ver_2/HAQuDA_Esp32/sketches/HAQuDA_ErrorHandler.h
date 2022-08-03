#pragma once

typedef enum {
	none,
	failedToConnectToWiFi,
	failedToCreateAP
} ErrorTypes;

class HAQuDA_ErrorHandler {
  private:

  public:
	static ErrorTypes currErrorToDisp;
	HAQuDA_ErrorHandler();
	~HAQuDA_ErrorHandler();
};