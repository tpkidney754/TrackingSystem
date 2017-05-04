#include "profiling.h"

static void PRO_MotorBasics();
static void PRO_MotorMain();
static void PRO_ImageCapture();
static void PRO_delta(timespec_t *start, timespec_t *stop, timespec_t *delta_t);

void PRO_RunProfilingSuite(void)
{
    //PRO_MotorBasics();
    //PRO_MotorMain();
    PRO_ImageCapture();
}

static void PRO_MotorMain()
{
    timespec_t startTime;
    timespec_t stopTime;
    timespec_t elapsedTime;

    uint32_t sum = 0;
    uint32_t max = 0;
    uint32_t averageTime = 0;
    uint32_t temp = 0;

    for (uint32_t i = 0; i < TEST_ITERATIONS; i++)
    {
        clock_gettime(CLOCK_REALTIME, &startTime);
        MC_Main(640);
        clock_gettime(CLOCK_REALTIME, &stopTime);
        PRO_delta(&startTime, &stopTime, &elapsedTime);
        temp = (uint32_t) elapsedTime.tv_nsec / 1000;
        sum += temp;
        max = (temp > max) ? temp : max;
    }

    averageTime = sum / TEST_ITERATIONS;
    printf("\nMotorMain Max: %d, average: %d", max, averageTime);
}

static void PRO_ImageCapture()
{
    timespec_t startTime;
    timespec_t stopTime;
    timespec_t elapsedTime;

    IplImage* frame;
    Mat gray;
    vector<Vec3f> circles;

    Size size(HRES,VRES);

    uint32_t sum = 0;
    uint32_t max = 0;
    uint32_t averageTime = 0;
    uint32_t temp = 0;

    uint32_t capture_status;
    uint32_t continue_running;
    CvCapture* capture;
    pthread_mutex_t system_mutex;

    capture = (CvCapture*) cvCreateCameraCapture(0);
    cvSetCaptureProperty( capture, CV_CAP_PROP_FRAME_WIDTH, HRES );
    cvSetCaptureProperty( capture, CV_CAP_PROP_FRAME_HEIGHT, VRES );

    pthread_mutex_init( &system_mutex, NULL );
    sem_t capture_sem;

// Mutex protected variable
    uint32_t error_offset = 0;

    sem_post( &capture_sem );

    printf("Starting ProImageProfiling\n");

    for (uint32_t i = 0; i < TEST_ITERATIONS; i++)
    {
        clock_gettime(CLOCK_REALTIME, &startTime);

        sem_wait( &capture_sem );
        capture_status = 0;
        //cvSetCaptureProperty( capture, CV_CAP_PROP_FPS, 2 );
        frame = cvQueryFrame( capture );

        Mat mat_frame( cv::cvarrToMat(frame) );
        cvtColor( mat_frame, gray, CV_BGR2GRAY );
        GaussianBlur( gray, gray, Size( 9, 9 ), 2, 2 );
        HoughCircles(gray, circles, CV_HOUGH_GRADIENT, 1, gray.rows / 8, 100, 50, 0, 0);

        if( circles.size() < 1 )
        {
            pthread_mutex_lock(&system_mutex);
            error_offset = 0;
            pthread_mutex_unlock(&system_mutex);
        }
        else
        {
            pthread_mutex_lock(&system_mutex);
            error_offset = cvRound(circles[0][0]);
            pthread_mutex_unlock(&system_mutex);
            //printf("x: %d, y: %d\n", cvRound(circles[0][0]), cvRound(circles[0][1]));
        }
        capture_status = 2 - continue_running;
        sem_post( &capture_sem );

        clock_gettime(CLOCK_REALTIME, &stopTime);
        PRO_delta(&startTime, &stopTime, &elapsedTime);
        temp = (uint32_t) elapsedTime.tv_nsec / 1000;
        sum += temp;
        max = (temp > max) ? temp : max;
        printf("cycle %d\n",i);

    }

    averageTime = sum / TEST_ITERATIONS;
    printf("\nMotorMain Max: %d, average: %d\n", max, averageTime);
}

static void PRO_MotorBasics()
{
    timespec_t startTime;
    timespec_t stopTime;
    timespec_t elapsedTime;
    uint32_t sum[NUM_TESTS] = {0, 0, 0, 0};
    uint32_t max[NUM_TESTS] = {0, 0, 0, 0};
    uint32_t averageTime[NUM_TESTS];
    uint32_t temp;

    for (uint32_t i = 0; i < TEST_ITERATIONS; i++)
    {
        // Updating the pulsewidth modulation
        clock_gettime(CLOCK_REALTIME, &startTime);
        PWM_ChangeDutyCycle(100, 3);
        PWM_ChangeDutyCycle(100, 4);
        clock_gettime(CLOCK_REALTIME, &stopTime);
        PRO_delta(&startTime, &stopTime, &elapsedTime);
        temp = (uint32_t) elapsedTime.tv_nsec / 1000;
        sum[0] += temp;
        max[0] = (temp > max[0]) ? temp : max[0];

        //Changing to clockwise time
        clock_gettime(CLOCK_REALTIME, &startTime);
        MC_CircleClockwise();
        clock_gettime(CLOCK_REALTIME, &stopTime);
        PRO_delta(&startTime, &stopTime, &elapsedTime);
        temp = (uint32_t) elapsedTime.tv_nsec / 1000;
        sum[1] += temp;
        max[1] = (temp > max[1]) ? temp : max[1];

        //Changing to counterclockwise time
        clock_gettime(CLOCK_REALTIME, &startTime);
        MC_CircleCounterClockwise();
        clock_gettime(CLOCK_REALTIME, &stopTime);
        PRO_delta(&startTime, &stopTime, &elapsedTime);
        temp = (uint32_t) elapsedTime.tv_nsec / 1000;
        sum[2] += temp;
        max[2] = (temp > max[2]) ? temp : max[2];

        //Stopping the motors
        clock_gettime(CLOCK_REALTIME, &startTime);
        MC_Stop();
        clock_gettime(CLOCK_REALTIME, &stopTime);
        PRO_delta(&startTime, &stopTime, &elapsedTime);
        temp = (uint32_t) elapsedTime.tv_nsec / 1000;
        sum[3] += temp;
        max[3] = (temp > max[3]) ? temp : max[3];
    }

    for (uint32_t i = 0; i < NUM_TESTS; i++)
    {
        averageTime[i] = sum[i] / TEST_ITERATIONS;
    }

    printf("Changing the PWM for both motors takes an average of %d us, max: %d us\n", averageTime[0], max[0]);
    printf("Changing direction to clockwise takes an average of %d microseconds, max: %d us\n",averageTime[1], max[1]);
    printf("Changing direction to counter-clockwise takes an average of %d microseconds, max: %d us\n", averageTime[2], max[2]);
    printf("Stopping both motors takes an average of %d microseconds, max: %d\n",averageTime[3], max[3]);
}

static void PRO_delta(timespec_t *start, timespec_t *stop, timespec_t *delta_t)
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
