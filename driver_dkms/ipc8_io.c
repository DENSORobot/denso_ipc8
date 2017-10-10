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
#include <linux/slab.h>
#include <linux/string.h>
#include <asm/uaccess.h>
#include "ipc8_ioctl.h"
#include "ipc8_common.h"
#include "ipc8_io.h"

int initialize_io(PORT_IO __iomem *io, IO_INFO *io_info)
{
	DEBUG_PRINT("-->\n");
	
	/* I/O Power Supply Type initialization */
	io_info->io_pwr_mode = IO_POWER_UNKNOWN;
	
	/* Get IO board type */
	io_info->io_board_type = bar2_read16(&io->board_np_type);
	
	DEBUG_PRINT("<--\n");
	return 0;
}

void get_io_state(IO_INFO *io_info, RECVDMAMAP_STATUS2 *recvdma)
{
	u16 safety_state;
	
	io_info->mini_in = recvdma->mini_io;
	io_info->hand_in = recvdma->hand_io;
	safety_state     = recvdma->safety_state;
	if (safety_state & 0x03) {	/* bit[1:0] */
		io_info->emergency_stop = true;
	}
	else {
		io_info->emergency_stop = false;
	}
	if (safety_state & 0x0C) {	/* bit[3:2] */
		io_info->dead_man       = true;
	}
	else {
		io_info->dead_man       = false;
	}
	if (safety_state & 0xC0) {	/* bit[7:6] */
		io_info->protect_stop   = true;
	}
	else {
		io_info->protect_stop   = false;
	}
	if (safety_state & 0x30) {	/* bit[5:4] */
		io_info->auto_enable    = true;
	}
	else {
		io_info->auto_enable    = false;
	}
	io_info->io_pwr_mode = recvdma->io_power_state;
}

void set_io_state(IO_INFO *io_info, SENDDMAMAP_STATUS *senddma)
{
	senddma->mini_io = io_info->mini_out;
	senddma->hand_io = io_info->hand_out;
}

void read_write_io(SENDDMAMAP_STATUS *send_dma,
				   RECVDMAMAP_STATUS2 *recv_dma, 
				   IO_INFO *io_info, 
				   IO_ERROR *io_error)
{
	u16 safety;
	
	/* I/O input information acquisition */
	io_info->mini_in = recv_dma->mini_io;
	io_info->hand_in = recv_dma->hand_io;
	
	/* I/O safety information acquisition */
	safety = recv_dma->safety_state;
	io_info->emergency_stop = (safety & EMERGENCY_SW);
	io_info->dead_man       = ((safety & DEAD_MAN_SW) == DEAD_MAN_SW);
	io_info->protect_stop   = (safety & PRT_STOP_SW);
	io_info->auto_enable    = (safety & AUTO_ENB_SW);
	
	/* I/O output information set */
	send_dma->mini_io = io_info->mini_out;
	send_dma->hand_io = io_info->hand_out;
}

int check_powermode(IO_POWERMODE* io_powermode, s32 io_pwr_mode)
{
	int ret = 1;
	io_powermode->outval = 0;
	
	DEBUG_PRINT("-->\n");
	
	switch (io_pwr_mode) {
	case IO_POWER_EXTERNAL:
	case IO_POWER_NO_SUPPLY:
		if (IO_POWER_EXTERNAL == io_powermode->mode) {
			io_powermode->outval = 0;
			ret = 0;
		}
		else {
			io_powermode->outval = IO_POWER_ERR_ALREADY;
			ret = -EALREADY;
		}
		break;
	case IO_POWER_INTERNAL:
		if (IO_POWER_INTERNAL == io_powermode->mode) {
			io_powermode->outval = 0;
			ret = 0;
		}
		else {
			io_powermode->outval = IO_POWER_ERR_ALREADY;
			ret = -EALREADY;
		}
		break;
	case IO_POWER_UNKNOWN:
	default:
		break;
	}
	
	DEBUG_PRINT("<--\n");
	return ret;
}

int do_ioctl_read_io(void* arg, const IO_INFO *io_info, const IO_ERROR *io_error, bool is_mini)
{
	int ret = 0;
	IO_READ io_read;
	
	DEBUG_PRINT("-->\n");
	
	memset(&io_read, 0, sizeof(IO_READ));
	
	switch (io_info->io_pwr_mode) {
	case IO_POWER_EXTERNAL:
	case IO_POWER_INTERNAL:
		if (0 < io_error->error) {
			io_read.outval = IO_ERR_ERROR_RAISED;
			ret = -EIO;
		}
		else {
			if (is_mini) {
				/* MINI I/O */
				io_read.io_input  = io_info->mini_in;
				io_read.io_output = io_info->mini_out;
			}
			else {
				/* HAND I/O */
				io_read.io_input  = (u16)(io_info->hand_in & HAND_IO_MASK_ALL);
				io_read.io_output = (u16)(io_info->hand_out & HAND_IO_MASK_ALL);
			}
		}
		break;
	default:
		io_read.outval = IO_ERR_MODE_UNKNOWN;
		ret = -EIO;
		break;
	}
	
	if (copy_to_user(arg, &io_read, sizeof(IO_READ))) {
		ret = -EFAULT;
	}
	
	DEBUG_PRINT("<--\n");
	return ret;
}

int do_ioctl_write_io(void* arg, IO_INFO* io_info, const IO_ERROR *io_error, bool is_mini)
{
	int ret = 0;
	IO_WRITE io_write;
	u16 tmp1, tmp2;
	
	DEBUG_PRINT("-->\n");
	
	if (copy_from_user(&io_write, arg, sizeof(IO_WRITE))) {
		ret = -EFAULT;
	}
	else {
		io_write.outval = 0;
		
		switch (io_info->io_pwr_mode) {
		case IO_POWER_EXTERNAL:
		case IO_POWER_INTERNAL:
			if (0 < io_error->error) {
				io_write.outval = IO_ERR_ERROR_RAISED;
				ret = -EIO;
			}
			else {
				tmp1 = (io_write.io_output & io_write.io_mask);
				if (is_mini) {
					/* MINI I/O */
					tmp2 = (io_info->mini_out & ~(io_write.io_mask));
					io_info->mini_out = (tmp1 | tmp2);
				}
				else {
					/* HAND I/O */
					tmp2 = (io_info->hand_out & ~(io_write.io_mask));
					io_info->hand_out = (u8)((tmp1 | tmp2) & HAND_IO_MASK_ALL);
				}
			}
			break;
		default:
			io_write.outval = IO_ERR_MODE_UNKNOWN;
			ret = -EIO;
			break;
		}
		
		if (copy_to_user(arg, &io_write, sizeof(IO_WRITE))) {
			ret = -EFAULT;
		}
	}
	
	DEBUG_PRINT("<--\n");
	return ret;
}

int do_ioctl_read_safetyio(void* arg, const IO_INFO *io_info)
{
	int ret = 0;
	IO_SAFETY io_safety;
	
	DEBUG_PRINT("-->\n");
	
	memset(&io_safety, 0, sizeof(IO_SAFETY));
	
	io_safety.emergency_stop = io_info->emergency_stop;
	io_safety.dead_man       = io_info->dead_man;
	io_safety.protect_stop   = io_info->protect_stop;
	io_safety.auto_enable    = io_info->auto_enable;
	
	if (copy_to_user(arg, &io_safety, sizeof(IO_SAFETY))) {
		ret = -EFAULT;
	}
	
	DEBUG_PRINT("<--\n");
	return ret;
}

int do_ioctl_read_ioboardtype(void* arg, const IO_INFO *io_info)
{
	int ret = 0;
	VAR_DATA var_data;
	
	DEBUG_PRINT("-->\n");
	
	memset(&var_data, 0, sizeof(VAR_DATA));
	
	var_data.data = io_info->io_board_type;
	var_data.outval = 0;
	
	if (copy_to_user(arg, &var_data, sizeof(VAR_DATA))) {
		ret = -EFAULT;
	}
	
	DEBUG_PRINT("<--\n");
	return ret;
}

