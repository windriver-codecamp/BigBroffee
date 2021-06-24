#ifdef __VXWORKS__
#include <vxWorks.h>
#include <sys/select.h>
#include <evdevLib.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/fcntlcom.h>

#include "input-event-codes.h"

#define DEV_NAME  "/input/event"

#define GAMEPAD1 "HJD-X"
#define GAMEPAD2 "X-Box 360 controller"
#define GAMEPAD3 "Speed-Link Competition Pro"

int WaitGKey(int delay, int *axis_x, int *axis_y);
int identify_gamepad();

#endif