#pragma once
#include "mutex.h"
typedef struct CondVar {pthread_cond_t value;} CondVar;
void condvarSignal(CondVar*);
void condvarWait(CondVar*,Mutex*);
