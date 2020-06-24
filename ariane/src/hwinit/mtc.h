#ifndef _HWINIT_MTC_H_
#define _HWINIT_MTC_H_

#include "types.h"
//negative targetFreq => periodic training is allowed as parameter, as return value periodic training is required
int mtc_perform_memory_training(int targetFreq);
//performs periodic training if the time interval since last one is greater than periodic training timeout
u32 mtc_redo_periodic_training(u32 lastPeriodicMs);

#endif
