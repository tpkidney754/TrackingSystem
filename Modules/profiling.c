#include "profiling.h"

static timespec_t startTime;
static timespec_t stopTime;
static timespec_t elapsedTime;

void PRO_RunProfilingSuite(void)
{
    // Updating the pulsewidth modulation
    clock_gettime(CLOCK_REALTIME, &startTime);
    PWM_ChangeDutyCycle(100, 3);
    PWM_ChangeDutyCycle(100, 4);
    clock_gettime(CLOCK_REALTIME, &stopTime);
    PRO_delta(&startTime, &stopTime, &elapsedTime);
    printf("Changing the PWM for both motors took %d microseconds\n",
            (uint32_t) elapsedTime.tv_nsec/1000);

    //Changing to clockwise time
    clock_gettime(CLOCK_REALTIME, &startTime);
    MC_CircleClockwise();
    clock_gettime(CLOCK_REALTIME, &stopTime);
    PRO_delta(&startTime, &stopTime, &elapsedTime);
    printf("Changing direction to clockwise took %d microseconds\n",
            (uint32_t) elapsedTime.tv_nsec/1000);

    //Changing to counterclockwise time
    clock_gettime(CLOCK_REALTIME, &startTime);
    MC_CircleCounterClockwise();
    clock_gettime(CLOCK_REALTIME, &stopTime);
    PRO_delta(&startTime, &stopTime, &elapsedTime);
    printf("Changing direction to counter-clockwise took %d microseconds\n",
            (uint32_t) elapsedTime.tv_nsec/1000);

    //Stopping the motors
    clock_gettime(CLOCK_REALTIME, &startTime);
    MC_Stop();
    clock_gettime(CLOCK_REALTIME, &stopTime);
    PRO_delta(&startTime, &stopTime, &elapsedTime);
    printf("Stopping both motors took %d microseconds\n",
            (uint32_t) elapsedTime.tv_nsec/1000);
}

void PRO_delta(struct timespec *start, struct timespec *stop, struct timespec *delta_t)
{
    int dt_sec=stop->tv_sec - start->tv_sec;
    int dt_nsec=stop->tv_nsec - start->tv_nsec;

    if(dt_sec >= 0)
    {
        if(dt_nsec >= 0)
        {
            delta_t->tv_sec=dt_sec;
            delta_t->tv_nsec=dt_nsec;
        }
        else
        {
            delta_t->tv_sec=dt_sec-1;
            delta_t->tv_nsec=NSEC_PER_SEC+dt_nsec;
        }
    }
    else
    {
        if(dt_nsec >= 0)
        {
            delta_t->tv_sec=dt_sec;
            delta_t->tv_nsec=dt_nsec;
        }
        else
        {
            delta_t->tv_sec=dt_sec-1;
            delta_t->tv_nsec=NSEC_PER_SEC+dt_nsec;
        }
    }

}
