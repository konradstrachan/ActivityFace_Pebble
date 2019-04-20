#include <pebble.h>
#include "msgexchange.h"

#include "main.h"
#include "watchface.h"
#include "shared.h"
#include "persistence.h"

void OnMXCInit()
{
    g_configHasReceivedConfiguration = false;
    
    // Register callbacks
    app_message_register_inbox_received(OnMXCInbox_received_callback);
    app_message_register_inbox_dropped(OnMXCInbox_dropped_callback);
    app_message_register_outbox_failed(OnMXCOubox_failed_callback);
    app_message_register_outbox_sent(OnMXCOubox_sent_callback);
    
    // Open AppMessage
    app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

void OnMXCDeInit()
{    
}

void OnMXCInbox_received_callback(DictionaryIterator* iterator, void* context)
{
    APP_LOG(APP_LOG_LEVEL_INFO, "Received options data");
    g_configHasReceivedConfiguration = true;
    
    // Based on example implmentation at https://developer.pebble.com/guides/user-interfaces/app-configuration/

    Tuple* configShowEmojiTpl = dict_find(iterator, MESSAGE_KEY_CONFIG_SHOWEMOJI);
    if(configShowEmojiTpl) 
    {
        g_configShowEmoji = configShowEmojiTpl->value->int32 == 1;
        APP_LOG(APP_LOG_LEVEL_INFO, "Decoded MESSAGE_KEY_CONFIG_SHOWEMOJI : %d -> %d", (int)configShowEmojiTpl->value->int32, g_configShowEmoji);
    }
    else
    {
        APP_LOG(APP_LOG_LEVEL_ERROR, "MESSAGE_KEY_CONFIG_SHOWEMOJI not found");
    }

    Tuple* configUseAltDateFormatTpl = dict_find(iterator, MESSAGE_KEY_CONFIG_DATEFORMAT);
    if(configUseAltDateFormatTpl) 
    {
        bool newValueForConfigOption = strcmp(configUseAltDateFormatTpl->value->cstring, "1" ) == 0;

        if( g_configUseAlternativeDateFormat != newValueForConfigOption )    
        {
            // Date setting has changed meaning we need to update the array holidng the date
            
            APP_LOG(APP_LOG_LEVEL_INFO, "Config value for date has changed, triggering time refresh");
            g_configUseAlternativeDateFormat = newValueForConfigOption;
            InitialiseTimeData();
        }
        
        APP_LOG(APP_LOG_LEVEL_INFO, "Decoded MESSAGE_KEY_CONFIG_DATEFORMAT : %d -> %d", (int)configUseAltDateFormatTpl->value->int32, g_configUseAlternativeDateFormat);
    }
    else
    {
        APP_LOG(APP_LOG_LEVEL_ERROR, "MESSAGE_KEY_CONFIG_DATEFORMAT not found");
    }

    APP_LOG(APP_LOG_LEVEL_INFO, "Decoded options data : Show Emoji %d Date format %d", g_configShowEmoji, g_configUseAlternativeDateFormat);
    
    // Things may need to be changed on the screen, refresh the screen
    RequestDisplayRefresh();
    
    // Write the data we've just received in to persisted data so we don't lose it on the next restart
    WritePersistedOptionData();
}

void OnMXCInbox_dropped_callback(AppMessageResult reason, void *context)
{
    APP_LOG(APP_LOG_LEVEL_WARNING, "Receive connection dropped!");
}

void OnMXCOubox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context)
{
    APP_LOG(APP_LOG_LEVEL_ERROR, "Request failed!");
}

void OnMXCOubox_sent_callback(DictionaryIterator *iterator, void *context)
{
    APP_LOG(APP_LOG_LEVEL_INFO, "Sent request...");
}

void GetData()
{
    if(g_configHasReceivedConfiguration)
    {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Call to get configuration data failed as it has already been received!");
        return;    
    }
    
    // Begin dictionary
    DictionaryIterator* iter;
    app_message_outbox_begin(&iter);
    // TODO write the current settings
    dict_write_end(iter);

    // Send the message!
    app_message_outbox_send();    
}