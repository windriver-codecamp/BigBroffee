#include "keyboard_event.hpp"

int WaitKey(int delay) {
	int evdevFd = ERROR;
	int key = -1;
	int res = -1;
	unsigned int msgCount = 0;
	EV_DEV_MSG evdevMsg;

	evdevFd = open(EV_DEV_NAME, 0, 0);
	if (ERROR == evdevFd)
		return ERROR;
#ifdef CLEAR_EVMSGQ
	/* clear messages */
	if (ERROR == ioctl(evdevFd, FIONREAD, (char *) &msgCount)) {
		msgCount = 0;
	}

	while (msgCount >= sizeof(EV_DEV_MSG)) {
		if (sizeof(EV_DEV_MSG)
				!= read(evdevFd, (char *) &evdevMsg, sizeof(EV_DEV_MSG))) {
			break;
		}

		if (ERROR == ioctl(evdevFd, FIONREAD, (char *) &msgCount)) {
			break;
		}
	}
#endif /* CLEAR_EVMSGQ */
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
#ifdef WAIT_KEY_DBG
			printf("%s:%d - got %d messages\n", __FUNCTION__, __LINE__, msgCount);
#endif
			while (msgCount > 0) {
				readBytes = (int) read(evdevFd, (char *) &evdevMsg,
						sizeof(EV_DEV_MSG));
				if (readBytes == sizeof(EV_DEV_MSG)) {
					switch (evdevMsg.msgType) {
					case EV_DEV_MSG_KBD:
							if (evdevMsg.msgData.kbdData.state == 1) {
#ifdef WAIT_KEY_DBG
								printf("%s:%d - got key %d\n", __FUNCTION__, __LINE__, evdevMsg.msgData.kbdData.value);
#endif
								key = evdevMsg.msgData.kbdData.value;
								/* cleanup */
#ifndef CLEAR_EVMSGQ
								goto __FUNCTION__end;
#else
								int n = 0;
								if (ERROR == ioctl(evdevFd, FIONREAD, (char *) &n)) {
									goto __FUNCTION__end;
								}
								while (n >= sizeof(EV_DEV_MSG)) {
									if (sizeof(EV_DEV_MSG)
											!= read(evdevFd, (char *) &evdevMsg, sizeof(EV_DEV_MSG))) {
										msgCount = 0;
										break;
									}

									if (ERROR == ioctl(evdevFd, FIONREAD, (char *) &n)) {
										goto __FUNCTION__end;
									}
								}
								msgCount = 0;
#endif /* CLEAR_EVMSGQ */
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
#ifdef WAIT_KEY_DBG
	printf("%s:%d - END - key = %d\n", __FUNCTION__, __LINE__, key);
#endif
	close(evdevFd);
	return key;
}