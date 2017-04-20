#include "includeall.h"

int main()
{
    PWM_Init();

    for (uint32_t pwm = 0; pwm <= 100; pwm += 10)
    {
        printf("Changing duty cycle to %d%%\n", pwm );
        PWM_ChangeDutyCycle(pwm);
        for (uint32_t i = 0; i < 0xDEADBEEF; i += 10);
    }
}
