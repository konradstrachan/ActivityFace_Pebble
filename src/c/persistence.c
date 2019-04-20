#include <pebble.h>
#include "persistence.h"
#include "watchface.h"

#include "debuggingswitches.h"

static const uint32_t persistedData_timeStamp = 1337;

//static const uint32_t persistedData_hour0 = 0;
// ...
//static const uint32_t persistedData_hour23 = 23;

static const uint32_t persistedData_optionEmoji = 1001;
static const uint32_t persistedData_optionAlternativeDateFormat = 1002;

union converter_t
{
    struct converterStructure
    {
        uint16_t first;
        uint16_t second;
    } part16_t;

    struct converterStructure part16;
    uint32_t part32;
};

void ConvertDataToPersistedInt(uint16_t historical, uint16_t current, uint32_t* resultant)
{
    union converter_t conv;
    conv.part16.first = historical;
    conv.part16.second = current;
    *resultant = conv.part32;
}

void ConvertPersistedIntToData(uint32_t data, uint16_t* historical, uint16_t* current)
{
    union converter_t conv;
    conv.part32 = data;
    *historical = conv.part16.first;
    *current = conv.part16.second;
}

bool DoesPersistedDataExist()
{
#if DEBUG_READ_DUMMY_DATA
    APP_LOG(APP_LOG_LEVEL_DEBUG, "DEBUG_READ_DUMMY_DATA - ignoring persisted data");
    return false;
#endif
#if DEBUG_IGNORE_PERSISTED_DATA
    APP_LOG(APP_LOG_LEVEL_DEBUG, "DEBUG_IGNORE_PERSISTED_DATA - ignoring persisted data");
    return false;
#endif

    if(!persist_exists(persistedData_timeStamp))
    {
        APP_LOG(APP_LOG_LEVEL_INFO, "Persisted data does not exist");
        return false;    
    }
    
    time_t timeStamp = (time_t)persist_read_int(persistedData_timeStamp);
    time_t now = time(NULL);
    
    int dataAge = (int)(now - timeStamp);

    if(dataAge > 15 * 60)
    {
        // if delta is greater than 15 minutes then consider data invalid
        APP_LOG(APP_LOG_LEVEL_INFO, "Persisted data is too old %d, now %d", (int)timeStamp, (int)now);
        return false;
    }
    APP_LOG(APP_LOG_LEVEL_INFO, "Persisted data is %d s old (but this is ok...)", (int)(dataAge));

    return true;
}

void LoadPersistedData()
{
    int currentActiveBlock = GetCurrentHourBlockFromTime();
    
    for( int i = 0; i < 24; ++i )
    {
        uint32_t data = persist_read_int(i);

        uint16_t historical = 0;
        uint16_t current = 0;
        ConvertPersistedIntToData(data, &historical, &current);
        
        g_Canvas.arrayBars[i].previousPercentage = (int)historical;
        
        if(i <= currentActiveBlock)
        {
            // Load current data only for the current day
            g_Canvas.arrayBars[i].currentPercentage = (int)current;
        }
        else
        {
            // If we have data for greater than the current active hour that's old 
            // data from yeterday, we don't want this as it's a new day
            g_Canvas.arrayBars[i].currentPercentage = 0;
        }
        
        g_Canvas.arrayBars[i].incomplete = i == currentActiveBlock;
        
#if DEBUG_PERSISTED_DATA
        APP_LOG(APP_LOG_LEVEL_INFO, "Read persisted data for block %d - %d %d", i, (int)g_Canvas.arrayBars[i].previousPercentage, (int)g_Canvas.arrayBars[i].currentPercentage);
#endif
    }
    
    APP_LOG(APP_LOG_LEVEL_INFO, "Finished reading from persisted data");
}

void WritePersistedData()
{
#if DEBUG_READ_DUMMY_DATA
    APP_LOG(APP_LOG_LEVEL_DEBUG, "DEBUG_READ_DUMMY_DATA - not writing persisted data");
    return;
#endif
    
    for( int i = 0; i < 24; ++i )
    {
        uint32_t data = 0;
        
        ConvertDataToPersistedInt(g_Canvas.arrayBars[i].previousPercentage, 
                                  g_Canvas.arrayBars[i].currentPercentage,
                                  &data);
        
        uint32_t statusCode = persist_write_int(i, data);
        
        if(statusCode != S_SUCCESS)
        {
            // LOG etc...    
        }
    }
    
    persist_write_int(persistedData_timeStamp, (uint32_t)time(NULL));
    
    APP_LOG(APP_LOG_LEVEL_INFO, "Wrote data to persistence store");
}

bool DoesPersistedOptionDataExist()
{
    if(!persist_exists(persistedData_optionEmoji) || !persist_exists(persistedData_optionAlternativeDateFormat))
    {
        APP_LOG(APP_LOG_LEVEL_INFO, "Persisted option data does not exist");
        return false;    
    }
    
    return true;
}

void LoadPersistedOptionData()
{
    g_configShowEmoji = persist_read_int(persistedData_optionEmoji);
    g_configUseAlternativeDateFormat = persist_read_int(persistedData_optionAlternativeDateFormat);
    
    APP_LOG(APP_LOG_LEVEL_INFO, "Loaded persisted option data - Emoji-%d Alt Date-%d", g_configShowEmoji, g_configUseAlternativeDateFormat);
}

void WritePersistedOptionData()
{
    persist_write_int(persistedData_optionEmoji, g_configShowEmoji);
    persist_write_int(persistedData_optionAlternativeDateFormat, g_configUseAlternativeDateFormat);
    
    APP_LOG(APP_LOG_LEVEL_INFO, "Wrote persisted option data");
}