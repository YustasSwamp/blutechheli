#ifndef __PROTO_H__
#define __PROTO_H__

enum source {
	HELI = 0,
	PHONE = 1,
};

enum direction {
	PHONE_TO_HELI = 0,
	HELI_TO_PHONE = 1,
};

void action(int socket, enum source);

#endif
