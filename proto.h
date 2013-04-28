#ifndef __PROTO_H__
#define __PROTO_H__

int heli_init_aap(int socket);
int heli_connect(void);
int start_heli_control_thread(void);

void change_light(void);
void turnoff_heli(void);
void set_throttle(int val);
#define THROTTLE_MAX 128
void set_pitch(int val);
#define PITCH_MAX 255
void set_yaw(int val);
#define YAW_MAX 255
void inc_trimmer(void);
void dec_trimmer(void);
#define TRIMMER_MAX 20

#endif
