#pragma once

#include "WS2812.h"
#include "HAQuDA_DisplayInterface.h"

extern HAQuDA_DisplayInterface *myDisplayInterface_tasks;

void createTasks(HAQuDA_DisplayInterface *currUI);
void deleteTasks();