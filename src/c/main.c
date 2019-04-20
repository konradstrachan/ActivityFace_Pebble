#include <pebble.h>
#include "main.h"
  
#include "watchface.h"
#include "healthdata.h"
#include "msgexchange.h"
#include "unobstructedarea.h"
#include "shared.h"

void handle_init(void) 
{
    APP_LOG(APP_LOG_LEVEL_DEBUG, "App init");
    
    // Create window
    g_pWindow = window_create();
  
    window_stack_push(g_pWindow, 
                      true);  // animate in the window
  
    window_set_background_color(g_pWindow, GColorBlack);
  
    // Create drawing canvas layer
    Layer* pRootLayer = window_get_root_layer(g_pWindow);
    GRect frame = layer_get_frame(pRootLayer);
  
    // Create canvas layer
    g_pCanvasLayer = layer_create(frame);

    // append the canvas layer to the root layer of the window
    layer_add_child(pRootLayer, g_pCanvasLayer);
  
    // register for painting of canvas layer
    layer_set_update_proc(g_pCanvasLayer, &display_layer_update_callback);
  
    // register with the time handler
    tick_timer_service_subscribe(MINUTE_UNIT, OnTimeChange);
    
    // register SDK4 unobstructed area handlers
    UnobstructedAreaHandlers handlers = { 
        .will_change = OnUnobstructedAreaWillChangeHandler,
        .change = OnUnobstructedAreaChangeHandler,
        .did_change = OnUnobstructedAreaDidChangeHandler
    };

    unobstructed_area_service_subscribe(handlers, NULL);
  
    // Feed the random number generator
    srand( time( NULL ) );
  
    // Finally initialise the watchface
    OnWatchfaceInit();
     
    // initialise message exchange for config data
    OnMXCInit();
  
    // and finally finally, initialise the timer
    RegisterTimerForNextTick();
     
#if defined(PBL_HEALTH)
    // Attempt to subscribe 
    if(!health_service_events_subscribe(OnHealthAPIEvent, NULL)) 
    {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Health not available!");
    }
#else
    APP_LOG(APP_LOG_LEVEL_ERROR, "Health not available!");
    // Fake the call so the app is not stuck on the "just a moment please" screen
    OnHealthAPIEvent(HealthEventSignificantUpdate, NULL);
#endif
}

void display_layer_update_callback(Layer* pLayer, GContext* ctx) 
{
    if( pLayer == g_pCanvasLayer )
    {   
        OnWatchfacePaint( ctx );
    }
}

void OnTimer(void* data)
{  
    // trigger logic
    OnWatchfaceTick();
  
    // Trigger redraw
    layer_mark_dirty( g_pCanvasLayer );
    
    // Finally register for next callback
    RegisterTimerForNextTick();
}

void RegisterTimerForNextTick()
{
    g_pTimer = app_timer_register( 1000 * 60 * 10, // every 10 minutes
                                 (AppTimerCallback) OnTimer, 
                                 NULL);    // Nothing to pass in to callback
}

void handle_deinit(void)
{
    APP_LOG(APP_LOG_LEVEL_DEBUG, "App de-init");
    
    // Deregister for callbacks
    app_timer_cancel( g_pTimer );
    health_service_events_unsubscribe();
    unobstructed_area_service_unsubscribe();
    
    // Let the watchface do any custom de-init work
    OnWatchfaceDeInit();
    
    OnMXCDeInit();
    
    // Let's clean up after ourselves nicely
    layer_destroy( g_pCanvasLayer );
    window_stack_pop_all(true);
    window_destroy( g_pWindow );
}

int main(void) 
{
    handle_init();
    app_event_loop();
    handle_deinit();
}

void OnTimeChange(struct tm *tick_time, TimeUnits units_changed)
{
    if((units_changed & MINUTE_UNIT) == 0)
    {
        return;    
    }
    
    if(    g_nTimeHour == tick_time->tm_hour 
        && g_nTimeMinute == tick_time->tm_min )
    {
        return;    
    }
    
    g_nTimeHour = tick_time->tm_hour;
    g_nTimeMinute = tick_time->tm_min;
    
    OnTimeUpdated(tick_time);
      
    // Trigger redraw
    RequestDisplayRefresh();
}

void OnHealthAPIEvent(HealthEventType event, void* context)
{
    switch(event) 
    {
    case HealthEventSignificantUpdate:
        // From SDK: All data is considered as outdated, apps should re-read all health data. This can happen 
        // on a change of the day or in other cases that significantly change the underlying data.
        RefreshAllHealthData();
        break;
        
    case HealthEventMovementUpdate:
        // From SDK: Recent values around HealthMetricStepCount, HealthMetricActiveSeconds, 
        // HealthMetricWalkedDistanceMeters, and HealthActivityMask changed.
        
        // Only update data from the last hour
        UpdateLastHourOfHealthData();
        break;
        
    case HealthEventMetricAlert:
        // Ignore sleep data
        break;
        
    case HealthEventHeartRateUpdate:
        // Ignore heart rate data
        UpdateHeartRateData(true);
        break;
        
    case HealthEventSleepUpdate:
        // Do nothing
        break;
    }
}

void RequestDisplayRefresh()
{
    layer_mark_dirty( g_pCanvasLayer );
}