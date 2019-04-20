#pragma once

#include "shared.h"

// ----- Type declarations
  
// ----- Function declarations

void OnWatchfaceInit();
void OnWatchfaceDeInit();
void OnWatchfaceTick();
void OnWatchfacePaint(GContext* ctx);
void OnTimeUpdated(struct tm* tick_time);

void InitialiseTimeData();

uint32_t GetCurrentHourBlockFromTime();

int GetRandomNumber( int nMin, int nMax );
