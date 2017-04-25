#include "motorcontroller.h"

static uint8_t pinsFileLocation[NUM_GPIO_PINS][30] =
{
    GPIO_PIN_66_LOC,
    GPIO_PIN_67_LOC,
    GPIO_PIN_69_LOC,
    GPIO_PIN_68_LOC
};

const static uint32_t pins[NUM_GPIO_PINS] = {66, 67, 69, 68};

void MC_Init(void)
{
    uint8_t buffer[100];

    for (uint32_t i = 0; i < NUM_GPIO_PINS; i++)
    {
        sprintf(buffer, "echo %d > %s", pins[i], GPIO_EXPORT_PATH);
        system(buffer);
    }

    for (uint32_t i = 0; i < NUM_GPIO_PINS; i++)
    {
        sprintf(buffer, "echo out > %s/%s", pinsFileLocation[i], "direction");
        system(buffer);
        sprintf(buffer, "echo 1 > %s/%s", pinsFileLocation[i], "value");
        system(buffer);
    }
}

void MC_Forward(MC_Motor_t motor)
{
    uint8_t buffer[100];
    switch (motor)
    {
        //This will need testing to make sure becuae the datasheet is vague about the direction
        case LeftMotor:
            //LEFT_MOTOR_IN1 high
            //LEFT_MOTOR_IN2 low
            sprintf(buffer, "echo 1 > %s/%s", pinsFileLocation[EN1], "value");
            system(buffer);
            sprintf(buffer, "echo 0 > %s/%s", pinsFileLocation[EN2], "value");
            system(buffer);
            break;
        case RightMotor:
            //RIGHT_MOTOR_IN3 high
            //RIGHT_MOTOR_IN4 low
            sprintf(buffer, "echo 1 > %s/%s", pinsFileLocation[EN3], "value");
            system(buffer);
            sprintf(buffer, "echo 0 > %s/%s", pinsFileLocation[EN4], "value");
            system(buffer);
            break;
    }
}

void MC_Reverse(MC_Motor_t motor)
{
    uint8_t buffer[100];
    switch (motor)
    {
        //This will need testing to make sure becuae the datasheet is vague about the direction
        case LeftMotor:
            sprintf(buffer, "echo 0 > %s/%s", pinsFileLocation[EN1], "value");
            system(buffer);
            sprintf(buffer, "echo 1 > %s/%s", pinsFileLocation[EN2], "value");
            system(buffer);
            break;
        case RightMotor:
            sprintf(buffer, "echo 0 > %s/%s", pinsFileLocation[EN3], "value");
            system(buffer);
            sprintf(buffer, "echo 1 > %s/%s", pinsFileLocation[EN4], "value");
            system(buffer);
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
    file = fopen(strcat(pinsFileLocation[EN1], "/value"), "w");
    fprintf(file, "0");
    fclose(file);

    file = fopen(strcat(pinsFileLocation[EN2], "/value"), "w");
    fprintf(file, "0");
    fclose(file);

    file = fopen(strcat(pinsFileLocation[EN3], "/value"), "w");
    fprintf(file, "0");
    fclose(file);

    file = fopen(strcat(pinsFileLocation[EN4], "/value"), "w");
    fprintf(file, "0");
    fclose(file);

    /*
    uint8_t buffer[100];
    sprintf(buffer, "echo 0 > %s/%s", pinsFileLocation[EN1], "value");
    system(buffer);
    sprintf(buffer, "echo 0 > %s/%s", pinsFileLocation[EN2], "value");
    system(buffer);
    sprintf(buffer, "echo 0 > %s/%s", pinsFileLocation[EN3], "value");
    system(buffer);
    sprintf(buffer, "echo 0 > %s/%s", pinsFileLocation[EN4], "value");
    system(buffer);
    */
}
