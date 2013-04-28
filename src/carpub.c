#include <zmq.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <syslog.h>
#include <pthread.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <linux/can.h>

#include "canstore.h"
#include "carpub.h"

#define PUB_DELAY_US 1000*1000

struct carpub_priv_data * carpub_init(const char *connect_str, canstore_t canstore)
{
    carpub_t carpub_data = malloc(sizeof(struct carpub_priv_data));

    syslog(LOG_DEBUG, "initializing carpub");
    void *context = zmq_ctx_new ();
    carpub_data->sock = zmq_socket(context, ZMQ_PUB);
    int rc = zmq_bind (carpub_data->sock, connect_str);
    assert (rc==0);

    carpub_data->canstore = canstore;

    return carpub_data;
}

void *carpub_task(void *void_carpub_data)
{
    carpub_t carpub_data = (carpub_t)void_carpub_data;

    char req_buffer[100];
    char resp_buffer[100];
    int status;
    double data;
    syslog(LOG_DEBUG, "starting carpub task");
    for (;;) {
        data = canstore_get(carpub_data->canstore, 0);
        sprintf(resp_buffer, "SOC %f", data);
        status = carpub_publish(carpub_data, resp_buffer);
        usleep(PUB_DELAY_US);
    }
}

int carpub_start(carpub_t carpub_data)
{
    pthread_t carpub_thread;

    pthread_create(&carpub_thread, NULL, carpub_task, (void *)carpub_data);
}

void carpub_log_error(void)
{
    int e = zmq_errno();
    const char *errstr;
    errstr = zmq_strerror(e);
    syslog(LOG_DEBUG, "carpub error: %s", errstr);
}

// send a response to the client
int carpub_publish(carpub_t carpub_data, char *buffer)
{
    int status;
    status = zmq_send(carpub_data->sock, buffer, strlen(buffer), 0);
    if (status < 0) {
        carpub_log_error();
        return -1;
    }
    return 0;
}
