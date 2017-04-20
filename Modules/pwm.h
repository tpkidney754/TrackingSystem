#ifndef __PWM__
#define __PWM__

#include "includeall.h"

// Global definitions
#define CAPE_MANAGER_FILE      "/sys/devices/bone_capemgr.9/slots"
#define ENABLE_CAPE_MANAGER    "cape-universaln"
#define PWM_PERIOD_NS          500

// Function prototypes
void PWM_Init();
void PWM_ChangeDutyCycle(uint32_t dutyCycle);
#endif // __PWM__
