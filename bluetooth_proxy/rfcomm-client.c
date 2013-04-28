#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

#include "proto.h"

int main(int argc, char **argv)
{
	struct sockaddr_rc addr = { 0 };
	int s, status;
	char dest[18] = "00:11:67:e5:b9:3e";
	char buf[1024] = {0};
	int bytes_read;
	unsigned char i = 0;

	// set the connection parameters (who to connect to)

	// allocate a socket
	s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
	if (s < 0) {
		perror("Opening RFCOMM socket");
		return -1;
	}

	addr.rc_family = AF_BLUETOOTH;
	addr.rc_channel = (uint8_t) 3;
	str2ba( dest, &addr.rc_bdaddr );

	// connect to server
	status = connect(s, (struct sockaddr *)&addr, sizeof(addr));
	if (status < 0) {
		perror("Connect failed");
		close(s);
		return -1;
	}

	printf("Connect success!\n");

	action(s, PHONE);

	close(s);
	return 0;
}
