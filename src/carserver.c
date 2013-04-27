#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <syslog.h>
#include <pthread.h>
#include <signal.h>

#include <linux/can.h>

#include "can.h"
#include "server.h"

int cansock;
void *netsock;

struct can_frame frame;

// task for dealing with car related activities
void *car_task(void *id) {
    syslog(LOG_DEBUG, "starting car task");
    for (;;) {
        can_read(cansock, &frame);
        printf("Recv: 0x%X (%d): 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X\n", 
                frame.can_id, 
                frame.can_dlc,
                frame.data[0],
                frame.data[1],
                frame.data[2],
                frame.data[3],
                frame.data[4],
                frame.data[5],
                frame.data[6],
                frame.data[7]);
    }
}

// task for dealing with network activites
void *server_task(void *id) {
    char req_buffer[100];
    char resp_buffer[100];
    int status;
    syslog(LOG_DEBUG, "starting server task");
    for (;;) {
        status = server_get_req(netsock, req_buffer);
        if(status < 0) {
            printf("network error\n");
            continue;
        }
        printf("got: %s\n", req_buffer);
        sprintf(resp_buffer, "%X%X\0", frame.can_id, frame.data[0]);
        status = server_send_resp(netsock, resp_buffer);
    }
}

int main (void) {
    pthread_t car_thread;
    long car_thread_id = 0;
    pthread_t server_thread;
    long server_thread_id = 1;

    // initialize logging
    openlog("carserver", LOG_PID|LOG_CONS, LOG_USER);
    syslog(LOG_INFO, "starting carserver...");

    // initialize can
    cansock = can_init("vcan0");
    // initialize network
    netsock = server_init();

    // launch threads
    pthread_create(&car_thread, NULL, car_task, (void *)car_thread_id);
    pthread_create(&server_thread, NULL, server_task, (void *)server_thread_id);

    for (;;);

    return 0;
}
