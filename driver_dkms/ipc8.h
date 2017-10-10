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


#ifndef _IPC8_H
#define _IPC8_H

#include "ipc8_fan.h"
#include "ipc8_io.h"
#include "ipc8_ps_comm.h"
#include "ipc8_fram.h"
#include "ipc8_fpga.h"
#include "ipc8_pm.h"


/* Device Driver */
#define MODULE_NAME					"denso_ipc8"
#define MINOR_COUNT					(1)

#define PCI_VENDOR_ID_DNWA			(0xDE1B)
#define PCI_DEVICE_ID_IPC8			(0x1001)

#define DEFINE_MAP(type, name, addr)	struct { u8 rsv_##name[addr]; type name; } __attribute__ ((packed))


/* Address */
#define REG_BAR2_WDT_CTRL			(0x00010110)	/* Address of watch dog timer control */
#define REG_BAR2_HWVER				(0x00010120)	/* Address of hardware version        */
#define REG_BAR2_EXTIO				(0x00010220)	/* Address of external I/O            */
#define REG_BAR2_FANCTRL			(0x00010280)	/* Address of fan control             */
#define REG_BAR2_POB				(0x00011100)	/* Address of power output board      */
#define REG_BAR2_DMA				(0x00012000)	/* Address of dma configuration       */
#define REG_BAR2_IO					(0x00018000)	/* Address of I/O                     */
#define REG_BAR2_I2C				(0x00060300)	/* Address of I2C                     */
#define REG_BAR2_PSUCOMM			(0x00060400)	/* Address of psu control             */

#define REG_RECVDMA_PSUCOMM			(0x00A0)		/* Address of psu control             */
#define REG_RECVDMA_FANCTRL			(0x00DC)		/* Address of fan control             */
#define REG_RECVDMA_STATUS2			(0x00E0)		/* Address of status2                 */

#define REG_SENDDMA_FANCTRL			(0x004C)		/* Address of fan control             */
#define REG_SENDDMA_STATUS			(0x0060)		/* Address of status                  */
#define REG_SENDDMA_PSUCOMM			(0x0080)		/* Address of psu control             */
#define REG_SENDDMA_NEXTSEND		(0x00F0)		/* Address of next send               */

/* DMA */
#define DMA_COMMAND_ENABLE			(0x0004)
#define DMA_COMMAND_DISABLE			(0)
                                    
#define DMA_INTCLR_INTERRUPTION		(0x0001)
#define DMA_INTCLR_SEND_A			(0x0004)
#define DMA_INTCLR_SEND_B			(0x0008)
#define DMA_INTCLR_RECV_A			(0x0010)
#define DMA_INTCLR_RECV_B			(0x0020)
#define DMA_INTCLR					(DMA_INTCLR_INTERRUPTION | DMA_INTCLR_SEND_A | DMA_INTCLR_SEND_B | DMA_INTCLR_RECV_A | DMA_INTCLR_RECV_B)
                                    
#define DMA_CDT_LENGTH				(0x00000080)											/* CDT list size (128kB)             */

#define BAR0_BANK_A					(0x00000000)											/* Bank A                            */
#define BAR0_BANK_B					(0x00010000)											/* Bank B                            */
#define BAR0_INT_MASK				(0x000003EF)											/* DMA interruption mask             */

#define DMA_RECV_BUFFER_LENGTH		(0x00000200)											/* DMA receive buffer size (512Byte) */
#define DMA_SEND_BUFFER_LENGTH		(0x00000100)											/* DMA send buffer size (256Byte)    */

#define DMA_RECV_CDT_COUNT			(((DMA_RECV_BUFFER_LENGTH - 1) / DMA_CDT_LENGTH) + 1)	/* CDT list count for receiving      */
#define DMA_RECV_FIRST_LENGTH		(DMA_CDT_LENGTH)										/* First CDT size for receiving      */
#define DMA_RECV_LAST_CDT			(0x00010000)											/* Last CDT                          */
#define DMA_RECV_LAST_LENGTH		(DMA_CDT_LENGTH)										/* Last CDT size for receiving       */

#define DMA_SEND_CDT_COUNT			(((DMA_SEND_BUFFER_LENGTH - 1) / DMA_CDT_LENGTH) + 1)	/* CDT list count for sending        */
#define DMA_SEND_FIRST_LENGTH		(DMA_CDT_LENGTH)										/* First CDT size for sending        */
#define DMA_SEND_LAST_CDT			(0x00010000)											/* Last CDT                          */
#define DMA_SEND_LAST_LENGTH		(DMA_CDT_LENGTH)										/* Last CDT size for sending         */

/* Error Check */
#define INTSTATUS_PFAIL				(0x1000)		/* PFAIL                       */
#define INTSTATUS_WDT				(0x2000)		/* Watch dog timer             */
#define INTSTATUS_POB_OVP			(0x4000)		/* Power output board OVP      */

#define ERR_STATE_BATTERY			(0x0001)		/* Battery power limit warning */
#define ERR_STATE_FAN_STOP_BIT		(1)				/* Fan stop bit                */
#define ERR_STATE_FAN_STOP_MASK		(0x0F)			/* Fan stop mask               */

#define FAILSTATUS_WDT				(0x0001)		/* Watch dog timer fault       */
#define FAILSTATUS_IOWDT			(0x0002)		/* I/O watch dog timer fault   */
#define FAILSTATUS_PSUWDT			(0x0004)		/* PSU watch dog timer fault   */

#define FAILMASK_CLOSE				(0x0000)		/* Stop all error check        */
#define FAILMASK_PFAIL				(0x0008)		/* Start PFAIL check           */
#define FAILMASK_WDT				(0x0010)		/* Start watch dog timer check */
#define FAILMASK_EXTERR				(FAILMASK_PFAIL | FAILMASK_WDT)

/* Watch Dog Timer */
#define WDT_RESET					(0x0000)		/* Reset watch dog timer       */
#define WDT_ENABLE					(0x0001)		/* Enable watch dog timer      */
#define WDT_DISABLE					(0x0000)		/* Disable watch dog timer     */
#define WDT_ERRCLR					(0x0000)		/* Clear watch dog timer error */

/* Timer */
#define TIMER_INTERVAL_100MS		(100)
#define TIMER_INTERVAL_1S			(1000)

typedef struct TIMER_THREAD {
	struct timer_list timer;
	struct work_struct work;
} TIMER_THREAD;

/* Counter */
#define LIMIT_IO_CHECK				(4)						/* 4   [s]  */
#define LIMIT_TEMP_CHECK			(450000 / DMA_PERIODS)	/* 450 [ms] */

/* Temperature */
#define TEMPERATURE_MAP_MAX			(127)
static const char TEMPERATURE_MAP[TEMPERATURE_MAP_MAX + 1] = {
	 33,  33,  33,  34,  34,  34,  35,  35,  35,  36,  36,  36,  37,  37,  38,  38,
	 38,  39,  39,  40,  40,  40,  41,  41,  42,  42,  43,  43,  44,  44,  44,  45,
	 45,  46,  46,  47,  47,  48,  48,  49,  49,  50,  50,  51,  51,  52,  52,  53,
	 54,  54,  55,  55,  56,  56,  57,  58,  58,  59,  59,  60,  61,  61,  62,  63,
	 63,  64,  65,  65,  66,  67,  67,  68,  69,  70,  70,  71,  72,  72,  73,  74,
	 75,  76,  76,  77,  78,  79,  80,  80,  81,  82,  83,  84,  85,  86,  86,  87,
	 88,  89,  90,  91,  92,  93,  94,  95,  96,  97,  98,  99, 100, 101, 102, 103,
	104, 105, 106, 108, 109, 110, 111, 112, 113, 114, 116, 117, 118, 119, 121, 122
};


/* BAR0 */
typedef struct BAR0 {
	u64 command;																	/* 0x0000 */
	u64 int_status;																	/* 0x0008 */
	u64 int_mask;																	/* 0x0010 */
	u64 total_length;																/* 0x0018 */
	u64 rsv1;																		/* 0x0020 */
	u64 capture_mode;																/* 0x0028 */
	u64 cdt_count3;																	/* 0x0030 */
	u64 cdt_fraction3;																/* 0x0038 */
	u64 cdt_count4;																	/* 0x0040 */
	u64 cdt_fraction4;																/* 0x0048 */
	u64 bank_select;																/* 0x0050 */
	u64 cdt_data_length;															/* 0x0058 */
	u64 cdt_count1;																	/* 0x0060 */
	u64 cdt_fraction1;																/* 0x0068 */
	u64 cdt_count2;																	/* 0x0070 */
	u64 cdt_fraction2;																/* 0x0078 */
	u64 rsv2[2*2040];																/* 0x0080 */
	u64 cdt_list[4096];																/* 0x8000 */
} PACKED_NAME(BAR0);

/* BAR2 */
typedef struct PORT_WDT {
	u16 clear;																		/* 0x00010110 [W] */
	u16 enable;																		/* 0x00010112 [W] */
	u16 err_clear;																	/* 0x00010114 [W] */
} PACKED_NAME(PORT_WDT);

typedef struct PORT_HWVER {
	u16 rev_mode;																	/* 0x00010120 [R] */
} PACKED_NAME(PORT_HWVER);

typedef struct PORT_EXTIO {
	u16 led;																		/* 0x00010220 [W] */
	u16 fail_monitor;																/* 0x00010222 [R] */
	u16 fail_mask;																	/* 0x00010224 [W] */
	u16 in_data;																	/* 0x00010226 [R] */
} PACKED_NAME(PORT_EXTIO);

typedef struct PORT_DMA {
	u32 command;																	/* 0x00012000 [W] */
	u32 rsv1[3];																	/* 0x00012004 [-] */
	u64 rsv2;																		/* 0x00012010 [-] */
	u32 recv_total_length;															/* 0x00012018 [W] */
	u32 send_total_length;															/* 0x0001201C [W] */
	u64 rsv3[2*4];																	/* 0x00012020 [-] */
	u32 cdt_count1;																	/* 0x00012060 [W] */
	u32 rsv4[3];																	/* 0x00012064 [-] */
	u32 cdt_count2;																	/* 0x00012070 [W] */
	u32 rsv5[3];																	/* 0x00012074 [-] */
	u32 recv_status;																/* 0x00012080 [R] */
	u32 rsv6;																		/* 0x00012084 [-] */
	u32 send_status;																/* 0x00012088 [R] */
	u32 rsv7;																		/* 0x0001208C [-] */
} PACKED_NAME(PORT_DMA);

typedef union {
	DEFINE_MAP(PORT_WDT,     wdt_ctrl, REG_BAR2_WDT_CTRL);							/* 0x00010110- */
	DEFINE_MAP(PORT_HWVER,   hw_ver,   REG_BAR2_HWVER);								/* 0x00010120- */
	DEFINE_MAP(PORT_EXTIO,   ext_io,   REG_BAR2_EXTIO);								/* 0x00010220- */
	DEFINE_MAP(PORT_FANCTRL, fan_ctrl, REG_BAR2_FANCTRL);							/* 0x00010280- */
	DEFINE_MAP(PORT_POB,     pob,      REG_BAR2_POB);								/* 0x00011100- */
	DEFINE_MAP(PORT_DMA,     dma,      REG_BAR2_DMA);								/* 0x00012000- */
	DEFINE_MAP(PORT_IO,      io,       REG_BAR2_IO);								/* 0x00018000- */
	DEFINE_MAP(PORT_I2C,     i2c,      REG_BAR2_I2C);								/* 0x00060300- */
	DEFINE_MAP(PORT_PSUCOMM, psu_comm, REG_BAR2_PSUCOMM);							/* 0x00060400- */
} BAR2;

/* RECVDMAMAP */
typedef struct RECVDMAMAP_STATUS1 {
	u16 int_monitor;																/* 0x0000 */
	u16 ext_err;																	/* 0x0002 */
	u16 oc_err;																		/* 0x0004 */
	u8  fo_err;																		/* 0x0006 */
	u8  interrupt_skip_count;														/* 0x0007 */
	u16 mini_io_rise_edge;															/* 0x0008 */
	u16 mini_io_fall_edge;															/* 0x000A */
	u8  hand_io_rise_edge;															/* 0x000C */
	u8  rsv1;																		/* 0x000D */
	u8  hand_io_fall_edge;															/* 0x000E */
	u8  rsv2;																		/* 0x000F */
	u32 safety_state_edge;															/* 0x0010 */
	u32 rsv3;																		/* 0x0014 */
} PACKED_NAME(RECVDMAMAP_STATUS1);

typedef union {
	RECVDMAMAP_STATUS1 status1;														/* 0x0000 */
	DEFINE_MAP(RECVDMAMAP_PSUCOMM, psu_comm, REG_RECVDMA_PSUCOMM);					/* 0x00A0 */
	DEFINE_MAP(RECVDMAMAP_FANCTRL, fan,      REG_RECVDMA_FANCTRL);					/* 0x00DC */
	DEFINE_MAP(RECVDMAMAP_STATUS2, status2,  REG_RECVDMA_STATUS2);					/* 0x00E0 */
} RECVDMAMAP;

/* SENDDMAMAP */
typedef struct SENDDMAMAP_NEXTSEND {
	u32 total_size;																	/* 0x00F0 */
	u32 cdt_count;																	/* 0x00F4 */
	u32 rsv1;																		/* 0x00F8 */
	u32 send_cdt_list_addr;															/* 0x00FC */
} PACKED_NAME(SENDDMAMAP_NEXTSEND);

typedef union {
	DEFINE_MAP(SENDDMAMAP_FANCTRL,  fan,                   REG_SENDDMA_FANCTRL);	/* 0x004C */
	DEFINE_MAP(SENDDMAMAP_STATUS,   status,                REG_SENDDMA_STATUS);		/* 0x0060 */
	DEFINE_MAP(SENDDMAMAP_PSUCOMM,  psu_comm,              REG_SENDDMA_PSUCOMM);	/* 0x0080 */
	DEFINE_MAP(SENDDMAMAP_NEXTSEND, next_cycle_send_state, REG_SENDDMA_NEXTSEND);	/* 0x00F0 */
} SENDDMAMAP;

/* Process enumeration */
typedef enum INIT_PROC {
	INIT_CLSREGISTER,		/* Register class                      */
	INIT_ALLOCCHRDEV,		/* Alloc character device number       */
	INIT_CHRDEVINIT,		/* Initialize character device         */
	INIT_DEVICECREATE,		/* Create .ko file                     */
	INIT_PCIREGISTER,		/* Register PCI device                 */
	INIT_MAX,				/* Number of initialize process        */
} INIT_PROC;

typedef enum PROBE_PROC {
	PROBE_ALLOCDEVICE,		/* Alloc device structure              */
	PROBE_DEVICEINIT,		/* Initialize device structure         */
	PROBE_PCIENABLE,		/* Enable PCI device                   */
	PROBE_ALLOCRESOURCE,	/* Alloc PCI resource                  */
	PROBE_MAX,				/* Number of probe process             */
} PROBE_PROC;

typedef enum ALLOC_PROC {
	ALLOC_REQUESTREGION,	/* Alloc resource region               */
	ALLOC_INTURRUPT,		/* Alloc interrupt number              */
	ALLOC_IOMEM,			/* Map memory                          */
	ALLOC_DMAINIT,			/* Initialize DMA                      */
	ALLOC_MODULEINIT,		/* Initialize Module                   */
	ALLOC_MAX,				/* Number of alloc process             */
} ALLOC_PROC;

typedef enum MODULE_PROC {
	MODULE_FRAM,			/* Initialize FRAM module              */
	MODULE_IO,				/* Initialize I/O module               */
	MODULE_FAN,				/* Initialize FAN module               */
	MODULE_FPGA,			/* Initialize FPGA module              */
	MODULE_PSUCOMM,			/* Initialize PSU module               */
	MODULE_MAX,				/* Number of initialize module process */
} MODULE_PROC;


/* Device Structure */
typedef struct IPC8_DEVICE {
	struct pci_dev *pdev;					/* Address of PCI device                   */
	union __iomem {							/* Address of BAR0, 1                      */
		u8 *iomem0;
		BAR0 *bar0mem;
	};
	union __iomem {							/* Address of BAR2                         */
		u8* iomem2;
		BAR2 *bar2mem;
	};
	u8 __iomem *iomem3;						/* Address of BAR3                         */
	u8 __iomem *iomem4;						/* Address of BAR4                         */
	
	dma_addr_t recv_dma_physicaddr;			/* Physical address of receiving DMA       */
	union {									/* Address of receiving DMA                */
		u8 *recv_dma_buf;
		RECVDMAMAP *recv_dma_addr;
	};
	dma_addr_t send_dma_physicaddr;			/* Physical address of sending DMA         */
	union {									/* Address of sending DMA                  */
		u8 *send_dma_buf;
		SENDDMAMAP *send_dma_addr;
	};
	int irq;								/* Number of interrupt handler             */
	spinlock_t mem0_lock;					/* Spinlock for BAR0 read/write            */
	spinlock_t mem2_lock;					/* Spinlock for BAR2 read/write            */
	spinlock_t mem3_lock;					/* Spinlock for BAR3 read/write            */
	spinlock_t irq_lock;					/* Spinlock for interrupt handler          */
	u32 *fram_buffer;						/* FRAM local buffer                       */
	FAN_INFO fan_info;						/* Structure for fan information           */
	FAN_ERROR fan_error;					/* Structure for fan error information     */
	FPGA_INFO fpga_info;					/* Structure for fpga information          */
	FPGA_ERROR fpga_error;					/* Structure for fpga error information    */
	IO_INFO io_info;						/* Structure for I/O information           */
	IO_ERROR io_error;						/* Structure for I/O error information     */
	PSU_INFO psu_info;						/* Structure for psu information           */
	PSU_ERROR psu_error;					/* Structure for psu error information     */
	PM_INFO pm_info;						/* Structure for pm information            */
	PM_ERROR pm_error;						/* Structure for pm error information      */
	u16 mb_version;							/* Version of motion board                 */
	int cpu_tj_max;							/* Corrected value for CPU temperature     */
	TEMP_DATA temp_data;					/* Structure for temperature information   */
	struct work_struct temp_work;
	struct work_struct detect_pm;
	struct work_struct supply_pob;
	int io_check_count;						/* Counter for I/O check                   */
	int temp_check_count;					/* Counter for temperature                 */
	atomic_t fpga_wdt_count;				/* Counter for fpga watch dog timer        */
	atomic_t pob_state_lock;
	struct workqueue_struct *workqueue;
	TIMER_THREAD timer_100ms;
	TIMER_THREAD timer_1s;
	/* Debug */
	u32 addr_bar0;
	u32 addr_bar2;
	u32 ofs_fram;
} IPC8_DEVICE;

#endif /* _IPC8_H */
