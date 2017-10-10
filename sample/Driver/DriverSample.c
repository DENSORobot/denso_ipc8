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
#include <stdint.h>
#include <stdbool.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include "ipc8_ioctl.h"

#define DRIVER_NAME	"/dev/denso_ipc8"

#define FAN_MINSPEED_PER	(60)		/* Minimum fan speed [%] */
#define FAN_MAXSPEED_PER	(100)		/* Maximum fan speed [%] */

int fd;

void bit_disp(unsigned int dt)
{
	int i, len;
	
	len = 16;
	for (i = len - 1; i >= 0; i--){
		printf("%u", (dt>>i) & 0x0001);
	}

	printf("\n");
}

void do_ioctl(int num)
{
	int ret    = 0;
	int val    = 0;
	int mode   = 0;
	int cmd    = 0;
	struct FRAM_DATA fram;
	int i, element;
	struct IO_POWERMODE io_pw_mode;
	unsigned long tmp;
	struct IO_READ io_read;
	struct IO_WRITE io_write;
	struct IO_SAFETY io_safety;
	struct FAN_SPEED fan_spd;
	struct PSU_VERSION psu_ver;
	struct VAR_DATA var_data;
	struct TEMP_DATA temp_data;
	struct FPGA_VERSION fpga_ver;
	struct IPC8_ERROR ipc8_error;
	
	switch(num)
	{
	/*>>> TODO: Customizable >>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
	case 0:
		printf("-> IPC8_IOCTL_READ_FRAM \n");
		printf("     Enter the offset. (Decimal) \n");
		printf("     offset=");
		scanf("%lu",&tmp);
		fram.offset = tmp;
		printf("       0x%08x(Hexa)  \n", fram.offset);
		printf("     Enter the read length. (Decimal) \n");
		printf("     length=");
		scanf("%lu",&tmp);
		fram.length = tmp;
		printf("       0x%08x(Hexa)  \n", fram.length);
		
		/* Area allocation */
		element = fram.length / 4;
		printf("     element=%d \n", element);
		fram.data = (uint32_t *)malloc(sizeof(uint32_t) * element);
		if(fram.data){
			ret = ioctl(fd, IPC8_IOCTL_READ_FRAM, &fram);
			printf("     ret=%d, outval=%d \n", ret, fram.outval);
			if(0 <= ret){
				if(0 == fram.outval){
					for(i=0; i < element; i++){
						printf("     read_data[%d]=0x%08x \n", i, fram.data[i]);
					}
				}
			}
		}
		else{
			printf("     malloc failure. \n");
			break;
		}
		
		/* Area release */
		if(fram.data){
			free(fram.data);
		}
		break;
	case 1:
		printf("-> IPC8_IOCTL_WRITE_FRAM \n");
		printf("     Do you write in 0xFFFFFFFF the entire area? (Yes=1, No=0) \n");
		printf("     Do you write in 0x11111111 the entire area? (Yes=2, No=0) \n");
		printf("     all=");
		scanf("%lu",&tmp);
		printf("\n");
		if(tmp){
			fram.offset = 0x00;
			printf("     offset=%d(Decimal) \n", fram.offset);
			printf("       0x%08x(Hexa)  \n", fram.offset);
			fram.length = 0x001E0000;
			printf("     length=%d(Decimal) \n", fram.length);
			printf("       0x%08x(Hexa)  \n", fram.length);
			printf("     access_addr=0x%08x - 0x%08x  \n", 
								fram.offset + 0x00220000, 
								fram.offset + 0x00220000 + fram.length);
			
			/* Area allocation */
			element = fram.length / 4;
			printf("     element=%d \n", element);
			fram.data = (uint32_t *)malloc(sizeof(uint32_t) * element);
			if(fram.data){
				for(i=0; i < element; i++){
					if(1 == tmp){
						fram.data[i] = 0xFFFFFFFF;
					}
					else if(2 == tmp){
						fram.data[i] = 0x11111111;
					}
					else{
						fram.data[i] = 0x00000000;
					}
				}
				printf("     I attempt to write. \n");
			}
			else{
				printf("     malloc failure. \n");
				break;
			}
		}
		else{
			printf("     Enter the offset. (Decimal) \n");
			printf("     offset=");
			scanf("%lu",&tmp);
			fram.offset = tmp;
			printf("       0x%08x(Hexa)  \n", fram.offset);
			printf("     Enter the read length. (Decimal) \n");
			printf("     length=");
			scanf("%lu",&tmp);
			fram.length = tmp;
			printf("       0x%08x(Hexa)  \n", fram.length);
			printf("     access_addr=0x%08x - 0x%08x  \n", 
								fram.offset + 0x00220000, 
								fram.offset + 0x00220000 + fram.length);
			
			/* Area allocation */
			element = fram.length / 4;
			printf("     element=%d \n", element);
			fram.data = (uint32_t *)malloc(sizeof(uint32_t) * element);
			if(fram.data){
				printf("     Enter the write data. (Decimal by 32bit) \n");
				for(i=0; i < element; i++){
					printf("     write_data[%d]=", i);
					scanf("%lu",&tmp);
					fram.data[i] = tmp;
					printf("       0x%08x(Hexa)  \n", fram.data[i]);
				}
			}
			else{
				printf("     malloc failure. \n");
				break;
			}
		}
		
		ret = ioctl(fd, IPC8_IOCTL_WRITE_FRAM, &fram);
		printf("     ret=%d, outval=%d \n", ret, fram.outval);
		
		/* Area release */
		if(fram.data){
			free(fram.data);
		}
		break;
	case 2:
		printf("-> IPC8_IOCTL_READ_MINIIO \n");
		ret = ioctl(fd, IPC8_IOCTL_READ_MINIIO, &io_read);
		printf("     ret=%d, outval=%d \n", ret, io_read.outval);
		printf("     io_input (d)=0x%08x \n", io_read.io_input);
		printf("     io_output(d)=0x%08x \n", io_read.io_output);
		printf("     io_input (b)=");
		bit_disp(io_read.io_input);
		printf("     io_output(b)=");
		bit_disp(io_read.io_output);
		break;
	case 3:
		printf("-> IPC8_IOCTL_WRITE_MINIIO \n");
		printf("     Enter the value to be written to the IO. (Decimal:16bit) \n");
		printf("     val=");
		scanf("%d",&val);
		printf("     val(b) =");
		bit_disp(val);
		io_write.io_output = val;
		printf("     Enter the mask to be written to the IO. (Decimal:16bit) \n");
		printf("     mask=");
		scanf("%d",&val);
		printf("     mask(b)=");
		bit_disp(val);
		io_write.io_mask = val;
		ret = ioctl(fd, IPC8_IOCTL_WRITE_MINIIO, &io_write);
		printf("\n");
		printf("     ret=%d, outval=%d \n", ret, io_write.outval);
		break;
	case 4:
		printf("-> IPC8_IOCTL_READ_HANDIO \n");
		ret = ioctl(fd, IPC8_IOCTL_READ_HANDIO, &io_read);
		printf("     ret=%d, outval=%d \n", ret, io_read.outval);
		printf("     io_input (d)=0x%08x \n", io_read.io_input);
		printf("     io_output(d)=0x%08x \n", io_read.io_output);
		printf("     io_input (b)=");
		bit_disp(io_read.io_input);
		printf("     io_output(b)=");
		bit_disp(io_read.io_output);
		break;
	case 5:
		printf("-> IPC8_IOCTL_WRITE_HANDIO \n");
		printf("     Enter the value to be written to the IO. (Decimal:8bit) \n");
		printf("     val=");
		scanf("%d",&val);
		printf("     val(b) =");
		bit_disp(val);
		io_write.io_output = val;
		printf("     Enter the mask to be written to the IO. (Decimal:8bit) \n");
		printf("     mask=");
		scanf("%d",&val);
		printf("     mask(b)=");
		bit_disp(val);
		io_write.io_mask = val;
		ret = ioctl(fd, IPC8_IOCTL_WRITE_HANDIO, &io_write);
		printf("\n");
		printf("     ret=%d, outval=%d \n", ret, io_write.outval);
		break;
	case 6:
		printf("-> IPC8_IOCTL_READ_SAFETYIO \n");
		ret = ioctl(fd, IPC8_IOCTL_READ_SAFETYIO, &io_safety);
		printf("     ret=%d, outval=%d \n", ret, io_safety.outval);
		printf("     emergency_stop =0x%02x \n", io_safety.emergency_stop);
		printf("     dead_man       =0x%02x \n", io_safety.dead_man);
		printf("     protect_stop   =0x%02x \n", io_safety.protect_stop);
		printf("     auto_enable    =0x%02x \n", io_safety.auto_enable);
		break;
	case 7:
		printf("-> IPC8_IOCTL_READ_IOPOWERMODE \n");
		ret = ioctl(fd, IPC8_IOCTL_READ_IOPOWERMODE, &io_pw_mode);
		if(0 <= ret){
			printf("     ret=%d, outval=%d \n", ret, io_pw_mode.outval);
			printf("     mode=%d \n", io_pw_mode.mode);
		}
		break;
	case 8:
		printf("-> IPC8_IOCTL_WRITE_IOPOWERMODE \n");
		printf("     Select power mode. \n");
		printf("     1: EXTERNAL \n");
		printf("     2: INTERNAL \n");
		printf("     mode=");
		scanf("%d",&mode);
		printf("\n");
		
		if(1 == mode){
			io_pw_mode.mode = IO_POWER_EXTERNAL;
		}
		else if (2 == mode){
			io_pw_mode.mode = IO_POWER_INTERNAL;
		}
		else {
			printf("Invalid number is entered ! \n");
#if 0
			break;
#else
			io_pw_mode.mode = mode;	/* Error root test */
#endif
		}
		
		ret = ioctl(fd, IPC8_IOCTL_WRITE_IOPOWERMODE, &io_pw_mode);
		printf("     ret=%d, outval=%d \n", ret, io_pw_mode.outval);
		break;
	case 9:
		printf("-> IPC8_IOCTL_READ_FANSPEED \n");
		ret = ioctl(fd, IPC8_IOCTL_READ_FANSPEED, &fan_spd);
		if(0 <= ret){
			printf("     ret=%d, outval=%d \n", ret, fan_spd.outval);
			printf("     speed=%d \n", fan_spd.speed);
		}
		break;
	case 10:
		printf("-> IPC8_IOCTL_WRITE_FANSPEED \n");
		printf("     Enter the speed setting. \n");
		printf("     Range: %d-%d \n", FAN_MINSPEED_PER, FAN_MAXSPEED_PER);
		printf("     val=");
		scanf("%d",&val);
		printf("\n");
		fan_spd.speed = val;
		ret = ioctl(fd, IPC8_IOCTL_WRITE_FANSPEED, &fan_spd);
		printf("     ret=%d, outval=%d \n", ret, fan_spd.outval);
		break;
	case 11:
		printf("-> IPC8_IOCTL_READ_FPGAVERSION \n");
		ret = ioctl(fd, IPC8_IOCTL_READ_FPGAVERSION, &fpga_ver);
		printf("     ret=%d, outval=%d \n", ret, fpga_ver.outval);
		printf("     motion_ver=%s \n", fpga_ver.motion_ver);
		printf("     io_ver    =%s \n", fpga_ver.io_ver);
		break;
	case 12:
		printf("-> IPC8_IOCTL_READ_PSUVERSION \n");
		ret = ioctl(fd, IPC8_IOCTL_READ_PSUVERSION, &psu_ver);
		printf("     ret=%d, outval=%d \n", ret, psu_ver.outval);
		printf("     sf_ver=0x%04x \n", psu_ver.sf_ver);
		printf("     hw_ver=0x%04x \n", psu_ver.hw_ver);
		break;
	case 13:
		printf("-> IPC8_IOCTL_READ_MBVERSION \n");
		ret = ioctl(fd, IPC8_IOCTL_READ_MBVERSION, &var_data);
		printf("     ret=%d, outval=%d \n", ret, var_data.outval);
		printf("     data=0x%04x \n", var_data.data);
		break;
	case 14:
		printf("-> IPC8_IOCTL_READ_ACPOWERMODE \n");
		ret = ioctl(fd, IPC8_IOCTL_READ_ACPOWERMODE, &var_data);
		printf("     ret=%d, outval=%d \n", ret, var_data.outval);
		printf("     data=%d \n", var_data.data);
		break;
	case 15:
		printf("-> IPC8_IOCTL_READ_ACINPUTVOLTAGE \n");
		ret = ioctl(fd, IPC8_IOCTL_READ_ACINPUTVOLTAGE, &var_data);
		printf("     ret=%d, outval=%d \n", ret, var_data.outval);
		printf("     data=%d \n", var_data.data);
		break;
	case 16:
		printf("-> IPC8_IOCTL_READ_IOBOARDTYPE \n");
		ret = ioctl(fd, IPC8_IOCTL_READ_IOBOARDTYPE, &var_data);
		printf("     ret=%d, outval=%d \n", ret, var_data.outval);
		printf("     data=%d \n", var_data.data);
		break;
	case 17:
		printf("-> IPC8_IOCTL_READ_TEMPERATURE \n");
		ret = ioctl(fd, IPC8_IOCTL_READ_TEMPERATURE, &temp_data);
		printf("     ret=%d, outval=%d \n", ret, temp_data.outval);
		printf("     CpuTemp        =%d \n", temp_data.cpu_temp);
		printf("     PowerMotherTemp=%d \n", temp_data.power_mother_temp);
		printf("     MotorPowerTemp =%d \n", temp_data.motor_power_temp);
		break;
	case 18:
		printf("-> IPC8_IOCTL_READ_IPC8ERROR \n");
		ret = ioctl(fd, IPC8_IOCTL_READ_IPC8ERROR, &ipc8_error);
		printf("     ret=%d\n", ret);
		printf("     io_error.error              =0x%x \n", ipc8_error.io_error.error);
		printf("     io_error.mini_io_oc         =0x%x \n", ipc8_error.io_error.mini_io_oc);
		printf("     io_error.hand_io_oc         =0x%x \n", ipc8_error.io_error.hand_io_oc);
		printf("     io_error.io_fuse            =0x%x \n", ipc8_error.io_error.io_fuse);
		printf("     fan_error.fan_life_warn     =0x%x \n", ipc8_error.fan_error.fan_life_warn);
		printf("     fan_error.fan_stop_error    =0x%x \n", ipc8_error.fan_error.fan_stop_error);
		printf("     fpga_error.error            =0x%x \n", ipc8_error.fpga_error.error);
		printf("     psu_error.error             =0x%x \n", ipc8_error.psu_error.error);
		printf("     psu_error.psu_communication =0x%x \n", ipc8_error.psu_error.psu_communication);
		printf("     psu_error.psu_err_warn      =0x%x \n", ipc8_error.psu_error.psu_err_warn);
		printf("     pm_error.error              =0x%x \n", ipc8_error.pm_error.error);
		printf("     pm_error.pob_error[0]       =0x%x \n", ipc8_error.pm_error.pob_error[0]);
		printf("     pm_error.pob_error[1]       =0x%x \n", ipc8_error.pm_error.pob_error[1]);
		printf("     pm_error.pob_error[2]       =0x%x \n", ipc8_error.pm_error.pob_error[2]);
		printf("     pm_error.pob_error[3]       =0x%x \n", ipc8_error.pm_error.pob_error[3]);
		break;
	case 19:
		printf("-> IPC8_IOCTL_READ_POBAVAILABLE \n");
		ret = ioctl(fd, IPC8_IOCTL_READ_POBAVAILABLE, &var_data);
		printf("     ret=%d, outval=%d \n", ret, var_data.outval);
		printf("     data=%d \n", var_data.data);
		break;
	case 20:
		printf("-> IPC8_IOCTL_READ_POBSTATE \n");
		ret = ioctl(fd, IPC8_IOCTL_READ_POBSTATE, &var_data);
		printf("     ret=%d, outval=%d \n", ret, var_data.outval);
		printf("     data=%d \n", var_data.data);
		break;
	case 21:
		printf("-> IPC8_IOCTL_WRITE_POBSTATE \n");
		printf("     Enter the POB state. \n");
		printf("     val=");
		scanf("%d",&val);
		printf("\n");
		var_data.data = val;
		ret = ioctl(fd, IPC8_IOCTL_WRITE_POBSTATE, &var_data);
		printf("     ret=%d, outval=%d \n", ret, var_data.outval);
		break;
	/*<<<<<<<<< Customizable <<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
	default:
		printf("Invalid number is entered ! \n");
		break;
	}
	
	if(-1 == ret){
		printf("ERR(%d): %s\n", errno, strerror(errno));
	}
	
	printf("\n");
}

void ioctl_menu(void)
{
	int num = 0;
	
	while(1){
		printf("[Test ioctl] \n");
		/*>>> TODO: Customizable >>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
		printf("  0: IPC8_IOCTL_READ_FRAM                \n");
		printf("  1: IPC8_IOCTL_WRITE_FRAM               \n");
		printf("  2: IPC8_IOCTL_READ_MINIIO              \n");
		printf("  3: IPC8_IOCTL_WRITE_MINIIO             \n");
		printf("  4: IPC8_IOCTL_READ_HANDIO              \n");
		printf("  5: IPC8_IOCTL_WRITE_HANDIO             \n");
		printf("  6: IPC8_IOCTL_READ_SAFETYIO            \n");
		printf("  7: IPC8_IOCTL_READ_IOPOWERMODE         \n");
		printf("  8: IPC8_IOCTL_WRITE_IOPOWERMODE        \n");
		printf("  9: IPC8_IOCTL_READ_FANSPEED            \n");
		printf(" 10: IPC8_IOCTL_WRITE_FANSPEED           \n");
		printf(" 11: IPC8_IOCTL_READ_FPGAVERSION         \n");
		printf(" 12: IPC8_IOCTL_READ_PSUVERSION          \n");
		printf(" 13: IPC8_IOCTL_READ_MBVERSION           \n");
		printf(" 14: IPC8_IOCTL_READ_ACPOWERMODE         \n");
		printf(" 15: IPC8_IOCTL_READ_ACINPUTVOLTAGE      \n");
		printf(" 16: IPC8_IOCTL_READ_IOBOARDTYPE         \n");
		printf(" 17: IPC8_IOCTL_READ_TEMPERATURE         \n");
		printf(" 18: IPC8_IOCTL_READ_IPC8ERROR           \n");
		printf(" 19: IPC8_IOCTL_READ_POBAVAILABLE        \n");
		printf(" 20: IPC8_IOCTL_READ_POBSTATE            \n");
		printf(" 21: IPC8_IOCTL_WRITE_POBSTATE           \n");
		/*<<<<<<<<< Customizable <<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
		printf(" 99: Go back to the test list \n");
		printf("\n");
		printf("Please select the ioctl number.\n");
		printf("number=");
		scanf("%d",&num);
		printf("\n");
		
		if(99 == num){
			break;
		}
		else{
			do_ioctl(num);
		}
	}
}

void dispatch_process(int num)
{
	int ret = 0;
	
	switch(num)
	{
	case 1:
		printf("-> Driver Open \n");
		fd = open(DRIVER_NAME, O_RDWR);
		if(-1 == fd){
			printf("ERR(%d): %s\n", errno, strerror(errno));
		}
		else{
			printf("Success ! \n");
		}
		break;
	case 2:
		printf("-> Driver Close \n");
		ret = close(fd);
		if(-1 == fd){
			printf("ERR(%d): %s\n", errno, strerror(errno));
		}
		else{
			printf("Success ! \n");
			fd = 0;
		}
		break;
	case 3:
		printf("-> Driver IOCTL \n");
		if(!fd){
			printf("Driver is not open. \n");
			printf("Please attempting and then re-open. \n");
		}
		else{
			ioctl_menu();
		}
		break;
	default:
		printf("Invalid number is entered ! \n");
		break;
	}
	printf("\n");
}

int main(int argc, char *argv[])
{
	int num = 0;
	fd = 0;
	
	while(1){
		printf("[Test list] \n");
		printf("1: Driver Open \n");
		printf("2: Driver Close \n");
		printf("3: Driver IOCTL \n");
		printf("0: Exit \n");
		printf("\n");
		printf("Please select the list number.\n");
		printf("number=");
		scanf("%d",&num);
		printf("\n");
		
		if(0 == num){
			break;
		}
		else{
			dispatch_process(num);
		}
	}
	
	return 0;
}
