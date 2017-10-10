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


#ifndef _IPC8_FPGA_H
#define _IPC8_FPGA_H


#define FPGA_ADDR_MOTION_VER	(0x0037FFC0)				/* Address of fpga motion version */
#define FPGA_ADDR_IO_VER		(0x003FFFC0)				/* Address of fpga I/O version    */
#define FPGA_VER_LENGTH			(0x00000040)				/* Version size (64Byte)          */

#define FPGA_WDT_ALLOW			(3)							/* 3 * TIMER_INTERVAL_100MS [ms]  */

typedef enum FPGA_AREA {
	FPGA_AREA_MOTION,										/* FPGA motion */
	FPGA_AREA_IO,											/* FPGA I/O    */
	FPGA_AREA_MAX,
} FPGA_AREA;

typedef struct FPGA_INFO {
	char fpga_version[FPGA_AREA_MAX][FPGA_VER_LENGTH];		/* FPGA version string */
} FPGA_INFO;



int initialize_fpga(u8 __iomem *iomem3, FPGA_INFO *FpgaInfo);
int do_ioctl_read_fpgaversion(void* arg, const FPGA_INFO *fpga_info);

#endif /* _IPC8_FPGA_H */
