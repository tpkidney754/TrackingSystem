#include "pwm.h"

void PWM_Init()
{
    FILE *file = fopen(CAPE_MANAGER_FILE, "w");

    if (file == NULL)
    {
        // @todo: Change to syslog and create an assert.
        printf("Error opening file!\n");
        exit(1);
    }

    fprintf(file, ENABLE_CAPE_MANAGER);

    fclose(file);

    system("config-pin P9.14 pwm");
    // @todo: Check that file /sys/devices/ocp.*/P9_14_pinumx.*/state reads pwm

    file = fopen("/sys/class/pwm/export", "w");
    if (file == NULL)
    {
        // @todo: Change to syslog and create an assert.
        printf("Error opening file!\n");
        exit(1);
    }
    fprintf(file, "3");

    fclose(file);

    file = fopen("/sys/class/pwm/pwm3/period_ns", "w");
    fprintf(file, "50000");
    fclose(file);

    file = fopen("/sys/class/pwm/pwm3/duty_ns", "w");
    fprintf(file, "25000");
    fclose(file);

    file = fopen("/sys/class/pwm/pwm3/run", "w");
    fprintf(file, "1");
    fclose(file);
}

void PWM_ChangeDutyCycle(uint32_t dutyCycle, uint32_t pin)
{
    FILE *file;
    switch (pin)
    {
       case 0:
            file = fopen("/sys/class/pwm/pwm3/duty_ns", "w");
            fprintf(file, "%d", (PWM_PERIOD_NS * dutyCycle));
            fclose(file);
            break;
        case 1:
            file = fopen("/sys/class/pwm/pwm4/duty_ns", "w");
            fprintf(file, "%d", (PWM_PERIOD_NS * dutyCycle));
            fclose(file);
            break;
    }
}
