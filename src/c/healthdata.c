#include <pebble.h>
#include "healthdata.h"
#include "persistence.h"
#include "watchface.h"
#include "emoticon.h"
#include "main.h"

#include "debuggingswitches.h"

// ----- Local declarations

// TODO - make selectable max(NUMBER_OF_STEPS_PER_HOUR_SCALE_MAX, MAX_DATA); etc

#define NUMBER_OF_STEPS_PER_HOUR_SCALE_MAX 1000

// maximum every 15 minutes
#define MAX_FREQUENCY_TO_UPDATE_HOURLY_HEALTH_DATA 15 * 60

void RefreshAllHealthData()
{   
    APP_LOG(APP_LOG_LEVEL_INFO, "Refreshing all health data...");
    if(!g_initialLoadComplete)
    {
        APP_LOG(APP_LOG_LEVEL_INFO, "Initial data load detected!");
        g_initialLoadComplete = true;
        
        // check for cached data
        if(DoesPersistedDataExist())
        {
            // Load from persisted data
            APP_LOG(APP_LOG_LEVEL_INFO, "Loading from persisted data...");
            LoadPersistedData();
            
            // TODO - for now we don't persist heart rate data
            UpdateHeartRateData(false);        // all the data
            // END TODO
            
            // Set the current bar
            int currentBlock = GetCurrentHourBlockFromTime();
            APP_LOG(APP_LOG_LEVEL_INFO, "Guessing current block is %d", currentBlock);
            
            if(currentBlock > 0 && currentBlock < 24)
            {
                g_Canvas.arrayBars[currentBlock].incomplete = true;
            }
            
            // Initialise the time so we are not waiting for the first time callback
            // this will also initialise the emoticon
            InitialiseTimeData();
            
#if DEBUG_READ_DUMMY_DATA
            PopulateHealthDataWithTestValues();
#endif

            // Refresh the screen
            RequestDisplayRefresh();
            return;
        }
        // else carry on with loading from health api...
        
        APP_LOG(APP_LOG_LEVEL_INFO, "No persisted data detected, loading from API...");
    }
    
    // Use the average daily value from midnight to the current time
    UpdateHistoricalHealthData();
    
    // current data
    UpdateAllCurrentHealthDataFromLastDay();
    UpdateHeartRateData(false);        // all the data
    
#if DEBUG_READ_DUMMY_DATA
    PopulateHealthDataWithTestValues();
#endif

    // Refresh screen
    RequestDisplayRefresh();
}

void UpdateHistoricalHealthData()
{     
    // Historical averaged data
    APP_LOG(APP_LOG_LEVEL_INFO, "Updating historical health data...");
    
    for( uint32_t bucketIter = 0; bucketIter < g_buckets; ++bucketIter )
    {
        const time_t start = time_start_of_today()     
            - 24*60*60                                 // Go to yesterday
            + (bucketIter * g_stepInSeconds);
        const time_t end = start + g_stepInSeconds;

        HealthServiceAccessibilityMask mask 
            = health_service_metric_averaged_accessible(HealthMetricStepCount, 
                                                        start, 
                                                        end, 
                                                        HealthServiceTimeScopeDaily);    // averaged across all days in the week
        
        if(mask & HealthServiceAccessibilityMaskAvailable) 
        {
            // Getting the average data between two time periods is very easy - we just request it
            HealthValue average = health_service_sum_averaged(HealthMetricStepCount, start, end, HealthServiceTimeScopeDaily);

            // Calculate the percentage
            int percentageOfTarget = (int)(((int)average / (float)NUMBER_OF_STEPS_PER_HOUR_SCALE_MAX)*100.0);

            // And store it
            g_Canvas.arrayBars[bucketIter].previousPercentage = percentageOfTarget;

#if DEBUG_HEALTHAPIS
            APP_LOG(APP_LOG_LEVEL_INFO, "Historical step count: %d-%d : %d steps %d percent", (int)start, (int)end, (int)average, percentageOfTarget);
#endif
        }
    }
    
    // Don't write to persistence here - if we are updating historical data we will update the current data
    // next. At the end of updating the current data we will persist all the information
}

void UpdateAllCurrentHealthDataFromLastDay()
{   
    APP_LOG(APP_LOG_LEVEL_INFO, "Updating all current health data...");
    
    bool fBreakOnNextLoop = false;
    
    uint32_t bucketIter = 0;

    // Update buckets for the hours passed (and the current hour)
    for( ; bucketIter < g_buckets; ++bucketIter )
    {
        if(fBreakOnNextLoop)
        {
            break;
        }
        
        time_t start = time_start_of_today() 
            + (bucketIter * g_stepInSeconds);
        time_t end = start + g_stepInSeconds;
        
        if(end > time(NULL))
        {
            // If the current bucket time finishes later than the current time
            // constrain the query to the current time
            end = time(NULL);
            
            // Don't continue after this
            fBreakOnNextLoop = true;
        }
        
        if( end > start )
        {
            // Calculating current non-averaged data is a little more of a headache...
            GetHealthDataBetweenTwoTimeTs(start, end, bucketIter, fBreakOnNextLoop);
        }
        else
        {
            // if the end time is before the start time something has gone wrong
            APP_LOG(APP_LOG_LEVEL_ERROR, "Updating current step data and end is before start!");
            fBreakOnNextLoop = true;
        }
    }
    
    // Clear up the rest of the buckets
    for( ; bucketIter < g_buckets; ++bucketIter )
    {
        // TODO - annoying to do this each time, but should be a
        // relatively lightweight operation
        g_Canvas.arrayBars[bucketIter].currentPercentage = 0;
        g_Canvas.arrayBars[bucketIter].incomplete = false;
    }
    
    UpdateEmoticon();
}

void UpdateLastHourOfHealthData()
{   
    APP_LOG(APP_LOG_LEVEL_INFO, "Updating last hour of health data...");
    
    // Update time data
    time_t currentTime = time(NULL);
    time_t timeSinceLastUpdate = currentTime - g_updateHealthDataForHourLastCalledtime;
    
    // Make sure we are not calling this function too often to save battery power
    if(timeSinceLastUpdate <= MAX_FREQUENCY_TO_UPDATE_HOURLY_HEALTH_DATA)
    {
        APP_LOG(APP_LOG_LEVEL_INFO, "The last health data update for this hour was %d s ago. Too soon to update again!", (int)timeSinceLastUpdate);
        return;
    }
    
    APP_LOG(APP_LOG_LEVEL_INFO, "The last health data update was %d s ago, updating now...", (int)timeSinceLastUpdate);
    g_updateHealthDataForHourLastCalledtime = currentTime;
    
    struct tm* currentTimeTm = localtime(&currentTime);
    // Get from the start of the hour
    currentTimeTm->tm_min = 0;
    time_t start = mktime(currentTimeTm);
    time_t end = currentTime;
        
    GetHealthDataBetweenTwoTimeTs(start, end, GetCurrentHourBlockFromTime(), true);
    
    // Update the previously active block/hour's flag to remove highlight since 
    // the hour may have changed and the GetHealthDataBetweenTwoTimeTs function
    // will only update the current hour
    UpdateCurrentlyActiveBlockIfHourHasChanged();
    
    UpdateEmoticon();
}

uint32_t GetHeartRateDataBetweenTwoTimeTs(time_t start, time_t end)
{
    // Minute records (convert from seconds to minutes)
    const uint32_t max_records = (end - start) / 60;
    
    if(max_records == 0)
    {
        APP_LOG(APP_LOG_LEVEL_ERROR, "max_records can't be zero");
        return 0;
    }

    HealthMinuteData* minute_data = (HealthMinuteData*)malloc(max_records * sizeof(HealthMinuteData));
    memset(minute_data, 0, max_records * sizeof(HealthMinuteData));

    health_service_get_minute_history(minute_data, max_records, &start, &end);

    // Actual results received may be different and set in health_service_get_minute_history call
    const uint32_t actualMinuteRecordsReturned = (end - start)/60;

    uint32_t bpmTotalInPeriod = 0;
    uint32_t totalValidDataPoints = 0;
    for(uint32_t i = 0; i < actualMinuteRecordsReturned; ++i) 
    {       
        if(! minute_data[i].is_invalid
           && minute_data[i].heart_rate_bpm > 0)
        {
            bpmTotalInPeriod += minute_data[i].heart_rate_bpm;
            ++totalValidDataPoints;
        }
    }
    free(minute_data);
    
    uint32_t bpmInPeriod = 0;
    if(totalValidDataPoints > 0)
    {
        bpmInPeriod = (uint32_t)(bpmTotalInPeriod / (double)totalValidDataPoints);
    }
    
#if DEBUG_HEALTHAPIHEARTRATE
    APP_LOG(APP_LOG_LEVEL_INFO, "Average heart rate: %d-%d : %d bmp %d records", (int)start, (int)end, (int)bpmInPeriod, (int)totalValidDataPoints);
#endif

    return bpmInPeriod;
}

void GetHealthDataBetweenTwoTimeTs(time_t start, time_t end, uint32_t bucketIter, bool currentAciveHour)
{
    // Minute records (convert from seconds to minutes)
    const uint32_t max_records = (end - start) / 60;

    HealthMinuteData* minute_data = (HealthMinuteData*)malloc(max_records * sizeof(HealthMinuteData));
    memset(minute_data, 0, max_records * sizeof(HealthMinuteData));

    health_service_get_minute_history(minute_data, max_records, &start, &end);

    // Actual results received may be different and set in health_service_get_minute_history call
    const uint32_t actualMinuteRecordsReturned = (end - start)/60;

    uint32_t stepsInTimePeriod = 0;
    for(uint32_t i = 0; i < actualMinuteRecordsReturned; ++i) 
    {

#if DEBUG_HEALTHAPIS
        APP_LOG(APP_LOG_LEVEL_INFO, "  Item %d steps: %d, valid %d", (int)i, (int)minute_data[i].steps, (int)(!minute_data[i].is_invalid));
#endif
        
        if(! minute_data[i].is_invalid)
        {
            stepsInTimePeriod += minute_data[i].steps;
        }
    }
    free(minute_data);
        
    int percentageOfTarget = (int)(((int)stepsInTimePeriod / (float)NUMBER_OF_STEPS_PER_HOUR_SCALE_MAX)*100.0);
    g_Canvas.arrayBars[bucketIter].currentPercentage = percentageOfTarget;
    g_Canvas.arrayBars[bucketIter].incomplete = currentAciveHour;
#if DEBUG_HEALTHAPIS
    APP_LOG(APP_LOG_LEVEL_INFO, "Current step count: %d-%d : %d steps %d percent in %d records", (int)start, (int)end, (int)stepsInTimePeriod, percentageOfTarget, (int)max_records);
#endif

}

void UpdateCurrentlyActiveBlockIfHourHasChanged()
{
    int currentActiveBlock = GetCurrentHourBlockFromTime();
    
    if(currentActiveBlock >= (int)g_buckets)
    {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Invalid active block returned when attempting to change hour : %d", (int)currentActiveBlock );
        return;
    }
    
    if(currentActiveBlock > 0)
    {
        if(g_Canvas.arrayBars[currentActiveBlock - 1].incomplete)
        {
            // the hour has changed
            g_Canvas.arrayBars[currentActiveBlock - 1].incomplete = false;
        }
    }
    else
    {
        // If the hour is 0 then wrap around
        if(g_Canvas.arrayBars[g_buckets - 1].incomplete)
        {
            // the hour has changed
            g_Canvas.arrayBars[g_buckets - 1].incomplete = false;
        }
    }
}

void PopulateHealthDataWithTestValues()
{
    // For debugging make pretty bars
    for( int nX = 0; nX < CANVAS_NUMBER_BARS_X ; ++nX )
    {
        g_Canvas.arrayBars[nX].previousPercentage = GetRandomNumber( 0,100 );
        g_Canvas.arrayBars[nX].currentPercentage = GetRandomNumber( 0,100 );
        g_Canvas.arrayBars[nX].incomplete = false;
    }
    
    g_Canvas.arrayBars[GetRandomNumber( 0,23 )].incomplete = true;
}

void UpdateHeartRateData(bool lastHourOnly)
{
    time_t start = 0;
    uint32_t arrayIndex = 0;

    // Number of ticks in 24 hours divided by array size
    const uint32_t arraySize = sizeof(g_HeartRateData.heartRate)/ sizeof(uint32_t);
    const uint32_t arrayElementsInAnHour = arraySize / 24;
    const uint32_t ticksInTimePeriod = (3600 * 24) / arraySize;
    
    if(lastHourOnly)
    {
        APP_LOG(APP_LOG_LEVEL_INFO, "Updating last hour of heart rate data...");
        
        // Update time data
        time_t currentTime = time(NULL);
        time_t timeSinceLastUpdate = currentTime - g_updateHeartRateDataForHourLastCalledtime;
        
        // Make sure we are not calling this function too often to save battery power
        if(timeSinceLastUpdate <= MAX_FREQUENCY_TO_UPDATE_HOURLY_HEALTH_DATA)
        {
            APP_LOG(APP_LOG_LEVEL_INFO, "The last heart rate data update for this hour was %d s ago. Too soon to update again!", (int)timeSinceLastUpdate);
            return;
        }
        
        APP_LOG(APP_LOG_LEVEL_INFO, "The last heart rate data update was %d s ago, updating now...", (int)timeSinceLastUpdate);
        g_updateHeartRateDataForHourLastCalledtime = currentTime;
        
        struct tm* currentTimeTm = localtime(&currentTime);
        // Get from the start of the hour
        currentTimeTm->tm_min = 0;
        start = mktime(currentTimeTm);
        
        // calculate from where we are updating the data
        arrayIndex = currentTimeTm->tm_hour * arrayElementsInAnHour;
    }
    else       
    {
        APP_LOG(APP_LOG_LEVEL_INFO, "Updating and refreshing all heart rate data");
    
        // First clear data
        for( uint32_t i = 0; i < arraySize ; ++i)
        {
            g_HeartRateData.heartRate[i] = 0;
        }
        
        start = time_start_of_today();
    }
    
    while(1)
    {
        time_t end = start + ticksInTimePeriod;
        
        if(end >= time(NULL))
        {
#if DEBUG_HEALTHAPIHEARTRATE
            APP_LOG(APP_LOG_LEVEL_INFO, "Finished");
#endif
            break;    
        }
#if DEBUG_HEALTHAPIHEARTRATE
        APP_LOG(APP_LOG_LEVEL_INFO, "Start %d End %d now %d", (int)start, (int)end, (int)time(NULL));
#endif
        g_HeartRateData.heartRate[arrayIndex] = GetHeartRateDataBetweenTwoTimeTs(start, end);
        
        start = end;
        ++arrayIndex;
    }
    
    APP_LOG(APP_LOG_LEVEL_INFO, "Finished refreshing all heart rate data");
}