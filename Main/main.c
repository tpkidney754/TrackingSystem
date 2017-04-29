#include "includeall.h"

static void Initialization();
static void PrintIntructions();

int main()
{
    Initialization();
    static uint32_t pwm = 75;

    char input;
    char newline;
    PrintIntructions();
    while(1)
    {
        printf("Please enter a command: ");
        scanf("%c%c", &input, &newline);

        switch (input)
        {
            case 'w':
                MC_Forward(0);
                MC_Forward(1);
                break;
            case 's':
                MC_Reverse(0);
                MC_Reverse(1);
                break;
            case 'd':
                MC_CircleClockwise();
                break;
            case 'a':
                MC_CircleCounterClockwise();
                break;
            case 'i':
                if (pwm <= 100)
                {
                    pwm += 5;
                    PWM_ChangeDutyCycle(pwm, 0);
                    PWM_ChangeDutyCycle(pwm, 1);
                }
                break;
            case 'k':
                if (pwm >= 75)
                {
                    pwm -= 5;
                    PWM_ChangeDutyCycle(pwm, 0);
                    PWM_ChangeDutyCycle(pwm, 1);
                }
                break;
            case ' ':
                MC_Stop();
                break;
            case 'q':
                MC_Stop();
                exit(0);
                break;
            case 'p':
                PrintIntructions();
                break;
            default:
                printf("%c is an invalid command.\n", input);
                PrintIntructions();
        }
    }
}

void Initialization()
{
    PWM_Init();
    MC_Init();
}

void PrintIntructions()
{
    printf("Forward:          w\n\
Backward:         s\n\
Clockwise:        d\n\
CounterClockwise: a\n\
IncreaseSpeed:    i\n\
DecreaseSpeed:    k\n\
PrintCommands:    p\n\
Quit:             q\n\
Stop:            ' '\n");
}
