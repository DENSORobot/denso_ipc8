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
#include "ipc8_fram.h"

u8 __iomem *fram_sys;
u8 __iomem *fram_usr;
u32        *fram_local_sys;
u32        *fram_local_usr;


int initialize_fram(u8 __iomem *iomem0, u32 *fram_buffer)
{
	int i, addr;
	
	fram_sys       = iomem0 + FRAM_OFFSET;
	fram_usr       = iomem0 + FRAM_USER_OFFSET;
	fram_local_sys = fram_buffer;
	fram_local_usr = fram_buffer + FRAM_RESERV / FRAM_ACCESS_SIZE;
	
	/* Reads the entire local FRAM */
	for (i=0, addr = 0; addr < FRAM_LENGTH; i++, addr += FRAM_ACCESS_SIZE) {
		fram_buffer[i] = bar0_read32(fram_sys + addr);
	}
	
	return 0;
}

static int check_ioctl_argument(FRAM_DATA *fram_data)
{
	int ret = 0;
	fram_data->outval = 0;
	
	/* Error checking */
	if ((fram_data->offset % FRAM_ACCESS_SIZE) 
		|| (fram_data->length % FRAM_ACCESS_SIZE)) {
		fram_data->outval = FRAM_ERR_ALIGNMENT;
		ret = -EINVAL;
	}
	else if ((FRAM_USER_LENGTH < fram_data->offset) 
			|| (FRAM_USER_LENGTH < (fram_data->offset + fram_data->length))) {
		fram_data->outval = FRAM_ERR_OVERRUN;
		ret = -EINVAL;
	}
	
	return ret;
}

int do_ioctl_read_fram(void* arg)
{
	int ret = 0;
	FRAM_DATA fram;
	int i, addr;
	u32 read_data;
	
	DEBUG_PRINT("-->\n");
	
	if (copy_from_user(&fram, arg, sizeof(FRAM_DATA))) {
		ret = -EFAULT;
	}
	else {
		ret = check_ioctl_argument(&fram);
		if (0 == ret) {
			/* Read out by 32bit */
			for (i=0, addr=fram.offset; addr < fram.length; i++, addr += FRAM_ACCESS_SIZE) {
				read_data = fram_local_usr[i];
				if (put_user(read_data, fram.data+i)) {
					printk(KERN_ERR "%s: put_user failed. \n", __func__);
					fram.outval = FRAM_ERR_OTHER;
					ret = -EFAULT;
					break;
				}
			}
		}
		if (copy_to_user((void *)arg, &fram, sizeof(FRAM_DATA))) {
			ret = -EFAULT;
		}
	}
	
	DEBUG_PRINT("<--\n");
	return ret;
}

int do_ioctl_write_fram(void* arg)
{
	int ret = 0;
	FRAM_DATA fram;
	int i, addr;
	u32 write_data;
	size_t written = 0;
	
	DEBUG_PRINT("-->\n");
	
	if (copy_from_user(&fram, (void *)arg, sizeof(FRAM_DATA))) {
		ret = -EFAULT;
	}
	else {
		ret = check_ioctl_argument(&fram);
		if (0 == fram.outval) {
			/* Writing by 32bit */
			for (i=0, addr=fram.offset; addr < fram.length; i++, addr += FRAM_ACCESS_SIZE) {
				if (get_user(write_data, fram.data+i)) {
					printk(KERN_ERR "%s: get_user failed. \n", __func__);
					fram.outval = FRAM_ERR_OTHER;
					ret = -EFAULT;
					break;
				}
				else {
					written = bar0_write32(write_data, fram_usr + addr, false);
					if (0 <= written) {
						fram_local_usr[i] = write_data;
					}
					else {
						printk(KERN_ERR "%s: Write error. \n", __func__);
						fram.outval = FRAM_ERR_WRITE;
						ret = -EIO;
						break;
					}
				}
			}
		}
		if (copy_to_user((void *)arg, &fram, sizeof(FRAM_DATA))) {
			ret = -EFAULT;
		}
	}
	
	DEBUG_PRINT("<--\n");
	return ret;
}

/* Debug */
u32 fram_read_local_sys(u32 ofs)
{
	return fram_local_sys[ofs];
}

size_t fram_write_local_sys(u32 ofs, u32 write_data)
{
	size_t written = 0;
	int addr = ofs * FRAM_ACCESS_SIZE;
	
	written = bar0_write32(write_data, fram_sys + addr, false);
	if (0 <= written) {
		fram_local_sys[ofs] = write_data;
	}
	return written;
}
