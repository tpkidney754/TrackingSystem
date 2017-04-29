#ifndef __MOTOR__
#define __MOTOR__

#include "includeall.h"

#define GPIO_EXPORT_PATH  "/sys/class/gpio/export"

#define GPIO_PIN_66_LOC   "/sys/class/gpio/gpio66"
#define GPIO_PIN_67_LOC   "/sys/class/gpio/gpio67"
#define GPIO_PIN_69_LOC   "/sys/class/gpio/gpio69"
#define GPIO_PIN_68_LOC   "/sys/class/gpio/gpio68"

#define LEFT_MOTOR_IN1_DIRECTION    "/sys/class/gpio/gpio66/direction"
#define LEFT_MOTOR_IN2_DIRECTION    "/sys/class/gpio/gpio67/direction"
#define RIGHT_MOTOR_IN3_DIRECTION   "/sys/class/gpio/gpio69/direction"
#define RIGHT_MOTOR_IN4_DIRECTION   "/sys/class/gpio/gpio68/direction"

#define LEFT_MOTOR_IN1_VALUE    "/sys/class/gpio/gpio66/value"
#define LEFT_MOTOR_IN2_VALUE    "/sys/class/gpio/gpio67/value"
#define RIGHT_MOTOR_IN3_VALUE   "/sys/class/gpio/gpio69/value"
#define RIGHT_MOTOR_IN4_VALUE   "/sys/class/gpio/gpio68/value"

#define NUM_GPIO_PINS     4

typedef enum _MC_Motor_t
{
    LeftMotor = 0,
    RightMotor,
} MC_Motor_t;

typedef enum _MC_EnPin_t
{
    EN1 = 0,
    EN2,
    EN3,
    EN4
} MC_EnPin_t;

void MC_Init(void);
void MC_Forward(MC_Motor_t motor);
void MC_Reverse(MC_Motor_t motor);
void MC_CircleClockwise(void);
void MC_CircleCounterClockwise(void);
void MC_Stop(void);

#endif // __MOTOR__
