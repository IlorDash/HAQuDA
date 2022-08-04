#pragma once

typedef enum {
	noneError,
	failedToConnectToWiFi,
	failedToCreateAP
} errorTypes_enum;

class HAQuDA_ErrorHandler {
  private:

  public:

	HAQuDA_ErrorHandler();
	~HAQuDA_ErrorHandler();
};