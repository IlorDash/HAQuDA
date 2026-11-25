#pragma once

class HAQuDA_IApRequest {
  public:
	virtual ~HAQuDA_IApRequest() = default;

	// False when the restart cannot be scheduled now (a network check runs or a restart is already coming). Loop task only.
	virtual bool RequestAccessPoint() = 0;
};
