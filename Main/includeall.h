/*#if __STDC_VERSION__ >= 199901L
#define _XOPEN_SOURCE 600
#else
#define _XOPEN_SOURCE 500
#endif /* __STDC_VERSION__ */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <string.h>

#include "motorcontroller.h"
#include "pwm.h"
#include "profiling.h"

#define HRES 640
#define VRES 480
// Defined values
#define TIMER_S      0
#define TIMER_NS     100000000
#define CYCLE_RUNS   20
// freq between 1 to 10 Hz
#define CAPTURE_FREQ 1
#define MOTOR_FREQ   1
#define SYNC_FREQ    10
