#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

// Optional: use these functions to add debug or error prints to your application
#define DEBUG_LOG(msg,...)
//#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

void* threadfunc(void* thread_param)
{

    // TODO: wait, obtain mutex, wait, release mutex as described by thread_data structure
    // hint: use a cast like the one below to obtain thread arguments from your parameter
    //struct thread_data* thread_func_args = (struct thread_data *) thread_param;
    
    int ret_val;
    bool status = true;
    struct thread_data* thread_func_args = (struct thread_data *) thread_param;
    
    ret_val = usleep((thread_func_args -> wait_to_obtain_ms) * 1000); 
    if(ret_val != 0)
    {
       status = false;
    }

    ret_val = pthread_mutex_lock(thread_func_args -> mutex);   
    if(ret_val != 0)
    {
       ERROR_LOG("mutex locking failed\n");
       status = false;
    }

    ret_val = usleep((thread_func_args -> wait_to_release_ms) * 1000);   
    if(ret_val != 0)
    {
       status = false;
    }
    
    ret_val = pthread_mutex_unlock(thread_func_args -> mutex); 
    if(ret_val != 0)
    {
       ERROR_LOG("mutex unlocking failed\n");
       status = false;
    }
    
    
    thread_func_args-> thread_complete_success = status;
    
    return thread_param;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
    /**
     * TODO: allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
     * using threadfunc() as entry point.
     *
     * return true if successful.
     * 
     * See implementation details in threading.h file comment block
     */
    
    int ret_val;

    struct thread_data* thread_param = (struct thread_data*) malloc(sizeof(struct thread_data));
    
            
    thread_param -> wait_to_obtain_ms = wait_to_obtain_ms;
    thread_param -> wait_to_release_ms = wait_to_release_ms;
    thread_param -> mutex = mutex;
    thread_param -> thread_complete_success = false;


    ret_val = pthread_create(thread, NULL, threadfunc, thread_param);
     
    if(ret_val == 0)
    {            
        return true;    //thread creation successful
    }
        

    ERROR_LOG("Failed to create thread : error %d", ret_val);
         
    return false;
}

