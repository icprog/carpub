#ifndef CAN_H_INCLUDED
#define CAN_H_INCLUDED

int can_init(const char *ifname);
int can_read(int cansock, struct can_frame *frame);

#endif
