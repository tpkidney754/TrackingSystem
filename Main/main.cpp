/******************************************************************************
FIXME - Info from Ex4

    OpenCV capture, transform, display project

    Much of the code based on previous projects provided from ECEN 5623

    Author: B. Heberlein
            M. Lennartz

    University of Colorado at Boulder: ECEN 5623 - Spring 2017

    This project is intended to have multiple modes that have seperate
    functions.  There is an analysis mode that will aquire the hardcoded
    camera, and take a series of image captures, perform a tranform, and
    display those images in a created window.  The number of captures and
    displays is a hardcoded value that will be used for calculating the
    average frame-rate.

    The other mode is the actual running mode that will perform a similar
    function as analysis but will have a soft-timer with hardcoded delay
    times.  The timer will start the capture thread, and check if it
    meets the associated deadline.  This process will run until the 'q'
    button is pressed exiting the capture process and stopping the
    soft-timer.  A failure rate will then be calculated based on the
    total number of captures and misses.

    The project also has other modes such as debug and a default display
    mode.
******************************************************************************/
#include "includeall.h"

/*****************************************************************************
* L O C A L F U N C T I O N S
*****************************************************************************/
// Pthread definitions
static void *SoftTimer    ( void *threadid );
static void *ImageCapture ( void *threadid );
static void *MotorControl ( void *threadid );

// Function declarations
static void print_scheduler(void);
static void camera_init(void);

// Global CPU handle
static cpu_set_t cpu;

//Restructured: grouped by individual threads
////////////////////////////////////////////
struct sched_param    nrt_param;

// Main
       pthread_t      main_thread;
       pthread_attr_t main_sched_attr;
struct sched_param    main_param;

// SoftTimer
       pthread_t      timer_thread;
       pthread_attr_t timer_sched_attr;
struct sched_param    timer_param;

//ImageCapture
       pthread_t      capture_thread;
       pthread_attr_t capture_sched_attr;
struct sched_param    capture_param;
       sem_t          capture_sem;

//MotorControl
       pthread_t      motor_thread;
       pthread_attr_t motor_sched_attr;
struct sched_param    motor_param;
       sem_t          motor_sem;

//Mutex
pthread_mutex_t system_mutex;
////////////////////////////////////////////

////////////////////////////////////////////
// Global variables
////////////////////////////////////////////
static uint32_t capture_status;
static uint32_t motor_status;
static uint32_t continue_running;

// Mutex protected variable
static uint32_t error_offset = 0;
////////////////////////////////////////////
VideoCapture cam;
CvCapture* capture;

/*******************************************************************
main

Used to determine which mode to run based on command line inputs.

Will perform initial setup and pthread creation.
*******************************************************************/
int main( int argc, char* argv[] )
{
    string log = "[ MAIN    ] ";
    int rc;
    int result,ii;
    string input,cut;
    int rt_max_prio, rt_min_prio;

    PWM_Init();
    MC_Init();
    PWM_ChangeDutyCycle(100,0);
    PWM_ChangeDutyCycle(100,1);

    //Global Var setup
    continue_running = 1;
    capture_status   = 1;
    motor_status     = 1;
    error_offset     = 0;
    openlog( NULL, LOG_CONS, LOG_USER );

    // PRO_RunProfilingSuite();
    // return 0;
    cam.open(0);
    cam.set(CV_CAP_PROP_FRAME_WIDTH, HRES);
    cam.set(CV_CAP_PROP_FRAME_HEIGHT, VRES);
    cam.set(CV_CAP_PROP_FPS, FPS);
    cam.set(CV_CAP_PROP_BUFFERSIZE, 1);

    cout << log << "Opened camera on video 0" << endl;

//    camera_init();

    // capture = (CvCapture*) cvCreateCameraCapture(0);
    // cvSetCaptureProperty( capture, CV_CAP_PROP_FRAME_WIDTH, HRES );
    // cvSetCaptureProperty( capture, CV_CAP_PROP_FRAME_HEIGHT, VRES );
    // cvSetCaptureProperty( capture, CV_CAP_PROP_FPS, FPS);
    //CPU select for setting affinity later
    //for multi-core system, this could be modified
    //hardcoded for single cpu system
    CPU_ZERO( &cpu );    //zero out set
    CPU_SET(0, &cpu );     //add single cpu

    cout << log << "Before adjustments to scheduling policy: " << endl;
    print_scheduler();

    //Mutex setup
    // Used to lock thread to set/update circle location information
    pthread_mutex_init( &system_mutex, NULL );

    //Scheduler setup
    pthread_attr_init( &main_sched_attr        );
    pthread_attr_init( &timer_sched_attr    );
    pthread_attr_init( &capture_sched_attr    );
    pthread_attr_init( &motor_sched_attr    );

    pthread_attr_setinheritsched( &main_sched_attr,     PTHREAD_EXPLICIT_SCHED );
    pthread_attr_setinheritsched( &timer_sched_attr,     PTHREAD_EXPLICIT_SCHED );
    pthread_attr_setinheritsched( &capture_sched_attr,     PTHREAD_EXPLICIT_SCHED );
    pthread_attr_setinheritsched( &motor_sched_attr,     PTHREAD_EXPLICIT_SCHED );

    pthread_attr_setschedpolicy( &main_sched_attr,    SCHED_FIFO );
    pthread_attr_setschedpolicy( &timer_sched_attr,   SCHED_FIFO );
    pthread_attr_setschedpolicy( &capture_sched_attr, SCHED_FIFO );
    pthread_attr_setschedpolicy( &motor_sched_attr,   SCHED_FIFO );

    rt_max_prio = sched_get_priority_max( SCHED_FIFO );
    rt_min_prio = sched_get_priority_min( SCHED_FIFO );

    rc=sched_getparam(getpid(), &nrt_param);

    main_param.sched_priority    = rt_max_prio;
    timer_param.sched_priority   = rt_max_prio;
    capture_param.sched_priority = rt_max_prio-1;
    motor_param.sched_priority   = rt_max_prio-2;


    if( sched_setscheduler( getpid(), SCHED_FIFO, &main_param ) )
    {
        cout << log << "ERROR; sched_setscheduler rc is " << rc << endl;
        perror(NULL);
        exit(-1);
    }

    //Checking scheduler
    cout << log << "After adjustments to scheduling policy: " << endl;
    print_scheduler();

    pthread_attr_setschedparam(&timer_sched_attr, &timer_param);
    pthread_attr_setschedparam(&capture_sched_attr, &capture_param);
    pthread_attr_setschedparam(&motor_sched_attr, &motor_param);
    pthread_attr_setschedparam(&main_sched_attr, &main_param);

//    sem_post(&capture_sem);
    //Create timer thread
    if( pthread_create( &timer_thread, &timer_sched_attr, SoftTimer, (void*)0))
    {
        cout << log << "ERROR!! Could not create timer thread!" << endl;
        perror(NULL);
        return -1;
    }
    if( pthread_create( &capture_thread, &capture_sched_attr, ImageCapture, (void*)0))
    {
        cout << log << "ERROR!! Could not create ImageCapture thread!" << endl;
        perror(NULL);
        return -1;
    }
    if( pthread_create( &motor_thread, &motor_sched_attr, MotorControl, (void*)0))
    {
        cout << log << "ERROR!! Could not create MotorControl thread!" << endl;
        perror(NULL);
        return -1;
    }

    //Waits until threads are completed
    pthread_join( timer_thread,   NULL );
    pthread_join( capture_thread, NULL );
    pthread_join( motor_thread,   NULL );

    //Destroy threads attributes
    if(pthread_attr_destroy( &timer_sched_attr ) != 0)
    {
        perror("attr destroy");
    }
    if(pthread_attr_destroy( &capture_sched_attr ) != 0)
    {
        perror("attr destroy");
    }
    if(pthread_attr_destroy( &motor_sched_attr ) != 0)
    {
        perror("attr destroy");
    }
    //Destroy semaphore
    sem_destroy( &capture_sem );
    sem_destroy( &motor_sem   );

    //Reset scheduler to previous state
    rc = sched_setscheduler( getpid(), SCHED_OTHER, &nrt_param );


    cout << log << "Done, exiting." << endl;
    return 0;
}
/*******************************************************************
SoftTimer

*******************************************************************/
void *SoftTimer( void *threadid )
{
    struct   timespec req;
    struct   timespec currentTime;
    uint32_t cycle_cnt = 0;
    uint32_t c_s_cnt   = 0;
    uint32_t m_s_cnt   = 0;
    uint32_t c_m_cnt   = 0;
    uint32_t m_m_cnt   = 0;
    uint32_t c_stall   = 0;
    uint32_t m_stall   = 0;

    int32_t semval    = 0;

    string   log       = "[ TIMER   ] ";

    cout << log << "Setup hold time" << endl;

    req.tv_sec  = 2;//TIMER_S;
    req.tv_nsec = 0;//TIMER_NS;

    while(continue_running)
    {
        nanosleep( &req, NULL );
        //Syslog statement here - Time_stamp
        sem_post(&capture_sem);
    }

    pthread_exit( NULL );
}

/*******************************************************************
ImageCapture

*******************************************************************/
void *ImageCapture( void *threadid ){

    struct timespec    currentTime;
           string      log = "[ ImgCap  ] ";

    // IplImage* frame;
    Mat frame, gray;
    vector<Vec3f> circles;

    Size size(HRES,VRES);

    while(continue_running)
    {
        // Semaphore used to sync timing from SoftTimer
        sem_wait( &capture_sem );
        //Syslog statement here - Image cap start with time_stamp
        
        cam.set(CV_CAP_PROP_FPS, FPS);
        cam >> frame;

        cvtColor( frame, gray, CV_BGR2GRAY );
        GaussianBlur( gray, gray, Size( 9, 9 ), 2, 2 );
        HoughCircles(gray, circles, CV_HOUGH_GRADIENT, 1, gray.rows / 8, 100, 50, 0, 0);

        if( circles.size() < 1 )
        {
            printf("no circles dammit\n");
        }
        else
        {
            //Syslog statement here - error_offset with time_stamp
            error_offset = cvRound(circles[0][0]);
            printf("x: %d, y: %d\n", cvRound(circles[0][0]), cvRound(circles[0][1]));
        }

        sem_post(&motor_sem);
    }
}

/*******************************************************************
MotorControl

*******************************************************************/
void *MotorControl( void *threadid )
{

    struct timespec currentTime;
           string   log = "[ MtrCtrl ] ";

    uint32_t localErrorOffset = 0;
    while(continue_running)
    {
        // Semaphore used to sync timing from SoftTimer
        sem_wait( &motor_sem );
        //Syslog statement here - motor start with time_stamp
        
        localErrorOffset = error_offset;
        error_offset = 0;

        if (localErrorOffset)
        {
            //Syslog statement here - localErrorOffset with time_stamp
            MC_Main(localErrorOffset);
        }

        motor_status = 1;
    }
}

/*******************************************************************
print_scheduler

Function used to display current Pthread policy.
*******************************************************************/
void print_scheduler(void)
{
    int schedType;
    string log = "[ SCHEDULE] ";

    schedType = sched_getscheduler(getpid());

    switch(schedType)
    {
        case SCHED_FIFO:
            cout << log << "Pthread Policy is SCHED_FIFO" << endl;
            break;
        case SCHED_OTHER:
            cout << log << "Pthread Policy is SCHED_OTHER" << endl;
            break;
        case SCHED_RR:
            cout << log << "Pthread Policy is SCHED_OTHER" << endl;
            break;
        default:
            cout << log << "Pthread Policy is UNKNOWN" << endl;
    }
}
