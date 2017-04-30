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
//Required header files
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <semaphore.h>
#include <sys/sem.h>
#include <assert.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <iomanip>

//OpenCV required packages
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

// Defined values
#define TIMER_S      0
#define TIMER_NS     100000000
#define CYCLE_RUNS   20
// freq between 1 to 10 Hz
#define CAPTURE_FREQ 1
#define MOTOR_FREQ   1
#define SYNC_FREQ    10

//Used packages
using namespace cv;
using namespace std;

// Pthread definitions
void *SoftTimer    ( void *threadid );
void *ImageCapture ( void *threadid );
void *MotorControl ( void *threadid );
//void *DisplayImage    ( void *threadid );
//void *FollowControl    ( void *threadid );

// Function declarations
void print_scheduler(void);

// Global CPU handle
cpu_set_t cpu;

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
unsigned int capture_status;
unsigned int motor_status;
unsigned int continue_running;

// Mutex protected variable
unsigned int error_offset;
////////////////////////////////////////////

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

    //Global Var setup
    continue_running = 1;
    capture_status   = 1;
    motor_status     = 1;
    error_offset     = 34;

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
    timer_param.sched_priority   = rt_max_prio-1;
    capture_param.sched_priority = rt_max_prio-2;
    motor_param.sched_priority   = rt_max_prio-3;


    if( sched_setscheduler( getpid(), SCHED_FIFO, &main_param ) ){
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

    //Create timer thread
    if( pthread_create( &timer_thread, 
                        &timer_sched_attr, 
                        SoftTimer, 
                        (void*)0                ) 
    ){
        cout << log << "ERROR!! Could not create timer thread!" << endl;
        perror(NULL);
        return -1;
    }
    if( pthread_create( &capture_thread, 
                        &capture_sched_attr, 
                        ImageCapture, 
                        (void*)0                ) 
    ){
        cout << log << "ERROR!! Could not create ImageCapture thread!" << endl;
        perror(NULL);
        return -1;
    }
    if( pthread_create( &motor_thread, 
                        &motor_sched_attr, 
                        MotorControl, 
                        (void*)0                ) 
    ){
        cout << log << "ERROR!! Could not create MotorControl thread!" << endl;
        perror(NULL);
        return -1;
    }
    
    //Waits until threads are completed
    pthread_join( timer_thread,   NULL );
    pthread_join( capture_thread, NULL );
    pthread_join( motor_thread,   NULL );

    //Destroy threads attributes
    if(pthread_attr_destroy( &timer_sched_attr ) != 0){
        perror("attr destroy");
    }
    if(pthread_attr_destroy( &capture_sched_attr ) != 0){
        perror("attr destroy");
    }
    if(pthread_attr_destroy( &motor_sched_attr ) != 0){
        perror("attr destroy");
    }
    //Destroy semaphore
    sem_destroy( &capture_sem );
    sem_destroy( &motor_sem   );

    //Reset scheduler to previous state
    rc=sched_setscheduler( getpid(), SCHED_OTHER, &nrt_param );


    cout << log << "Done, exiting." << endl;
    return 0;
}
/*******************************************************************
SoftTimer

*******************************************************************/
void *SoftTimer( void *threadid ){
    struct   timespec req;
    unsigned int      cycle_cnt = 0;
    unsigned int      c_s_cnt   = 0;
    unsigned int      m_s_cnt   = 0;
    unsigned int      c_m_cnt   = 0;
    unsigned int      m_m_cnt   = 0;
             string   log       = "[ TIMER   ] ";

    req.tv_sec  = TIMER_S;  // 0 secs
    req.tv_nsec = TIMER_NS; // 100 msecs (1e8 nanosecs)
    
    cout << log << "Setup hold time" << endl;
    nanosleep( &req, NULL );

    while(continue_running){

        nanosleep( &req, NULL );
/*
        if( cycle_cnt%CAPTURE_FREQ == 0 ){
            sem_post( &capture_sem );
        }
        if( cycle_cnt%MOTOR_FREQ == 0 ){
        }
*/
        if( cycle_cnt%SYNC_FREQ == 0){
            if(capture_status != 1){
                cout << log << "Capture missed deadline" << endl;
                c_m_cnt++;
            }
            if(motor_status != 1){
                cout << log << "Motor missed deadline" << endl;
                m_m_cnt++;
            }
            capture_status = 0;
            motor_status   = 0;

            sem_post( &capture_sem );
            sem_post( &motor_sem );
            c_s_cnt++;
            m_s_cnt++;
        }
        if( cycle_cnt == 9999 ){
            cycle_cnt = 0;
        }else if(cycle_cnt == SYNC_FREQ*(CYCLE_RUNS-1) ){
            continue_running = 0;
            cout << log << "CR set to 0" << endl;
        } else {
            cycle_cnt++;
        }
    }

    // Evaluate last cycle
    nanosleep( &req, NULL );
    if(capture_status != 1){
        cout << log << "Capture missed deadline" << endl;
        c_m_cnt++;
    }
    if(motor_status != 1){
        cout << log << "Motor missed deadline" << endl;
        m_m_cnt++;
    }

    cout << log << "Capture : " << c_s_cnt << " starts, " << c_m_cnt << " missed." <<endl;
    cout << log << "Motor   : " << m_s_cnt << " starts, " << m_m_cnt << " missed." <<endl;

    pthread_exit( NULL );
}

/*******************************************************************
ImageCapture

*******************************************************************/
void *ImageCapture( void *threadid ){

    struct timespec    current;
           string      log = "[ ImgCap  ] ";

    while(continue_running){
        // Semaphore used to sync timing from SoftTimer
        sem_wait( &capture_sem );

        // Mutex used to access shared memory
        // ImageCapture - Update circle location information
        pthread_mutex_lock( &system_mutex );

        // Using the timestamp information as placeholder
        clock_gettime( CLOCK_REALTIME, &current );
        cout << log << "Image Capture running : " <<
            current.tv_sec << "." <<
            setfill('0') << setw(9) << current.tv_nsec << endl;
        error_offset += 14;
        // Release lock
        pthread_mutex_unlock( &system_mutex );

        // Delay as placeholder
        for(unsigned int ii=0; ii<1000000; ii++);   

        capture_status = 1;     
    }
}

/*******************************************************************
MotorControl

*******************************************************************/
void *MotorControl( void *threadid ){

    struct timespec current;
           string   log = "[ MtrCtrl ] ";

    while(continue_running){
        // Semaphore used to sync timing from SoftTimer
        sem_wait( &motor_sem );

        // Mutex used to access shared memory
        // MotorControl - Get circle location information
        pthread_mutex_lock( &system_mutex );

        // Using the timestamp information as placeholder
        clock_gettime( CLOCK_REALTIME, &current );
        cout << log << "Motor Control running : " << 
            current.tv_sec << "." <<
            setfill('0') << setw(9) << current.tv_nsec << endl;
        cout << log << "Error offset = " << error_offset << endl;
        // Release lock
        pthread_mutex_unlock( &system_mutex );

        // Delay as placeholder
        for(unsigned int ii=0; ii<1000000; ii++);

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

    switch(schedType){
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




