#include <pebble.h>
#include "unobstructedarea.h"
#include "main.h"

void OnUnobstructedAreaWillChangeHandler(GRect final_unobstructed_screen_area, void* context)
{
}

void OnUnobstructedAreaChangeHandler(AnimationProgress progress, void* context)
{
}

void OnUnobstructedAreaDidChangeHandler(void* context)
{
    RequestDisplayRefresh();
}