#pragma once

#include "WS2812.h"
#include "HAQuDA_UI.h"
#include <vector>

HAQuDA_UI *myUI_tasks;

void createTasks(HAQuDA_UI *currUI);
void deleteTasks();