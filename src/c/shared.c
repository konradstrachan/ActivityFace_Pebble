#include <pebble.h>
#include "shared.h"

int g_nTimeHour;
int g_nTimeMinute;

struct SquareCanvas g_Canvas;

struct HeartRateData g_HeartRateData;

bool g_isFridayBetween5And6PM;
bool g_initialLoadComplete;

char g_time[TIME_BUFFER_LENGTH];            // XX:YY__\0
char g_date[DATE_BUFFER_LENGTH];            // DD/MM/YY :-)
char g_emoji[EMOJI_BUFFER_LENGTH];

GColor g_coltopColor1;
GColor g_coltopColor2; 
GColor g_colbottomColor1;
GColor g_colbottomColor2;

uint32_t g_buckets = CANVAS_NUMBER_BARS_X;
time_t g_stepInSeconds = (24 / CANVAS_NUMBER_BARS_X)*60*60;

time_t g_updateHealthDataForHourLastCalledtime;
time_t g_updateHeartRateDataForHourLastCalledtime;

bool g_configHasReceivedConfiguration = false;
bool g_configShowEmoji = true;
bool g_configUseAlternativeDateFormat = false;

Window* g_pWindow = NULL;
Layer* g_pCanvasLayer = NULL;
AppTimer* g_pTimer= NULL;