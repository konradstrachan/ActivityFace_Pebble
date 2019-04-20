#pragma once

#define CANVAS_NUMBER_BARS_X 24

#define EMOJI_BUFFER_LENGTH 6
extern char g_emoji[EMOJI_BUFFER_LENGTH];
#define TIME_BUFFER_LENGTH 8
extern char g_time[TIME_BUFFER_LENGTH];            // XX:YY__\0
#define DATE_BUFFER_LENGTH 12
extern char g_date[DATE_BUFFER_LENGTH];            // DD/MM/YY :-)

extern uint32_t g_buckets;
extern time_t g_stepInSeconds;

extern time_t g_updateHealthDataForHourLastCalledtime;
extern time_t g_updateHeartRateDataForHourLastCalledtime;

struct Bar
{
    int previousPercentage;      // 0 - 100 only
    int currentPercentage;       // 0 - 100 only
    bool incomplete;             // Is this bar currently still pending data
};

struct SquareCanvas
{
    struct Bar arrayBars[CANVAS_NUMBER_BARS_X]; 
};

struct HeartRateData
{
    uint32_t heartRate[48];    // 24 hours of data with two samples per hour
};

extern struct SquareCanvas g_Canvas;
extern int g_nTimeHour;
extern int g_nTimeMinute;

extern struct HeartRateData g_HeartRateData;

extern bool g_isFridayBetween5And6PM;
extern bool g_initialLoadComplete;

extern GColor g_coltopColor1;
extern GColor g_coltopColor2; 
extern GColor g_colbottomColor1;
extern GColor g_colbottomColor2;

// Configurable options

extern bool g_configHasReceivedConfiguration;
extern bool g_configShowEmoji;
extern bool g_configUseAlternativeDateFormat;        // i.e. US

extern Window* g_pWindow;
extern Layer* g_pCanvasLayer;
extern AppTimer* g_pTimer;