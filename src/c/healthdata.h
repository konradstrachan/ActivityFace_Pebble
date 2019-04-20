#pragma once

#include "shared.h"

void RefreshAllHealthData();
void GetHealthDataBetweenTwoTimeTs(time_t start, time_t end, uint32_t bucketIter, bool currentAciveHour);
uint32_t GetCurrentHourBlockFromTime();

void UpdateLastHourOfHealthData();
void UpdateHistoricalHealthData();
void UpdateAllCurrentHealthDataFromLastDay();

void UpdateCurrentlyActiveBlockIfHourHasChanged();

void PopulateHealthDataWithTestValues();

void UpdateHeartRateData(bool lastHourOnly);
uint32_t GetHeartRateDataBetweenTwoTimeTs(time_t start, time_t end);