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


#ifndef _IPC8_FAN_H
#define _IPC8_FAN_H

#include "ipc8_ioctl.h"

/* Debug */
#define FAN_ADDR_OFS_SPEED1		(0x10290)
#define FAN_ADDR_OFS_SPEED2		(0x10298)
#define FAN_ADDR_OFS_SPEED3		(0x102A0)
#define FAN_ADDR_OFS_SPEED4		(0x102A8)

#define FAN_INITSPEED_PER		(60)		/* Initial fan speed [%] */
#define FAN_MINSPEED_PER		(60)		/* Minimum fan speed [%] */
#define FAN_MAXSPEED_PER		(100)		/* Maximum fan speed [%] */

#define FAN_CTRL_SPD_READ_ENB	(1)

typedef struct FAN_INFO {
	u32 fan_speed;							/* Fan speed [%]     */
	u32 fan_speed_duty;						/* Fan speed [Duty%] */
} FAN_INFO;

/* BAR2 */
typedef struct PORT_FANCTRL {
	u16 fan_life_warn;						/* 0x00010280 [R] */
	u8 rsv1[6];								/* 0x00010282 [-] */
	u16 fan_ctrl_speed_read_enb;			/* 0x00010288 [R] */
} PACKED_NAME(PORT_FANCTRL);

/* RECVDMAMAP */
typedef struct RECVDMAMAP_FANCTRL {
	u8  select;								/* 0x00DC */
	u8  rsv1;								/* 0x00DD */
	u16 sense;								/* 0x00DE */
} PACKED_NAME(RECVDMAMAP_FANCTRL);

/* SENDDMAMAP */
typedef struct SENDDMAMAP_FANCTRL {
	u8  sens_select;						/* 0x004C */
	u8  speed;								/* 0x004D */
	u16 rsv1;								/* 0x004E */
	u64 rsv2[2];							/* 0x0050 */
} PACKED_NAME(SENDDMAMAP_FANCTRL);



int initialize_fan(PORT_FANCTRL __iomem *fan_ctrl, 
				   FAN_INFO *fan_info, 
				   FAN_ERROR *fan_error);
void set_fan_speed_dma(SENDDMAMAP_FANCTRL *send_dma, 
					   FAN_INFO *fan_info);
void set_fan_speed_local(FAN_INFO *fan_info, u16 fan_speed);
int do_ioctl_read_fanspeed(void* arg, const FAN_INFO *fan_info);
int do_ioctl_write_fanspeed(void* arg, FAN_INFO* fan_info);
/* Dbug */
u16 get_actual_fan_speed(void __iomem *addr);

#endif /* _IPC8_FAN_H */

