#ifdef __VXWORKS__
#include <vxWorks.h>
#include <sys/select.h>
#include <evdevLib.h>

int WaitKey(int delay);

#endif