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


#ifndef _IPC8_COMMON_H
#define _IPC8_COMMON_H

#define ENABLE_DEBUG_PRINT		(0)		/* enable:1, disable:0 */

#if ENABLE_DEBUG_PRINT
#define DEBUG_PRINT(fmt, ...)	\
	printk(KERN_DEBUG "%s(): "fmt, __func__, ## __VA_ARGS__)
#else
#define DEBUG_PRINT(fmt, ...)
#endif	/* ENABLE_DEBUG_PRINT */

#define PACKED_NAME(name)		__attribute__((packed)) name

int bar0_write32(u32 val, void __iomem *addr, bool verify);
u32 bar0_read32(void __iomem *addr);
int bar2_write16(u16 val, void __iomem *addr, bool verify);
u16 bar2_read16(void __iomem *addr);
int bar2_write32(u32 val, void __iomem *addr, bool verify);
u32 bar2_read32(void __iomem *addr);
u16 bar3_read16(void __iomem *addr);

#endif /* _IPC8_COMMON_H */
