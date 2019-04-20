#pragma once

void DrawBar(GContext* ctx, int nBarID);
void DrawRadialBar(GContext* ctx, int nBarID);

void DrawWatchFace(GContext* ctx);

void DrawWatchfacePleaseWaitForHealthData(GContext* ctx);
void DrawWatchfaceFull(GContext* ctx);
void DrawWatchfaceRadial(GContext* ctx);
void DrawWatchfaceCommonParts(GContext* ctx);

uint32_t GetBarMaxHeightInPixels();

void DrawHeartRateData(GContext* ctx);
void DrawLineChart(GContext* ctx, GRect chartPos, uint32_t scaleMin, uint32_t scaleMax, uint32_t* dataPoints, uint32_t numberOfDataPoints, bool inverseDrawingStyle);