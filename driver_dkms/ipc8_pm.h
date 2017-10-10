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


#ifndef _IPC8_PM_H
#define _IPC8_PM_H

#define PM_MAX					(4)				/* The maximum number of power module   */

#define IOEXP_ID_MASK			(0x0003)		/* I/O expander ID: Mask                */
#define IOEXP_ID_POB			(0x0000)		/* I/O expander ID: Power output board  */
#define IOEXP_ID_NOTHING		(-1)			/* I/O expander ID: Unreceived          */

#define I2C_UNREC				(0x80000000)	/* I2C unreceived                       */
#define I2C_COMM_WAIT			(250)			/* I2C wait communication    : 250 [ms] */
#define LIMIT_I2C_CHECK			(3)
#define LIMIT_I2C_RETRY			(100)

#define POB_OVP_CHK_ENABLE		(1)				/* OVP check enable  */
#define POB_OVP_CHK_DISABLE		(0)				/* OVP check disable */
#define POB_OVP_WAIT			(1000)			/* 1000 [ms]         */

#define LIMIT_POB_CHECK			(2)				/* 200 [ms] */
#define LIMIT_POB_RETRY			(3)

#define POB_UNREC_ACK			(0x0001)		/* Unreceived Ack             */
#define POB_UNEXP_ACK			(0x0002)		/* Unexpected Ack receive     */
#define POB_UNKNOWN_ID			(0x0004)		/* Unknown Id                 */
#define POB_FUSE_CTRL			(0x0008)		/* Controller side fuse blows */
#define POB_FUSE_5V				(0x0010)		/*  5V  fuse blows            */
#define POB_FUSE_12V			(0x0020)		/* 12V  fuse blows            */
#define POB_FUSE_24V_B			(0x0040)		/* 24VB fuse blows            */
#define POB_FUSE_24V_A			(0x0080)		/* 24VA fuse blows            */
#define POB_FUSE_ALL			(POB_FUSE_5V | POB_FUSE_12V | POB_FUSE_24V_A | POB_FUSE_24V_B)

static const char IPM_COMBINATION[PM_MAX][2] = {{0, 3}, {1, 2}, {4, 5}, {6, 7}};

typedef struct PORT_POB {
	u16 board_switch;							/* 0x00011100 [R/W] */
	u16 power_supply;							/* 0x00011102 [R/W] */
	u32 rsv1;									/* 0x00011104 [-]   */
	u64 rsv2;									/* 0x00011108 [-]   */
	u32 rsv3;									/* 0x00011110 [-]   */
	u16 rsv4;									/* 0x00011114 [-]   */
	u16 ovp_clear;								/* 0x00011116 [W]   */
	u16 ovp_check_enable;						/* 0x00011118 [R/W] */
	u16 ovp_status;								/* 0x0001111A [R]   */
} PACKED_NAME(PORT_POB);

typedef struct PORT_I2C {
	u32 rsv1[4];								/* 0x00060300 [-] */
	u32 rsv2;									/* 0x00060310 [-] */
	u32 rsv3;									/* 0x00060314 [-] */
	u64 rsv4;									/* 0x00060318 [-] */
	u32 io_expander[4];							/* 0x00060320 [R] */
} PACKED_NAME(PORT_I2C);

typedef struct PM_INFO {
	bool enable_pob;							/* Enable power output board                   */
	bool enable_pob_check;						/* Enable power output board error check       */
	bool is_pob[PM_MAX];						/* The i'th power module is power output board */
	bool output;								/* The state of power output board             */
	int  pob_check_count;
	int  pob_retry_count[PM_MAX];
} PM_INFO;


void detect_ioexp_board(PORT_POB __iomem *pob, 
						PORT_I2C __iomem *i2c, 
						PM_INFO *pm_info, 
						PM_ERROR *pm_error);
void check_pob_error(PORT_POB __iomem *pob, 
					 PM_INFO *pm_info, 
					 PM_ERROR *pm_error, 
					 u16 int_monitor, 
					 u16 break_fuse);
void stop_pob_supply(PORT_POB __iomem *pob, PM_INFO *pm_info);
void start_pob_supply(struct work_struct *work);
int do_ioctl_read_pobavailable(void *arg, const PM_INFO *pm_info);
int do_ioctl_read_pobstate(void *arg, const PM_INFO *pm_info);

#endif /* _IPC8_PM_H */
