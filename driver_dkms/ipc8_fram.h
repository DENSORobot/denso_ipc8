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


#ifndef _IPC8_FRAM_H
#define _IPC8_FRAM_H


#define FRAM_ACCESS_SIZE	(4)								/* FRAM access size                 */

#define FRAM_OFFSET			(0x00200000)					/* FRAM start address               */
#define FRAM_LENGTH			(0x00200000)					/* FRAN buffer length               */
#define FRAM_RESERV			(0x00020000)					/* Reserved buffer length           */
#define FRAM_USER_OFFSET	(FRAM_OFFSET + FRAM_RESERV)		/* FRAM start address for user data */
#define FRAM_USER_LENGTH	(FRAM_LENGTH - FRAM_RESERV)		/* FRAM buffer length for user data */



int initialize_fram(u8 __iomem *iomem0, u32 *fram_buffer);
int do_ioctl_read_fram(void* arg);
int do_ioctl_write_fram(void* arg);
/* Debug */
u32 fram_read_local_sys(u32 ofs);
size_t fram_write_local_sys(u32 ofs, u32 write_data);

#endif /* _IPC8_FRAM_H */
