#pragma once

// ----- Function declarations
  
// Standard boiler plate
void handle_init(void);
void handle_deinit(void);
int main(void);

void display_layer_update_callback(Layer* pLayer, GContext* ctx);
void button_config_provider( Window* pWindow );

// Button click callbacks
void back_single_click_handler( ClickRecognizerRef recognizer, void *context );
void down_single_click_handler( ClickRecognizerRef recognizer, void *context );
void middle_single_click_handler( ClickRecognizerRef recognizer, void *context );
void up_single_click_handler( ClickRecognizerRef recognizer, void *context );

// Timing
void OnTimer(void* data);
void RegisterTimerForNextTick();
void OnTimeChange(struct tm *tick_time, TimeUnits units_changed);

// Health API
void OnHealthAPIEvent(HealthEventType event, void *context);

// Rendering
void RequestDisplayRefresh();