#pragma once
    
// ----- Type declarations
    
#define APPCONFIGID_SHOWEMOJI 0
#define APPCONFIGID_DATEFORMAT 1
    
// ----- Global declarations

// ----- Function declarations

void GetData();

void OnMXCInit();
void OnMXCDeInit();

void OnMXCInbox_received_callback(DictionaryIterator *iterator, void *context);
void OnMXCInbox_dropped_callback(AppMessageResult reason, void *context);
void OnMXCOubox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context);
void OnMXCOubox_sent_callback(DictionaryIterator *iterator, void *context);
