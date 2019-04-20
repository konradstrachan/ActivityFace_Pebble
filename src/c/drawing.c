#include <pebble.h>
#include "drawing.h"
#include "persistence.h"
#include "shared.h"

#define CANVAS_BAR_WIDTH_PREVIOUS 11
#define CANVAS_BAR_WIDTH_CURRENT 8
#define CANVAS_BAR_BUFFER 1
#define CANVAS_BAR_WIDTH_AND_BUFFER 12

#define CANVAS_NUMBER_BARS_X_ROW 12
  
#define CANVAS_BAR_HEIGHT_MAX_PX 52

void DrawWatchFace(GContext* ctx)
{
    if(!g_initialLoadComplete && !DoesPersistedDataExist())
    {
        DrawWatchfacePleaseWaitForHealthData(ctx);    
    }
    else
    {
#if defined(PBL_PLATFORM_CHALK)
        DrawWatchfaceRadial(ctx);
#else
        DrawWatchfaceFull(ctx);
#endif
    }
}

int GetOffsetFromLeftdge()
{
#if defined(PBL_RECT)
  return 0;
#elif defined(PBL_ROUND)
  return 18;
#endif
}

int GetOffsetFromTopEdge()
{
#if defined(PBL_RECT)
  return 0;
#elif defined(PBL_ROUND)
  return 6;
#endif
}

int GetWidthOfScreen()
{
#if defined(PBL_RECT)
  return 144;
#elif defined(PBL_ROUND)
  return 180;
#endif
}

uint32_t GetBarMaxHeightInPixels()
{
    GRect unrestricted_bounds = layer_get_bounds(g_pCanvasLayer);
    GRect restricted_bounds = layer_get_unobstructed_bounds(g_pCanvasLayer);
    uint32_t amountToShrinkBars = ( unrestricted_bounds.size.h - restricted_bounds.size.h ) / 2.0;
    return CANVAS_BAR_HEIGHT_MAX_PX - amountToShrinkBars;
}

int GetHeightOfScreen()
{
    GRect bounds = layer_get_unobstructed_bounds(g_pCanvasLayer);
    return bounds.size.h;
}

void DrawWatchfacePleaseWaitForHealthData(GContext* ctx)
{
    GFont timeFont = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);
    GRect textRect;
    
    textRect.origin.x = 0;
    textRect.origin.y = GetOffsetFromTopEdge() + CANVAS_BAR_HEIGHT_MAX_PX - 2 - 10;
    textRect.size.w = GetWidthOfScreen();
    textRect.size.h = GetOffsetFromTopEdge() + 10;
    
    graphics_context_set_text_color(ctx, g_coltopColor2);
    graphics_draw_text(ctx, 
                       "Reading HealthAPI data",
                       timeFont,
                       textRect,
                       GTextOverflowModeTrailingEllipsis,
                       GTextAlignmentCenter,
                       NULL);
    
    graphics_context_set_text_color(ctx, GColorWhite);
    textRect.origin.y += 20;

    graphics_draw_text(ctx, 
                       "\U0001F62C",
                       fonts_get_system_font(FONT_KEY_GOTHIC_28),
                       textRect,
                       GTextOverflowModeTrailingEllipsis,
                       GTextAlignmentCenter,
                       NULL);
    
    graphics_context_set_text_color(ctx, g_colbottomColor2);
    textRect.origin.y += 40;

    graphics_draw_text(ctx, 
                       "Just a moment please...",
                       timeFont,
                       textRect,
                       GTextOverflowModeTrailingEllipsis,
                       GTextAlignmentCenter,
                       NULL);
}

void DrawWatchfaceCommonParts(GContext* ctx)
{
    GRect textRect;
    
    //             
    // Draw the time
    //

    GFont timeFont = fonts_get_system_font(FONT_KEY_LECO_36_BOLD_NUMBERS);
    
    textRect.origin.x = GetOffsetFromLeftdge();
    textRect.origin.y = GetOffsetFromTopEdge() + GetBarMaxHeightInPixels() - 2;
    textRect.size.w = 144;
    textRect.size.h = GetOffsetFromTopEdge() + 10;
    
    graphics_context_set_text_color(ctx, GColorWhite);
    graphics_draw_text(ctx, 
                       &g_time[0],
                       timeFont,
                       textRect,
                       GTextOverflowModeTrailingEllipsis,
                       GTextAlignmentCenter,
                       NULL);
    //
    // Draw the date
    //
    
    GFont dateFont = fonts_get_system_font(FONT_KEY_LECO_20_BOLD_NUMBERS);
    
    textRect.origin.x = GetOffsetFromLeftdge();
    
    if(g_configShowEmoji)
    {
        // When drawing the emoji, indent the date to make space
        textRect.origin.x += 24;
    }
    
    textRect.origin.y = GetOffsetFromTopEdge() + GetBarMaxHeightInPixels() + 35 + 1;
    textRect.size.w = GetOffsetFromLeftdge() + 144 - textRect.origin.x;
    textRect.size.h = GetOffsetFromTopEdge() + 10;
    
    graphics_context_set_text_color(ctx, GColorWhite);
    graphics_draw_text(ctx, 
                       &g_date[0],
                       dateFont,
                       textRect,
                       GTextOverflowModeTrailingEllipsis,
                       GTextAlignmentCenter,
                       NULL);
    
    //
    // Draw the emoji
    //
    
    if(g_configShowEmoji)
    {
        GFont emojiFont = fonts_get_system_font(FONT_KEY_GOTHIC_28);
        
        textRect.origin.x = GetOffsetFromLeftdge() + 30;
        textRect.origin.y = GetOffsetFromTopEdge() + GetBarMaxHeightInPixels() + 35 - 6;
        textRect.size.w = GetOffsetFromLeftdge() + 40;
        textRect.size.h = GetOffsetFromTopEdge() + 10;
        
        graphics_context_set_text_color(ctx, GColorWhite);
        graphics_draw_text(ctx, 
                           &g_emoji[0],
                           emojiFont,
                           textRect,
                           GTextOverflowModeTrailingEllipsis,
                           GTextAlignmentLeft,
                           NULL);
    }
}

void DrawWatchfaceRadial(GContext* ctx)
{
    // Only for the chalk platform
#ifndef PBL_PLATFORM_CHALK
    APP_LOG(APP_LOG_LEVEL_ERROR, "Called DrawWatchfaceRadial for non-Chalk platform!");
    return;
#endif
    
    //
    // Draw the bars radially
    //
    
    for( int nBarId = 0; nBarId < CANVAS_NUMBER_BARS_X ; ++nBarId )
    {     
        DrawRadialBar(ctx, 
                      nBarId);
    }
    
    DrawWatchfaceCommonParts(ctx);
}

void DrawWatchfaceFull(GContext* ctx)
{
    //
    // Draw the bars
    //
    
    for( int nBarId = 0; nBarId < CANVAS_NUMBER_BARS_X ; ++nBarId )
    {     
        DrawBar(ctx, 
                nBarId);
    }
    
    //
    // Draw the lines for the bottom of the first row and top of the second row and labels
    //
    
    // Top line
    GPoint topLinePoint1;
    topLinePoint1.x = GetOffsetFromLeftdge();
    topLinePoint1.y = GetOffsetFromTopEdge() + GetBarMaxHeightInPixels();
    
    GPoint topLinePoint2;
    topLinePoint2.x = GetOffsetFromLeftdge() + 144;
    topLinePoint2.y = GetOffsetFromTopEdge() + GetBarMaxHeightInPixels();
    
    graphics_context_set_stroke_color(ctx, g_coltopColor1);
    graphics_draw_line(ctx, topLinePoint1, topLinePoint2);
    
    // Bottom line
    GPoint bottomLinePoint1;
    bottomLinePoint1.x = GetOffsetFromLeftdge();
    bottomLinePoint1.y = GetOffsetFromTopEdge() + 168 - GetBarMaxHeightInPixels() - 1;
    
    GPoint bottomLinePoint2;
    bottomLinePoint2.x = GetOffsetFromLeftdge() + 144;
    bottomLinePoint2.y = GetOffsetFromTopEdge() + 168 - GetBarMaxHeightInPixels() - 1;
    
    graphics_context_set_stroke_color(ctx, g_colbottomColor1);
    graphics_draw_line(ctx, bottomLinePoint1, bottomLinePoint2);
    
    // Draw the labels
    
    GFont smallFont = fonts_get_system_font(FONT_KEY_GOTHIC_14);
    const int massageFactor = 2;
    
    static char lineTopBuffer[44] = {'\0'};
    static char lineBottomBuffer[44] = {'\0'};
    
    if(clock_is_24h_style())
    {
        strcpy(&lineTopBuffer[0], "0                                       11\0");
        strcpy(&lineBottomBuffer[0], "12                                       23\0");
    }
    else
    {
        strcpy(&lineTopBuffer[0], "AM                                         \0");
        strcpy(&lineBottomBuffer[0], "                                        PM\0");
    }
    
    GRect textRect;
    textRect.origin.x = 0;
    textRect.origin.y = GetOffsetFromTopEdge() + GetBarMaxHeightInPixels() - massageFactor;
    textRect.size.w = GetWidthOfScreen();
    textRect.size.h = GetOffsetFromTopEdge() + 10;
    
    graphics_context_set_text_color(ctx, g_coltopColor1);
    graphics_draw_text(ctx, 
                       &lineTopBuffer[0],
                       smallFont,
                       textRect,
                       GTextOverflowModeTrailingEllipsis,
                       GTextAlignmentCenter,
                       NULL);
    
    textRect.origin.y = GetOffsetFromTopEdge() + GetHeightOfScreen() - GetBarMaxHeightInPixels() - 20 + massageFactor + 1;
    textRect.size.h = GetOffsetFromTopEdge() + 10;
    
    graphics_context_set_text_color(ctx, g_colbottomColor1);
    graphics_draw_text(ctx, 
                       &lineBottomBuffer[0],
                       smallFont,
                       textRect,
                       GTextOverflowModeTrailingEllipsis,
                       GTextAlignmentCenter,
                       NULL);

    DrawWatchfaceCommonParts(ctx);
    
    //
    // Draw heart rate information (if required)
    //
    
    DrawHeartRateData(ctx);
}

void DrawBar(GContext* ctx, int nBarID)
{   
    bool firstRow = true;
    
    const int nYPosRow1StartOrigin = GetOffsetFromTopEdge() + GetBarMaxHeightInPixels();
    const int nYPosRow2StartOrigin = GetOffsetFromTopEdge() + GetHeightOfScreen() - GetBarMaxHeightInPixels(); // height of screen minus bar max height
    
    int nXPosOriginPx = GetOffsetFromLeftdge() + (nBarID * CANVAS_BAR_WIDTH_AND_BUFFER);
    int nYPosOriginPx = nYPosRow1StartOrigin;
        
    if(nBarID >= CANVAS_NUMBER_BARS_X_ROW)
    {
        // This means this is the second row    
        firstRow = false;
        
        nXPosOriginPx = GetOffsetFromLeftdge() + ((nBarID - CANVAS_NUMBER_BARS_X_ROW) * CANVAS_BAR_WIDTH_AND_BUFFER);
        nYPosOriginPx = nYPosRow2StartOrigin;
    }
    
    const float nPercentageFilledPrevious = (g_Canvas.arrayBars[nBarID].previousPercentage) / 100.0;
    const int nHeightOfBarPxPrevious = (int)(GetBarMaxHeightInPixels() * nPercentageFilledPrevious);
    
    GRect square;
    
    //
    // Draw previous data
    //
    
    GColor colour = g_coltopColor1;
    
    if(firstRow)
    {
        colour = g_coltopColor1;
    }
    else
    {
        colour = g_colbottomColor1;
    }

    if(firstRow)
    {
        square.origin = GPoint(nXPosOriginPx, nYPosOriginPx - nHeightOfBarPxPrevious);    
    }
    else
    {
        square.origin = GPoint(nXPosOriginPx, nYPosOriginPx);
    }

    square.size = GSize(CANVAS_BAR_WIDTH_PREVIOUS, nHeightOfBarPxPrevious);

    graphics_context_set_fill_color(ctx, colour);
    graphics_fill_rect( ctx, square, 0, GCornerNone );

    //
    // Draw current data
    //
    
    const float nPercentageFilledCurrent = (g_Canvas.arrayBars[nBarID].currentPercentage) / 100.0;
    const int nHeightOfBarPxCurrent = (int)(GetBarMaxHeightInPixels() * nPercentageFilledCurrent);
    
    if(firstRow)
    {
        colour = g_coltopColor2;
    }
    else
    {
        colour = g_colbottomColor2;
    }
    
    if(firstRow)
    {
        square.origin = GPoint(nXPosOriginPx, nYPosOriginPx - nHeightOfBarPxCurrent);    
    }
    else
    {
        square.origin = GPoint(nXPosOriginPx, nYPosOriginPx);
    }
    
    square.size = GSize(CANVAS_BAR_WIDTH_CURRENT, nHeightOfBarPxCurrent);

    if(g_Canvas.arrayBars[nBarID].incomplete)
    {
        // Data still coming in for this bar, draw it in an active colour
        colour = GColorWhite;
        graphics_context_set_fill_color(ctx, colour);
    }

    graphics_context_set_fill_color(ctx, colour);
    graphics_fill_rect( ctx, square, 0, GCornerNone );
}

void DrawRadialBar(GContext* ctx, int nBarID)
{   
    GRect screenRect;
    screenRect.origin.x = 0;
    screenRect.origin.y = 0;
    screenRect.size.w = GetWidthOfScreen();
    screenRect.size.h = GetHeightOfScreen();
    
    const int32_t angleStep = 360 / 24.0;
    int32_t angleStart = angleStep * nBarID;
    int32_t angleEndPrevious = (angleStep * (nBarID + 1)) - 1;
    int32_t angleEndCurrent = (angleStep * (nBarID + 1)) - 4;
    
    GColor colour = g_coltopColor1;
    
    if(nBarID >= 12)
    {
        colour = g_colbottomColor1;
    }
    
    //
    // Previous
    //
    
    const uint16_t maxThickness = 36;
    
    const float nPercentageFilledPrevious = (g_Canvas.arrayBars[nBarID].previousPercentage) / 100.0;
    uint16_t thickness = (uint16_t)(nPercentageFilledPrevious * maxThickness);
    
    graphics_context_set_fill_color(ctx, colour);
    graphics_fill_radial(ctx, screenRect, GOvalScaleModeFitCircle, thickness, DEG_TO_TRIGANGLE(angleStart), DEG_TO_TRIGANGLE(angleEndPrevious));
    
    // 
    // Current
    //
    
    if(g_Canvas.arrayBars[nBarID].incomplete)
    {
        // Data still coming in for this bar, draw it in an active colour
        colour = GColorWhite;
    }
    else if(nBarID < 12)
    {
        colour = g_coltopColor2;
    }
    else
    {
        colour = g_colbottomColor2;
    }
    
    const float nPercentageFilledCurrent = (g_Canvas.arrayBars[nBarID].currentPercentage) / 100.0;
    thickness = (uint16_t)(nPercentageFilledCurrent * maxThickness);
    
    graphics_context_set_fill_color(ctx, colour);
    graphics_fill_radial(ctx, screenRect, GOvalScaleModeFitCircle, thickness, DEG_TO_TRIGANGLE(angleStart), DEG_TO_TRIGANGLE(angleEndCurrent));
    
}

void DrawHeartRateData(GContext* ctx)
{
    const uint32_t graphBpmMin = 00;
    const uint32_t graphBpmMax = 160;
    
    GRect topChart;
    topChart.origin.x = 0;
    topChart.origin.y = 0;
    topChart.size.w = GetWidthOfScreen();
    topChart.size.h = GetBarMaxHeightInPixels();

    uint32_t sizeOfData = sizeof(g_HeartRateData.heartRate)/sizeof(uint32_t);
    uint32_t halfSizeOfData = sizeOfData / 2;
    
    DrawLineChart(ctx, topChart, graphBpmMin, graphBpmMax, &g_HeartRateData.heartRate[0], halfSizeOfData, false);
    
    GRect bottomChart;
    bottomChart.origin.x = 0;
    bottomChart.origin.y = GetHeightOfScreen() - GetBarMaxHeightInPixels();
    bottomChart.size.w = GetWidthOfScreen();
    bottomChart.size.h = GetBarMaxHeightInPixels();
    
    DrawLineChart(ctx, bottomChart, graphBpmMin, graphBpmMax, &g_HeartRateData.heartRate[halfSizeOfData], halfSizeOfData, true);
}

void DrawLineChart(GContext* ctx, GRect chartPos, uint32_t scaleMin, uint32_t scaleMax, uint32_t* dataPoints, uint32_t numberOfDataPoints, bool inverseDrawingStyle)
{
    APP_LOG(APP_LOG_LEVEL_INFO,"Drawing chart (min %d, max %d, number of data points %d, inverse %d)", (int)scaleMin, (int)scaleMax, (int)numberOfDataPoints, inverseDrawingStyle == true ? 1 : 0 );
    
    const uint32_t widthOfSingleDataPoint = chartPos.size.w / numberOfDataPoints;
    const uint32_t verticalRange = scaleMax - scaleMin;
    uint32_t currentXPosPx = chartPos.origin.x;
    
    for( uint32_t dataPoint = 0; dataPoint < numberOfDataPoints; ++dataPoint )
    {
        if(*dataPoints == 0)
        {
            // TODO: Break instead?
            continue;    
        }
        
        uint32_t normalisedDataPoint = *dataPoints;
        
        // Normalise the data so if it is below or above the thresholds we can still do something with it
        // TODO - perhaps it's better to discard it?
        if(normalisedDataPoint < scaleMin )
        {
            normalisedDataPoint = scaleMin;
        }
        
        if(normalisedDataPoint > scaleMax )
        {
            normalisedDataPoint = scaleMax;
        }
        
        const uint32_t posYPx = chartPos.origin.y + ((normalisedDataPoint - scaleMin) / (double)verticalRange) * (double)chartPos.size.h;
        
        graphics_context_set_stroke_color(ctx, GColorWhite);
        GPoint p1;
        GPoint p2;
        
        p1.x = currentXPosPx;       
        p2.x = currentXPosPx + widthOfSingleDataPoint;

        if(inverseDrawingStyle)
        {
            // For bottom graph drawing top to bottom
            p1.y = posYPx;
            p2.y = posYPx;
        }
        else
        {
            // For top graph drawing bottom to top
            p1.y = chartPos.size.h - posYPx;
            p2.y = chartPos.size.h - posYPx;
        }
        
        // TODO for monochrome draw inverse pixel
        graphics_draw_line(ctx, p1, p2);
        
        // Increment pointer for the next data point
        ++dataPoints;
        currentXPosPx += widthOfSingleDataPoint;
    }
}