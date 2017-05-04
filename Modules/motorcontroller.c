#include "motorcontroller.h"

static const char pinsFileLocation[NUM_GPIO_PINS][30] =
{
    GPIO_PIN_66_LOC,
    GPIO_PIN_67_LOC,
    GPIO_PIN_69_LOC,
    GPIO_PIN_68_LOC
};

static const char pinsFileDirection[NUM_GPIO_PINS][35] =
{
    LEFT_MOTOR_IN1_DIRECTION,
    LEFT_MOTOR_IN2_DIRECTION,
    RIGHT_MOTOR_IN3_DIRECTION,
    RIGHT_MOTOR_IN4_DIRECTION
};

static const char pinsFileValue[NUM_GPIO_PINS][35] =
{
    LEFT_MOTOR_IN1_VALUE,
    LEFT_MOTOR_IN2_VALUE,
    RIGHT_MOTOR_IN3_VALUE,
    RIGHT_MOTOR_IN4_VALUE
};

const static uint32_t pins[NUM_GPIO_PINS] = {66, 67, 69, 68};

const static uint32_t midPoint = HRES >> 1;

void MC_Init(void)
{
    FILE* file;

    for (uint32_t i = 0; i < NUM_GPIO_PINS; i++)
    {
        file = fopen(GPIO_EXPORT_PATH, "w");
        fprintf(file, "%d", pins[i]);
        fclose(file);
    }

    for (uint32_t i = 0; i < NUM_GPIO_PINS; i++)
    {
        file = fopen(pinsFileDirection[i], "w");
        fprintf(file, "out");
        fclose(file);
    }

    MC_Stop();
}

void MC_Main(uint32_t errorOffset)
{
    struct   timespec req;
    uint32_t difference = 0;

    // if( errorOffset == 0)
    // {
    //     MC_Stop();
    // }
    // else if (errorOffset > midPoint + 100)
    // {
    //     MC_CircleClockwise();
    // }
    // else if (errorOffset < midPoint - 100)
    // {
    //     MC_CircleCounterClockwise();
    // }
    // else
    // {
    //     MC_Stop();
    // }

    req.tv_sec  = 0;  // 0 secs
    //req.tv_nsec = TIMER_NS; // 100 msecs (1e8 nanosecs)
    if (midPoint > errorOffset)
    {
        difference = midPoint - errorOffset;
    }
    else
    {
        difference = errorOffset - midPoint;
    }

    switch (difference / 100)
    {
        case 1: req.tv_nsec = MOVE_100;
        case 2: req.tv_nsec = MOVE_200;
        case 3: req.tv_nsec = MOVE_300;
    }

    if (errorOffset > (midPoint + 100))
    {
        MC_CircleClockwise();
        nanosleep(&req, NULL);
        MC_Stop();
       // printf("moving: %d\n", difference);
    }
    else if (errorOffset < (midPoint - 100))
    {
        MC_CircleCounterClockwise();
        nanosleep(&req, NULL);
        MC_Stop();
       // printf("moving: %d\n", difference);
    }
}

void MC_Forward(MC_Motor_t motor)
{
    FILE* file;
    switch (motor)
    {
        case LeftMotor:
            file = fopen(pinsFileValue[EN1], "w");
            fprintf(file, "1");
            fclose(file);

            file = fopen(pinsFileValue[EN2], "w");
            fprintf(file, "0");
            fclose(file);

            break;
        case RightMotor:
            file = fopen(pinsFileValue[EN3], "w");
            fprintf(file, "1");
            fclose(file);

            file = fopen(pinsFileValue[EN4], "w");
            fprintf(file, "0");
            fclose(file);

            break;
    }
}

void MC_Reverse(MC_Motor_t motor)
{
    FILE* file;
    switch (motor)
    {
        case LeftMotor:
            file = fopen(pinsFileValue[EN1], "w");
            fprintf(file, "0");
            fclose(file);

            file = fopen(pinsFileValue[EN2], "w");
            fprintf(file, "1");
            fclose(file);

            break;
        case RightMotor:
            file = fopen(pinsFileValue[EN3], "w");
            fprintf(file, "0");
            fclose(file);

            file = fopen(pinsFileValue[EN4], "w");
            fprintf(file, "1");
            fclose(file);

            break;
    }
}

void MC_CircleClockwise(void)
{
    MC_Forward(LeftMotor);
    MC_Reverse(RightMotor);
}

void MC_CircleCounterClockwise(void)
{
    MC_Forward(RightMotor);
    MC_Reverse(LeftMotor);
}

void MC_Stop(void)
{
    FILE* file;
    for (uint32_t i = 0; i < NUM_GPIO_PINS; i++)
    {
        file = fopen(pinsFileValue[i], "w");
        fprintf(file, "0");
        fclose(file);
    }
}
