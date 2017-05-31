#include <ptcl_codec/ptcl_packet_hdlc.h>
#include <algorithm>

unsigned short crc_tab_[256] = {0x0000,  0x1189,  0x2312,  0x329b,  0x4624,  0x57ad,  0x6536,  0x74bf,
								0x8c48,  0x9dc1,  0xaf5a,  0xbed3,  0xca6c,  0xdbe5,  0xe97e,  0xf8f7,
								0x1081,  0x0108,  0x3393,  0x221a,  0x56a5,  0x472c,  0x75b7,  0x643e,
								0x9cc9,  0x8d40,  0xbfdb,  0xae52,  0xdaed,  0xcb64,  0xf9ff,  0xe876,
								0x2102,  0x308b,  0x0210,  0x1399,  0x6726,  0x76af,  0x4434,  0x55bd,
								0xad4a,  0xbcc3,  0x8e58,  0x9fd1,  0xeb6e,  0xfae7,  0xc87c,  0xd9f5,
								0x3183,  0x200a,  0x1291,  0x0318,  0x77a7,  0x662e,  0x54b5,  0x453c,
								0xbdcb,  0xac42,  0x9ed9,  0x8f50,  0xfbef,  0xea66,  0xd8fd,  0xc974,
								0x4204,  0x538d,  0x6116,  0x709f,  0x0420,  0x15a9,  0x2732,  0x36bb,
								0xce4c,  0xdfc5,  0xed5e,  0xfcd7,  0x8868,  0x99e1,  0xab7a,  0xbaf3,
								0x5285,  0x430c,  0x7197,  0x601e,  0x14a1,  0x0528,  0x37b3,  0x263a,
								0xdecd,  0xcf44,  0xfddf,  0xec56,  0x98e9,  0x8960,  0xbbfb,  0xaa72,
								0x6306,  0x728f,  0x4014,  0x519d,  0x2522,  0x34ab,  0x0630,  0x17b9,
								0xef4e,  0xfec7,  0xcc5c,  0xddd5,  0xa96a,  0xb8e3,  0x8a78,  0x9bf1,
								0x7387,  0x620e,  0x5095,  0x411c,  0x35a3,  0x242a,  0x16b1,  0x0738,
								0xffcf,  0xee46,  0xdcdd,  0xcd54,  0xb9eb,  0xa862,  0x9af9,  0x8b70,
								0x8408,  0x9581,  0xa71a,  0xb693,  0xc22c,  0xd3a5,  0xe13e,  0xf0b7,
								0x0840,  0x19c9,  0x2b52,  0x3adb,  0x4e64,  0x5fed,  0x6d76,  0x7cff,
								0x9489,  0x8500,  0xb79b,  0xa612,  0xd2ad,  0xc324,  0xf1bf,  0xe036,
								0x18c1,  0x0948,  0x3bd3,  0x2a5a,  0x5ee5,  0x4f6c,  0x7df7,  0x6c7e,
								0xa50a,  0xb483,  0x8618,  0x9791,  0xe32e,  0xf2a7,  0xc03c,  0xd1b5,
								0x2942,  0x38cb,  0x0a50,  0x1bd9,  0x6f66,  0x7eef,  0x4c74,  0x5dfd,
								0xb58b,  0xa402,  0x9699,  0x8710,  0xf3af,  0xe226,  0xd0bd,  0xc134,
								0x39c3,  0x284a,  0x1ad1,  0x0b58,  0x7fe7,  0x6e6e,  0x5cf5,  0x4d7c,
								0xc60c,  0xd785,  0xe51e,  0xf497,  0x8028,  0x91a1,  0xa33a,  0xb2b3,
								0x4a44,  0x5bcd,  0x6956,  0x78df,  0x0c60,  0x1de9,  0x2f72,  0x3efb,
								0xd68d,  0xc704,  0xf59f,  0xe416,  0x90a9,  0x8120,  0xb3bb,  0xa232,
								0x5ac5,  0x4b4c,  0x79d7,  0x685e,  0x1ce1,  0x0d68,  0x3ff3,  0x2e7a,
								0xe70e,  0xf687,  0xc41c,  0xd595,  0xa12a,  0xb0a3,  0x8238,  0x93b1,
								0x6b46,  0x7acf,  0x4854,  0x59dd,  0x2d62,  0x3ceb,  0x0e70,  0x1ff9,
								0xf78f,  0xe606,  0xd49d,  0xc514,  0xb1ab,  0xa022,  0x92b9,  0x8330,
								0x7bc7,  0x6a4e,  0x58d5,  0x495c,  0x3de3,  0x2c6a,  0x1ef1,  0x0f78};


ptcl_packet_hdlc::ptcl_packet_hdlc(int packet_type)
{
	is_send_packet_ = packet_type;
	reset();
}

ptcl_packet_hdlc::~ptcl_packet_hdlc()
{

}

void ptcl_packet_hdlc::reset()
{
	frame_header_ = (frame_header_struct *)buf_packet_;
	len_header_ = 11;
	buf_data_area_ = buf_packet_ + len_header_;
	len_packet_ = 0;
	len_data_area_ = 0;
}


void ptcl_packet_hdlc::format_packet()
{
	if (len_data_area_ > 0)
	{
		len_packet_ = 0;

		frame_header_->frame_start = 0x7e;

		int length = (len_data_area_+10+2) | (0<<11) | (0x0a<<12);
		frame_header_->frame_type_area = length;
		std::reverse(buf_packet_+1,buf_packet_+3);

		frame_header_->hcs = get_crc(buf_packet_+1,8);
		
		len_packet_ = len_header_ + len_data_area_;

		unsigned short int crc = get_crc(buf_packet_+1,len_data_area_+10);
		memcpy(buf_packet_+len_data_area_+11,&crc,2);
		len_packet_ = len_packet_ + 2;

		buf_packet_[len_packet_++] = 0x7e;
	}
	else
	{
		len_packet_ = 0;

		frame_header_->frame_start = 0x7e;

		int length = (len_data_area_+10) | (0<<11) | (0x0a<<12);
		frame_header_->frame_type_area = length;
		std::reverse(buf_packet_+1,buf_packet_+3);

		frame_header_->hcs = get_crc(buf_packet_+1,8);
		
		len_packet_ = len_header_ + len_data_area_;

		buf_packet_[len_packet_++] = 0x7e;
	}
}

int ptcl_packet_hdlc::parse_packet()
{
	int result = parse_header();
	if (result <= 0)
		return result;

	if (len_data_area_ > 0)
	{
		unsigned short int crc = get_crc(buf_packet_+1,len_data_area_+10);
		unsigned short int crc1 = 0;
		memcpy(&crc1,buf_packet_+len_data_area_+11,2);
		if (crc != crc1)
			return -1;
	}
	
	if (buf_packet_[len_packet_-1] != 0x7e)
		return -1;

	return len_packet_;
}

int ptcl_packet_hdlc::parse_header()
{
	if (frame_header_->frame_start != 0x7e)
		return -1;

	unsigned short int frame_type_area = frame_header_->frame_type_area;
	unsigned char *p = (unsigned char *)&frame_type_area;
	std::reverse(p,p+2);
	int length = frame_type_area & 0x03ff;
	if (length == 10)
		len_data_area_ = 0;
	else if (length > 10)
		len_data_area_ = length - 10 - 2;
	else
		return -1;

	if (frame_header_->hcs != get_crc((unsigned char *)frame_header_+1,8))
		return -1;

	if (len_data_area_ == 0)
		len_packet_ = len_header_ + len_data_area_ + 1;
	else
		len_packet_ = len_header_ + len_data_area_ + 3;

	return len_header_;
}

int ptcl_packet_hdlc::parse_header(unsigned char *buffer,int len_buffer)
{
	if (len_buffer < len_header_)
		return 0;

	frame_header_ = (frame_header_struct *)buffer;
	return parse_header();
}

unsigned int ptcl_packet_hdlc::get_device_addr()
{
	unsigned int dev_addr = 0;
	if (is_send_packet_ == 1)
	{
		dev_addr = (frame_header_->device_addr_dst[2]>>1)*128+(frame_header_->device_addr_dst[3]>>1);
	}
	else
	{
		dev_addr = (frame_header_->device_addr_src[2]>>1)*128+(frame_header_->device_addr_src[3]>>1);
	}
	return dev_addr;
}

int ptcl_packet_hdlc::get_frame_seq()
{
	return 0;
}

unsigned char ptcl_packet_hdlc::get_host_id()
{
	if (is_send_packet_ == 1)
	{
		return frame_header_->host_id_src >> 1;
	}
	else
	{
		return frame_header_->host_id_dst >> 1;
	}
}

void ptcl_packet_hdlc::set_device_addr(unsigned short dev_addr)
{
	unsigned char high_addr = dev_addr/128;
	unsigned char low_addr = dev_addr%128;
	unsigned int addr = (1<<17)+(high_addr<<9)+(low_addr<<1) + 1;
	if (is_send_packet_ == 1)
	{
		memcpy(frame_header_->device_addr_dst,&addr,4);
		std::reverse(frame_header_->device_addr_dst,frame_header_->device_addr_dst+4);
	}
	else
	{
		memcpy(frame_header_->device_addr_src,&addr,4);
		std::reverse(frame_header_->device_addr_src,frame_header_->device_addr_src+4);
	}
}


void ptcl_packet_hdlc::set_host_id(unsigned char id)
{
	if (is_send_packet_ == 1)
	{
		frame_header_->host_id_src = 0x01 | ((id << 1) & 0xfe);
	}
	else
	{
		frame_header_->host_id_dst = 0x01 | ((id << 1) & 0xfe);
	}
}

void ptcl_packet_hdlc::set_frame_seq(unsigned char seq)
{

}

unsigned char ptcl_packet_hdlc::get_ctrl_code_area()
{
	return frame_header_->ctrl_code_area;
}

void ptcl_packet_hdlc::set_ctrl_code_area(unsigned char code)
{
	frame_header_->ctrl_code_area = code;
}


unsigned char ptcl_packet_hdlc::get_cca_recv_seq()
{
	return frame_header_->cca_recv_seq;
}

void ptcl_packet_hdlc::set_cca_recv_seq(unsigned char seq)
{
	frame_header_->cca_recv_seq = seq;
}

unsigned char ptcl_packet_hdlc::get_cca_pf()
{
	return frame_header_->cca_pf;
}

void ptcl_packet_hdlc::set_cca_pf(unsigned char pf)
{
	frame_header_->cca_pf = pf;
}

unsigned char ptcl_packet_hdlc::get_cca_send_seq()
{
	return frame_header_->cca_send_seq;
}

void ptcl_packet_hdlc::set_cca_send_seq(unsigned char seq)
{
	frame_header_->cca_send_seq = seq;
}

unsigned char ptcl_packet_hdlc::get_cca_end()
{
	return frame_header_->cca_end;
}

void ptcl_packet_hdlc::set_cca_end(unsigned char end)
{
	frame_header_->cca_end = end;
}
	
unsigned short int ptcl_packet_hdlc::get_crc(unsigned char* buffer,int length)
{
	unsigned short int crc = 0xffff;
	for (int n=0;n<length;n++)
	{
		crc = (crc >> 8) ^ crc_tab_[(crc^buffer[n]) & 0xff];
	}

	unsigned short int trialfcs = (crc^0xffff);
	//trialfcs = ((trialfcs >> 8) & 0x00ff) + ((trialfcs << 8) & 0xff00);
	return trialfcs;
}

/*
ptcl_packet_hdlc::ptcl_packet_hdlc()
{
	reset();
}

ptcl_packet_hdlc::~ptcl_packet_hdlc()
{

}

void ptcl_packet_hdlc::reset()
{
	frame_header_ = (frame_header_struct *)buf_packet_;
	set_fcs_tab();
	len_header_ = 9;
	buf_data_area_ = buf_packet_ + 9;
	len_packet_ = 0;
	len_data_area_ = 0;
}

void ptcl_packet_hdlc::format_packet()
{
	len_packet_ = 0;

	frame_header_->frame_start = 0x7e;
	frame_header_->frame_type = 0x0a;
	frame_header_->s = 0;
	frame_header_->len = len_data_area_ + 10;
	frame_header_->len_data_area = (frame_header_->len_data_area<<8) + (frame_header_->len_data_area>>8);
	len_packet_ = len_data_area_ + len_header_;

	unsigned short crc = get_check_sum(buf_packet_ + 1,frame_header_->len_data_area);
	memcpy(&buf_packet_[len_packet_++],&crc,2);
	buf_packet_[len_packet_+2] = 0x7e;
}

int ptcl_packet_hdlc::parse_packet()
{
	int result = parse_header();
	if (result <= 0)
		return result;

	unsigned short temp;
	memcpy(&temp,&buf_packet_[len_header_+len_data_area_],2);
	unsigned short crc = get_check_sum(buf_packet_+1,frame_header_->len_data_area);
	if (crc != temp)
		return -1;
	
	if (buf_packet_[len_packet_-1] != 0x7e)
		return -1;

	return len_packet_;
}

int ptcl_packet_hdlc::parse_header()
{
	len_header_ = 9;

	if (frame_header_->frame_start == 0xfa)
		return 9;

	if (frame_header_->frame_start != 0x7e)
		return -1;
	len_data_area_ = ((frame_header_->len_data_area>>8) & 0xfff) + (frame_header_->len_data_area << 8) - 10;
	if (len_data_area_ > (MAX_PACKET_LENGTH - len_header_ - 3))
		return -1;

	len_packet_ = len_header_ + len_data_area_ + 3;

	return len_header_;
}

int ptcl_packet_hdlc::parse_header(unsigned char *buffer,int len_buffer)
{
	if (len_buffer < len_header_)
		return 0;

	frame_header_ = (frame_header_struct *)buffer;
	return parse_header();
}

unsigned int ptcl_packet_hdlc::get_device_addr()
{
	unsigned int temp = 0;
	memcpy(&temp,frame_header_->device_addr,4);
	return (temp >> 17);
}

int ptcl_packet_hdlc::get_frame_seq()
{
	return frame_header_->sss;
}

void ptcl_packet_hdlc::set_device_addr(unsigned int addr)
{
	addr = (1<<17) + (addr<<1) + 1;
	memcpy(frame_header_->device_addr,&addr,4);
	std::reverse(frame_header_->device_addr,frame_header_->device_addr+4);
}

unsigned char ptcl_packet_hdlc::get_ctrl_code()
{
	return frame_header_->ctrl_code_area;
}

void ptcl_packet_hdlc::set_ctrl_code(unsigned char code)
{
	frame_header_->ctrl_code_area = code;
}

unsigned char ptcl_packet_hdlc::get_header_start()
{
	return frame_header_->frame_start;
}

unsigned char ptcl_packet_hdlc::get_host_id()
{
	return (frame_header_->host_id >> 1);
}

void ptcl_packet_hdlc::set_host_id(unsigned char host_id)
{
	frame_header_->host_id = (host_id << 1)+1;
}

unsigned short ptcl_packet_hdlc::get_check_sum(unsigned char *buffer,int len,unsigned short fcs)
{
	unsigned short trialfcs;
	while (len--)
	{
		fcs = (fcs>>8) ^fcs_tab_[(fcs ^ *buffer++) & 0xff];
	}
	trialfcs = (fcs^0xffff);
	trialfcs = ((trialfcs >> 8) & 0x00ff) + ((trialfcs << 8) & 0xff00);

	return trialfcs;
}

unsigned char ptcl_packet_hdlc::get_frame_header()
{
	return frame_header_->frame_start;
}

unsigned char ptcl_packet_hdlc::get_send_seq()
{
	return frame_header_->sss;
}

void ptcl_packet_hdlc::set_send_seq(unsigned char seq)
{
	frame_header_->sss = seq;
}

unsigned char ptcl_packet_hdlc::get_recv_seq()
{
	return frame_header_->rrr;
}

void ptcl_packet_hdlc::set_recv_seq(unsigned char seq)
{
	frame_header_->rrr = seq;
}

void ptcl_packet_hdlc::set_ctrl_flag(unsigned char flag)
{
	frame_header_->flag = flag;
}

void ptcl_packet_hdlc::set_ctrl_pf(unsigned char pf)
{
	frame_header_->p_and_f = pf;
}

void ptcl_packet_hdlc::set_fcs_tab()
{
	unsigned int b, v;
	int i;
	memset(fcs_tab_,0,sizeof(fcs_tab_));
	for (b = 0;b<256;b++) {
		if (b % 8 == 0)
			printf("\n");
		v = b;
		for (i = 8; i--; )
			v = v & 1 ? (v >> 1) ^ 0x8408 : v >> 1;
		fcs_tab_[b] = v & 0xff;
	}
}*/