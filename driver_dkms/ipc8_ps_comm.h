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


#ifndef _IPC8_PS_COMM_H
#define _IPC8_PS_COMM_H


#define DEBUG_RCV						(0)		/* enable:1, disable:0 */

#define PSU_START_BIT_ON				(0x00)

#define PSU_ENQ							(5)

#define PSU_CMDCLS_SEND					(0x04)
#define PSU_CMDCLS_RESP					(0x0E)

#define PSU_ENGINE_ID					(0x90)
#define PSU_PSCPU_ID					(0x40)

#define PSU_CMD(Code)					((u16)(PSU_CMDCLS_SEND | ((Code) << 8)))		/* Command definition for psu communication  */
#define PSU_CMD_GETSWVER				PSU_CMD(0x05)									/* Command: Get psu software version         */
#define PSU_CMD_GETHWVER				PSU_CMD(0x06)									/* Command: Get psu hardware version         */
#define PSU_CMD_ERRWARN					PSU_CMD(0x07)									/* Command: Psu error warning                */
#define PSU_CMD_ACVOLT					PSU_CMD(0x08)									/* Command: Get AC input voltage             */
#define PSU_CMD_READY					PSU_CMD(0x09)									/* Command: Start communication              */
#define PSU_CMD_ACMODE					PSU_CMD(0x0C)									/* Command: Get AC power mode                */
#define PSU_CMD_IOPWR_ON				PSU_CMD(0x32)									/* Command: I/O internal power on            */
#define PSU_CMD_TEMPERATURE				PSU_CMD(0x33)									/* Command: Get temperature                  */

#define PSU_DATA_ACVOLT					((u16)(0x00 | 0x01 << 8))						/* Data: Get AC input voltage                */
#define PSU_DATA_TEMPERATURE_PSU		(0x00)											/* Data: Get psu cpu temperature             */
#define PSU_DATA_TEMPERATURE_MOTPWR		(0x01)											/* Data: Get motor power temperature         */

#define PSU_RESPONSE(Code)				((u16)(PSU_CMDCLS_RESP | ((Code) << 8)))		/* Response definition for psu communication */
#define PSU_RESPONSE_ACK				PSU_RESPONSE(0x02)								/* Response: ACK                             */
#define PSU_RESPONSE_NAK				PSU_RESPONSE(0x82)								/* Response: NAK                             */
#define PSU_RESPONSE_REJ				PSU_RESPONSE(0x01)								/* Response: REJ                             */

#define PSU_POS_ENQ						(0)
#define PSU_POS_LEN						(1)

#define PSU_CLOCK						(64000000)										/* PSU communication clock (60MHz)           */
#define PSU_BAUDRATE					(83333)											/* PSU communication baudrate (83.333kHz)    */
#define PSU_DIVRATE						(762)											/* PSU communication division ratio          */
#define PSU_BYTEINTERVAL				(10)											/* PSU communication byte interval           */
#define PSU_BLOCKBYTES					(1)
#define PSU_BLOCKINTERVAL				(0)
#define PSU_DIVRATE_AND_BLOCK			(PSU_DIVRATE | (PSU_BLOCKINTERVAL << 16) | (PSU_BLOCKBYTES << 24))

#define PACKET_RETRY_MASK				(0xF0)
#define PACKET_RETRY_FLAG				(0x10)
#define PACKET_RETRY_COUNT				(0x20)

#define SZ_PSU_PACKET					(32)											/* Maximum packet size                       */
#define SZ_PSU_PACKET_HEADER			(6)												/* Header size                               */
#define SZ_PSU_PACKET_CHECK				(2)												/* Check digit size                          */
#define SZ_PSU_PACKET_MIN				(SZ_PSU_PACKET_HEADER + SZ_PSU_PACKET_CHECK)	/* Minimam packet size                       */
#define SZ_PSU_PACKET_DATA				(SZ_PSU_PACKET - SZ_PSU_PACKET_MIN)				/* Maximum packet data size                  */

#define SZ_RECVDMA_PACKET				(15)											/* Receive DMA buffer size                   */

#define LIMIT_TIMEOUT					(40000 / DMA_PERIODS)							/* 40 [ms]                                   */
#define LIMIT_TIMEOUT_RETRY				(8)
#define LIMIT_SEND_NAK					(8)
#define LIMIT_RECV_NAK					(8)

#define SEND_QUEUE_SIZE					(5 + 1)
#define RECV_QUEUE_SIZE					(5 + 1)
#define STACK_SIZE						(4)

#define PSU_COMM_RECV_UNKNOWN			(0x0001)										/* PSU Communication: Receive unknown packet        */
#define PSU_COMM_RECV_REJ				(0x0002)										/* PSU Communication: Receive REJ packet            */
#define PSU_COMM_RECV_NAK_OVER			(0x0004)										/* PSU Communication: Receive NAK packet limit over */
#define PSU_COMM_SEND_NAK_OVER			(0x0008)										/* PSU Communication: Send NAK packet limit over    */
#define PSU_COMM_TIMEOUT_OVER			(0x0010)										/* PSU Communication: Timeout retry limit over      */

#define DMA_PERIODS						(250)											/* 250 [us]                                  */


typedef enum PSU_USE_PACKET {
	PSU_USE_QUEUE,					/* sending_packet = send_data.packet */
	PSU_USE_ACK,					/* sending_packet = ack_packet       */
	PSU_USE_NAK,					/* sending_packet = nak_packet       */
} PSU_USE_PACKET;

typedef union PSU_PACKET {
	struct {
		u8 enq;
		u8 len;
		u16 command;
		u8 from_id;
		u8 to_id;
		u8 data[SZ_PSU_PACKET_DATA + SZ_PSU_PACKET_CHECK];
	};
	u8 buf[SZ_PSU_PACKET];
} PSU_PACKET;

typedef struct PSU_COMM_DATA {		/* Structure for psu communication */
	PSU_PACKET packet;				/* PSU packet                      */
	u8* result_addr;				/* Address of communication result */
	u8 result_size;					/* Size of result_addr             */
	bool wait;						/* Synchronous wait                */
} PSU_COMM_DATA;

typedef struct PSU_COMM_STATE {
	int timeout_retry_count;
	int nak_send_count;
	int nak_recv_count;
	PSU_USE_PACKET use_packet;
} PSU_COMM_STATE;

typedef struct SEND_DATA_QUEUE {
	PSU_COMM_DATA queue[SEND_QUEUE_SIZE];
	int head;
	int tail;
} SEND_DATA_QUEUE;

typedef struct RECV_PACKET_QUEUE {
	PSU_PACKET queue[RECV_QUEUE_SIZE];
	int head;
	int tail;
} RECV_PACKET_QUEUE;

typedef struct COMM_STATE_STACK {
	PSU_COMM_STATE stack[STACK_SIZE];
	int pos;
} COMM_STATE_STACK;

typedef struct PSU_PARAM {
	PSU_COMM_DATA send_data;		/* Last dequeued data from SEND_DATA_QUEUE */
	PSU_PACKET ack_packet;			/* ACK packet                              */
	PSU_PACKET nak_packet;			/* NAK packet                              */
	PSU_PACKET recv_packet;			/* Receiving packet                        */
	PSU_PACKET dispatch_packet;		/* Dispatch packet                         */
	int send_pos;					/* Offset of sending packet                */
	int recv_pos;					/* Offset of receiving packet              */
	int timeout_count;				/* Counter for timeout                     */
	bool flag_no_dequeue;			/* Dequeue prohibition flag                */
	bool flag_receiving;			/* Receiving flag                          */
	bool flag_sending;				/* Sending flag                            */
	bool sync_wait;					/* Synchronous wait flag                   */
	PSU_COMM_STATE comm_state;		/* Current communication state             */
} PSU_PARAM;

typedef struct PSU_IFNO {
	u16 psu_sf_version;				/* PSU software version */
	u16 psu_hw_version;				/* PSU hardware version */
	u8 ac_pwr_mode;					/* AC power mode        */
	u16 ac_input_voltage;			/* AC input voltage     */
} PSU_INFO;

/* BAR2 */
typedef struct PORT_PSUCOMM {
	u32 div_rate_and_blkbyte;		/* 0x00060400 [W] */
	u32 byte_interval;				/* 0x00060404 [W] */
} PACKED_NAME(PORT_PSUCOMM);

/* RECVDMAMAP */
typedef struct RECVDMAMAP_PSUCOMM {
	u8  rx_data_length;				/* 0x00A0 */
	u8  rx_data[15];				/* 0x00A1 */
	u64 rsv1[2];					/* 0x00B0 */
} PACKED_NAME(RECVDMAMAP_PSUCOMM);

/* SENDDMAMAP */
typedef struct SENDDMAMAP_PSUCOMM {
	u8  tx_start_bit;				/* 0x0080 */
	u8  tx_data;					/* 0x0081 */
	u16 rsv1;						/* 0x0082 */
	u32 rsv2;						/* 0x0084 */
	u64 rsv3;						/* 0x0088 */
	u64 rsv4[2];					/* 0x0090 */
} PACKED_NAME(SENDDMAMAP_PSUCOMM);



int initialize_psu_comm(PORT_PSUCOMM __iomem *psu_comm, 
						PSU_INFO *psu_info);
void release_psu_comm(PORT_PSUCOMM __iomem *psu_comm);
PSU_PACKET create_psu_packet(u16 command, 
							 u8 len, 
							 const void* buf);
int send_enqueue(PSU_COMM_DATA pkt);
int set_tx_pkt_sync(PSU_COMM_DATA s_data, PSU_PACKET *r_data);
void power_communication(SENDDMAMAP_PSUCOMM *send_dma, 
						 RECVDMAMAP_PSUCOMM *recv_dma, 
						 PSU_INFO *psu_info, 
						 PSU_ERROR *psu_error);
int do_ioctl_read_psuversion(void* arg, const PSU_INFO *psu_info);
int do_ioctl_read_acpowermode(void* arg, const PSU_INFO *psu_info);
int do_ioctl_read_acinputvoltage(void* arg, const PSU_INFO *psu_info);

#endif /* _IPC8_PS_COMM_H */
