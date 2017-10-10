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


#ifndef _IPC8_IO_H
#define _IPC8_IO_H


#define MINI_IO_MASK_ALL		(0xFFFF)
#define HAND_IO_MASK_ALL		(0x00FF)

#define IO_ACCESS_CHECK1		(0x00AA)
#define IO_ACCESS_CHECK2		(0x0055)

#define EMERGENCY_SW			(0x0001 | 0x0002)
#define DEAD_MAN_SW				(0x0004 | 0x0008)
#define AUTO_ENB_SW				(0x0010 | 0x0020)
#define PRT_STOP_SW				(0x0040 | 0x0080)

#define IO_FUSE_INTERNAL		(0x01 | 0x02)		/* Error bit for I/O fuse: Internal power           */
#define IO_FUSE_MINIIO			(0x04 | 0x08)		/* Error bit for I/O fuse: Mini I/O                 */
#define IO_FUSE_HANDIO			(0x10 | 0x20)		/* Error bit for I/O fuse: Hand I/O                 */
#define IO_FUSE_NO_SUPPLY		(0x0100)            /* Error bit for I/O fuse: Power no supply          */
#define IO_FUSE_REVERSE			(0x0200)            /* Error bit for I/O fuse: Power reverse connection */

#define IO_CHECK_EXTERNAL		(IO_FUSE_MINIIO | IO_FUSE_HANDIO)
#define IO_CHECK_INTERNAL		(IO_FUSE_INTERNAL | IO_FUSE_MINIIO | IO_FUSE_HANDIO)

typedef struct IO_INFO {
	u16 mini_in;									/* Mini I/O input : b0-15  */
	u16 mini_out;									/* Mini I/O output: b16-31 */
	u16 hand_in;									/* Hand I/O input : b48-55 */
	u16 hand_out;									/* Hand I/O input : b64-71 */
	bool emergency_stop;							/* Emergency stop          */
	bool dead_man;									/* Deadman switch          */
	bool protect_stop;								/* Protect stop            */
	bool auto_enable;								/* Auto enable             */
	s32 io_pwr_mode;								/* I/O power mode          */
	s32 io_board_type;								/* I/O board type          */
} IO_INFO;

/* BAR2 */
typedef struct PORT_IO {
	u8 rsv1[56];									/* 0x00018000 [-] */
	u16 board_np_type;								/* 0x00018038 [R] */
	u8 rsv2[114];									/* 0x0001803A [-] */
	u16 out_pwr_state;								/* 0x000180AC [R] */
	u8 rsv3[130];									/* 0x000180AE [-] */
	u16 power_mode;									/* 0x00018130 [R] */
} PACKED_NAME(PORT_IO);

/* RECVDMAMAP */
typedef struct RECVDMAMAP_STATUS2 {
	u8  pob_status[4];								/* 0x00E0 */
	u8  mother_temp;								/* 0x00E4 */
	u8  rsv2;										/* 0x00E5 */
	u8  rsv3;										/* 0x00E6 */
	u8  i2c_status;									/* 0x00E7 */
	u16 mini_io;									/* 0x00E8 */
	u8  hand_io;									/* 0x00EA */
	u8  rsv5;										/* 0x00EB */
	u16 safety_state;								/* 0x00EC */
	u16 rsv6;										/* 0x00EE */
	u16 mini_io_ov_temp;							/* 0x00F0 */
	u8  hand_io_ov_temp;							/* 0x00F2 */
	u8  rsv7;										/* 0x00F3 */
	u8  access_check1;								/* 0x00F4 */
	u8  err_state;									/* 0x00F5 */
	u16 break_fuse;									/* 0x00F6 */
	u16 rsv9;										/* 0x00F8 */
	u8  access_check2;								/* 0x00FA */
	u8  rsv10;										/* 0x00FB */
	u8  io_power_poly_sw;							/* 0x00FC */
	u8  io_power_state;								/* 0x00FD */
	u8  access_check3;								/* 0x00FE */
	u8  rsv11;										/* 0x00FF */
} PACKED_NAME(RECVDMAMAP_STATUS2);

/* SENDDMAMAP */
typedef struct SENDDMAMAP_STATUS {
	u16 mini_io;									/* 0x0060 */
	u8  hand_io;									/* 0x0062 */
	u8  rsv1;										/* 0x0063 */
	u8  edge_clear;									/* 0x0064 */
	u8  rsv2;										/* 0x0065 */
	u8  access_check3;								/* 0x0066 */
	u8  rsv3;										/* 0x0067 */
} PACKED_NAME(SENDDMAMAP_STATUS);



int initialize_io(PORT_IO __iomem *io, IO_INFO *io_info);
void get_io_state(IO_INFO *io_info, RECVDMAMAP_STATUS2 *recvdma);
void set_io_state(IO_INFO *io_info, SENDDMAMAP_STATUS *senddma);
void read_write_io(SENDDMAMAP_STATUS *send_dma,
				   RECVDMAMAP_STATUS2 *recv_dma, 
				   IO_INFO *io_info, 
				   IO_ERROR *io_error);
int check_powermode(IO_POWERMODE* io_powermode, 
					s32 io_pwr_mode);
int do_ioctl_read_io(void* arg, 
					 const IO_INFO *io_info, 
					 const IO_ERROR *io_error, 
					 bool is_mini);
int do_ioctl_write_io(void* arg, 
					  IO_INFO* io_info, 
					  const IO_ERROR *io_error, 
					  bool is_mini);
int do_ioctl_read_safetyio(void* arg, const IO_INFO *io_info);
int do_ioctl_read_ioboardtype(void* arg, const IO_INFO *io_info);

#endif /* _IPC8_IO_H */
