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
#include "ipc8_fan.h"

static u16 calc_fan_speed_duty(u16 fan_speed)
{
	u16 duty = 100;
	
	if (20 >= fan_speed) {
		duty = 15;
	}
	else if ((20 < fan_speed) && (26 >= fan_speed)) {
		duty = fan_speed * 19 / 25;
	}
	else if ((26 < fan_speed) && (92 >= fan_speed)) {
		duty = (fan_speed * 114 - 500) / 125;
	}
	else if ((92 < fan_speed) && (100 >= fan_speed)) {
		duty = (fan_speed * 38 - 2300) / 15;
	}
	else if (100 < fan_speed) {
		duty = 100;
	}
	
	return duty;
}

static void chk_fan_life(PORT_FANCTRL __iomem *fan_ctrl, FAN_ERROR *fan_error)
{
	DEBUG_PRINT("-->\n");
	
	fan_error->fan_life_warn = bar2_read16(&fan_ctrl->fan_life_warn);
	
	DEBUG_PRINT("<--\n");
}

int initialize_fan(PORT_FANCTRL __iomem *fan_ctrl, 
				   FAN_INFO *fan_info, 
				   FAN_ERROR *fan_error)
{
	int ret = 0;
	
	DEBUG_PRINT("-->\n");
	
	/* Initial value(Fan Speed) */
	set_fan_speed_local(fan_info, FAN_INITSPEED_PER);
	
	/* Check the life of the fan */
	chk_fan_life(fan_ctrl, fan_error);
	
	DEBUG_PRINT("<--\n");
	return ret;
}

static u16 calc_fan_speed_per(u16 fan_speed_duty){
	return 100 * 15000000 / 7600 / fan_speed_duty;
}

u16 get_actual_fan_speed(void __iomem *addr)
{
	u16 speed, duty;
	
	DEBUG_PRINT("-->\n");
	
	duty = bar2_read16(addr);
	
	speed = calc_fan_speed_per(duty);
	
	DEBUG_PRINT("<-- [duty=%d][speed=%d]\n",  duty, speed);
	return speed;
}

void set_fan_speed_dma(SENDDMAMAP_FANCTRL *send_dma, FAN_INFO *fan_info)
{
	send_dma->speed = (u8)fan_info->fan_speed_duty;
}

void set_fan_speed_local(FAN_INFO *fan_info, u16 fan_speed)
{
	fan_info->fan_speed      = fan_speed;
	fan_info->fan_speed_duty = calc_fan_speed_duty(fan_speed);
}

int do_ioctl_read_fanspeed(void* arg, const FAN_INFO *fan_info)
{
	int ret = 0;
	FAN_SPEED fan_speed;
	
	DEBUG_PRINT("-->\n");
	
	memset(&fan_speed, 0, sizeof(FAN_SPEED));
	
	fan_speed.speed = fan_info->fan_speed;
	fan_speed.outval = 0;
	if (copy_to_user(arg, &fan_speed, sizeof(FAN_SPEED))) {
		ret = -EFAULT;
	}
	
	DEBUG_PRINT("<--\n");
	return ret;
}

int do_ioctl_write_fanspeed(void* arg, FAN_INFO* fan_info)
{
	int ret = 0;
	FAN_SPEED fan_speed;
	
	DEBUG_PRINT("-->\n");
	
	if (copy_from_user(&fan_speed, arg, sizeof(FAN_SPEED))) {
		ret = -EFAULT;
	}
	else {
		if ((FAN_MINSPEED_PER <= fan_speed.speed) 
			&& (FAN_MAXSPEED_PER >= fan_speed.speed)) {
			set_fan_speed_local(fan_info, fan_speed.speed);
			fan_speed.outval = 0;
		}
		else {
			fan_speed.outval = FAN_ERR_SPEED;
			ret = -EINVAL;
		}
		if (copy_to_user(arg, &fan_speed, sizeof(FAN_SPEED))) {
			ret = -EFAULT;
		}
	}
	
	DEBUG_PRINT("<--\n");
	return ret;
}

