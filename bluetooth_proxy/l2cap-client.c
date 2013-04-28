#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>

#include "proto.h"

int main(int argc, char **argv)
{
    struct sockaddr_l2 addr = { 0 };
    int s, status;
    char *message = "hello!";
    char dest[18] = "00:11:67:e5:b9:3e";
    char buf[1024] = { 0 };
    int bytes_read;

    // allocate a socket
    s = socket(AF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);
    if (s < 0) {
	    perror("Opening L2CAP socket");
	    return -1;
    }

    // set the connection parameters (who to connect to)
    addr.l2_family = AF_BLUETOOTH;
    addr.l2_psm = htobs(0x0003);
    str2ba( dest, &addr.l2_bdaddr );

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
}
