#include <ptcl_codec/ptcl_packet_sg.h>
#include <ptcl_codec/ptcl_module.h>

ptcl_packet_sg::ptcl_packet_sg(void)
{
	reset();
}

ptcl_packet_sg::~ptcl_packet_sg(void)
{
}

void ptcl_packet_sg::reset()
{
	frame_header_ = (frame_header_struct *)buf_packet_;
	len_header_ = 14;
	buf_data_area_ = buf_packet_ + len_header_;
	len_packet_ = 0;
	len_data_area_ = 0;
}

void ptcl_packet_sg::format_packet()
{
	len_packet_ = 0;

	frame_header_->frame_start = 0x68;
	frame_header_->data_area_start = 0x68;

//#ifdef PTCL_YNDY_PROTOCOL
//	frame_header_->ptcl_flag = 2;
//#else 
//	frame_header_->ptcl_flag = 1;
//#endif

	frame_header_->len_data_area = len_data_area_ + 8;
	frame_header_->len_area_ex = frame_header_->len_area;

	len_packet_ = len_header_ + len_data_area_;

	buf_packet_[len_packet_++] = get_check_sum(buf_packet_ + 6,frame_header_->len_data_area);
	buf_packet_[len_packet_++] = 0x16;
}

int ptcl_packet_sg::parse_packet()
{
	int result = parse_header();
	if (result <= 0)
		return result;

	unsigned char crc = get_check_sum(buf_packet_ + 6,frame_header_->len_data_area);
	if (crc != buf_packet_[len_header_ + len_data_area_])
		return -1;

	if (buf_packet_[len_packet_-1] != 0x16)
		return -1;

	return len_packet_;
}

int ptcl_packet_sg::parse_header()
{
	if (frame_header_->frame_start != 0x68 || frame_header_->data_area_start != 0x68)
		return -1;

	if (frame_header_->len_area != frame_header_->len_area_ex)
		return -1;
	
	len_data_area_ = frame_header_->len_data_area - 8;
	if (len_data_area_ < 0 || len_data_area_ > (MAX_PACKET_LENGTH - len_header_ - 2))
		return -1;

	len_packet_ = len_header_ + len_data_area_ + 2;

	return len_header_;
}

int ptcl_packet_sg::parse_header(unsigned char *buffer,int len_buffer)
{
	if (len_buffer < len_header_)
		return 0;

	frame_header_ = (frame_header_struct *)buffer;
	return parse_header();
}

unsigned int ptcl_packet_sg::get_device_addr()
{
	unsigned int addr = 0;
	memcpy(&addr,frame_header_->device_addr,4);
	return addr;
}

unsigned char ptcl_packet_sg::get_host_id()
{
	return frame_header_->host_id;
}

int ptcl_packet_sg::get_frame_seq()
{
	return frame_header_->frame_seq;
}

void ptcl_packet_sg::set_device_addr(unsigned int addr)
{
	memcpy(frame_header_->device_addr,&addr,4);
}

unsigned char ptcl_packet_sg::get_host_id_area()
{
	return frame_header_->host_id_area;
}

void ptcl_packet_sg::set_host_id_area(unsigned char area)
{
	frame_header_->host_id_area = area;
}

void ptcl_packet_sg::set_host_id(unsigned char id)
{
	frame_header_->host_id = id;
}

void ptcl_packet_sg::set_frame_seq(unsigned char seq)
{
	frame_header_->frame_seq = seq;
}

unsigned char ptcl_packet_sg::get_afn_ctrl()
{
	return frame_header_->afn_ctrl;
}

void ptcl_packet_sg::set_afn_ctrl(unsigned char afn)
{
	frame_header_->afn_ctrl = afn;
}

unsigned char ptcl_packet_sg::get_fcv()
{
	return frame_header_->fcv;
}

void ptcl_packet_sg::set_fcv(unsigned char fcv)
{
	frame_header_->fcv = fcv;
}

unsigned char ptcl_packet_sg::get_fcv_acd()
{
	return frame_header_->fcb_acd;
}

void ptcl_packet_sg::set_fcv_acd(unsigned char fcb_acd)
{
	frame_header_->fcb_acd = fcb_acd;
}

unsigned char ptcl_packet_sg::get_frm()
{
	return frame_header_->frm;
}

void ptcl_packet_sg::set_frm(unsigned char frm)
{
	frame_header_->frm = frm;
}


unsigned char ptcl_packet_sg::get_dir()
{
	return frame_header_->dir;
}

void ptcl_packet_sg::set_dir(unsigned char dir)
{
	frame_header_->dir = dir;
}

unsigned char ptcl_packet_sg::get_ctrl_code_area()
{
	return frame_header_->ctrl_code_area;
}

void ptcl_packet_sg::set_ctrl_code_area(unsigned char area)
{
	frame_header_->ctrl_code_area = area;
}

unsigned char ptcl_packet_sg::get_group_addr()
{
	return frame_header_->group_addr;
}

void ptcl_packet_sg::set_group_addr(unsigned char group_addr)
{
	frame_header_->group_addr = group_addr;
}

unsigned char ptcl_packet_sg::get_afn()
{
	return frame_header_->afn;
}

void ptcl_packet_sg::set_afn(unsigned char afn)
{
	frame_header_->afn = afn;
}


unsigned char ptcl_packet_sg::get_con()
{
	return frame_header_->con;
}

void ptcl_packet_sg::set_con(unsigned char con)
{
	frame_header_->con = con;
}

unsigned char ptcl_packet_sg::get_fin()
{
	return frame_header_->fin;
}

void ptcl_packet_sg::set_fin(unsigned char fin)
{
	frame_header_->fin = fin;
}


unsigned char ptcl_packet_sg::get_fri()
{
	return frame_header_->fri;
}

void ptcl_packet_sg::set_fri(unsigned char fri)
{
	frame_header_->fri = fri;
}


unsigned char ptcl_packet_sg::get_tpv()
{
	return frame_header_->tpv;
}

void ptcl_packet_sg::set_tpv(unsigned char tpv)
{
	frame_header_->tpv = tpv;
}


unsigned char ptcl_packet_sg::get_seq_area()
{
	return frame_header_->seq_area;
}

void ptcl_packet_sg::set_seq_area(unsigned char area)
{
	frame_header_->seq_area = area;
}

void ptcl_packet_sg::set_ptcl_flag(unsigned char flag)
{
	frame_header_->ptcl_flag = flag;
}

unsigned char ptcl_packet_sg::get_ptcl_flag()
{
	return frame_header_->ptcl_flag;
}

//int ptcl_packet_sg::format_pnfn(pnfn_data *pnfn,unsigned char *buffer,bool is_sg04)
//{
//	if (pnfn == NULL || buffer == NULL)
//		return 0;
//
//	pnfn_struct *pnfn_s = (pnfn_struct *)buffer;
//
//	if (is_sg04 == false)
//	{
//		int pn = pnfn->pn;
//		if (pn == 0)
//		{
//			pnfn_s->pn[0] = 0;
//			pnfn_s->pn[1] = 0;
//		}
//		else
//		{
//			pnfn_s->pn[0] = 1 << ((pn - 1) % 8);
//			pnfn_s->pn[1] = ((pn - 1) / 8) + 1;
//		}
//
//		int fn = pnfn->fn;
//		if (fn == 0)
//		{
//			pnfn_s->fn[0] = 0;
//			pnfn_s->fn[1] = 0;
//		}
//		else
//		{
//			pnfn_s->fn[0] = 1 << ((fn - 1) % 8);
//			pnfn_s->fn[1] = (fn - 1) / 8;
//		}
//	}
//	else
//	{
//		int pn = pnfn->pn;
//		if (pn == 0)
//		{
//			pnfn_s->pn[0] = 0;
//			pnfn_s->pn[1] = 0;
//		}
//		else
//		{
//			pnfn_s->pn[0] = 1 << ((pn - 1) % 8);
//			pnfn_s->pn[1] = 1 << ((pn - 1) / 8);
//		}
//
//		int fn = pnfn->fn;
//		if (fn == 0)
//		{
//			pnfn_s->fn[0] = 0;
//			pnfn_s->fn[1] = 0;
//		}
//		else
//		{
//			pnfn_s->fn[0] = 1 << ((fn - 1) % 8);
//			pnfn_s->fn[1] = (fn - 1) / 8;
//		}
//	}
//	
//	return 4;
//}
//
//int ptcl_packet_sg::parse_pnfn(unsigned char *buffer,pnfn_data *pnfn,bool is_sg04)
//{
//	if (pnfn == NULL || buffer == NULL)
//		return 0;
//
//	pnfn_struct *pnfn_s = (pnfn_struct *)buffer;
//
//	if (is_sg04 == false)
//	{
//		if (pnfn_s->pn[0] == 0 || pnfn_s->pn[1] == 0)
//		{
//			pnfn->pn = 0;
//		}
//		else
//		{
//			pnfn->pn = 8 * (pnfn_s->pn[1] - 1);
//			for (int n=0;n<8;n++)
//			{
//				if (((pnfn_s->pn[0]>>n) & 0x01) == 0x01)
//				{
//					pnfn->pn = pnfn->pn + n+1;
//				}
//			}
//		}
//
//		if (pnfn_s->fn[0] == 0)
//		{
//			pnfn->fn = 0;
//		}
//		else
//		{
//			pnfn->fn = 8 * pnfn_s->fn[1];
//			for (int n=0;n<8;n++)
//			{
//				if (((pnfn_s->fn[0]>>n) & 0x01) == 0x01)
//				{
//					pnfn->fn = pnfn->fn + n+1;
//				}
//			}
//		}
//	}
//	else
//	{
//		if (pnfn_s->pn[0] == 0 || pnfn_s->pn[1] == 0)
//		{
//			pnfn->pn = 0;
//		}
//		else
//		{
//			for(int n=0;n<8;n++)
//			{
//				if( ((pnfn_s->pn[1]>>n) & 0x01) == 1)
//				{
//					pnfn->pn = 8 * n;
//					break;
//				}
//			}
//
//			for (int n=0;n<8;n++)
//			{
//				if (((pnfn_s->pn[0]>>n) & 0x01) == 0x01)
//				{
//					pnfn->pn = pnfn->pn + n+1;
//				}
//			}
//		}
//
//		if (pnfn_s->fn[0] == 0)
//		{
//			pnfn->fn = 0;
//		}
//		else
//		{
//			pnfn->fn = 8 * pnfn_s->fn[1];
//			for (int n=0;n<8;n++)
//			{
//				if (((pnfn_s->fn[0]>>n) & 0x01) == 0x01)
//				{
//					pnfn->fn = pnfn->fn + n+1;
//				}
//			}
//		}
//	}
//
//	return 4;
//}
//
//int ptcl_packet_sg::format_ec(ec_data *ec,unsigned char *buffer)
//{
//	if (ec == NULL || buffer == NULL)
//		return 0;
//
//	buffer[0] = ec->important_ec;
//	buffer[1] = ec->generic_ec;
//
//	return 2;
//}
//
//int ptcl_packet_sg::parse_ec(unsigned char *buffer,ec_data *ec)
//{
//	if (ec == NULL || buffer == NULL)
//		return 0;
//
//	ec->important_ec = buffer[0];
//	ec->generic_ec = buffer[1];
//	
//	return 2;
//}
//
//int ptcl_packet_sg::format_tp(tp_data *tp,unsigned char *buffer)
//{
//	if (tp == NULL || buffer == NULL)
//		return 0;
//
//	buffer[0] = tp->pfc;
//	buffer[1] = ptcl_module::bin_to_bcd(tp->sec);
//	buffer[2] = ptcl_module::bin_to_bcd(tp->min);
//	buffer[3] = ptcl_module::bin_to_bcd(tp->hour);
//	buffer[4] = ptcl_module::bin_to_bcd(tp->day);
//	buffer[5] = tp->time_out;
//
//	return 6;
//}
//
//int ptcl_packet_sg::parse_tp(unsigned char *buffer,tp_data *tp)
//{
//	if (tp == NULL || buffer == NULL)
//		return 0;
//
//	tp->pfc = buffer[0];
//	tp->sec = ptcl_module::bcd_to_bin(buffer[1]);
//	tp->min = ptcl_module::bcd_to_bin(buffer[2]);
//	tp->hour = ptcl_module::bcd_to_bin(buffer[3]);
//	tp->day = ptcl_module::bcd_to_bin(buffer[4]);
//	tp->time_out = buffer[5];
//
//	return 6;
//}
//
//int ptcl_packet_sg::get_aux_len()
//{
//	int len_aux = 0;
//	//计算附加信息长度
//	if (frame_header_->dir == 1 && frame_header_->fcb_acd == 1) //上行报文 而且 要求访问
//	{
//		len_aux = len_aux + 2;//加事件标志位
//	}
//	if (frame_header_->dir == 0 && 
//		(frame_header_->afn == 0x01 || frame_header_->afn == 0x04 || 
//		 frame_header_->afn == 0x05 || frame_header_->afn == 0x06 || 
//		 frame_header_->afn == 0x0F || frame_header_->afn == 0x10)) //下行报文 而且 要求密码
//	{
//		len_aux = len_aux + 2;//密码权限
//	}
//	if (frame_header_->tpv)//时间标识
//	{
//		len_aux = len_aux + 6;
//	}	
//	return len_aux;
//}
