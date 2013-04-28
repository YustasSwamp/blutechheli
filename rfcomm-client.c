/*
 * Copyright (c) 2012 Alexey Makhalov <makhaloff@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or 
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <unistd.h>


#include "proto.h"


char cmd_packet[] = { 0x55, 0x11, 0x00, 0x43, 0x00, 0x00, 0x00, 0x01, /* 7 */
	 /*               -throttle-  ---pitch--  ----yaw---  light      */
	0x78, 0x30, 0x30, 0x30, 0x30, 0x37, 0x66, 0x37, 0x66, 0x30, 0x61,
	0xa3 };
#define CMD_SIZE 20
#define CMD_CONTROL_OFFSET 11
#define TRIMMER_MASK	0x1f
#define TRIMMER_SHIFT	0
#define LIGHT_MASK 	0xe0
#define LIGHT_SHIFT	5
#define YAW_MASK	0xff00 
#define YAW_SHIFT	8
#define PITCH_MASK	0xff0000
#define PITCH_SHIFT	16
#define THROTTLE_MASK	0xff000000 
#define THROTTLE_SHIFT	24
#define DEFAULT_MASK	0xffffff00
#define DEFAULT_VALUE	0x007f7f0a
unsigned long cmd_control = DEFAULT_VALUE;

void change_light(void)
{
	static light = 0;
	light = (light + 1) & 0x7;
	cmd_control = (cmd_control & ~LIGHT_MASK) | (light << LIGHT_SHIFT);
}
void turnoff_heli(void)
{
	cmd_control = DEFAULT_VALUE | (7 << LIGHT_SHIFT);
}

void set_throttle(int val)
{
	if ( val > THROTTLE_MAX )
		val = THROTTLE_MAX;
	if ( val < 0 )
		val = 0;

	if (val == 0) /* set all values to default */
		cmd_control = (cmd_control & ~DEFAULT_MASK) | DEFAULT_VALUE;
	else
		cmd_control = (cmd_control & ~THROTTLE_MASK) | (val << THROTTLE_SHIFT);
}

void set_pitch(int val)
{
	if ( val > PITCH_MAX )
		val = PITCH_MAX;
	if ( val < 0 )
		val = 0;

	cmd_control = (cmd_control & ~PITCH_MASK) | (val << PITCH_SHIFT);
}

void set_yaw(int val)
{
	if ( val > YAW_MAX )
		val = YAW_MAX;
	if ( val < 0 )
		val = 0;

	cmd_control = (cmd_control & ~YAW_MASK) | (val << YAW_SHIFT);
}

int get_trimmer(void)
{
	int val = (cmd_control & TRIMMER_MASK) >> TRIMMER_SHIFT;
}

void dec_trimmer(void)
{
	int val = get_trimmer() - 1;
	if ( val < 0 )
		val = 0;

	cmd_control = (cmd_control & ~TRIMMER_MASK) | (val << TRIMMER_SHIFT);
}

void inc_trimmer(void)
{
	int val = get_trimmer() + 1;
	if ( val > TRIMMER_MAX )
		val = TRIMMER_MAX;

	cmd_control = (cmd_control & ~TRIMMER_MASK) | (val << TRIMMER_SHIFT);
}

int send_cmd_packet(int socket)
{
	int i;
	int csum;
	cmd_packet[5]++;
	if (!cmd_packet[5]) cmd_packet[4]++;

	sprintf(&cmd_packet[CMD_CONTROL_OFFSET], "%08x", cmd_control);

	csum = 0;
	for (i = 1; i < CMD_SIZE - 2; i++)
		csum += cmd_packet[i];
	cmd_packet[CMD_SIZE - 1] = (0x100 - csum) & 0xff;

	/* send data */
	return write(socket, cmd_packet, CMD_SIZE);
}

void *threadfunc(void *parm)
{
	int socket = (int)parm;
	int status;
	char buf[1024];

	while (1) {
		status = send_cmd_packet(socket);
		if (status < 0) {
			perror("Send");
			printf("Trying to reconnect...\n");
			cmd_packet[5] = 0;
			cmd_packet[4] = 0;
			cmd_control = DEFAULT_VALUE;
			socket = heli_connect();
		}

		/* receive data */
#if 0			
		status = read(socket, buf, sizeof(buf));
		if (status < 0)
		{
			perror("Receive");
			return;
		}
#endif
		usleep(100000);
	}
}

void finalize(int s){
	printf("\nTurn of helicopter.\n");
	turnoff_heli();
	usleep(200000);
	exit(1); 
}

void set_sigint_handler(void)
{

	struct sigaction sigIntHandler;

	sigIntHandler.sa_handler = finalize;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;

	sigaction(SIGINT, &sigIntHandler, NULL);
}

int heli_connect(void)
{
	struct sockaddr_rc addr = { 0 };
	int status;
	char dest[18] = "00:11:67:e5:b9:3e";
	char buf[1024] = {0};
	int bytes_read;
	unsigned char i = 0;
	int heli_socket;


	// set the connection parameters (who to connect to)

	// allocate a socket
	heli_socket = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
	if (heli_socket < 0) {
		perror("Opening RFCOMM socket");
		exit (1);
	}

	addr.rc_family = AF_BLUETOOTH;
	addr.rc_channel = (uint8_t) 3;
	str2ba( dest, &addr.rc_bdaddr );

	// connect to server
	status = connect(heli_socket, (struct sockaddr *)&addr, sizeof(addr));
	if (status < 0) {
		perror("Connect failed");
		close(heli_socket);
		exit (1);
	}

	printf("Connect success!\n");

	status = heli_init_aap(heli_socket);
	if (status < 0) {
		close(heli_socket);
		exit (1);
	}

	return heli_socket;
}

int start_heli_control_thread(void)
{
	int heli_socket;
	pthread_t thread;

	heli_socket = heli_connect();

	pthread_create(&thread, NULL, threadfunc, (void *)heli_socket);

	set_sigint_handler();

	return 0;
}
