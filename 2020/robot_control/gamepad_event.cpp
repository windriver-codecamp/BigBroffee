#include "gamepad_event.hpp"

char evdevName[50];

int identify_gamepad()
{
	char devName[50];
	int evdevCoreFd = ERROR;
	int devCount = 0;
	int i = 0;
	char evdevName_test[50];
	int evdevFd = ERROR;

	evdevCoreFd = open (DEV_NAME, 0, 0);
	ERROR == ioctl (evdevCoreFd, EV_DEV_IO_GET_DEV_COUNT, &devCount);
	bzero (devName, sizeof (devName));
	
	for (i = 0; i < devCount; i++)
	{	
		sprintf(evdevName_test, "%s%d", DEV_NAME, i);
		evdevFd = open(evdevName_test, 0, 0);

		if (ERROR == ioctl (evdevFd, EV_DEV_IO_GET_NAME, (char *)devName))
        {
			printf ("EV_DEV_IO_GET_NAME failed!\n");
			return -1;		
		}

		if ( strncmp(devName, GAMEPAD1, sizeof(GAMEPAD1)) == 0 ||
		     strncmp(devName, GAMEPAD2, sizeof(GAMEPAD2)) == 0 || 
			 strncmp(devName, GAMEPAD3, sizeof(GAMEPAD3)) == 0 )
		{
			sprintf(evdevName, "%s%d", DEV_NAME, i);
			printf("Gamepad name = %s\n", devName);
			printf("Device name = %s\n", evdevName);
			close(evdevFd);
			break;
		}
		
		close(evdevFd);	
	}
	close(evdevCoreFd);
	return 0;

}

int WaitGKey(int delay, int *axis_x, int *axis_y) {
	int evdevFd = ERROR;
	int key = -1;
	int res = -1;
	unsigned int msgCount = 0;
	EV_DEV_EVENT evdevEvent;

	evdevFd = open(evdevName, 0, 0);
	if (ERROR == evdevFd)
		return ERROR;
	
	fd_set readfds;
	FD_ZERO(&readfds);
	struct timeval wt = {0};
	struct timeval *pwt = NULL;
	int readBytes = 0;
	if (delay > 0) {
		wt.tv_sec = delay / 1000;
		wt.tv_usec = (delay % 1000) * 1000;
		pwt = &wt;
	}
	FD_SET(evdevFd, &readfds);
	res = select(evdevFd + 1, &readfds, NULL, NULL, pwt);
	if (res > 0) {
		if (FD_ISSET(evdevFd, &readfds)) {
			if (ioctl(evdevFd, FIONREAD, (char *) &msgCount) == ERROR) {
				goto __FUNCTION__end;
			}
			while (msgCount > 0) {
				readBytes = (int) read(evdevFd, (char *) &evdevEvent,
						sizeof(EV_DEV_EVENT));
				if (readBytes == sizeof(EV_DEV_EVENT)) {
					switch (evdevEvent.type) {
					case EV_DEV_KEY:
						if (evdevEvent.value == 1) {
							key = evdevEvent.code;
						}
                	case EV_DEV_ABS:
                    {
						switch (evdevEvent.code)
							{
							case EV_DEV_PTR_ABS_X:
									*axis_x = evdevEvent.value;
								break;

							case EV_DEV_PTR_ABS_Y:
									*axis_y = evdevEvent.value;
								break;

							default:
								break;
							}
						}
					}
				}
				if (ioctl(evdevFd, FIONREAD, (char *) &msgCount) == ERROR) {
					break;
				}
			}
		}
	}
__FUNCTION__end:
	close(evdevFd);
	return key;
}