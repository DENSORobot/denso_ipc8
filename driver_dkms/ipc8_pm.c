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
#include <linux/delay.h>
#include <linux/atomic.h>
#include <linux/workqueue.h>
#include <asm/uaccess.h>
#include "ipc8_common.h"
#include "ipc8_ioctl.h"
#include "ipc8.h"


static void enable_power_output_board(PORT_POB __iomem *pob, 
									  PM_INFO *pm_info, 
									  PM_ERROR *pm_error, 
									  const char *ioexp)
{
	int i;
	u16 mask = 0;
	
	DEBUG_PRINT("-->\n");
	
	for (i=0; i<PM_MAX; i++) {
		if (IOEXP_ID_NOTHING != *(ioexp + i)) {
			switch (*(ioexp + i)) {
			case IOEXP_ID_POB:
				pm_info->is_pob[i] = true;
				mask |= (1 << (4 * i));
				break;
			default:
				/* Error recording */
				pm_error->error |= PM_ERROR_POB;
				pm_error->pob_error[i] = POB_UNKNOWN_ID;
				break;
			}
		}
	}
	
	bar2_write16(mask, &pob->board_switch, false);
	
	DEBUG_PRINT("<--\n");
}

static void get_ioexp_board(PORT_I2C __iomem *i2c, char *ioexp)
{
	int i;
	u32 val;
	
	for (i=0; i<PM_MAX; i++) {
		val = bar2_read32(&i2c->io_expander[i]);
		if(!(val & I2C_UNREC)) {
			*(ioexp + i) = (char)(val &  IOEXP_ID_MASK);
		}
		else {
			*(ioexp + i) = IOEXP_ID_NOTHING;
		}
	}
}

void detect_ioexp_board(PORT_POB __iomem *pob, 
						PORT_I2C __iomem *i2c, 
						PM_INFO *pm_info, 
						PM_ERROR *pm_error)
{
	int check_count = 0;
	int retry_count = 0;
	char ioexp_now[PM_MAX];
	char ioexp_before[PM_MAX];
	
	DEBUG_PRINT("-->\n");
	
	memset(ioexp_now, 0, PM_MAX);
	memset(ioexp_before, IOEXP_ID_NOTHING, PM_MAX);
	
	while (1) {
		get_ioexp_board(i2c, ioexp_now);
		
		if (!memcmp(ioexp_before, ioexp_now, PM_MAX)) {
			check_count++;
			if (LIMIT_I2C_CHECK <= check_count) {
				break;
			}
		}
		else {
			retry_count++;
			check_count = 1;
			
			if (LIMIT_I2C_RETRY <= retry_count) {
				memset(ioexp_now, IOEXP_ID_NOTHING, PM_MAX);
				/* Error recording */
				pm_error->error |= PM_ERROR_TIMEOUT;
				break;
			}
			else {
				memcpy(ioexp_before, ioexp_now, PM_MAX);
			}
		}
		
		msleep(I2C_COMM_WAIT);
	}
	
	/* Enable power output board */
	enable_power_output_board(pob, pm_info, pm_error, ioexp_now);
	
	DEBUG_PRINT("<--\n");
}

void check_pob_error(PORT_POB __iomem *pob, 
					 PM_INFO *pm_info, 
					 PM_ERROR *pm_error, 
					 u16 int_monitor, 
					 u16 break_fuse)
{
	int i;
	char jnt1, jnt2;
	
	if (pm_info->enable_pob_check) {
		if (int_monitor & INTSTATUS_POB_OVP) {
			stop_pob_supply(pob, pm_info);
			/* Error recording */
			pm_error->error |= PM_ERROR_POB_OVP;
		}
		
		for (i=0; i<PM_MAX; i++) {
			if (pm_info->is_pob[i]) {
				jnt1 = IPM_COMBINATION[i][0];
				jnt2 = IPM_COMBINATION[i][1];
				
				if ((break_fuse & (1 << jnt1))
					|| (break_fuse & (1 << jnt2))) {
					stop_pob_supply(pob, pm_info);
					/* Error recording */
					pm_error->error        |= PM_ERROR_POB;
					pm_error->pob_error[i] |= POB_FUSE_CTRL;
				}
			}
		}
	}
}

void stop_pob_supply(PORT_POB __iomem *pob, PM_INFO *pm_info)
{
	bar2_write16(POB_OVP_CHK_DISABLE, &pob->ovp_check_enable, false);
	
	pm_info->enable_pob_check = false;
	
	bar2_write16(POB_OFF, &pob->power_supply, false);
	
	pm_info->output = false;
}

void start_pob_supply(struct work_struct *work)
{
	IPC8_DEVICE *dev;
	u16 mask = 0;
	int i;
	char jnt1, jnt2;
	
	dev = container_of(work, IPC8_DEVICE, supply_pob);
	
	for (i=0; i<PM_MAX;i++) {
		if (dev->pm_info.is_pob[i]) {
			jnt1 = IPM_COMBINATION[i][0];
			jnt2 = IPM_COMBINATION[i][1];
			
			mask |= 1 << jnt1;
			mask |= 1 << jnt2;
		}
	}
	
	if (0 < mask) {
		bar2_write16(mask, &dev->bar2mem->pob.power_supply, false);
		dev->pm_info.output = true;
		msleep(POB_OVP_WAIT);
		bar2_write16(POB_OVP_CHK_ENABLE, &dev->bar2mem->pob.ovp_check_enable, false);
		dev->pm_info.enable_pob_check = true;
	}
	
	atomic_set(&dev->pob_state_lock, 0);
}

int do_ioctl_read_pobavailable(void *arg, const PM_INFO *pm_info)
{
	int i;
	int ret = 0;
	VAR_DATA var_data;
	
	DEBUG_PRINT("-->\n");
	
	memset(&var_data, 0, sizeof(VAR_DATA));
	
	for (i=0; i<PM_MAX; i++) {
		if (pm_info->is_pob[i]) {
			var_data.data |= (1 << i);
		}
	}
	
	if (copy_to_user(arg, &var_data, sizeof(VAR_DATA))) {
		ret = -EFAULT;
	}
	
	DEBUG_PRINT("<--\n");
	return ret;
}

int do_ioctl_read_pobstate(void *arg, const PM_INFO *pm_info)
{
	int ret = 0;
	VAR_DATA var_data;
	
	DEBUG_PRINT("-->\n");
	
	memset(&var_data, 0, sizeof(VAR_DATA));
	
	var_data.data = ((0 == pm_info->output)? POB_OFF : POB_ON);
	var_data.outval = 0;
	
	if (copy_to_user(arg, &var_data, sizeof(VAR_DATA))) {
		ret = -EFAULT;
	}
	
	DEBUG_PRINT("<--\n");
	return ret;
}

