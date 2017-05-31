#include <ptcl_codec/ptcl_module_sg.h>

ptcl_module_sg *ptcl_module_sg::instance_ = NULL;

ptcl_module_sg::ptcl_module_sg(void)
{
}

ptcl_module_sg::~ptcl_module_sg(void)
{
}

ptcl_module_sg *ptcl_module_sg::get_instance()
{
	if (instance_ == NULL)
		instance_ = new ptcl_module_sg();
	return instance_;
}

void ptcl_module_sg::format_packet_read_data_history(	ptcl_packet_sg *packet,
														pnfn_data_uint* v_data_uint,int count_data_uint,
														tp_data *tp)
{
	if (packet == NULL)
		return ;

	//控制域功能码
	packet->set_ptcl_flag(2);
	packet->set_afn_ctrl(0x0B);
	packet->set_frm(1);
	packet->set_fcv(0);
	packet->set_fcv_acd(0);
	packet->set_dir(0);

	packet->set_group_addr(0);
	//主站地址 序列号域
	packet->set_host_id(host_id_);

	//应用层功能码
	packet->set_afn(AFN_READ_DATA_HISTORY);


	//序列域
	unsigned int addr = packet->get_device_addr();
	unsigned char seq = get_frame_seq(addr);
	packet->set_frame_seq(seq);
	
	if (tp)
		packet->set_tpv(1);
	else
		packet->set_tpv(0);
	packet->set_fri(1);
	packet->set_fin(1);
	packet->set_con(0);

	//用户数据域
	int len_data = 0;
	unsigned char buffer[1024];
	memset(buffer,0,1024);

	//数据单元
	for (int n=0;n<count_data_uint;n++)
	{
		if (len_data + 4 + v_data_uint[n].len_data >= 1024)
			break;

		int len = format_pnfn(&v_data_uint[n].pnfn,buffer + len_data);
		len_data = len_data + len;
		memcpy(buffer + len_data,v_data_uint[n].data_buffer,v_data_uint[n].len_data);
		len_data = len_data + v_data_uint[n].len_data;
	}

	//TP
	if (tp)
	{
		tp->pfc = seq;
		int len = format_tp(tp,buffer + len_data);
		len_data = len_data + len;
	}

	packet->set_data_area_buffer(buffer,len_data);

	packet->format_packet();
}

void ptcl_module_sg::format_packet_confirm_frame(ptcl_packet_sg *packet)
{
	if (packet == NULL)
		return;

	//控制域功能码
	packet->set_ptcl_flag(2);
	packet->set_afn_ctrl(0x00);	
	packet->set_frm(0);
	packet->set_fcv(0);
	packet->set_fcv_acd(0);
	packet->set_dir(0);

	//主站地址 序列号域
	packet->set_host_id(host_id_);

	//应用层功能码
	packet->set_afn(AFN_ACK);

	packet->set_tpv(0);
	packet->set_fri(1);
	packet->set_fin(1);
	packet->set_con(0);

	//用户数据域
	int len_data = 0;
	unsigned char buffer[8];
	memset(buffer,0,8);

	buffer[0] = 0x00;
	buffer[1] = 0x00;
	buffer[2] = 0x01;
	buffer[3] = 0x00;

	len_data = 4;

	packet->set_data_area_buffer(buffer,len_data);

	packet->format_packet();
}

void ptcl_module_sg::format_packet_set_param(ptcl_packet_sg *packet, pnfn_data_uint* v_data_uint,
											 int count_data_uint,unsigned char *psw, 
											 tp_data *tp)
{
	if (packet == NULL)
		return ;

	//控制域功能码
	packet->set_afn_ctrl(0x0A);
	packet->set_frm(1);
	packet->set_fcv(0);
	packet->set_fcv_acd(0);
	packet->set_dir(0);

	packet->set_group_addr(0);
	//主站地址 序列号域
	packet->set_host_id(host_id_);

	//应用层功能码
	packet->set_afn(AFN_COMMAND);


	//序列域
	unsigned int addr = packet->get_device_addr();
	unsigned char seq = get_frame_seq(addr);
	packet->set_frame_seq(seq);

	if (tp)
		packet->set_tpv(1);
	else
		packet->set_tpv(0);
	packet->set_fri(1);
	packet->set_fin(1);
	packet->set_con(1);

	//用户数据域
	int len_data = 0;
	unsigned char buffer[1024];
	memset(buffer,0,1024);

	//数据单元
	for (int n=0;n<count_data_uint;n++)
	{
		if (len_data + 4 + v_data_uint[n].len_data >= 1024)
			break;

		int len = format_pnfn(&v_data_uint[n].pnfn,buffer + len_data);
		len_data = len_data + len;
		memcpy(buffer + len_data,v_data_uint[n].data_buffer,v_data_uint[n].len_data);
		len_data = len_data + v_data_uint[n].len_data;
	}
	//pss
	if (psw)
	{
		unsigned char crc[16] = {0};
		memcpy(buffer+len_data,crc,16);
		len_data = len_data + 16;
	}

	//TP
	if (tp)
	{
		tp->pfc = seq;
		int len = format_tp(tp,buffer + len_data);
		len_data = len_data + len;
	}

	packet->set_data_area_buffer(buffer,len_data);

	packet->format_packet();
}

void ptcl_module_sg::format_packet_write_param(ptcl_packet_sg *packet, pnfn_data_uint* v_data_uint,
											 int count_data_uint,unsigned char *psw, 
											 tp_data *tp)
{
	if (packet == NULL)
		return ;

	//控制域功能码
	packet->set_ptcl_flag(2);
	packet->set_afn_ctrl(0x0A);
	packet->set_frm(1);
	packet->set_fcv(0);
	packet->set_fcv_acd(0);
	packet->set_dir(0);

	packet->set_group_addr(0);
	//主站地址 序列号域
	packet->set_host_id(host_id_);

	//应用层功能码
	packet->set_afn(AFN_SET_PARAM);

	//序列域
	unsigned int addr = packet->get_device_addr();
	unsigned char seq = get_frame_seq(addr);
	packet->set_frame_seq(seq);

	if (tp)
		packet->set_tpv(1);
	else
		packet->set_tpv(0);
	packet->set_fri(1);
	packet->set_fin(1);
	packet->set_con(1);

	//用户数据域
	int len_data = 0;
	unsigned char buffer[1024];
	memset(buffer,0,1024);

	//数据单元
	for (int n=0;n<count_data_uint;n++)
	{
		if (len_data + 4 + v_data_uint[n].len_data >= 1024)
			break;

		int len = format_pnfn(&v_data_uint[n].pnfn,buffer + len_data);
		len_data = len_data + len;
		memcpy(buffer + len_data,v_data_uint[n].data_buffer,v_data_uint[n].len_data);
		len_data = len_data + v_data_uint[n].len_data;
	}
	//pss
	if (psw)
	{
		unsigned char crc[16] = {0};
		memcpy(buffer+len_data,crc,16);
		len_data = len_data + 16;
	}

	//TP
	if (tp)
	{
		tp->pfc = seq;
		int len = format_tp(tp,buffer + len_data);
		len_data = len_data + len;
	}

	packet->set_data_area_buffer(buffer,len_data);

	packet->format_packet();
}

void ptcl_module_sg::format_packet_transmit(ptcl_packet_sg *packet, pnfn_data_uint &data_uint, unsigned char *psw, tp_data *tp)
{
	if (packet == NULL)
		return ;

	//控制域功能码
	packet->set_afn_ctrl(0x0A);
	packet->set_frm(1);
	packet->set_fcv(0);
	packet->set_fcv_acd(0);
	packet->set_dir(0);

	packet->set_group_addr(0);
	//主站地址 序列号域
	packet->set_host_id(host_id_);

	//应用层功能码
	packet->set_afn(AFN_DATA_TRANSMIT);


	//序列域
	unsigned int addr = packet->get_device_addr();
	unsigned char seq = get_frame_seq(addr);
	packet->set_frame_seq(seq);

	if (tp)
		packet->set_tpv(1);
	else
		packet->set_tpv(0);
	packet->set_fri(1);
	packet->set_fin(1);
	packet->set_con(0);

	//用户数据域
	int len_data = 0;
	unsigned char buffer[1024];
	memset(buffer,0,1024);

	//数据单元
	int len = format_pnfn(&data_uint.pnfn, buffer + len_data);
	len_data = len_data + len;
	memcpy(buffer + len_data, data_uint.data_buffer, data_uint.len_data);
	len_data = len_data + data_uint.len_data;

	//pss
	if (psw)
	{
		unsigned char crc[16] = {0};
		memcpy(buffer+len_data,crc,16);
		len_data = len_data + 16;
	}

	//TP
	if (tp)
	{
		tp->pfc = seq;
		int len = format_tp(tp,buffer + len_data);
		len_data = len_data + len;
	}

	packet->set_data_area_buffer(buffer,len_data);

	packet->format_packet();
}

void ptcl_module_sg::format_packet_rand_data(ptcl_packet_sg *packet, pnfn_data_uint* v_data_uint,int count_data_uint,
								  tp_data *tp)
{
	if (packet == NULL)
		return ;

	//控制域功能码
	packet->set_ptcl_flag(2);
	packet->set_afn_ctrl(0x0B);
	packet->set_frm(1);
	packet->set_fcv(0);
	packet->set_fcv_acd(0);
	packet->set_dir(0);

	packet->set_group_addr(0);
	//主站地址 序列号域
	packet->set_host_id(host_id_);

	//应用层功能码
	packet->set_afn(AFN_READ_DATA_REALTIME);


	//序列域
	unsigned int addr = packet->get_device_addr();
	unsigned char seq = get_frame_seq(addr);
	packet->set_frame_seq(seq);

	if (tp)
		packet->set_tpv(1);
	else
		packet->set_tpv(0);
	packet->set_fri(1);
	packet->set_fin(1);
	packet->set_con(0);

	//用户数据域
	int len_data = 0;
	unsigned char buffer[1024];
	memset(buffer,0,1024);

	//数据单元
	for (int n=0;n<count_data_uint;n++)
	{
		if (len_data + 4 + v_data_uint[n].len_data >= 1024)
			break;

		int len = format_pnfn(&v_data_uint[n].pnfn,buffer + len_data);
		len_data = len_data + len;
	}


	//TP
	if (tp)
	{
		tp->pfc = seq;
		int len = format_tp(tp,buffer + len_data);
		len_data = len_data + len;
	}

	packet->set_data_area_buffer(buffer,len_data);

	packet->format_packet();
}

int ptcl_module_sg::format_pnfn(pnfn_data *pnfn,unsigned char *buffer)
{
	if (pnfn == NULL || buffer == NULL)
		return 0;

	pnfn_struct *pnfn_s = (pnfn_struct *)buffer;


	int pn = pnfn->pn;
	if (pn == 0)
	{
		pnfn_s->pn[0] = 0;
		pnfn_s->pn[1] = 0;
	}
	else
	{
		pnfn_s->pn[0] = 1 << ((pn - 1) % 8);
		pnfn_s->pn[1] = ((pn - 1) / 8) + 1;
	}

	int fn = pnfn->fn;
	if (fn == 0)
	{
		pnfn_s->fn[0] = 0;
		pnfn_s->fn[1] = 0;
	}
	else
	{
		pnfn_s->fn[0] = 1 << ((fn - 1) % 8);
		pnfn_s->fn[1] = (fn - 1) / 8;
	}


	return 4;
}

int ptcl_module_sg::parse_pnfn(unsigned char *buffer,pnfn_data *pnfn)
{
	if (pnfn == NULL || buffer == NULL)
		return 0;

	pnfn_struct *pnfn_s = (pnfn_struct *)buffer;

	if (pnfn_s->pn[0] == 0 || pnfn_s->pn[1] == 0)
	{
		pnfn->pn = 0;
	}
	else
	{
		pnfn->pn = 8 * (pnfn_s->pn[1] - 1);
		for (int n=0;n<8;n++)
		{
			if (((pnfn_s->pn[0]>>n) & 0x01) == 0x01)
			{
				pnfn->pn = pnfn->pn + n+1;
			}
		}
	}

	if (pnfn_s->fn[0] == 0)
	{
		pnfn->fn = 0;
	}
	else
	{
		pnfn->fn = 8 * pnfn_s->fn[1];
		for (int n=0;n<8;n++)
		{
			if (((pnfn_s->fn[0]>>n) & 0x01) == 0x01)
			{
				pnfn->fn = pnfn->fn + n+1;
			}
		}
	}
	
	return 4;
}

int ptcl_module_sg::format_ec(ec_data *ec,unsigned char *buffer)
{
	if (ec == NULL || buffer == NULL)
		return 0;

	buffer[0] = ec->important_ec;
	buffer[1] = ec->generic_ec;

	return 2;
}

int ptcl_module_sg::parse_ec(unsigned char *buffer,ec_data *ec)
{
	if (ec == NULL || buffer == NULL)
		return 0;

	ec->important_ec = buffer[0];
	ec->generic_ec = buffer[1];

	return 2;
}

int ptcl_module_sg::format_tp(tp_data *tp,unsigned char *buffer)
{
	if (tp == NULL || buffer == NULL)
		return 0;

	buffer[0] = tp->pfc;
	buffer[1] = ptcl_module::bin_to_bcd(tp->sec);
	buffer[2] = ptcl_module::bin_to_bcd(tp->min);
	buffer[3] = ptcl_module::bin_to_bcd(tp->hour);
	buffer[4] = ptcl_module::bin_to_bcd(tp->day);
	buffer[5] = tp->time_out;

	return 6;
}

int ptcl_module_sg::parse_tp(unsigned char *buffer,tp_data *tp)
{
	if (tp == NULL || buffer == NULL)
		return 0;

	tp->pfc = buffer[0];
	tp->sec = ptcl_module::bcd_to_bin(buffer[1]);
	tp->min = ptcl_module::bcd_to_bin(buffer[2]);
	tp->hour = ptcl_module::bcd_to_bin(buffer[3]);
	tp->day = ptcl_module::bcd_to_bin(buffer[4]);
	tp->time_out = buffer[5];

	return 6;
}

int ptcl_module_sg::get_aux_len()
{
	//int len_aux = 0;
	////计算附加信息长度
	//if (frame_header_->dir == 1 && frame_header_->fcb_acd == 1) //上行报文 而且 要求访问
	//{
	//	len_aux = len_aux + 2;//加事件标志位
	//}
	//if (frame_header_->dir == 0 && 
	//	(frame_header_->afn == 0x01 || frame_header_->afn == 0x04 || 
	//	frame_header_->afn == 0x05 || frame_header_->afn == 0x06 || 
	//	frame_header_->afn == 0x0F || frame_header_->afn == 0x10)) //下行报文 而且 要求密码
	//{
	//	len_aux = len_aux + 2;//密码权限
	//}
	//if (frame_header_->tpv)//时间标识
	//{
	//	len_aux = len_aux + 6;
	//}	
	//return len_aux;
	return 0;
}

void ptcl_module_sg::format_packet_confirm_event_push( ptcl_packet_sg *packet, 
													   unsigned char afn, 
													   unsigned char seq, 
													   pnfn_data_uint &data_uint, 
													   tp_data *tp /* = NULL */ )
{
	if (packet == NULL)
		return;

	//控制域功能码
	packet->set_ptcl_flag(2);
	packet->set_afn_ctrl(0);
	packet->set_frm(0);
	packet->set_fcv(0);
	packet->set_fcv_acd(0);
	packet->set_dir(0);

	//主站地址 序列号域
	packet->set_group_addr(0);
	packet->set_host_id(host_id_);

	//应用层功能码
	packet->set_afn(afn);

	//序列域
	if (tp)
		packet->set_tpv(1);
	else
		packet->set_tpv(0);
	packet->set_fri(1);
	packet->set_fin(1);
	packet->set_con(0);
	packet->set_frame_seq(seq);

	//用户数据域
	int len_data = 0;
	unsigned char buffer[1024];
	memset(buffer,0,1024);

	//数据单元
	memcpy(buffer + len_data, data_uint.data_buffer, data_uint.len_data);
	len_data = len_data + data_uint.len_data;

	//TP
	if (tp)
	{
		int len = format_tp(tp, buffer + len_data);
		len_data = len_data + len;
	}

	packet->set_data_area_buffer(buffer,len_data);

	packet->format_packet();
}