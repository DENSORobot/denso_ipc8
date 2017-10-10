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


#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/proc_fs.h>
#include <linux/pci.h>
#include <linux/delay.h>
#include <linux/version.h>
#include <linux/interrupt.h>
#include <linux/atomic.h>
#include <asm/uaccess.h>
#include <asm/msr.h>
#include <acpi/acpi_drivers.h>
#include <acpi/processor.h>
#include "ipc8_common.h"
#include "ipc8_ioctl.h"
#include "ipc8.h"


static dev_t ipc8_devid;
static struct cdev c_dev;
static IPC8_DEVICE *ipc8_pci;


int bar0_write32(u32 val, void __iomem *addr, bool verify)
{
	int ret = 0;	/* Return -1 write error during */
	u32 read;
	unsigned long flags;
	
	if (ipc8_pci) {
		spin_lock_irqsave(&ipc8_pci->mem0_lock, flags);
		
		iowrite32(val, addr);
		
		spin_unlock_irqrestore(&ipc8_pci->mem0_lock, flags);
		
		if (verify) {
			read = bar0_read32(addr);
			if (read != val) {
				printk(KERN_ERR "%s: verify error. (write=0x%x read=0x%x) \n", __func__, val, read);
				ret = -1;
			}
		}
		
	}
	
	return ret;
}

u32 bar0_read32(void __iomem *addr)
{
	u32 ret = 0;
	unsigned long flags;
	
	if (ipc8_pci) {
		spin_lock_irqsave(&ipc8_pci->mem0_lock, flags);
		
		ret = ioread32(addr);
		
		spin_unlock_irqrestore(&ipc8_pci->mem0_lock, flags);
	}
	
	return ret;
}

int bar2_write16(u16 val, void __iomem *addr, bool verify)
{
	int ret = 0;	/* Return -1 write error during */
	u16 read;
	unsigned long flags;
	
	if (ipc8_pci) {
		spin_lock_irqsave(&ipc8_pci->mem2_lock, flags);
		
		iowrite16(val, addr);
		
		spin_unlock_irqrestore(&ipc8_pci->mem2_lock, flags);
		
		if (verify) {
			read = bar2_read16(addr);
			if (read != val) {
				printk(KERN_ERR "%s: verify error. (write=0x%08x read=0x%04x) \n", __func__, val, read);
				ret = -1;
			}
		}
	}
	
	return ret;
}

u16 bar2_read16(void __iomem *addr)
{
	u16 ret = 0;
	unsigned long flags;
	
	if (ipc8_pci) {
		spin_lock_irqsave(&ipc8_pci->mem2_lock, flags);
		
		ret = ioread16(addr);
		
		spin_unlock_irqrestore(&ipc8_pci->mem2_lock, flags);
	}
	
	return ret;
}

int bar2_write32(u32 val, void __iomem *addr, bool verify)
{
	int ret = 0;	/* Return -1 write error during */
	u16 read;
	unsigned long flags;
	
	if (ipc8_pci) {
		spin_lock_irqsave(&ipc8_pci->mem2_lock, flags);
		
		iowrite32(val, addr);
		
		spin_unlock_irqrestore(&ipc8_pci->mem2_lock, flags);
		
		if (verify) {
			read = bar2_read32(addr);
			if (read != val) {
				printk(KERN_ERR "%s: verify error. (write=0x%08x read=0x%04x) \n", __func__, val, read);
				ret = -1;
			}
		}
	}
	
	return ret;
}

u32 bar2_read32(void __iomem *addr)
{
	u32 ret = 0;
	unsigned long flags;
	
	if (ipc8_pci) {
		spin_lock_irqsave(&ipc8_pci->mem2_lock, flags);
		
		ret = ioread32(addr);
		
		spin_unlock_irqrestore(&ipc8_pci->mem2_lock, flags);
	}
	
	return ret;
}

u16 bar3_read16(void __iomem *addr)
{
	u16 ret = 0;
	unsigned long flags;
	
	if (ipc8_pci) {
		spin_lock_irqsave(&ipc8_pci->mem3_lock, flags);
		
		ret = ioread16(addr);
		
		spin_unlock_irqrestore(&ipc8_pci->mem3_lock, flags);
	}
	
	return ret;
}


static void get_cpu_temperature(IPC8_DEVICE *pdev)
{
	int i;
	int ret = 0;
	int msr_addr;
	u32 eax, edx;
	int cpu_cores, index, temp, msr, min_msr = INT_MAX;
	
	/* Get online number of cpu cores */
	cpu_cores = num_online_cpus();
	
	msr_addr = 0x0000019C;
	for (i=0; i<cpu_cores; i++) {
		ret = rdmsr_safe_on_cpu(i, msr_addr, &eax, &edx);
		msr = (eax >> 16) & 0x7F;
		if (msr < min_msr) {
			min_msr = msr;
		}
	}
	
	index = pdev->cpu_tj_max - min_msr;
	
	/* Temperature correction map */
	if (index > 0) {
		if (index < TEMPERATURE_MAP_MAX) {
			temp = TEMPERATURE_MAP[index];
		}
		else {
			temp = TEMPERATURE_MAP[TEMPERATURE_MAP_MAX];
		}
	}
	else {
		temp = TEMPERATURE_MAP[0];
	}
	pdev->temp_data.cpu_temp = temp;
}

static void get_temperature(IPC8_DEVICE *pdev)
{
	PSU_COMM_DATA data;
	u16 send_data = 0;
	
	pdev->temp_check_count++;
	
	if (LIMIT_TEMP_CHECK <= pdev->temp_check_count) {
		pdev->temp_check_count = 0;
		
		/* CPU temperature acquisition */
		get_cpu_temperature(pdev);
		
		/* Set for information acquisition packet */
		/* Power CPU temperature */
		memset(&data, 0, sizeof(PSU_COMM_DATA));
		send_data = PSU_DATA_TEMPERATURE_PSU;
		data.packet      = create_psu_packet(PSU_CMD_TEMPERATURE, 1, &send_data);
		data.result_addr = (u8 *)&pdev->temp_data.power_mother_temp;
		data.result_size = sizeof(pdev->temp_data.power_mother_temp);
		data.wait        = false;
		send_enqueue(data);
		
		/* Motor power temperature */
		memset(&data, 0, sizeof(PSU_COMM_DATA));
		send_data = PSU_DATA_TEMPERATURE_MOTPWR;
		data.packet      = create_psu_packet(PSU_CMD_TEMPERATURE, 1, &send_data);
		data.result_addr = (u8 *)&pdev->temp_data.motor_power_temp;
		data.result_size = sizeof(pdev->temp_data.motor_power_temp);
		data.wait        = false;
		send_enqueue(data);
		
		/* Input voltage run value */
		memset(&data, 0, sizeof(PSU_COMM_DATA));
		send_data = PSU_DATA_ACVOLT;
		data.packet      = create_psu_packet(PSU_CMD_ACVOLT, 2, &send_data);
		data.result_addr = (u8 *)&pdev->psu_info.ac_input_voltage;
		data.result_size = sizeof(pdev->psu_info.ac_input_voltage);
		data.wait        = false;
		send_enqueue(data);
	}
}

static void temp_worker(struct work_struct *work)
{
	IPC8_DEVICE *dev;
	
	dev = container_of(work, IPC8_DEVICE, temp_work);
	
	get_temperature(dev);
}

static void detect_module_capacity_worker(struct work_struct *work)
{
	IPC8_DEVICE *dev;
	
	dev = container_of(work, IPC8_DEVICE, detect_pm);
	
	if (dev->pm_info.enable_pob) {
		detect_ioexp_board(&dev->bar2mem->pob, 
							&dev->bar2mem->i2c, 
							&dev->pm_info, 
							&dev->pm_error);
	}
}

static void check_io_error(IPC8_DEVICE *pdev)
{
	u16 fuse;
	u8 check1, check2;
	
	/* I/O fuse status acquisition */
	fuse = (((u16)pdev->recv_dma_addr->status2.io_power_state << 8) 
			| (u16)pdev->recv_dma_addr->status2.io_power_poly_sw);
	
	/* External power supply reverse connection check */
	if (fuse & IO_FUSE_REVERSE) {
		/* Error recording */
		pdev->io_error.error   |= IO_ERROR_FUSE;
		pdev->io_error.io_fuse  = IO_FUSE_REVERSE;
		return;
	}
	
	/* External power supply check */
	if (fuse & IO_FUSE_NO_SUPPLY) {
		/* Error recording */
		pdev->io_error.error   |= IO_ERROR_FUSE;
		pdev->io_error.io_fuse  = IO_FUSE_NO_SUPPLY;
		return;
	}
	
	/* Error clear */
	pdev->io_error.error   &= ~IO_ERROR_FUSE;
	pdev->io_error.io_fuse  = 0;
	
	switch (pdev->io_info.io_pwr_mode) {
	case IO_POWER_EXTERNAL:
	case IO_POWER_INTERNAL:
		if (LIMIT_IO_CHECK <= pdev->io_check_count) {
			/* I/O fuse check */
			pdev->io_error.io_fuse = 
				(pdev->io_info.io_pwr_mode == IO_CHECK_INTERNAL ? IO_CHECK_INTERNAL : IO_CHECK_EXTERNAL) 
				& fuse;
			if (pdev->io_error.io_fuse) {
				/* Error recording */
				pdev->io_error.error |= IO_ERROR_FUSE;
			}
			
			/* I/O access check */
			check1 = pdev->recv_dma_addr->status2.access_check1;
			check2 = pdev->recv_dma_addr->status2.access_check2;
			if ((check1 != IO_ACCESS_CHECK1) 
				|| (check2 != IO_ACCESS_CHECK2)) {
				/* Error recording */
				pdev->io_error.error |= IO_ERROR_ACCESS;
			}
			
			/* Mini I/O heating status check */
			pdev->io_error.mini_io_oc = 
				pdev->recv_dma_addr->status2.mini_io_ov_temp;
			if (pdev->io_error.mini_io_oc) {
				/* Error recording */
				pdev->io_error.error |= IO_ERROR_MINI_OC;
			}
			
			/* Hand I/O heating status check */
			pdev->io_error.hand_io_oc = 
				(u16)pdev->recv_dma_addr->status2.hand_io_ov_temp & HAND_IO_MASK_ALL;
			if (pdev->io_error.hand_io_oc) {
				/* Error recording */
				pdev->io_error.error |= IO_ERROR_HAND_OC;
			}
		}
		else {
			pdev->io_check_count++;
		}
		break;
	default:
		break;
	}
}

void check_pob_state(IPC8_DEVICE *pdev)
{
	int i;
	u8 i2c_state;
	u8 pob_state;
	u16 local_error;
	
	if (pdev->pm_info.enable_pob 
			&& pdev->pm_info.enable_pob_check) {
		pdev->pm_info.pob_check_count++;
		if (LIMIT_POB_CHECK <= pdev->pm_info.pob_check_count) {
			pdev->pm_info.pob_check_count = 0;
			i2c_state = pdev->recv_dma_addr->status2.i2c_status;
			
			for (i=0; i<PM_MAX;i++) {
				local_error = 0;
				if (!(i2c_state & (1 << i))) {
					pob_state = pdev->recv_dma_addr->status2.pob_status[i];
					local_error = (pob_state & POB_FUSE_ALL);
				}
				
				if (0 < local_error) {
					if (LIMIT_POB_RETRY <= pdev->pm_info.pob_retry_count[i]) {
						stop_pob_supply(&pdev->bar2mem->pob, &pdev->pm_info);
						
						/* Error recording */
						pdev->pm_error.error |= PM_ERROR_POB;
						pdev->pm_error.pob_error[i] |= local_error;
						
						pdev->pm_info.pob_retry_count[i] = 0;
					}
					else {
						pdev->pm_info.pob_retry_count[i]++;
					}
				}
				else {
					pdev->pm_info.pob_retry_count[i] = 0;
				}
			}
		}
	}
}

static void timer_thread_100ms(unsigned long arg)
{
	IPC8_DEVICE *dev = (IPC8_DEVICE *)arg;
	
	/* Delay processing execution */
	queue_work(dev->workqueue, &dev->timer_100ms.work);
	
	/* Timer update */
	dev->timer_100ms.timer.expires = jiffies + 
				msecs_to_jiffies(TIMER_INTERVAL_100MS);
	add_timer(&dev->timer_100ms.timer);
}

static void timer_worker_100ms(struct work_struct *work)
{
	IPC8_DEVICE *dev;
	TIMER_THREAD *timer;
	int cnt;
	
	timer = container_of(work, TIMER_THREAD, work);
	dev   = container_of(timer, IPC8_DEVICE, timer_100ms);
	
	if (!(dev->fpga_error.error & FPGA_ERROR_WDT)) {
		/* Watchdog count increment */
		cnt = atomic_inc_return(&dev->fpga_wdt_count);
		if (FPGA_WDT_ALLOW < cnt) {
			printk(KERN_WARNING "%s: Watchdog detection ! (cnt:%d) \n", __func__, cnt);
			dev->fpga_error.error |= FPGA_ERROR_WDT;
		}
	}
	
	/* Condition monitoring of POB */
	check_pob_state(dev);
}

static void timer_thread_1s(unsigned long arg)
{
	IPC8_DEVICE *dev = (IPC8_DEVICE *)arg;
	
	/* Delay processing execution */
	queue_work(dev->workqueue, &dev->timer_1s.work);
	
	/* Timer update */
	dev->timer_1s.timer.expires = jiffies + 
				msecs_to_jiffies(TIMER_INTERVAL_1S);
	add_timer(&dev->timer_1s.timer);
}

static void timer_worker_1s(struct work_struct *work)
{
	IPC8_DEVICE *dev;
	TIMER_THREAD *timer;
	
	timer = container_of(work, TIMER_THREAD, work);
	dev   = container_of(timer, IPC8_DEVICE, timer_1s);
	
	check_io_error(dev);
	
	if (0 == dev->io_error.error) {
		if (IO_POWER_NO_SUPPLY == dev->io_info.io_pwr_mode) {
			bar2_write16(IO_POWER_EXTERNAL, &dev->bar2mem->io.power_mode, false);
			dev->io_info.io_pwr_mode = IO_POWER_EXTERNAL;
		}
	}
}

static void initialize_timer_thread(void)
{
	DEBUG_PRINT("-->\n");
	
	/* Watchdog count initialization */
	atomic_set(&ipc8_pci->fpga_wdt_count, 0);
	
	/* Work queue initialization  */
	INIT_WORK(&ipc8_pci->timer_100ms.work, timer_worker_100ms);
	INIT_WORK(&ipc8_pci->timer_1s.work, timer_worker_1s);
	
	/* Timer initialization  */
	init_timer(&ipc8_pci->timer_100ms.timer);
	ipc8_pci->timer_100ms.timer.data = (unsigned long)ipc8_pci;
	ipc8_pci->timer_100ms.timer.function = timer_thread_100ms;
	ipc8_pci->timer_100ms.timer.expires = jiffies + 
					msecs_to_jiffies(TIMER_INTERVAL_100MS);
	
	init_timer(&ipc8_pci->timer_1s.timer);
	ipc8_pci->timer_1s.timer.data = (unsigned long)ipc8_pci;
	ipc8_pci->timer_1s.timer.function = timer_thread_1s;
	ipc8_pci->timer_1s.timer.expires = jiffies + 
					msecs_to_jiffies(TIMER_INTERVAL_1S);
	
	/* Start Timer */
	add_timer(&ipc8_pci->timer_100ms.timer);
	add_timer(&ipc8_pci->timer_1s.timer);
	
	DEBUG_PRINT("<--\n");
}

static void terminate_timer_thread(void)
{
	DEBUG_PRINT("-->\n");
	
	/* Synchronization waiting until timer Delete */
	del_timer_sync(&ipc8_pci->timer_100ms.timer);
	del_timer_sync(&ipc8_pci->timer_1s.timer);
	/* Work queue flash */
	flush_workqueue(ipc8_pci->workqueue);
	/* Work queue Delete */
	destroy_workqueue(ipc8_pci->workqueue);
	
	DEBUG_PRINT("<--\n");
}


static int do_ioctl_write_iopowermode(void* arg)
{
	int ret = 0, check = 0;
	IO_POWERMODE io_pw_mode;
	u16 fuse;
	PSU_COMM_DATA s_data;
	PSU_PACKET r_data;
	int comm_ret;
	
	DEBUG_PRINT("-->\n");
	
	if (copy_from_user(&io_pw_mode, (void *)arg, sizeof(IO_POWERMODE))) {
		ret = -EFAULT;
		goto EndProc;
	}
	
	io_pw_mode.outval = 0;
	
	if((IO_POWER_EXTERNAL != io_pw_mode.mode) 
		&& (IO_POWER_INTERNAL != io_pw_mode.mode)) {
		io_pw_mode.outval = IO_POWER_ERR_MODE;
		ret = -EINVAL;
		goto EndProc;
	}
	
	check = check_powermode(&io_pw_mode, ipc8_pci->io_info.io_pwr_mode);
	if (0 >= check) {
		ret = check;
		goto EndProc;
	}
	
	/* Fuse state acquisition */
	fuse = bar2_read16(&ipc8_pci->bar2mem->io.out_pwr_state);
	
	/* External power supply reverse connection check */
	if (fuse & IO_FUSE_REVERSE) {
		/* Error recording */
		ipc8_pci->io_error.error   |= IO_ERROR_FUSE;
		ipc8_pci->io_error.io_fuse |= IO_FUSE_REVERSE;
		
		io_pw_mode.outval = IO_POWER_ERR_REVERSE;
		ret = -EIO;
		goto EndProc;
	}
	
	/* Error Clear */
	ipc8_pci->io_error.error &= ~IO_ERROR_FUSE;
	ipc8_pci->io_error.io_fuse = 0;
	
	switch (io_pw_mode.mode) {
	case IO_POWER_INTERNAL:
		if (!(fuse & IO_FUSE_NO_SUPPLY)) {
			/* In trying to internal power, but the external power is supplied */
			io_pw_mode.outval = IO_POWER_ERR_INTERNAL_SUPPLY;
			ret = -EIO;
		}
		else {
			bar2_write16((u16)io_pw_mode.mode, &ipc8_pci->bar2mem->io.power_mode, false);
			memset(&s_data, 0, sizeof(PSU_COMM_DATA));
			memset(&r_data, 0, sizeof(PSU_PACKET));
			
			/* 24VL_IO_ON */
			s_data.packet      = create_psu_packet(PSU_CMD_IOPWR_ON, 0, NULL);
			s_data.result_addr = NULL;
			s_data.result_size = 0;
			s_data.wait        = true;
			
			comm_ret = set_tx_pkt_sync(s_data, &r_data);
			if (0 < ipc8_pci->psu_error.error) {
				io_pw_mode.outval = IO_POWER_ERR_COMMUNICATION;
				ret = -ECOMM;
			}
			else {
				ipc8_pci->io_info.io_pwr_mode = IO_POWER_INTERNAL;
				io_pw_mode.outval = 0;
			}
		}
		break;
	case IO_POWER_EXTERNAL:
		if (!(fuse & IO_FUSE_NO_SUPPLY)) {
			bar2_write16((u16)io_pw_mode.mode, &ipc8_pci->bar2mem->io.power_mode, false);
			ipc8_pci->io_info.io_pwr_mode = IO_POWER_EXTERNAL;
			io_pw_mode.outval = 0;
		}
		else {
			ipc8_pci->io_info.io_pwr_mode = IO_POWER_NO_SUPPLY;
			io_pw_mode.outval = IO_POWER_WRN_NO_SUPPLY;
		}
		break;
	default:
		break;
	}
	
EndProc:
	if (copy_to_user(arg, &io_pw_mode, sizeof(IO_POWERMODE))) {
		ret = -EFAULT;
	}
	
	DEBUG_PRINT("<--\n");
	return ret;
}

static int do_ioctl_read_iopowermode(void* arg)
{
	int ret = 0;
	IO_POWERMODE io_pw_mode;
	
	DEBUG_PRINT("-->\n");
	
	memset(&io_pw_mode, 0, sizeof(IO_POWERMODE));
	
	io_pw_mode.mode = ipc8_pci->io_info.io_pwr_mode;
	io_pw_mode.outval = 0;
	
	if (copy_to_user(arg, &io_pw_mode, sizeof(IO_POWERMODE))) {
		ret = -EFAULT;
	}
	
	DEBUG_PRINT("<--\n");
	return ret;
}

static int do_ioctl_read_mbversion(void* arg)
{
	int ret = 0;
	VAR_DATA var_data;
	
	DEBUG_PRINT("-->\n");
	
	memset(&var_data, 0, sizeof(VAR_DATA));
	
	var_data.data   = ipc8_pci->mb_version;
	var_data.outval = 0;
	
	if (copy_to_user(arg, &var_data, sizeof(VAR_DATA))) {
		ret = -EFAULT;
	}
	
	DEBUG_PRINT("<--\n");
	return ret;
}

static int do_ioctl_read_temperature(void* arg)
{
	int ret = 0;
	struct TEMP_DATA temp_data;
	
	DEBUG_PRINT("-->\n");
	
	memset(&temp_data, 0, sizeof(struct TEMP_DATA));
	
	temp_data.cpu_temp          = ipc8_pci->temp_data.cpu_temp;
	temp_data.power_mother_temp = ipc8_pci->temp_data.power_mother_temp / 10;
	temp_data.motor_power_temp  = ipc8_pci->temp_data.motor_power_temp / 10;
	temp_data.outval = 0;
	
	if (copy_to_user(arg, &temp_data, sizeof(struct TEMP_DATA))) {
		ret = -EFAULT;
	}
	
	DEBUG_PRINT("<--\n");
	return ret;
}

static int do_ioctl_read_ipc8error(void* arg)
{
	int ret = 0;
	IPC8_ERROR ipc8_error;
	
	DEBUG_PRINT("-->\n");
	
	memset(&ipc8_error, 0, sizeof(IPC8_ERROR));
	
	/* Check I/O error */
	if (0 < ipc8_pci->io_error.error) {
		DEBUG_PRINT("io_error.error = %d \n", ipc8_pci->io_error.error);
		ret |= IPC8_ERROR_IO;
	}
	ipc8_error.io_error.error      = ipc8_pci->io_error.error;
	ipc8_error.io_error.mini_io_oc = ipc8_pci->io_error.mini_io_oc;
	ipc8_error.io_error.hand_io_oc = ipc8_pci->io_error.hand_io_oc;
	ipc8_error.io_error.io_fuse    = ipc8_pci->io_error.io_fuse;
	
	/* Check FAN error */
	if ((0 < ipc8_pci->fan_error.fan_life_warn) 
		|| ipc8_pci->fan_error.fan_stop_error) {
		ret |= IPC8_ERROR_FAN;
	}
	ipc8_error.fan_error.fan_life_warn  = ipc8_pci->fan_error.fan_life_warn;
	ipc8_error.fan_error.fan_stop_error = ipc8_pci->fan_error.fan_stop_error;
	
	/* Check FPGA error */
	if (0 < ipc8_pci->fpga_error.error) {
		ret |= IPC8_ERROR_FPGA;
	}
	ipc8_error.fpga_error.error = ipc8_pci->fpga_error.error;
	
	/* Check PSU error */
	if (0 < ipc8_pci->psu_error.error) {
		ret |= IPC8_ERROR_PSU;
	}
	ipc8_error.psu_error.error             = ipc8_pci->psu_error.error;
	ipc8_error.psu_error.psu_communication = ipc8_pci->psu_error.psu_communication;
	ipc8_error.psu_error.psu_err_warn      = ipc8_pci->psu_error.psu_err_warn;
	
	/* Check PM error */
	if (0 < ipc8_pci->pm_error.error) {
		ret |= IPC8_ERROR_PM;
	}
	memcpy(&ipc8_error.pm_error, &ipc8_pci->pm_error, sizeof(PM_ERROR));
	
	if (copy_to_user(arg, &ipc8_error, sizeof(IPC8_ERROR))) {
		ret = -EFAULT;
	}
	
	DEBUG_PRINT("<--\n");
	return ret;
}

static int do_ioctl_write_pobstate(void *arg)
{
	int ret = 0;
	VAR_DATA var_data;
	bool local_output;
	int lock, i;
	int count = 0;
	
	DEBUG_PRINT("-->\n");
	
	if (copy_from_user(&var_data, (void *)arg, sizeof(VAR_DATA))) {
		ret = -EFAULT;
		goto EndProc;
	}
	
	var_data.outval = 0;
	
	if ((POB_OFF != var_data.data) 
		&& (POB_ON != var_data.data)) {
		var_data.outval = POB_STATE_ERR_DATA;
		ret = -EINVAL;
		goto EndProc;
	}
	
	local_output = (var_data.data == 1);
	
	if (local_output == ipc8_pci->pm_info.output) {
		/* nothing to do */
		goto EndProc;
	}
	
	for (i = 0; i < PM_MAX; i++) {
		if (ipc8_pci->pm_info.is_pob[i]) count++;
	}
	if (1 < count) {
		var_data.outval = POB_STATE_ERR_MULTIBOARD;
		ret = -EIO;
		goto EndProc;
	}
	
	if (local_output) {
		if (0 < ipc8_pci->pm_error.error) {
			var_data.outval = POB_STATE_ERR_ERROR_RAISED;
			ret = -EIO;
			goto EndProc;
		}
		
		if (ipc8_pci->psu_error.error & PSU_ERROR_PFAIL) {
			var_data.outval = POB_STATE_ERR_PFAIL;
			ret = -EIO;
			goto EndProc;
		}
	}
	
	lock = atomic_read(&ipc8_pci->pob_state_lock);
	if (0 != lock) {
		var_data.outval = POB_STATE_ERR_PROCESSING;
		ret = -EBUSY;
		goto EndProc;
	}
	
	atomic_inc(&ipc8_pci->pob_state_lock);
	
	if (local_output) {
		queue_work(ipc8_pci->workqueue, &ipc8_pci->supply_pob);
	}
	else {
		stop_pob_supply(&ipc8_pci->bar2mem->pob, &ipc8_pci->pm_info);
		atomic_set(&ipc8_pci->pob_state_lock, 0);
	}
	
EndProc:
	if (copy_to_user(arg, &var_data, sizeof(VAR_DATA))) {
		ret = -EFAULT;
	}
	
	DEBUG_PRINT("<--\n");
	return ret;
}


static int ipc8_open(struct inode *inode, struct file *filp) 
{
	DEBUG_PRINT("-->\n");
	DEBUG_PRINT("<--\n");
	return 0;
}

static int ipc8_release(struct inode* inode, struct file* filp)
{
	DEBUG_PRINT("-->\n");
	DEBUG_PRINT("<--\n");
	return 0;
}

static long ipc8_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	
	DEBUG_PRINT("-->\n");
	DEBUG_PRINT("cmd=0x%x (%c nr=%d len=%d dir=%d)", cmd, _IOC_TYPE(cmd),
		_IOC_NR(cmd), _IOC_SIZE(cmd), _IOC_DIR(cmd));
	
	/* Magic number check */
	if (IPC8_MAGIC != _IOC_TYPE(cmd)) {
		printk(KERN_ERR "%s: Magic number error. (Magic=%c) \n", __func__, _IOC_TYPE(cmd));
		ret = -ENOTTY;
	}
	else {
		if (!ipc8_pci) {
			ret = -EFAULT;
		}
		else {
			/* Command processing branch */
			switch (cmd) {
			case IPC8_IOCTL_READ_FRAM:
				ret = do_ioctl_read_fram((void *)arg);
				break;
			case IPC8_IOCTL_WRITE_FRAM:
				ret = do_ioctl_write_fram((void *)arg);
				break;
			case IPC8_IOCTL_READ_MINIIO:
				ret = do_ioctl_read_io((void *)arg, 
										&ipc8_pci->io_info, 
										&ipc8_pci->io_error, 
										true);
				break;
			case IPC8_IOCTL_WRITE_MINIIO:
				ret = do_ioctl_write_io((void *)arg, 
										&ipc8_pci->io_info, 
										&ipc8_pci->io_error, 
										true);
				break;
			case IPC8_IOCTL_READ_HANDIO:
				ret = do_ioctl_read_io((void *)arg, 
										&ipc8_pci->io_info, 
										&ipc8_pci->io_error, 
										false);
				break;
			case IPC8_IOCTL_WRITE_HANDIO:
				ret = do_ioctl_write_io((void *)arg, 
										&ipc8_pci->io_info, 
										&ipc8_pci->io_error, 
										false);
				break;
			case IPC8_IOCTL_READ_SAFETYIO:
				ret = do_ioctl_read_safetyio((void *)arg, &ipc8_pci->io_info);
				break;
			case IPC8_IOCTL_READ_IOPOWERMODE:
				ret = do_ioctl_read_iopowermode((void *)arg);
				break;
			case IPC8_IOCTL_WRITE_IOPOWERMODE:
				ret = do_ioctl_write_iopowermode((void *)arg);
				break;
			case IPC8_IOCTL_READ_FANSPEED:
				ret = do_ioctl_read_fanspeed((void *)arg, &ipc8_pci->fan_info);
				break;
			case IPC8_IOCTL_WRITE_FANSPEED:
				ret = do_ioctl_write_fanspeed((void *)arg, &ipc8_pci->fan_info);
				break;
			case IPC8_IOCTL_READ_FPGAVERSION:
				ret = do_ioctl_read_fpgaversion((void *)arg, &ipc8_pci->fpga_info);
				break;
			case IPC8_IOCTL_READ_PSUVERSION:
				ret = do_ioctl_read_psuversion((void *)arg, &ipc8_pci->psu_info);
				break;
			case IPC8_IOCTL_READ_MBVERSION:
				ret = do_ioctl_read_mbversion((void *)arg);
				break;
			case IPC8_IOCTL_READ_ACPOWERMODE:
				ret = do_ioctl_read_acpowermode((void *)arg, &ipc8_pci->psu_info);
				break;
			case IPC8_IOCTL_READ_ACINPUTVOLTAGE:
				ret = do_ioctl_read_acinputvoltage((void *)arg, &ipc8_pci->psu_info);
				break;
			case IPC8_IOCTL_READ_IOBOARDTYPE:
				ret = do_ioctl_read_ioboardtype((void *)arg, &ipc8_pci->io_info);
				break;
			case IPC8_IOCTL_READ_TEMPERATURE:
				ret = do_ioctl_read_temperature((void *)arg);
				break;
			case IPC8_IOCTL_READ_IPC8ERROR:
				ret = do_ioctl_read_ipc8error((void *)arg);
				break;
			case IPC8_IOCTL_READ_POBAVAILABLE:
				ret = do_ioctl_read_pobavailable((void *)arg, &ipc8_pci->pm_info);
				break;
			case IPC8_IOCTL_READ_POBSTATE:
				ret = do_ioctl_read_pobstate((void *)arg, &ipc8_pci->pm_info);
				break;
			case IPC8_IOCTL_WRITE_POBSTATE:
				ret = do_ioctl_write_pobstate((void *)arg);
				break;
			default:
				printk(KERN_ERR "%s: Command number error. (Cmd=0x%x) \n", __func__, cmd);
				ret = -ENOTTY;
				break;
			}
		}
	}
	DEBUG_PRINT("<--\n");
	return ret;
}


static void check_error_info(IPC8_DEVICE *pdev)
{
	u16 int_monitor, fail_monitor;
	u8  err_state;
	u16 break_fuse;
	
	/* Error condition 1 acquisition */
	int_monitor = pdev->recv_dma_addr->status1.int_monitor;
	
	if (int_monitor & INTSTATUS_PFAIL) {
		pdev->psu_error.error |= PSU_ERROR_PFAIL;
	}
	
	if (int_monitor & INTSTATUS_WDT) {
		/* CPU WDT state acquisition */
		fail_monitor = pdev->recv_dma_addr->status1.ext_err;
		
		if (fail_monitor & FAILSTATUS_PSUWDT) {
			pdev->psu_error.error |= PSU_ERROR_WDT;
		}
		
		if (fail_monitor & FAILSTATUS_IOWDT) {
			pdev->io_error.error |= IO_ERROR_WDT;
		}
	}
	
	/* Error condition 2 acquisition */
	err_state = pdev->recv_dma_addr->status2.err_state;
	
	if (err_state & ERR_STATE_BATTERY) {
		pdev->fpga_error.error |= FPGA_ERROR_BATTERY;
	}
	
	pdev->fan_error.fan_stop_error 
		= (err_state >> ERR_STATE_FAN_STOP_BIT) & ERR_STATE_FAN_STOP_MASK;
	
	break_fuse = pdev->recv_dma_addr->status2.break_fuse;
	check_pob_error(&pdev->bar2mem->pob, 
					&pdev->pm_info, 
					&pdev->pm_error, 
					int_monitor, 
					break_fuse);
}

static void initialize_send_dma(SENDDMAMAP *send_dma)
{
	send_dma->status.edge_clear                        = 0xFF;
	send_dma->psu_comm.tx_start_bit                    = 0xFF;
	send_dma->psu_comm.tx_data                         = 0xFF;
	send_dma->next_cycle_send_state.total_size         = DMA_SEND_BUFFER_LENGTH;
	send_dma->next_cycle_send_state.cdt_count          = DMA_SEND_CDT_COUNT | DMA_SEND_LAST_CDT;
	send_dma->next_cycle_send_state.send_cdt_list_addr = offsetof(BAR0, cdt_list) + sizeof(u64) * DMA_SEND_CDT_COUNT;;
}

/* Interrupt handler */
static irqreturn_t ipc8_irq(int irq, void *data)
{
	IPC8_DEVICE *dev = (IPC8_DEVICE *)data;
	irqreturn_t ret = IRQ_NONE;
	u32 read  = 0;
	u32 val   = 0;
	unsigned long flags;
	
	spin_lock_irqsave(&dev->irq_lock, flags);
	
	read = bar0_read32(&dev->bar0mem->int_status);
	
	if (read == DMA_INTCLR_RECV_A) {
		/* Watchdog counter is cleared */
		atomic_set(&dev->fpga_wdt_count, 0);
		
		/* Clear send DMA buffer */
		initialize_send_dma(dev->send_dma_addr);
		
		/* Error detection */
		check_error_info(dev);
		
		/* IO acquisition and setting */
		read_write_io(&dev->send_dma_addr->status, 
					  &dev->recv_dma_addr->status2, 
					  &dev->io_info, 
					  &dev->io_error);
		
		/* Fan speed reflects */
		set_fan_speed_dma(&dev->send_dma_addr->fan, &dev->fan_info);
		
		/* PSCPU Communucation */
		power_communication(&dev->send_dma_addr->psu_comm, 
							&dev->recv_dma_addr->psu_comm, 
							&dev->psu_info, 
							&dev->psu_error);
		
		/* Temperature acquisition */
		queue_work(dev->workqueue, &dev->temp_work);
		
		/* Receive DMA start request */
		val = DMA_INTCLR;
		bar0_write32(val, &dev->bar0mem->int_status, false);
		
		val = 0x00000000;
		bar2_write32(val, &dev->bar2mem->dma.send_status, false);
	}
	
	ret = IRQ_HANDLED;
	
	spin_unlock_irqrestore(&dev->irq_lock, flags);
	
	return ret;
}

static void get_cpu_info(void)
{
	int cpuid_addr;
	u32 eax, ebx, ecx, edx;
	
	DEBUG_PRINT("-->\n");
	
	ipc8_pci->cpu_tj_max = 90;
	
	cpuid_addr = 0x00000001;
	cpuid(cpuid_addr, &eax, &ebx, &ecx, &edx);
	
	switch (eax) {
	case 0x00106CA:		/* Atom D525                      */
	case 0x00206A7:		/* Core i7-2715QE, Core i7-2655LE */
		ipc8_pci->cpu_tj_max = 100;
		break;
	default:
		break;
	}
	
	DEBUG_PRINT("<--\n");
}



static ssize_t bar0_addr_show(struct class *c, struct class_attribute *attr, char *data)
{
	if (ipc8_pci) {
		return sprintf(data, "%s:addr_bar0=%d(d)[0x%x(h)]\n", attr->attr.name, ipc8_pci->addr_bar0, ipc8_pci->addr_bar0);
	}
	else {
		return sprintf(data, "%s:Not be read\n", attr->attr.name);
	}
}

static ssize_t bar0_addr_store(struct class *c, struct class_attribute *attr, const char *buf, size_t count)
{
	if (ipc8_pci) {
		sscanf(buf, "%du", &ipc8_pci->addr_bar0);
	}
	
	return count;
}

static ssize_t bar0_data_show(struct class *c, struct class_attribute *attr, char *data)
{
	u32 val = 0;
	
	if (ipc8_pci) {
		val = bar0_read32(((ipc8_pci->iomem0)+(ipc8_pci->addr_bar0)));
	}
	else {
		return sprintf(data, "%s:Not be read\n", attr->attr.name);
	}
	return sprintf(data, "%s:data=0x%x(h)\n", attr->attr.name, val);
}

static ssize_t bar0_data_store(struct class *c, struct class_attribute *attr, const char *buf, size_t count)
{
	size_t written = 0;
	int val;
	
	if (ipc8_pci) {
		sscanf(buf, "%du", &val);
		written = bar0_write32((u32)val, ((ipc8_pci->iomem0)+(ipc8_pci->addr_bar0)), false);
		if (0 > written) {
			printk(KERN_ERR "%s: Cannot write value.\n", __func__);
		}
	}
	
	return count;
}

static ssize_t bar2_addr_show(struct class *c, struct class_attribute *attr, char *data)
{
	if (ipc8_pci) {
		return sprintf(data, "%s:addr_bar2=%d(d)[0x%x(h)]\n", attr->attr.name, ipc8_pci->addr_bar2, ipc8_pci->addr_bar2);
	}
	else {
		return sprintf(data, "%s:Not be read\n", attr->attr.name);
	}
}

static ssize_t bar2_addr_store(struct class *c, struct class_attribute *attr, const char *buf, size_t count)
{
	if (ipc8_pci) {
		sscanf(buf, "%du", &ipc8_pci->addr_bar2);
	}
	
	return count;
}

static ssize_t bar2_data_show(struct class *c, struct class_attribute *attr, char *data)
{
	u16 val = -1;
	
	if (ipc8_pci) {
		val = bar2_read16(((ipc8_pci->iomem2)+(ipc8_pci->addr_bar2)));
	}
	else {
		return sprintf(data, "%s:Not be read\n", attr->attr.name);
	}
	return sprintf(data, "%s:data=0x%x(h)\n", attr->attr.name, val);
}

static ssize_t bar2_data_store(struct class *c, struct class_attribute *attr, const char *buf, size_t count)
{
	size_t written = 0;
	int val;
	
	if (ipc8_pci) {
		sscanf(buf, "%du", &val);
		written = bar2_write16((u16)val, ((ipc8_pci->iomem2)+(ipc8_pci->addr_bar2)), false);
		if (0 > written) {
			printk(KERN_ERR "%s: Cannot write value.\n", __func__);
		}
	}
	
	return count;
}

static ssize_t fan_spd_show(struct class *c, struct class_attribute *attr, char *data)
{
	u16 fan1, fan2, fan3, fan4;
	
	if (ipc8_pci) {
		fan1 = get_actual_fan_speed(((ipc8_pci->iomem2)+FAN_ADDR_OFS_SPEED1));
		fan2 = get_actual_fan_speed(((ipc8_pci->iomem2)+FAN_ADDR_OFS_SPEED2));
		fan3 = get_actual_fan_speed(((ipc8_pci->iomem2)+FAN_ADDR_OFS_SPEED3));
		fan4 = get_actual_fan_speed(((ipc8_pci->iomem2)+FAN_ADDR_OFS_SPEED4));
	}else {
		return sprintf(data, "%s:Not be read\n", attr->attr.name);
	}
	return sprintf(data, "%s:[fan1=%d][fan2=%d][fan3=%d][fan4=%d]\n", attr->attr.name, fan1, fan2, fan3, fan4);
}

static ssize_t fan_spd_store(struct class *c, struct class_attribute *attr, const char *buf, size_t count)
{
	int val;
	
	sscanf(buf, "%du", &val);
	set_fan_speed_local(&ipc8_pci->fan_info, val);
	return count;
}

static ssize_t fram_ofs_show(struct class *c, struct class_attribute *attr, char *data)
{
	if (ipc8_pci) {
		return sprintf(data, "%s:ofs_fram=%d(d)[0x%x(h)]\n", attr->attr.name, ipc8_pci->ofs_fram, ipc8_pci->ofs_fram);
	}
	else {
		return sprintf(data, "%s:Not be read\n", attr->attr.name);
	}
}

static ssize_t fram_ofs_store(struct class *c, struct class_attribute *attr, const char *buf, size_t count)
{
	if (ipc8_pci) {
		sscanf(buf, "%du", &ipc8_pci->ofs_fram);
	}
	
	return count;
}

static ssize_t fram_data_show(struct class *c, struct class_attribute *attr, char *data)
{
	u32 val = 0;
	
	if (ipc8_pci) {
		val = fram_read_local_sys(ipc8_pci->ofs_fram);
	}
	else {
		return sprintf(data, "%s:Not be read\n", attr->attr.name);
	}
	return sprintf(data, "%s:data=0x%x(h)\n", attr->attr.name, val);
}

static ssize_t fram_data_store(struct class *c, struct class_attribute *attr, const char *buf, size_t count)
{
	size_t written = 0;
	int val;
	
	if (ipc8_pci) {
		sscanf(buf, "%du", &val);
		written = fram_write_local_sys(ipc8_pci->ofs_fram, (u32)val);
		if (0 > written) {
			printk(KERN_ERR "%s: Cannot write value.\n", __func__);
		}
	}
	
	return count;
}


static struct file_operations ipc8_fops = {
	.owner          = THIS_MODULE,
	.open           = ipc8_open,
	.release        = ipc8_release,
	.unlocked_ioctl = ipc8_ioctl,
};

static struct class_attribute ipc8_class_attrs[] = {
	__ATTR(bar0_addr,     0666, bar0_addr_show, bar0_addr_store),		/* Debugging interface */
	__ATTR(bar0_data,     0666, bar0_data_show, bar0_data_store),		/* Debugging interface */
	__ATTR(bar2_addr,     0666, bar2_addr_show, bar2_addr_store),		/* Debugging interface */
	__ATTR(bar2_data,     0666, bar2_data_show, bar2_data_store),		/* Debugging interface */
	__ATTR(fan_speed,     0666, fan_spd_show,   fan_spd_store),			/* Fan control         */
	__ATTR(fram_sys_ofs,  0666, fram_ofs_show,  fram_ofs_store),		/* FRAM control        */
	__ATTR(fram_sys_data, 0666, fram_data_show, fram_data_store),		/* FRAM control        */
	__ATTR_NULL,
};

static struct class ipc8_class = {
	.name        = MODULE_NAME,
	.owner       = THIS_MODULE,
	.class_attrs = ipc8_class_attrs,
};

static int initialize_module(void)
{
	int ret = 0;
	MODULE_PROC proc;
	u16 read_enb;
	
	DEBUG_PRINT("-->\n");
	
	for (proc=0; proc < MODULE_MAX; proc++) {
		switch (proc) {
		case MODULE_FRAM:
			ret = initialize_fram(ipc8_pci->iomem0, ipc8_pci->fram_buffer);
			break;
		case MODULE_IO:
			ret = initialize_io(&ipc8_pci->bar2mem->io, 
								&ipc8_pci->io_info);
			break;
		case MODULE_FAN:
			ret = initialize_fan(&ipc8_pci->bar2mem->fan_ctrl, 
								 &ipc8_pci->fan_info, 
								 &ipc8_pci->fan_error);
			break;
		case MODULE_FPGA:
			ret = initialize_fpga(ipc8_pci->iomem3, 
								  &ipc8_pci->fpga_info);
			break;
		case MODULE_PSUCOMM:
			ret = initialize_psu_comm(&ipc8_pci->bar2mem->psu_comm, 
									  &ipc8_pci->psu_info);
			break;
		default:
			ret = -EFAULT;
			break;
		}
		if (0 > ret) {
			goto ErrProc;
		}
	}
	
	/* Motion Board Version acquisition */
	ipc8_pci->mb_version = bar2_read16(&ipc8_pci->bar2mem->hw_ver.rev_mode);
	
	/* CPU information acquisition */
	get_cpu_info();
	
	/* Validation of power output board */
	read_enb = bar2_read16(&ipc8_pci->bar2mem->fan_ctrl.fan_ctrl_speed_read_enb);
	ipc8_pci->pm_info.enable_pob = (read_enb == FAN_CTRL_SPD_READ_ENB);
	
	DEBUG_PRINT("<--\n");
	return ret;

ErrProc:

	DEBUG_PRINT("<--\n");
	return ret;
}

static void release_module(void)
{
	DEBUG_PRINT("-->\n");
	
	release_psu_comm(&ipc8_pci->bar2mem->psu_comm);
	
	DEBUG_PRINT("<--\n");
}

static void start_dma(void)
{
	u32 val   = 0;
	
	DEBUG_PRINT("-->\n");
	
	val = DMA_INTCLR;
	bar0_write32(val, &ipc8_pci->bar0mem->int_status, false);
	
	val = DMA_COMMAND_ENABLE;
	bar2_write32(val, &ipc8_pci->bar2mem->dma.command, false);
	
	DEBUG_PRINT("<--\n");
}

static void stop_dma(void)
{
	u32 val   = 0;
	
	DEBUG_PRINT("-->\n");
	
	val = DMA_COMMAND_DISABLE;
	bar2_write32(val, &ipc8_pci->bar2mem->dma.command, false);
	
	DEBUG_PRINT("<--\n");
}

static void release_dma_buffer(void)
{
	DEBUG_PRINT("-->\n");
	
	if (ipc8_pci->recv_dma_buf) {
		pci_free_consistent(ipc8_pci->pdev, 
							PAGE_SIZE, 
							ipc8_pci->recv_dma_buf, 
							ipc8_pci->recv_dma_physicaddr);
		ipc8_pci->recv_dma_buf = NULL;
	}
	if (ipc8_pci->send_dma_buf) {
		pci_free_consistent(ipc8_pci->pdev, 
							PAGE_SIZE, 
							ipc8_pci->send_dma_buf, 
							ipc8_pci->send_dma_physicaddr);
		ipc8_pci->send_dma_buf = NULL;
	}
	
	DEBUG_PRINT("<--\n");
}

static void initialize_dma_register(void)
{
	u32 val = 0;
	dma_addr_t cdt_list;
	int i;

	DEBUG_PRINT("-->\n");

	/* DMA data length setting */
	val = DMA_CDT_LENGTH;
	bar0_write32(val, &ipc8_pci->bar0mem->cdt_data_length, false);

	/* DMA mask setting */
	val = BAR0_INT_MASK;
	bar0_write32(val, &ipc8_pci->bar0mem->int_mask, false);

	/* Receive DMA setting */
	val = BAR0_BANK_A;
	bar0_write32(val, &ipc8_pci->bar0mem->bank_select, false);
	
	cdt_list = ipc8_pci->recv_dma_physicaddr;
	for (i = 0; i < DMA_RECV_CDT_COUNT; i++) {
		val = cdt_list;
		bar0_write32(val, &ipc8_pci->bar0mem->cdt_list[i], false);
		cdt_list += DMA_CDT_LENGTH;
	}

	val = DMA_RECV_FIRST_LENGTH | (DMA_RECV_LAST_LENGTH << 16);
	bar0_write32(val, &ipc8_pci->bar0mem->cdt_fraction1, false);

	val = DMA_RECV_BUFFER_LENGTH;
	bar2_write32(val, &ipc8_pci->bar2mem->dma.recv_total_length, false);

	val = DMA_RECV_CDT_COUNT | DMA_RECV_LAST_CDT;
	bar2_write32(val, &ipc8_pci->bar2mem->dma.cdt_count1, false);

	/* Send DMA setting */
	val = BAR0_BANK_B;
	bar0_write32(val, &ipc8_pci->bar0mem->bank_select, false);

	cdt_list = ipc8_pci->send_dma_physicaddr;
	for (i = 0; i < DMA_SEND_CDT_COUNT; i++) {
		val = cdt_list;
		bar0_write32(val, &ipc8_pci->bar0mem->cdt_list[i], false);
		cdt_list += DMA_CDT_LENGTH;
	}

	val = DMA_SEND_FIRST_LENGTH | (DMA_SEND_LAST_LENGTH << 16);
	bar0_write32(val, &ipc8_pci->bar0mem->cdt_fraction2, false);

	val = DMA_SEND_BUFFER_LENGTH;
	bar2_write32(val, &ipc8_pci->bar2mem->dma.send_total_length, false);

	val = DMA_SEND_CDT_COUNT | DMA_SEND_LAST_CDT;
	bar2_write32(val, &ipc8_pci->bar2mem->dma.cdt_count2, false);


	val = BAR0_BANK_A;
	bar0_write32(val, &ipc8_pci->bar0mem->bank_select, false);

	DEBUG_PRINT("<--\n");
}

static int initialize_dma(void)
{
	int ret = 0;
	
	DEBUG_PRINT("-->\n");
	
	/* DMA mask setting */
	ret = pci_set_dma_mask(ipc8_pci->pdev, DMA_BIT_MASK(32));
	if (ret) {
		printk(KERN_ERR "%s: DMA mask setting failure. \n", __func__);
		goto ErrProc;
	}
	
	/* DMA buffer ensure */
	ipc8_pci->recv_dma_buf = pci_alloc_consistent(ipc8_pci->pdev, DMA_RECV_BUFFER_LENGTH,
			&ipc8_pci->recv_dma_physicaddr);
	ipc8_pci->send_dma_buf = pci_alloc_consistent(ipc8_pci->pdev, DMA_SEND_BUFFER_LENGTH,
			&ipc8_pci->send_dma_physicaddr);
	if ((!ipc8_pci->recv_dma_buf) || (!ipc8_pci->send_dma_buf)) {
		printk(KERN_ERR "%s: DMA for memory allocation failure. \n", __func__);
		ret = -ENOMEM;
		goto ErrProc;
	}
	
	/* Cleared to zero */
	memset(ipc8_pci->recv_dma_buf, 0, DMA_RECV_BUFFER_LENGTH);
	memset(ipc8_pci->send_dma_buf, 0, DMA_SEND_BUFFER_LENGTH);
	
	/* Transmit DMA data initialization */
	initialize_send_dma(ipc8_pci->send_dma_addr);
	
	/* DMA communication setting initialization */
	initialize_dma_register();
	
	DEBUG_PRINT("<--\n");
	return ret;
	
ErrProc:
	release_dma_buffer();
	DEBUG_PRINT("<--\n");
	return ret;
}

static void start_ext_fail_detection(void)
{
	u16 val = 0;
	
	DEBUG_PRINT("-->\n");
	
	/* External fault monitor start */
	val = FAILMASK_EXTERR;
	bar2_write16(val, &ipc8_pci->bar2mem->ext_io.fail_mask, false);
	
	/* CPU WDT start */
	val = WDT_ENABLE;
	bar2_write16(val, &ipc8_pci->bar2mem->wdt_ctrl.enable, false);
	val = WDT_RESET;
	bar2_write16(val, &ipc8_pci->bar2mem->wdt_ctrl.clear, false);
	val = WDT_ERRCLR;
	bar2_write16(val, &ipc8_pci->bar2mem->wdt_ctrl.err_clear, false);
	val = WDT_RESET;
	bar2_write16(val, &ipc8_pci->bar2mem->wdt_ctrl.clear, false);
	
	DEBUG_PRINT("<--\n");
}

static void stop_ext_fail_detection(void)
{
	u16 val = 0;
	
	DEBUG_PRINT("-->\n");
	
	/* External fault monitor stop */
	val = FAILMASK_CLOSE;
	bar2_write16(val, &ipc8_pci->bar2mem->ext_io.fail_mask, false);
	
	/* CPU WDT stop */
	val = WDT_DISABLE;
	bar2_write16(val, &ipc8_pci->bar2mem->wdt_ctrl.enable, false);
	val = WDT_ERRCLR;
	bar2_write16(val, &ipc8_pci->bar2mem->wdt_ctrl.err_clear, false);
	val = WDT_RESET;
	bar2_write16(val, &ipc8_pci->bar2mem->wdt_ctrl.clear, false);
	
	DEBUG_PRINT("<--\n");
}

static void unmap_iomem(void)
{
	DEBUG_PRINT("-->\n");
	
	if (ipc8_pci->iomem0) {
		pci_iounmap(ipc8_pci->pdev, ipc8_pci->iomem0);
		ipc8_pci->iomem0 = NULL;
	}
	if (ipc8_pci->iomem2) {
		pci_iounmap(ipc8_pci->pdev, ipc8_pci->iomem2);
		ipc8_pci->iomem2 = NULL;
	}
	if (ipc8_pci->iomem3) {
		pci_iounmap(ipc8_pci->pdev, ipc8_pci->iomem3);
		ipc8_pci->iomem3 = NULL;
	}
	if (ipc8_pci->iomem4) {
		pci_iounmap(ipc8_pci->pdev, ipc8_pci->iomem4);
		ipc8_pci->iomem4 = NULL;
	}
	
	DEBUG_PRINT("<--\n");
}

static int alloc_pci_resource(void)
{
	int ret = 0;
	ALLOC_PROC proc;
	
	DEBUG_PRINT("-->\n");
	
	for (proc=0; proc < ALLOC_MAX; proc++) {
		switch (proc) {
		case ALLOC_REQUESTREGION:
			/* Reserved PCI I/O and memory resources */
			ret = pci_request_regions(ipc8_pci->pdev, MODULE_NAME);
			if (ret) {
				printk(KERN_ERR "%s: Cannot obtain PCI resources. (err=0x%d) \n", __func__, ret);
				ret = -EIO;
			}
			break;
		case ALLOC_INTURRUPT:
			/* Work queue creation  */
			ipc8_pci->workqueue = create_workqueue("ipc8_wq");
			/* Work queue initialization  */
			INIT_WORK(&ipc8_pci->temp_work,  temp_worker);
			INIT_WORK(&ipc8_pci->detect_pm,  detect_module_capacity_worker);
			INIT_WORK(&ipc8_pci->supply_pob, start_pob_supply);
			
			/* Interrupt register */
			ipc8_pci->irq = ipc8_pci->pdev->irq;
			if (request_irq(ipc8_pci->irq, &ipc8_irq, IRQF_SHARED, MODULE_NAME, ipc8_pci)) {
				printk(KERN_ERR "%s: request_irq failure. \n", __func__);
				ret = -EIO;
			}
			break;
		case ALLOC_IOMEM:
			/* Mapping the PCI memory area */
			ipc8_pci->iomem0 = pci_iomap(ipc8_pci->pdev, 0, 0);
			if (!ipc8_pci->iomem0) {
				printk(KERN_ERR "%s: Cannot be mapped memory(0).  \n", __func__);
				ret = -EIO;
				break;
			}
			ipc8_pci->iomem2 = pci_iomap(ipc8_pci->pdev, 2, 0);
			if (!ipc8_pci->iomem2) {
				printk(KERN_ERR "%s: Cannot be mapped memory(2).  \n", __func__);
				ret = -EIO;
				break;
			}
			ipc8_pci->iomem3 = pci_iomap(ipc8_pci->pdev, 3, 0);
			if (!ipc8_pci->iomem3) {
				printk(KERN_ERR "%s: Cannot be mapped memory(3).  \n", __func__);
				ret = -EIO;
				break;
			}
			ipc8_pci->iomem4 = pci_iomap(ipc8_pci->pdev, 4, 0);
			if (!ipc8_pci->iomem4) {
				printk(KERN_ERR "%s: Cannot be mapped memory(4).  \n", __func__);
				ret = -EIO;
			}
			break;
		case ALLOC_DMAINIT:
			ret = initialize_dma();
			break;
		case ALLOC_MODULEINIT:
			ret = initialize_module();
			break;
		default:
			ret = -EFAULT;
			break;
		}
		if (0 > ret) {
			goto ErrProc;
		}
	}
	
	/* DMA transfer start  */
	start_dma();
	
	/* External fault monitoring start */
	start_ext_fail_detection();
	
	/* Timer initialization and start */
	initialize_timer_thread();
	
	/* POB lock status initialization */
	atomic_set(&ipc8_pci->pob_state_lock, 0);
	
	/* Detect module capacity */
	queue_work(ipc8_pci->workqueue, &ipc8_pci->detect_pm);
	
	DEBUG_PRINT("<--\n");
	return ret;
	
ErrProc:
	if (ALLOC_DMAINIT < proc) {
		release_dma_buffer();
	}
	if (ALLOC_IOMEM < proc) {
		unmap_iomem();
	}
	
	if (ALLOC_INTURRUPT < proc) {
		free_irq(ipc8_pci->irq, ipc8_pci);
	}
	
	if (ALLOC_REQUESTREGION < proc) {
		pci_release_regions(ipc8_pci->pdev);
	}
	
	DEBUG_PRINT("<--\n");
	return ret;
}

static void free_pci_resource(void)
{
	DEBUG_PRINT("-->\n");
	
	/* Timer stopp and delete */
	terminate_timer_thread();
	
	/* Interrupt deregistration */
	free_irq(ipc8_pci->irq, ipc8_pci);
	
	/* External fault monitoring stop */
	stop_ext_fail_detection();
	
	/* DMA transfer stop */
	stop_dma();
	
	/* Each module release */
	release_module();
	
	/* DMA buffer release */
	if (ipc8_pci->recv_dma_buf) {
		pci_free_consistent(ipc8_pci->pdev, 
							PAGE_SIZE, 
							ipc8_pci->recv_dma_buf, 
							ipc8_pci->recv_dma_physicaddr);
	}
	if (ipc8_pci->send_dma_buf) {
		pci_free_consistent(ipc8_pci->pdev, 
							PAGE_SIZE, 
							ipc8_pci->send_dma_buf, 
							ipc8_pci->send_dma_physicaddr);
	}
	
	/* PCI memory area unmapped */
	if (ipc8_pci->iomem0) {
		pci_iounmap(ipc8_pci->pdev, ipc8_pci->iomem0);
	}
	if (ipc8_pci->iomem2) {
		pci_iounmap(ipc8_pci->pdev, ipc8_pci->iomem2);
	}
	if (ipc8_pci->iomem3) {
		pci_iounmap(ipc8_pci->pdev, ipc8_pci->iomem3);
	}
	if (ipc8_pci->iomem4) {
		pci_iounmap(ipc8_pci->pdev, ipc8_pci->iomem4);
	}
	
	/* Release reserved PCI I/O and memory resources */
	pci_release_regions(ipc8_pci->pdev);
	
	DEBUG_PRINT("<--\n");
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,8,0)
static int __devinit
#else
static int
#endif
ipc8_probe(struct pci_dev *pdev, const struct pci_device_id *ent)
{
	int ret = 0;
	PROBE_PROC proc;
	
	DEBUG_PRINT("-->\n");
	
	for (proc=0; proc < PROBE_MAX; proc++) {
		switch (proc) {
		case PROBE_ALLOCDEVICE:
			/* Device structure area ensure */
			ipc8_pci = kzalloc(sizeof(IPC8_DEVICE), GFP_KERNEL);
			if (!ipc8_pci) {
				printk(KERN_ERR "%s: Cannot allocate memory. \n", __func__);
				ret = -ENOMEM;
			}
			break;
		case PROBE_DEVICEINIT:
			/* Keep PCI device pointer */
			ipc8_pci->pdev = pdev;
			
			/* Spin lock initialization */
			spin_lock_init(&ipc8_pci->mem0_lock);
			spin_lock_init(&ipc8_pci->mem2_lock);
			spin_lock_init(&ipc8_pci->mem3_lock);
			spin_lock_init(&ipc8_pci->irq_lock);
			
			/* Ensure local buffer of FRAM  */
			ipc8_pci->fram_buffer = kzalloc(sizeof(u32) 
						* (FRAM_LENGTH / FRAM_ACCESS_SIZE) + 1, GFP_KERNEL);
			if (!ipc8_pci->fram_buffer) {
				printk(KERN_ERR "%s: Cannot allocate memory. \n", __func__);
				ret = -ENOMEM;
			}
			break;
		case PROBE_PCIENABLE:
			/* Initialize device before it's used by a driver */
			ret = pci_enable_device(pdev);
			if (0 > ret) {
				printk(KERN_ERR "%s: Cannot enable new PCI device. (err=0x%d) \n", __func__, ret);
				ret = -EIO;
			}
			break;
		case PROBE_ALLOCRESOURCE:
			/* Enables bus-mastering for device dev */
			pci_set_master(pdev);
			
			/* Set private driver data pointer for a pci_dev */
			pci_set_drvdata(pdev, ipc8_pci);
			
			ret = alloc_pci_resource();
			break;
		default:
			ret = -EFAULT;
			break;
		}
		if (0 > ret) {
			goto ErrProc;
		}
	}
	
	DEBUG_PRINT("<--\n");
	return ret;
	
ErrProc:
	if (PROBE_PCIENABLE < proc) {
		pci_disable_device(pdev);
	}
	
	if (PROBE_DEVICEINIT < proc) {
		kfree(ipc8_pci->fram_buffer);
	}
	
	if (PROBE_ALLOCDEVICE < proc) {
		pci_set_drvdata(pdev, NULL);
		kfree(ipc8_pci);
	}
	
	DEBUG_PRINT("<--\n");
	return ret;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,8,0)
static void __devexit
#else
static void
#endif
ipc8_remove(struct pci_dev *pdev)
{
	DEBUG_PRINT("-->\n");
	
	if (ipc8_pci) {
		free_pci_resource();
		pci_disable_device(pdev);
		if (ipc8_pci->fram_buffer) {
			kfree(ipc8_pci->fram_buffer);
		}
		pci_set_drvdata(pdev, NULL);
		kfree(ipc8_pci);
	}
	
	DEBUG_PRINT("<--\n");
}

static struct pci_device_id ipc8_pci_tbl[] = {
	{
		.vendor = PCI_VENDOR_ID_DNWA,
		.device = PCI_DEVICE_ID_IPC8,
		.subvendor = PCI_ANY_ID,
		.subdevice = PCI_ANY_ID,
	},
	{ 0, },
};

static struct pci_driver ipc8_driver = {
	.name = MODULE_NAME,
	.id_table = ipc8_pci_tbl,
	.probe = ipc8_probe,
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,8,0)
	.remove = __devexit_p(ipc8_remove),
#else
	.remove = ipc8_remove,
#endif
};


static int ipc8_init(void)
{
	int ret = 0;
	INIT_PROC proc;
	struct device *dev;
	
	for (proc=0; proc < INIT_MAX; proc++) {
		switch (proc) {
		case INIT_CLSREGISTER:
			/* Class registration */
			ret = class_register(&ipc8_class);
			if (0 > ret) {
				printk(KERN_ERR "%s :class_register failed. err=%d \n", __func__, ret);
			}
			break;
		case INIT_ALLOCCHRDEV:
			/* Dynamic acquisition of character device number */
			ret = alloc_chrdev_region(	&ipc8_devid,
										0,
										MINOR_COUNT,
										MODULE_NAME);
			if (0 > ret) {
				printk(KERN_ERR "%s :alloc_chrdev_region failed. err=%d \n", __func__, ret);
			}
			break;
		case INIT_CHRDEVINIT:
			/* Character device initialization */
			cdev_init(&c_dev, &ipc8_fops);
			c_dev.owner = THIS_MODULE;
			
			/* Registration of a character device */
			ret = cdev_add(&c_dev, ipc8_devid, MINOR_COUNT);
			if (0 > ret) {
				printk(KERN_ERR "%s :cdev_add failed. err=%d \n", __func__, ret);
			}
			break;
		case INIT_DEVICECREATE:
			/* Creates a device and registers it with sysfs */
			dev = device_create(&ipc8_class, NULL, MKDEV(MAJOR(ipc8_devid), MINOR(ipc8_devid)),
						NULL, "%s", MODULE_NAME);
			if (IS_ERR(dev)) {
				ret = PTR_ERR(dev);
				printk(KERN_ERR "%s :device_create failed. err=%d \n", __func__, ret);
			}
			break;
		case INIT_PCIREGISTER:
			/* PCI driver registration */
			ret = pci_register_driver(&ipc8_driver);
			if (ret) {
				printk(KERN_ERR "%s :pci_register_driver failed. err=%d \n", __func__, ret);
			}
			break;
		default:
			ret = -EFAULT;
			break;
		}
		if (0 > ret) {
			goto ErrProc;
		}
	}
	
	printk(KERN_INFO "%s driver loaded. \n", MODULE_NAME);
	printk(KERN_INFO "major = %d\n", MAJOR(ipc8_devid));
	printk(KERN_INFO "minor = %d\n", MINOR(ipc8_devid));
	
	return ret;

ErrProc:
	if (INIT_DEVICECREATE < proc) {
		device_destroy(&ipc8_class, MKDEV(MAJOR(ipc8_devid), MINOR(ipc8_devid)));
	}
	if (INIT_CHRDEVINIT < proc) {
		cdev_del(&c_dev);
	}
	if (INIT_ALLOCCHRDEV < proc) {
		unregister_chrdev_region(ipc8_devid, MINOR_COUNT);
	}
	if (INIT_CLSREGISTER < proc) {
		class_destroy(&ipc8_class);
	}
	return ret;
}

static void ipc8_exit(void)
{
	/* PCI driver deregistration */
	pci_unregister_driver(&ipc8_driver);
	
	/* Device destroy */
	device_destroy(&ipc8_class, MKDEV(MAJOR(ipc8_devid), MINOR(ipc8_devid)));
	
	/* Character device delete */
	cdev_del(&c_dev);
	
	/* Character device deregistration */
	unregister_chrdev_region(ipc8_devid, MINOR_COUNT);
	
	/* Class destroy */
	class_destroy(&ipc8_class);
	
	printk(KERN_INFO "%s driver unloaded\n", MODULE_NAME);
}

module_init(ipc8_init);
module_exit(ipc8_exit);

#ifndef DRIVER_VERSION
#define DRIVER_VERSION "UNKNOWN"
#endif

MODULE_DESCRIPTION("DENSO IPC8 driver");
MODULE_AUTHOR("DENSO WAVE INCORPORATED");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");
