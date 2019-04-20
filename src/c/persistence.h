#pragma once

// Max size of persistent data block is 256 bytes

// 32bit / 8bits in byte = 4bytes
// 256/4 = 64 possible values stored in persisted data as 32bit values

// Store top 16bits = history percentage value
//       bottom 16bits = current pecentage value
// special key for timestamp data was saved
//       - if data more than 1 hour old, discard

void ConvertDataToPersistedInt(uint16_t historical, uint16_t current, uint32_t* resultant);

void ConvertPersistedIntToData(uint32_t data, uint16_t* historical, uint16_t* current);

bool DoesPersistedDataExist();
void LoadPersistedData();
void WritePersistedData();

bool DoesPersistedOptionDataExist();
void LoadPersistedOptionData();
void WritePersistedOptionData();