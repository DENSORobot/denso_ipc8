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


#include <linux/kernel.h>
#include <linux/string.h>
#include <asm/uaccess.h>
#include "ipc8_ioctl.h"
#include "ipc8_common.h"
#include "ipc8_fpga.h"

static void get_fpga_version(u32 base_addr, u16 *buf, u8 __iomem *iomem3)
{
	int i;
	
	DEBUG_PRINT("-->\n");
	
	for (i =0; i < (FPGA_VER_LENGTH / 2); i++) {
		buf[i] = bar3_read16(iomem3 + base_addr + 2 * i);
	}
	
	DEBUG_PRINT("<--\n");
}

int initialize_fpga(u8 __iomem *iomem3, FPGA_INFO *FpgaInfo)
{
	int ret = 0;
	
	DEBUG_PRINT("-->\n");
	
	/* FPGA version acquisition */
	get_fpga_version(FPGA_ADDR_MOTION_VER, 
					 (u16 *)FpgaInfo->fpga_version[FPGA_AREA_MOTION], 
					 iomem3);
	get_fpga_version(FPGA_ADDR_IO_VER,
					 (u16 *)FpgaInfo->fpga_version[FPGA_AREA_IO], 
					 iomem3);
	
	DEBUG_PRINT("<--\n");
	return ret;
}

int do_ioctl_read_fpgaversion(void* arg, const FPGA_INFO *fpga_info)
{
	int ret = 0;
	FPGA_VERSION fpga_ver;
	
	DEBUG_PRINT("-->\n");
	
	memset(&fpga_ver, 0, sizeof(FPGA_VERSION));
	
	memcpy(fpga_ver.motion_ver, 
			fpga_info->fpga_version[FPGA_AREA_MOTION], 
			sizeof(fpga_info->fpga_version[FPGA_AREA_MOTION]));
	memcpy(fpga_ver.io_ver, 
			fpga_info->fpga_version[FPGA_AREA_IO], 
			sizeof(fpga_info->fpga_version[FPGA_AREA_IO]));
	
	fpga_ver.outval = 0;
	
	if (copy_to_user((void *)arg, &fpga_ver, sizeof(FPGA_VERSION))) {
		ret = -EFAULT;
	}
	
	DEBUG_PRINT("<--\n");
	return ret;
}

