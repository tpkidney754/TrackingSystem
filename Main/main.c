#include "includeall.h"

int main()
{
    PWM_Init();
    MC_Init();

    PRO_RunProfilingSuite();
}
