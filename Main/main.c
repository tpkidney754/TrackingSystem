#include "includeall.h"
#include <time.h>

struct timespec startTime;
struct timespec stopTime;

int main()
{
    PWM_Init();
    MC_Init();

	clock_gettime(CLOCK_REALTIME, &startTime);
	clock_gettime(CLOCK_REALTIME, &stopTime);
	printf("Hello World at %ld:%ld\n", startTime.tv_sec, startTime.tv_nsec);
}
