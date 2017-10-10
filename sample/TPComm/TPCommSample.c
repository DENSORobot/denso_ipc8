/**
 *   Software License Agreement (GNU General Public License)
 *
 *   Copyright(c) 2014 DENSO WAVE INCORPORATED. All Right Reserved.
 *   
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License
 *   as published by the Free Software Foundation; either version 2
 *   of the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/input.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "stdint.h"
#include "TPComm/dn_tpcomm.h"

#ifndef DEBUG
#define DEBUG (0)
#endif

#define INPUT_MOUSE_EVENT "/dev/input/event%d"

#ifndef INPUT_MOUSE_NUMBER
#define INPUT_MOUSE_NUMBER (0)
#endif

#define TP_COM_CONNECT "com:%d:38400"

#ifndef TP_COM_PORT
#define TP_COM_PORT (0)
#endif

static volatile int m_flag;
static int m_fd_mouse;
static int m_rel_x, m_rel_y;
static volatile int m_start;

static HRESULT TPReadInput(int fd)
{
	HRESULT hr = E_INVALIDARG;
	struct input_event events[64];	
	size_t read_size;
	int i;
	
	if (fd) {
		read_size = read(fd, events, sizeof(events));
		printf("read_size=%d \n", (int)read_size);
		if (read_size == -1) {
			printf("read error(0x%x) \n", errno);
			return E_FAIL;
		}
		printf("---------------\n");
		for (i=0; i<(int)(read_size / sizeof(struct input_event)); i++) {
			printf("[%d] \n", i);
			printf("time=%ld.%06ld \n", events[i].time.tv_sec, events[i].time.tv_usec);
			printf("type=%d \n", events[i].type);
			printf("code=%d \n", events[i].code);
			printf("value=%d \n", events[i].value);
			printf("---------------\n");
		}
		hr = S_OK;
	}
	
	return hr;
}

static HRESULT TPWriteInput(struct timeval time, int type, int code, int value, int fd)
{
	HRESULT hr = E_INVALIDARG;
	struct input_event input;
	size_t write_size;
	
	if (fd) {
#if (DEBUG)
		printf("---------------\n");
		printf("time=%ld.%06ld \n", time.tv_sec, time.tv_usec);
		printf("type=%d \n", type);
		printf("code=%d \n", code);
		printf("value=%d \n", value);
		printf("---------------\n");
#endif
		input.time		= time;
		input.type		= type;
		input.code		= code;
		input.value	= value;
		write_size = write(fd, &input, sizeof(input));
		if (write_size == -1) {
			printf("write error(0x%x) \n", errno);
			hr = E_FAIL;
		}
		else {
			hr = S_OK;
		}
	}
	
	return hr;
}

static HRESULT TPWriteMouse(int mode, int rel_x, int rel_y)
{
	HRESULT hr = E_INVALIDARG;
	struct timeval time;
	int relative_x, relative_y;
	
	if (m_start == 0) {
		/*  First will be made only coordinate storage. */
		m_rel_x = rel_x;
		m_rel_y = rel_y;
		m_start = 1;
		return S_OK;
	}
	
	relative_x = (m_rel_x - rel_x) * -1;
	relative_y = (m_rel_y - rel_y) * -1;
	gettimeofday(&time, NULL);
	
	if (mode == MODE_TOUCH) {
		hr = TPWriteInput(time, EV_REL, REL_X, relative_x, m_fd_mouse);
		if (hr != S_OK) {
			printf("TPWriteInput(x) failure. (hr=0x%x) \n", hr);
		} 	
		hr = TPWriteInput(time, EV_REL, REL_Y, relative_y, m_fd_mouse);
		if (hr != S_OK) {
			printf("TPWriteInput(y) failure. (hr=0x%x) \n", hr);
		}
		hr = TPWriteInput(time, EV_SYN, SYN_REPORT, 0, m_fd_mouse);
		if (hr != S_OK) {
			printf("TPWriteInput(syn) failure. (hr=0x%x) \n", hr);
		}
		
		if (hr == S_OK) {
			m_rel_x = rel_x;
			m_rel_y = rel_y;
		}
#if (DEBUG)
		hr = TPReadInput(m_fd_mouse);
		if (hr != S_OK) {
			printf("TPReadInput() failure. (hr=0x%x) \n", hr);
		} 
#endif
	}

	return hr;
}

static HRESULT TPState(int state)
{
	static int count = 0;

	printf("State: %d\n", state);

	if((state == TP_TPLESS) || (state == TP_TPERROR)){
		count++;
	}else{
		count = 0;
	}

	/* If continue TPLess or TPError state 3 times, then farce exit */
	if(count > 3){
		m_flag = 0;
	}

	return S_OK;
};

static HRESULT TPKeyInfo(struct TP_KEY_INFO key_info)
{
	/* Check exit status */
	if(key_info.btn_ok && key_info.btn_cancel){
		m_flag = 0;
	}

	return S_OK;
};

static HRESULT TPTouchInfo(struct TP_TOUCH_INFO touch_info)
{
	HRESULT hr = S_OK;

	printf("%s at X: %d, Y: %d\n",
		((touch_info.mode == MODE_TOUCH) ? "Touch" : "Release"),
		touch_info.pos_x, touch_info.pos_y);
	
	if (touch_info.mode == MODE_TOUCH) {
		hr = TPWriteMouse(touch_info.mode, touch_info.pos_x, touch_info.pos_y);
		if (hr == S_OK) {
			printf("TPWriteMouse() success. \n");
		} 
		else {
			printf("TPWriteMouse() failure. (hr=0x%x) \n", hr);
		}
	}
	printf("\n");

	return hr;
};

int main(void)
{
	int fd;
	HRESULT hr;
	struct CALL_FUNC_TP callfunc;
	char devfile_name[255];

	/* Set callback functions */
	memset(&callfunc, 0, sizeof(struct CALL_FUNC_TP));
	callfunc.Call_TPState = &TPState;
	callfunc.Call_TPKeyInfo = &TPKeyInfo;
	callfunc.Call_TPTouchInfo = &TPTouchInfo;
	TPComm_SetCallFunc(&callfunc);

	/*  Open device file of the mouse */
	sprintf(devfile_name, INPUT_MOUSE_EVENT, INPUT_MOUSE_NUMBER);
	m_fd_mouse = open(devfile_name, (O_RDWR | O_NONBLOCK));
	if (m_fd_mouse == -1) {
		printf("open failure.(errno=0x%x) \n", errno);
		return -1;
	}

	/* Coordinate initialization */
	m_rel_x = 0;
	m_rel_y = 0;
	m_start = 0;

	/* Open connection */
	sprintf(devfile_name, TP_COM_CONNECT, TP_COM_PORT);
	hr = TPComm_Open(devfile_name, 500, 0, &fd);
	if(SUCCEEDED(hr)){
		printf("If you want to exit, then press both \"OK\" and \"Cancel\" buttons\n");

		/* Loop until press both "OK" and "Cancel" */
		m_flag = 1;
		while(m_flag){
#if (DEBUG)
			hr = TPReadInput(m_fd_mouse);
			if (hr != S_OK) {
				printf("TPReadInput() failure. (hr=0x%x) \n", hr);
			} 
#endif
			usleep(1000);
		}

		/* Close connection */
		TPComm_Close(&fd);
	}
	
	/* Close device file of the mouse */
	if (m_fd_mouse) {
		close(m_fd_mouse);
	}

	return 0;
};

