#include <pebble.h>
#include "watchface.h"
#include "persistence.h"
#include "utilities.h"
#include "drawing.h"
#include "main.h"
#include "healthdata.h"
#include "emoticon.h"

// ----- Debugging function switches

// ----- Local declarations

void OnWatchfaceInit()
{
    g_updateHealthDataForHourLastCalledtime = 0;
    g_updateHeartRateDataForHourLastCalledtime = 0;
    
    g_initialLoadComplete = false;
    g_isFridayBetween5And6PM = false;
    
#if defined(PBL_PLATFORM_APLITE) || defined(PBL_PLATFORM_DIORITE)
    // GColorPictonBlue = dithered 50% grey
    g_coltopColor1 = GColorPictonBlue;
    g_coltopColor2 = GColorWhite;
    g_colbottomColor1 = GColorPictonBlue;
    g_colbottomColor2 = GColorWhite;
#else
    g_coltopColor1 = GColorCobaltBlue;
    g_coltopColor2 = GColorPictonBlue;
    g_colbottomColor1 = GColorJazzberryJam;
    g_colbottomColor2 = GColorBrilliantRose;
#endif

    // Initialise the canvas
    for( int nX = 0; nX < CANVAS_NUMBER_BARS_X ; ++nX )
    {
        g_Canvas.arrayBars[nX].previousPercentage = 0;
        g_Canvas.arrayBars[nX].currentPercentage = 0;
        g_Canvas.arrayBars[nX].incomplete = false;
    }
    
    // Initialise options - must be done before initialising time data
    if(DoesPersistedOptionDataExist())
    {
        LoadPersistedOptionData();
    }
    
    // Initialise time data
    InitialiseTimeData();
    
    // Don't initialise health data here as this will take some time...
}

void OnWatchfaceDeInit()
{ 
    APP_LOG(APP_LOG_LEVEL_INFO, "Writing persisted data..");
    WritePersistedData();
    APP_LOG(APP_LOG_LEVEL_INFO, "Writing persisted data.. complete");
}

void OnWatchfaceTick()
{
#if 0
    // For debugging make pretty bars
    for( int nX = 0; nX < CANVAS_NUMBER_BARS_X ; ++nX )
    {
        g_Canvas.arrayBars[nX].previousPercentage = GetRandomNumber( 0,100 );
        g_Canvas.arrayBars[nX].currentPercentage = GetRandomNumber( 0,100 );
    }
    return;
#endif
}

void OnWatchfacePaint(GContext* ctx)
{
    DrawWatchFace(ctx);
}

uint32_t GetCurrentHourBlockFromTime()
{
    return (time(NULL) - time_start_of_today()) / g_stepInSeconds;   
}

void OnTimeUpdated(struct tm* tick_time)
{
    if(tick_time == NULL)
    {
        // todo - panic?
        return;    
    }

    // Check if we are that magic time between Friday 5 - 6pm
    if(   tick_time->tm_wday == 5
       && tick_time->tm_hour >= 17
       && tick_time->tm_hour < 18 )
    {
        g_isFridayBetween5And6PM = true;
    }
    else
    {
        g_isFridayBetween5And6PM = false;
    }
    
    static char* timeFormatter = NULL;
    
    if(clock_is_24h_style())
    {
        timeFormatter = "%H:%M";
    }
    else
    {
        timeFormatter = "%l:%M";
    }
    
    strftime(&g_time[0],
             TIME_BUFFER_LENGTH - 1,
             timeFormatter,
             tick_time);
    
    g_time[TIME_BUFFER_LENGTH - 1] = '\0';
    
    static char* dateFormatter = NULL;
    
    if(g_configUseAlternativeDateFormat)
    {
        // US style
        dateFormatter = "%m-%d ";
    }
    else
    {
        // Rest of the world style
        dateFormatter = "%d-%m ";
    }
    
    strftime(&g_date[0],
             DATE_BUFFER_LENGTH,
             dateFormatter,
             tick_time);
    
    g_date[DATE_BUFFER_LENGTH - 1] = '\0';
    
    // If the hour has changed we should update the currently
    // active block
    // TODO - this can maybe be optimised so it doesn't run each minute
    UpdateCurrentlyActiveBlockIfHourHasChanged();
    
    // the health data will not have changed, but this way the 
    // emoticon will update every minute which makes the watchface
    // feel more alive
    UpdateEmoticon();
}

void InitialiseTimeData()
{
    APP_LOG(APP_LOG_LEVEL_INFO, "Manually initialising time data");
    time_t currentTime = time(NULL);
    struct tm* currentTimeTm = localtime(&currentTime);
    OnTimeUpdated(currentTimeTm);    
}