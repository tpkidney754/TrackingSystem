#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <string.h>

#include <pthread.h>
#include <semaphore.h>
#include <sys/sem.h>
#include <assert.h>
#include <errno.h>
#include <iostream>
#include <iomanip>
#include <syslog.h>
//OpenCV required packages
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "motorcontroller.h"
#include "pwm.h"
#include "profiling.h"

#define HRES 640
#define VRES 480
// Defined values
#define TIMER_S      2
#define TIMER_NS     0
#define CYCLE_RUNS   200
// freq between 1 to 10 Hz
#define CAPTURE_FREQ 1
#define MOTOR_FREQ   1
#define SYNC_FREQ    1
#define FPS          8

#define TESTING 0
//Used packages
using namespace cv;
//using namespace cv2;
using namespace std;
