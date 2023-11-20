#include <stdint.h>
#include "../uwb_common_def.h"
uint16_t RangingResultFitelMedianMean(uint16_t* intput_sequence, uint32_t sequence_size);
uint16_t UWBKalmanFilter(uint16_t* unfiltered, uint32_t unfiltered_size);

void ranging_queue_refresh(ST_Ranging_Data* pst_ranging_dat, uint16_t new_data);
int ranging_data_fixup(uint16_t* last_history_data, uint16_t* indata);
int ranging_data_filter(uint16_t* history_pos, uint16_t* history_neg, uint16_t* bufResult);
