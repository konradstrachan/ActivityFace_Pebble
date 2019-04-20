#include <pebble.h>
#include "emoticon.h"
#include "utilities.h"
#include "shared.h"

void UpdateEmoticon()
{  
#if DEBUG_HAPPINESS
    APP_LOG(APP_LOG_LEVEL_INFO, "Updating emoticon...");
#endif
    
    if(!g_configShowEmoji)
    {
#if DEBUG_HAPPINESS
        APP_LOG(APP_LOG_LEVEL_INFO, "Skipping updating emoji as it is not configured to be shown");
#endif
    }
    
    // https://developer.pebble.com/guides/app-resources/system-fonts/#using-emoji-fonts
    
    if(g_isFridayBetween5And6PM)
    {
#if DEBUG_HAPPINESS
        APP_LOG(APP_LOG_LEVEL_INFO, "Ignoring health data as it's Friday5-6pm \\o/");
#endif
        switch(GetRandomNumber(0,2))
        {
        case 0:
            strcpy(&g_emoji[0], "\U0001F37A");
            break;
            
        case 1:
            strcpy(&g_emoji[0], "\U0001F37B");
            break;
            
        case 2:
        default:
            strcpy(&g_emoji[0], "\U0001F389");
            break;
        }
        return;
    }
    
    #define happinessLevel_asleep 0
    #define happinessLevel_veryunhappy 1
    #define happinessLevel_unhappy 2
    #define happinessLevel_happy 3
    #define happinessLevel_veryhappy 4
    #define happinessLevel_wow 5
    
    int happinessLevel = happinessLevel_asleep;
    
    // Determine based on the last two(?) hours
    int currentBar = 99;
    for( int nX = 0; nX < CANVAS_NUMBER_BARS_X ; ++nX )
    {
        if(g_Canvas.arrayBars[nX].incomplete)
        {
            // We have found the current hour block
#if DEBUG_HAPPINESS
            APP_LOG(APP_LOG_LEVEL_INFO, "Finding current bar, found %d", (int)nX);
#endif
            currentBar = nX;
            break;
        }
    }
    
    if(currentBar == 99)
    {
        // This value is to trap situation where the current bar is not found!
        APP_LOG(APP_LOG_LEVEL_ERROR, "Current bar not found, can't update emoticon!!");
        return;
    }
    
    int historical = 0;
    int current = 0;
    
    if(currentBar >= 2)
    {
        // If it is 2am or later we can just access the previous 
        // and previous-previous entries by index directly
        historical += g_Canvas.arrayBars[currentBar-1].previousPercentage;
        current += g_Canvas.arrayBars[currentBar-1].currentPercentage;
        historical += g_Canvas.arrayBars[currentBar-2].previousPercentage;
        current += g_Canvas.arrayBars[currentBar-2].currentPercentage;
    }
    else if(currentBar == 1)
    {
        // If it is 1am then we can access midnight (0000) but not 2300 so
        // manually wrap around to the last element
        historical += g_Canvas.arrayBars[0].previousPercentage;
        current += g_Canvas.arrayBars[0].currentPercentage;
        historical += g_Canvas.arrayBars[CANVAS_NUMBER_BARS_X-1].previousPercentage;
        current += g_Canvas.arrayBars[CANVAS_NUMBER_BARS_X-1].currentPercentage;
    }
    else
    {
        // It must be midnight (0000) which means we are the first element in the collection
        // so we can't access previous entries in the array. Instead manuall access entries
        // at the end of the collection
        historical += g_Canvas.arrayBars[CANVAS_NUMBER_BARS_X-1].previousPercentage;
        current += g_Canvas.arrayBars[CANVAS_NUMBER_BARS_X-1].currentPercentage;
        historical += g_Canvas.arrayBars[CANVAS_NUMBER_BARS_X-2].previousPercentage;
        current += g_Canvas.arrayBars[CANVAS_NUMBER_BARS_X-2].currentPercentage;
    }
    
    float currentAverage = (float)(current / 2.0);
    float historicalAverage = (float)(historical / 2.0);
    
    int percentageTarget = (int)( (currentAverage/historicalAverage) * 100.0);
    
    if(percentageTarget < 10)
    {
        happinessLevel = happinessLevel_asleep;
    }
    else if(percentageTarget < 50)
    {
        happinessLevel = happinessLevel_veryunhappy;
    }
    else if(percentageTarget < 80)
    {
        happinessLevel = happinessLevel_unhappy;
    }
    else if(percentageTarget < 105)
    {
        happinessLevel = happinessLevel_happy;
    }
    else if(percentageTarget < 150)
    {
        happinessLevel = happinessLevel_veryhappy;
    }
    else
    {
        happinessLevel = happinessLevel_wow;    
    }
    
#if DEBUG_HAPPINESS
    APP_LOG(APP_LOG_LEVEL_INFO, "Historical %d current %d, pc target %d, happiness level %d", historical, current, percentageTarget, happinessLevel);
#endif
    
    switch(happinessLevel)
    {
        case happinessLevel_asleep:
            strcpy(&g_emoji[0], "\U0001F634");        // sleeping
        break;
        
        case happinessLevel_veryunhappy:
            switch(GetRandomNumber(0,2))
            {
            case 0:
                strcpy(&g_emoji[0], "\U0001F620");        // angry face
                break;
                
            case 1:
                strcpy(&g_emoji[0], "\U0001F621");        // very angry face
                break;
                
            case 2:
            default:
                strcpy(&g_emoji[0], "\U0001F628");        // sad face
                break;
            }
            break;
        
        case happinessLevel_unhappy:
            switch(GetRandomNumber(0,2))
            {
            case 0:
                strcpy(&g_emoji[0], "\U0001F626");        // unhappy face
                break;
                
            case 1:
                strcpy(&g_emoji[0], "\U0001F615");        // sideways unhappy face
                break;
                
            case 2:
            default:
                strcpy(&g_emoji[0], "\U0001F61F");        // sad face
                break;
            }
            break;
        
        case happinessLevel_happy:
            switch(GetRandomNumber(0,2))
            {
            case 0:
                strcpy(&g_emoji[0], "\U0001F603");        // happy smiling face
                break;
                
            case 1:
                strcpy(&g_emoji[0], "\U0001F604");        // happy face
                break;
                
            case 2:
            default:
                strcpy(&g_emoji[0], "\U0001F62C");        // teeth happy face
                break;
            }
            break;
        
        case happinessLevel_veryhappy:
            switch(GetRandomNumber(0,2))
            {
            case 0:
                strcpy(&g_emoji[0], "\U0001F60E");        // cool face
                break;
                
            case 1:
                strcpy(&g_emoji[0], "\U0001F600");        // big smile face
                break;
                
            case 2:
            default:
                strcpy(&g_emoji[0], "\U0001F605");        // very happy face
                break;
            }
            break;
        
        case happinessLevel_wow:
            switch(GetRandomNumber(0,1))
            {
            case 0:
                strcpy(&g_emoji[0], "\U0001F631");        // open mouthed surprised face
                break;
                
            default:
            case 1:
                strcpy(&g_emoji[0], "\U0001F61D");        // thumbs up
                break;
            }
            break;
        
        default:
            strcpy(&g_emoji[0], "\U0001F636");        // unknown face
            break;
    }   
}