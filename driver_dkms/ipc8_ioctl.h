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


#ifndef _IPC8_IOCTL_H
#define _IPC8_IOCTL_H


#define FRAM_ERR_OTHER					(-1)
#define FRAM_ERR_ALIGNMENT				(-2)
#define FRAM_ERR_OVERRUN				(-3)
#define FRAM_ERR_WRITE					(-4)

#define IO_ERR_OTHER					(-1)
#define IO_ERR_ERROR_RAISED				(-2)
#define IO_ERR_MODE_UNKNOWN				(-3)

#define IO_POWER_ERR_OTHER				(-1)
#define IO_POWER_ERR_MODE				(-2)
#define IO_POWER_ERR_REVERSE			(-3)
#define IO_POWER_ERR_INTERNAL_SUPPLY	(-4)
#define IO_POWER_ERR_COMMUNICATION		(-5)
#define IO_POWER_ERR_ALREADY			(-6)
#define IO_POWER_WRN_NO_SUPPLY			(1)

#define PSU_ERR_OTHER					(-1)
#define PSU_ERR_COMMUNICATION			(-2)

#define FAN_ERR_OTHER					(-1)
#define FAN_ERR_SPEED					(-2)

#define POB_STATE_ERR_DATA				(-2)
#define POB_STATE_ERR_ERROR_RAISED		(-3)
#define POB_STATE_ERR_PFAIL				(-4)
#define POB_STATE_ERR_PROCESSING		(-5)
#define POB_STATE_ERR_MULTIBOARD		(-6)

#define IO_ERROR_ACCESS					(0x0001)	/* I/O Error: Access fault                  */
#define IO_ERROR_MINI_OC				(0x0002)	/* I/O Error: Mini I/O over current         */
#define IO_ERROR_HAND_OC				(0x0004)	/* I/O Error: Hand I/O over current         */
#define IO_ERROR_FUSE					(0x0008)	/* I/O Error: I/O fuse fault                */
#define IO_ERROR_WDT					(0x0010)	/* I/O Error: I/O watch dog timer timeout   */

#define PM_ERROR_TIMEOUT				(0x0001)	/* PM Error: Timeout initialize             */
#define PM_ERROR_POB					(0x0002)	/* PM Error: Power output board             */
#define PM_ERROR_POB_OVP				(0x0004)	/* PM Error: Power output board OVP         */

#define FPGA_ERROR_INTERRUPT			(0x0001)	/* FPGA Error: Interrupt fault              */
#define FPGA_ERROR_WDT					(0x0002)	/* FPGA Error: FPGA watch dog timer timeout */
#define FPGA_ERROR_BATTERY				(0x0004)	/* FPGA Error: Battery power limit warning  */

#define PSU_ERROR_PFAIL					(0x0001)	/* PSU Error: PFAIL                         */
#define PSU_ERROR_ERRWARN				(0x0002)	/* PSU Error: PSUfault                      */
#define PSU_ERROR_COMMUNICATION			(0x0004)	/* PSU Error: PSU communication fault       */
#define PSU_ERROR_WDT					(0x0008)	/* PSU Error: PSU watch dog timer timeout   */

#define IPC8_ERROR_IO					(0x01)
#define IPC8_ERROR_FAN					(0x02)
#define IPC8_ERROR_FPGA					(0x04)
#define IPC8_ERROR_PSU					(0x08)
#define IPC8_ERROR_PM					(0x10)


typedef enum IO_POWER {
	IO_POWER_UNKNOWN   =  -1,				/* Unknown I/O power mode                                */
	IO_POWER_EXTERNAL  =   0,				/* I/O external mode                                     */
	IO_POWER_INTERNAL  =   1,				/* I/O internal mode                                     */
	IO_POWER_NO_SUPPLY =   2,				/* I/O external mode, but the power isnot being supplied */
} IO_POWER;

typedef enum IO_BOARD {
	IO_BOARD_UNKNOWN   = -1,				/* Unknown I/O board type                                */
	IO_BOARD_NPN       =  0,				/* NPN I/O board                                         */
	IO_BOARD_PNP       =  1,				/* PNP I/O board                                         */
} IO_BOARD;

typedef enum AC_MODE {
	AC_MODE_UNKNOWN    =  0,				/* Unknown AC power mode                                 */
	AC_MODE_100V       =  1,				/* AC 100V                                               */
	AC_MOVE_200V       =  2,				/* AC 200V                                               */
} AC_MODE;

typedef enum POB_STATE {
	POB_OFF            = 0,					/* Power output board off                                */
	POB_ON             = 1,					/* Power output board on                                 */
} POB_STATE;


typedef struct FRAM_DATA {					/* Structure for reading or writing FRAM                 */
	uint32_t offset;						/* Address of FRAM to read or write                      */
	uint32_t *data;							/* Address of the buffer to read or write                */
	uint32_t length;						/* Length of the buffer to read or write                 */
	int32_t  outval;						/* Result                                                */
} FRAM_DATA;

typedef struct IO_READ {					/* Structure for reading I/O                             */
	uint16_t io_input;						/* I/O input                                             */
	uint16_t io_output;						/* I/O output                                            */
	int32_t  outval;						/* Result                                                */
} IO_READ;

typedef struct IO_WRITE {					/* Structure for writing I/O                             */
	uint16_t io_output;						/* I/O output                                            */
	uint16_t io_mask;						/* I/O mask                                              */
	int32_t  outval;						/* Result                                                */
} IO_WRITE;

typedef struct IO_SAFETY {					/* Structure for reading Safety I/O                      */
	bool     emergency_stop;				/* Emergency stop SW                                     */
	bool     dead_man;						/* Dead man SW                                           */
	bool     protect_stop;					/* Protect stop                                          */
	bool     auto_enable;					/* Auto enable                                           */
	int32_t  outval;						/* Result                                                */
} IO_SAFETY;

typedef struct IO_POWERMODE {				/* Structure for reading or writing I/O power mode       */
	int32_t mode;							/* I/O power mode                                        */
	int32_t outval;							/* Result                                                */
} IO_POWERMODE;

typedef struct FAN_SPEED {					/* Structure for reading or writing fan speed            */
	int32_t speed;							/* Fan speed [%] (60 - 100)                              */
	int32_t outval;							/* Result                                                */
} FAN_SPEED;

typedef struct FPGA_VERSION {				/* Structure for reading fpga version                    */
	char    motion_ver[64];					/* Motion board version                                  */
	char    io_ver[64];						/* I/O version                                           */
	int32_t outval;							/* Result                                                */
} FPGA_VERSION;

typedef struct PSU_VERSION {				/* Structure for reading psu version                     */
	uint16_t sf_ver;						/* Software version                                      */
	uint16_t hw_ver;						/* Hardware version                                      */
	int32_t  outval;						/* Result                                                */
} PSU_VERSION;

typedef struct VAR_DATA {					/* Structure for reading a variable                      */
	int32_t data;							/* Read data                                             */
	int32_t outval;							/* Result                                                */
} VAR_DATA;

typedef struct TEMP_DATA {					/* Structure for reading temperature                     */
	uint32_t cpu_temp;						/* Cpu temperature                                       */
	uint32_t power_mother_temp;				/* PSU temperature                                       */
	uint32_t motor_power_temp;				/* Motor temperature                                     */
	int32_t  outval;						/* Result                                                */
} TEMP_DATA;

typedef struct IO_ERROR {
	uint16_t error;							/* Status of I/O error                                   */
	uint16_t mini_io_oc;					/* Status of Mini I/O over current fuse                  */
	uint16_t hand_io_oc;					/* Status of Hand I/O over current fuse                  */
	uint16_t io_fuse;						/* Status of I/O fuse                                    */
} IO_ERROR;

typedef struct FAN_ERROR {
	uint8_t fan_life_warn;					/* Status of fan life warning                            */
	uint8_t fan_stop_error;					/* Status of fan stop error                              */
} FAN_ERROR;

typedef struct FPGA_ERROR {
	uint16_t error;							/* Status of fpga error                                  */
} FPGA_ERROR;

typedef struct PSU_ERROR {
	uint16_t error;							/* Status of psu error                                   */
	uint16_t psu_communication;				/* PSU error communication                               */
	uint8_t  psu_err_warn;					/* PSU error warning                                     */
} PSU_ERROR;

typedef struct PM_ERROR {
	uint16_t error;							/* Status of pm error                                    */
	uint16_t pob_error[4];					/* PM error power output board                           */
} PM_ERROR;

typedef struct IPC8_ERROR {
	IO_ERROR   io_error;
	FAN_ERROR  fan_error;
	FPGA_ERROR fpga_error;
	PSU_ERROR  psu_error;
	PM_ERROR   pm_error;
} IPC8_ERROR;


#define IPC8_MAGIC							'f'
#define IPC8_IOCTL_READ_FRAM				_IOWR (IPC8_MAGIC,  0, FRAM_DATA)
#define IPC8_IOCTL_WRITE_FRAM				_IOWR (IPC8_MAGIC,  1, FRAM_DATA)
#define IPC8_IOCTL_READ_MINIIO				_IOR  (IPC8_MAGIC,  2, IO_READ)
#define IPC8_IOCTL_WRITE_MINIIO				_IOWR (IPC8_MAGIC,  3, IO_WRITE)
#define IPC8_IOCTL_READ_HANDIO				_IOR  (IPC8_MAGIC,  4, IO_READ)
#define IPC8_IOCTL_WRITE_HANDIO				_IOWR (IPC8_MAGIC,  5, IO_WRITE)
#define IPC8_IOCTL_READ_SAFETYIO			_IOR  (IPC8_MAGIC,  6, IO_SAFETY)
#define IPC8_IOCTL_READ_IOPOWERMODE			_IOR  (IPC8_MAGIC,  7, IO_POWERMODE)
#define IPC8_IOCTL_WRITE_IOPOWERMODE		_IOWR (IPC8_MAGIC,  8, IO_POWERMODE)
#define IPC8_IOCTL_READ_FANSPEED			_IOR  (IPC8_MAGIC,  9, FAN_SPEED)
#define IPC8_IOCTL_WRITE_FANSPEED			_IOWR (IPC8_MAGIC, 10, FAN_SPEED)
#define IPC8_IOCTL_READ_FPGAVERSION			_IOR  (IPC8_MAGIC, 11, FPGA_VERSION)
#define IPC8_IOCTL_READ_PSUVERSION			_IOR  (IPC8_MAGIC, 12, PSU_VERSION)
#define IPC8_IOCTL_READ_MBVERSION			_IOR  (IPC8_MAGIC, 13, VAR_DATA)
#define IPC8_IOCTL_READ_ACPOWERMODE			_IOR  (IPC8_MAGIC, 14, VAR_DATA)
#define IPC8_IOCTL_READ_ACINPUTVOLTAGE		_IOR  (IPC8_MAGIC, 15, VAR_DATA)
#define IPC8_IOCTL_READ_IOBOARDTYPE			_IOR  (IPC8_MAGIC, 16, VAR_DATA)
#define IPC8_IOCTL_READ_TEMPERATURE			_IOR  (IPC8_MAGIC, 17, TEMP_DATA)
#define IPC8_IOCTL_READ_IPC8ERROR			_IOR  (IPC8_MAGIC, 18, IPC8_ERROR)
#define IPC8_IOCTL_READ_POBAVAILABLE		_IOR  (IPC8_MAGIC, 19, VAR_DATA)
#define IPC8_IOCTL_READ_POBSTATE			_IOR  (IPC8_MAGIC, 20, VAR_DATA)
#define IPC8_IOCTL_WRITE_POBSTATE			_IOWR (IPC8_MAGIC, 21, VAR_DATA)

#endif /* _IPC8_IOCTL_H */
