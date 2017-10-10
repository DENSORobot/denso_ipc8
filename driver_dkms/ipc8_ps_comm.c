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
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include "ipc8_ioctl.h"
#include "ipc8_common.h"
#include "ipc8_ps_comm.h"


wait_queue_head_t wait_q;
spinlock_t send_queue_lock;
spinlock_t recv_queue_lock;
spinlock_t stack_lock;
SEND_DATA_QUEUE   *send_data_queue;
RECV_PACKET_QUEUE *recv_packet_queue;
COMM_STATE_STACK  *comm_state_stack;
PSU_PARAM psu_param;


static u16 calc_cd(const PSU_PACKET* packet)
{
	u16 sum = PSU_ENQ;
	u8 len  = SZ_PSU_PACKET_HEADER + packet->len;
	u8* p   = ((u8*)packet) + 1;
	u8* end = ((u8*)packet) + len;
	
	while (p < end) {
		sum += (u16)*p;
		p++;
	}
	
	return (((sum & 0xFF00) >> 8) | ((sum & 0x00FF) << 8));
}

static void set_cd(PSU_PACKET* packet)
{
	*((u16*)&packet->data[packet->len]) = calc_cd(packet);
}

static bool check_cd(const PSU_PACKET* packet)
{
	return (*((const u16*)&packet->data[packet->len]) == calc_cd(packet));
}

PSU_PACKET create_psu_packet(u16 command, u8 len, const void* buf)
{
	PSU_PACKET packet;
	
	packet.enq     = PSU_ENQ;
	packet.len     = len;
	packet.command = command;
	packet.from_id = PSU_ENGINE_ID;
	packet.to_id   = PSU_PSCPU_ID;
	if (len > 0) {
		memcpy(packet.data, buf, len);
	}
	
	return packet;
}

static int next_index(int index, int queue_size)
{
	return (index + 1) % queue_size;
}

int send_enqueue(PSU_COMM_DATA pkt)
{
	int ret = -1;
	unsigned long flags;
	
	if (send_data_queue) {
		spin_lock_irqsave(&send_queue_lock, flags);
		
		if (next_index(send_data_queue->tail, SEND_QUEUE_SIZE) 
			== send_data_queue->head) {
			printk(KERN_WARNING "%s: Queue is full. \n", __func__);
		}
		else {
			send_data_queue->queue[send_data_queue->tail] = pkt;
			send_data_queue->tail 
				= next_index(send_data_queue->tail, SEND_QUEUE_SIZE);
			ret = 0;
		}
		
		spin_unlock_irqrestore(&send_queue_lock, flags);
	}
	return ret;
}

static int send_dequeue(PSU_COMM_DATA *pkt)
{
	int ret = -1;
	unsigned long flags;
	
	if (send_data_queue) {
		spin_lock_irqsave(&send_queue_lock, flags);
		
		/* Queue empty? */
		if (send_data_queue->head != send_data_queue->tail) {
			*pkt = send_data_queue->queue[send_data_queue->head];
			send_data_queue->head 
				= next_index(send_data_queue->head, SEND_QUEUE_SIZE);
			ret = 0;
		}
		
		spin_unlock_irqrestore(&send_queue_lock, flags);
	}
	
	return ret;
}

static int init_send_queue(void)
{
	int ret = -1;
	unsigned long flags;
	
	DEBUG_PRINT("-->\n");
	
	if (send_data_queue) {
		spin_lock_irqsave(&send_queue_lock, flags);
		
		send_data_queue->head = 0;
		send_data_queue->tail = 0;
		
		spin_unlock_irqrestore(&send_queue_lock, flags);
		ret = 0;
	}
	
	DEBUG_PRINT("<--\n");
	return ret;
}

static int recv_enqueue(PSU_PACKET pkt)
{
	int ret = -1;
	unsigned long flags;
	
	if (recv_packet_queue) {
		spin_lock_irqsave(&recv_queue_lock, flags);
		
		if (next_index(recv_packet_queue->tail, RECV_QUEUE_SIZE) 
			== recv_packet_queue->head) {
			printk(KERN_WARNING "%s: Queue is full. \n", __func__);
		}
		else {
			recv_packet_queue->queue[recv_packet_queue->tail] = pkt;
			recv_packet_queue->tail
				 = next_index(recv_packet_queue->tail, RECV_QUEUE_SIZE);
			ret = 0;
		}
		
		spin_unlock_irqrestore(&recv_queue_lock, flags);
	}
	return ret;
}

static int recv_dequeue(PSU_PACKET *pkt)
{
	int ret = -1;
	unsigned long flags;
	
	if (recv_packet_queue) {
		spin_lock_irqsave(&recv_queue_lock, flags);
		
		/* Queue empty? */
		if (recv_packet_queue->head != recv_packet_queue->tail) {
			*pkt = recv_packet_queue->queue[recv_packet_queue->head];
			recv_packet_queue->head 
				= next_index(recv_packet_queue->head, RECV_QUEUE_SIZE);
			ret = 0;
		}
		
		spin_unlock_irqrestore(&recv_queue_lock, flags);
	}
	
	return ret;
}

static int init_recv_queue(void)
{
	int ret = -1;
	unsigned long flags;
	
	DEBUG_PRINT("-->\n");
	
	if (recv_packet_queue) {
		spin_lock_irqsave(&recv_queue_lock, flags);
		
		recv_packet_queue->head = 0;
		recv_packet_queue->tail = 0;
		
		spin_unlock_irqrestore(&recv_queue_lock, flags);
		ret = 0;
	}
	
	DEBUG_PRINT("<--\n");
	return ret;
}

static int state_push_stack(PSU_COMM_STATE data)
{
	int ret = -1;
	unsigned long flags;
	
	if (comm_state_stack) {
		spin_lock_irqsave(&stack_lock, flags);
		
		/* Stack empty? */
		if (STACK_SIZE > comm_state_stack->pos) {
			comm_state_stack->stack[comm_state_stack->pos] = data;
			comm_state_stack->pos++;
			ret = 0;
		}
		
		spin_unlock_irqrestore(&stack_lock, flags);
	}
	
	return ret;
}

static int state_pop_stack(PSU_COMM_STATE *data)
{
	int ret = -1;
	unsigned long flags;
	
	if (comm_state_stack) {
		spin_lock_irqsave(&stack_lock, flags);
		
		if (0 < comm_state_stack->pos) {
			comm_state_stack->pos--;
			*data = comm_state_stack->stack[comm_state_stack->pos];
			ret = 0;
		}
		
		spin_unlock_irqrestore(&stack_lock, flags);
	}
	
	return ret;
}

static int init_state_stack(void)
{
	int ret = -1;
	unsigned long flags;
	
	DEBUG_PRINT("-->\n");
	
	if (comm_state_stack) {
		spin_lock_irqsave(&stack_lock, flags);
		
		comm_state_stack->pos = 0;
		ret = 0;
		
		spin_unlock_irqrestore(&stack_lock, flags);
	}
	
	DEBUG_PRINT("<--\n");
	return ret;
}


static void set_tx_data(PSU_INFO *psu_info)
{
	PSU_COMM_DATA data;
	
	DEBUG_PRINT("-->\n");
	
	/* Engine start-up completion */
	memset(&data, 0, sizeof(PSU_COMM_DATA));
	data.packet      = create_psu_packet(PSU_CMD_READY, 0, NULL);
	data.result_addr = NULL;
	data.result_size = 0;
	data.wait        = false;
	send_enqueue(data);
	
	/* Software version acquisition */
	memset(&data, 0, sizeof(PSU_COMM_DATA));
	data.packet      = create_psu_packet(PSU_CMD_GETSWVER, 0, NULL);
	data.result_addr = (u8 *)&psu_info->psu_sf_version;
	data.result_size = sizeof(psu_info->psu_sf_version);
	data.wait        = false;
	send_enqueue(data);
	
	/* Hard version acquisition */
	memset(&data, 0, sizeof(PSU_COMM_DATA));
	data.packet      = create_psu_packet(PSU_CMD_GETHWVER, 0, NULL);
	data.result_addr = (u8 *)&psu_info->psu_hw_version;
	data.result_size = sizeof(psu_info->psu_hw_version);
	data.wait        = false;
	send_enqueue(data);
	
	/* AC input voltage */
	memset(&data, 0, sizeof(PSU_COMM_DATA));
	data.packet      = create_psu_packet(PSU_CMD_ACMODE, 0, NULL);
	data.result_addr = (u8 *)&psu_info->ac_pwr_mode;
	data.result_size = sizeof(psu_info->ac_pwr_mode);
	data.wait        = false;
	send_enqueue(data);
	
	DEBUG_PRINT("<--\n");
}

static void release_psu_local_buffer(void)
{
	if (send_data_queue) {
		kfree(send_data_queue);
		send_data_queue = NULL;
	}
	if (recv_packet_queue) {
		kfree(recv_packet_queue);
		recv_packet_queue = NULL;
	}
	if (comm_state_stack) {
		kfree(comm_state_stack);
		comm_state_stack = NULL;
	}

}

int initialize_psu_comm(PORT_PSUCOMM __iomem *psu_comm, PSU_INFO *psu_info)
{
	int ret = -ENOMEM;
	u32 val = 0;
	u16 send_data = 0;	/* Dummy initialization */
	
	DEBUG_PRINT("-->\n");
	
	/* Initialization */
	memset(&psu_param, 0, sizeof(PSU_PARAM));
	
	psu_param.ack_packet = create_psu_packet(PSU_RESPONSE_ACK, 2, &send_data);
	psu_param.nak_packet = create_psu_packet(PSU_RESPONSE_NAK, 0, NULL);
	
	init_waitqueue_head(&wait_q);
	
	spin_lock_init(&send_queue_lock);
	spin_lock_init(&recv_queue_lock);
	spin_lock_init(&stack_lock);
	
	/* Memory allocation */
	send_data_queue   = kzalloc((sizeof(SEND_DATA_QUEUE)
								+ sizeof(PSU_COMM_DATA) * SEND_QUEUE_SIZE), 
								GFP_KERNEL);
	
	recv_packet_queue = kzalloc((sizeof(RECV_PACKET_QUEUE)
								+ sizeof(PSU_PACKET) * RECV_QUEUE_SIZE), 
								GFP_KERNEL);
	
	comm_state_stack  = kzalloc((sizeof(COMM_STATE_STACK)
								+ sizeof(PSU_COMM_STATE) * STACK_SIZE),
								GFP_KERNEL);
	
	if ((!send_data_queue)
		|| (!recv_packet_queue)
		|| (!comm_state_stack)) {
		printk(KERN_ERR "%s: Cannot allocate memory. \n", __func__);
		goto ErrProc;
	}
	
	/* Queue initialization */
	init_send_queue();
	init_recv_queue();
	
	/* Stack initialization */
	init_state_stack();
	
	/* Set transmit data  */
	set_tx_data(psu_info);
	
	/* Division ratio and block number of bytes  */
	val = PSU_DIVRATE_AND_BLOCK;
	bar2_write32(val, &psu_comm->div_rate_and_blkbyte, false);
	
	/* Block interval  */
	val = PSU_BYTEINTERVAL;
	bar2_write32(val, &psu_comm->byte_interval, false);
	
	/* Communication permission */
	val = (0x80000000 | PSU_DIVRATE_AND_BLOCK);
	bar2_write32(val, &psu_comm->div_rate_and_blkbyte, false);
	
	ret = 0;
	
	DEBUG_PRINT("<--\n");
	return ret;
	
ErrProc:
	release_psu_local_buffer();
	
	DEBUG_PRINT("<--\n");
	return ret;
}

void release_psu_comm(PORT_PSUCOMM __iomem *psu_comm)
{
	u32 val   = 0;
	
	DEBUG_PRINT("-->\n");
	
	/* Communication disabled */
	val = PSU_DIVRATE_AND_BLOCK;
	bar2_write32(val, &psu_comm->div_rate_and_blkbyte, false);
	
	release_psu_local_buffer();
	
	DEBUG_PRINT("<--\n");
}

int set_tx_pkt_sync(PSU_COMM_DATA s_data, PSU_PACKET *r_data)
{
	int ret = -1;
	long timeout;
	DEBUG_PRINT("-->\n");
	
	if (psu_param.sync_wait) {
		DEBUG_PRINT("wait sync \n");
		ret = -2;
		goto WAIT_SYNC;
	}
	
	ret = send_enqueue(s_data);
	if (0 <= ret) {
		psu_param.sync_wait = true;
		timeout = wait_event_interruptible_timeout(wait_q, 
												   (psu_param.sync_wait == false), 
												   (5 * HZ));
		if (0 >= timeout) {
			printk(KERN_ERR "%s: timeout \n", __func__);
		}
		else {
			ret = 0;
		}
	}
	
	if (0 <= ret) {
		*r_data = psu_param.dispatch_packet;
	}
	
WAIT_SYNC:
	DEBUG_PRINT("<--\n");
	return ret;
}


static void send_retry_packet(PSU_ERROR *psu_error)
{
	int res;
	
	psu_param.flag_receiving = false;
	psu_param.comm_state.timeout_retry_count++;
	
	if (LIMIT_TIMEOUT_RETRY <= psu_param.comm_state.timeout_retry_count) {
		/* Error recording */
		psu_error->error |= PSU_ERROR_COMMUNICATION;
		psu_error->psu_communication |= PSU_COMM_TIMEOUT_OVER;
		
		switch (psu_param.comm_state.use_packet) {
		case PSU_USE_NAK:
			res = state_pop_stack(&psu_param.comm_state);
			break;
		default:
			psu_param.flag_no_dequeue = false;
			break;
		}
	}
}

static void check_retry_count(PSU_ERROR *psu_error)
{
	int res;
	
	psu_param.timeout_count++;
	
	if (LIMIT_TIMEOUT <= psu_param.timeout_count) {
		switch (psu_param.comm_state.use_packet) {
		case PSU_USE_QUEUE:
			send_retry_packet(psu_error);
			break;
		case PSU_USE_ACK:
			/* Pop the communication state */
			res = state_pop_stack(&psu_param.comm_state);
		case PSU_USE_NAK:
			if (LIMIT_SEND_NAK == psu_param.comm_state.nak_send_count) {
				/* Pop the communication state */
				res = state_pop_stack(&psu_param.comm_state);
			}
			else {
				send_retry_packet(psu_error);
			}
		default:
			break;
		}
	}
}

static bool analyze_recv_packet(PSU_ERROR *psu_error)
{
	bool ret = false;
	int res;
	
	/* Checksum */
	if (check_cd(&psu_param.dispatch_packet)) {
		/* Checksum normal */
		/* Pop the communication state */
		res = state_pop_stack(&psu_param.comm_state);
		ret = true;
	}
	else {
		/* Checksum error */
		psu_param.comm_state.nak_send_count++;
		
		if (LIMIT_SEND_NAK <= psu_param.comm_state.nak_send_count) {
			/* Error recording */
			psu_error->error |= PSU_ERROR_COMMUNICATION;
			psu_error->psu_communication |= PSU_COMM_SEND_NAK_OVER;
		}
		else {
			/* NAK send */
			if (PSU_USE_NAK != psu_param.comm_state.use_packet) {
				/* Push the communication state */
				res = state_push_stack(psu_param.comm_state);
				/* Clear communication state */
				memset(&psu_param.comm_state, 0, sizeof(PSU_COMM_STATE));
				psu_param.comm_state.nak_send_count = 1;
				psu_param.comm_state.use_packet = PSU_USE_NAK;
			}
		}
		ret = false;
	}
	
	return ret;
}

static bool match_command(void)
{
	bool ret = false;
	u16 recv_cmd;
	
	recv_cmd = psu_param.dispatch_packet.data[0] 
			| (psu_param.dispatch_packet.data[1] << 8);
	
	if (psu_param.send_data.packet.command
		== recv_cmd) {
		ret = true;
	}
	
	return ret;
}

static void dispatch_ack_rej_packet(PSU_ERROR *psu_error, bool ack_packet)
{
	u8 len, recv_len;
	int i;
	
	/* Command match confirmation  */
	if (match_command()) {
		psu_param.flag_no_dequeue = false;
		if (ack_packet) {
			/* ACK */
			recv_len = (psu_param.dispatch_packet.len - 2);
			len = min(psu_param.send_data.result_size, recv_len );
			
			for (i=0; i < len; i++) {
				psu_param.send_data.result_addr[i] 
					= psu_param.dispatch_packet.data[(2 + len - i - 1)];
			}
		}
		else {
			/* REJ */
			/* Error recording */
			psu_error->error |= PSU_ERROR_COMMUNICATION;
			psu_error->psu_communication |= PSU_COMM_RECV_REJ;
		}
	}
	else {
		psu_param.flag_receiving = true;
	}
}

static void dispatch_error_warning_packet(PSU_ERROR *psu_error)
{
	int res;
	
	/* Push the communication state */
	res = state_push_stack(psu_param.comm_state);
	
	/* Clear communication state */
	memset(&psu_param.comm_state, 0, sizeof(PSU_COMM_STATE));
	
	/* Command set in the ACK packet */
	*((u16*)psu_param.ack_packet.data) = psu_param.dispatch_packet.command;
	psu_param.comm_state.use_packet = PSU_USE_ACK;
	
	/* Error recording */
	psu_error->error |= PSU_ERROR_ERRWARN;
	psu_error->psu_err_warn = psu_param.dispatch_packet.data[0];
}

static void dispatch_recv_packet(PSU_INFO *psu_info, PSU_ERROR *psu_error)
{
	bool ret;
	u16 command;
#if DEBUG_RCV
	int i;
	bool wait_flag = psu_param.send_data.wait;
	int len = SZ_PSU_PACKET_MIN + psu_param.dispatch_packet.len;
#endif	/* DEBUG_RCV */
	
	/* Packet analysis */
	ret = analyze_recv_packet(psu_error);
	if (!ret) {
		return;
	}
	
	/* Retry information removal */
	command = psu_param.dispatch_packet.command & ~PACKET_RETRY_MASK;
	
#if DEBUG_RCV
	DEBUG_PRINT("_/_/_/_/_/_/_/_/_/_/_/_/_/ \n");
	for (i=0; i < len; i++) {
		DEBUG_PRINT("[%d] 0x%04x \n",  i, psu_param.dispatch_packet.buf[i]);
	}
	DEBUG_PRINT("command length = %d     \n", len);
	DEBUG_PRINT("wait_flag      = %d     \n", wait_flag);
	DEBUG_PRINT("result_size    = %d     \n", psu_param.send_data.result_size);
	DEBUG_PRINT("command        = 0x%08x \n", command);
	DEBUG_PRINT("_/_/_/_/_/_/_/_/_/_/_/_/_/ \n");
#endif	/* DEBUG_RCV */
	
	switch (command) {
	case PSU_RESPONSE_ACK:
		dispatch_ack_rej_packet(psu_error, true);
		break;
	case PSU_RESPONSE_REJ:
		dispatch_ack_rej_packet(psu_error, false);
		break;
	case PSU_RESPONSE_NAK:
		psu_param.comm_state.nak_recv_count++;

		if (LIMIT_RECV_NAK <= psu_param.comm_state.nak_recv_count) {
			psu_param.flag_no_dequeue = false;
			/* Error recording */
			psu_error->error |= PSU_ERROR_COMMUNICATION;
			psu_error->psu_communication |= PSU_COMM_RECV_NAK_OVER;
		}
		break;
	case PSU_CMD_ERRWARN:
		dispatch_error_warning_packet(psu_error);
		break;
	default:
		psu_param.flag_no_dequeue = false;
		/* Error recording */
		psu_error->error |= PSU_ERROR_COMMUNICATION;
		psu_error->psu_communication |= PSU_COMM_RECV_UNKNOWN;
		break;
	}
}

static void get_recv_packet(RECVDMAMAP_PSUCOMM *recv_dma, 
							PSU_INFO *psu_info, 
							PSU_ERROR *psu_error, 
							int data_pos)
{
	int pos, len, ret;
	u8 rx_data, len_data;
	
	pos = psu_param.recv_pos;
	rx_data = recv_dma->rx_data[data_pos];
	
	switch (pos) {
	case PSU_POS_ENQ:
		if (PSU_ENQ != rx_data) {
			printk(KERN_WARNING "%s: Not ENQ ! (rx_data:0x%x) \n", __func__, rx_data);
			check_retry_count(psu_error);
			return;
		}
		break;
	case PSU_POS_LEN:
		if (SZ_PSU_PACKET_DATA < rx_data) {
			rx_data = SZ_PSU_PACKET_DATA;
		}
		break;
	default:
		break;
	}
	
	/* Data storage */
	psu_param.recv_packet.buf[pos] = rx_data;
	psu_param.recv_pos++;
	
	if (SZ_PSU_PACKET_MIN > psu_param.recv_pos) {
		return;
	}
	
	len_data = psu_param.recv_packet.buf[PSU_POS_LEN];
	len = SZ_PSU_PACKET_MIN + len_data;
	if (len <= psu_param.recv_pos) {
		psu_param.recv_pos = 0;
		ret = recv_enqueue(psu_param.recv_packet);
	}
}

static void set_send_packet(SENDDMAMAP_PSUCOMM *send_dma)
{
	int ret = -1;
	int pos, len;
	PSU_PACKET *pkt;
	
	/* Receive status check */
	if (!psu_param.flag_receiving) {
		/* Use packet determine */
		switch (psu_param.comm_state.use_packet) {
		case PSU_USE_ACK:
			pkt = &psu_param.ack_packet;
			break;
		case PSU_USE_NAK:
			pkt = &psu_param.nak_packet;
			break;
		case PSU_USE_QUEUE:
		default:
			/* Dequeue necessity check */
			if (!psu_param.flag_no_dequeue) {
				/* sync wake up ? */
				if (psu_param.sync_wait && psu_param.send_data.wait) {
					psu_param.sync_wait = false;
					wake_up_interruptible(&wait_q);
				}
				
				ret = send_dequeue(&psu_param.send_data);
				if (0 > ret) {
					return;
				}
				else{
					/* Clear communication state */
					memset(&psu_param.comm_state, 0, sizeof(PSU_COMM_STATE));
					/* Dequeue prohibit */
					psu_param.flag_no_dequeue = true;
				}
			}
			
			pkt = &psu_param.send_data.packet;
			break;
		}
		
		pos = psu_param.send_pos;
		len = SZ_PSU_PACKET_MIN + pkt->len;
		if (pos < len) {
			if (PSU_POS_ENQ == pos) {
				/*Retry setting */
				pkt->command |= (psu_param.comm_state.timeout_retry_count ? PACKET_RETRY_FLAG : 0);
				pkt->command |= (psu_param.comm_state.timeout_retry_count * PACKET_RETRY_COUNT);
				set_cd(pkt);
			}
			psu_param.flag_sending = true;
			/* There is unsent data */
			send_dma->tx_start_bit = PSU_START_BIT_ON;
			send_dma->tx_data      = pkt->buf[pos];
			psu_param.send_pos++;
		}
		else {
			/* Migrating to receive wait */
			psu_param.flag_sending    = false;
			psu_param.flag_receiving  = true;
			psu_param.send_pos        = 0;
			psu_param.timeout_count   = 0;
		}
	}
}

void power_communication(SENDDMAMAP_PSUCOMM *send_dma, 
						 RECVDMAMAP_PSUCOMM *recv_dma, 
						 PSU_INFO *psu_info, 
						 PSU_ERROR *psu_error)
{
	int len, i, ret;
	
	/* Send packet setting */
	set_send_packet(send_dma);
	
	/* Reception check */
	len = recv_dma->rx_data_length;
	if (0 < len) {
		if (SZ_RECVDMA_PACKET < len) {
			len = SZ_RECVDMA_PACKET;
		}
		for (i=0; i < len; i++) {
			get_recv_packet(recv_dma, psu_info, psu_error, i);
		}
	}
	else if (psu_param.flag_receiving) {
		check_retry_count(psu_error);
	}
	
	if (!psu_param.flag_sending) {
		ret = recv_dequeue(&psu_param.dispatch_packet);
		if (0 <= ret) {
			psu_param.flag_receiving = false;
			dispatch_recv_packet(psu_info, psu_error);
		}
	}
}

int do_ioctl_read_psuversion(void* arg, const PSU_INFO *psu_info)
{
	int ret = 0;
	PSU_VERSION psu_ver;
	
	DEBUG_PRINT("-->\n");
	
	memset(&psu_ver, 0, sizeof(PSU_VERSION));
	
	psu_ver.sf_ver = psu_info->psu_sf_version;
	psu_ver.hw_ver = psu_info->psu_hw_version;
	psu_ver.outval = 0;
	
	if (copy_to_user(arg, &psu_ver, sizeof(PSU_VERSION))) {
		ret = -EFAULT;
	}
	
	DEBUG_PRINT("<--\n");

	return ret;
}

int do_ioctl_read_acpowermode(void* arg, const PSU_INFO *psu_info)
{
	int ret = 0;
	VAR_DATA var_data;
	
	DEBUG_PRINT("-->\n");
	
	memset(&var_data, 0, sizeof(VAR_DATA));
	
	var_data.data = psu_info->ac_pwr_mode;
	var_data.outval = 0;
	
	if (copy_to_user(arg, &var_data, sizeof(VAR_DATA))) {
		ret = -EFAULT;
	}
	
	DEBUG_PRINT("<--\n");
	return ret;
}

int do_ioctl_read_acinputvoltage(void* arg, const PSU_INFO *psu_info)
{
	int ret = 0;
	VAR_DATA var_data;
	
	DEBUG_PRINT("-->\n");
	
	memset(&var_data, 0, sizeof(VAR_DATA));
	
	var_data.data = psu_info->ac_input_voltage;
	var_data.outval = 0;
	
	if (copy_to_user(arg, &var_data, sizeof(VAR_DATA))) {
		ret = -EFAULT;
	}
	
	DEBUG_PRINT("<--\n");
	return ret;
}
