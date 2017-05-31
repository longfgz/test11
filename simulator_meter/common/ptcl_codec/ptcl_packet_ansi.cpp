#pragma once

#include <ptcl_codec/ptcl_packet_ansi.h>
#include <ptcl_codec/ansi/ptcl.hpp>
#include <algorithm>

using namespace ptcl_utility::ansi_c12::dot18;

ptcl_packet_ansi::ptcl_packet_ansi(void)
{
	reset();
}

ptcl_packet_ansi::~ptcl_packet_ansi(void)
{
}

byte_t* ptcl_packet_ansi::GetDataPtr(void)
{
	return & frame_header_->afn;
}

void ptcl_packet_ansi::reset()
{
	len_header_ = 7;
	buf_data_area_ = buf_packet_ + len_header_;
	len_packet_ = 0;
	len_data_area_ = 0;
	frame_header_ = (frame_header_struct *)buf_packet_;
}

uint16_t ntohs(uint16_t param0)
{
	uint16_t param = param0;
	uint8_t * p = (uint8_t *) &param;
	uint8_t tmp = p[0];
	p[0] = p[1];
	p[1] = tmp;
	return param;
}


void ptcl_packet_ansi::format_packet()
{
	len_packet_ = 0;

	frame_header_->frame_start = 0xEE;
	frame_header_->frame_fill = 0x00;

	frame_header_->len_data_area = ntohs(len_data_area_ +1);

	len_packet_ = len_header_ + len_data_area_;

	unsigned short int crc = get_check_sum_ansi(buf_packet_ , len_header_+len_data_area_);
	unsigned char *p = (unsigned char *)&crc;

	buf_packet_[len_packet_++] = *p;
	buf_packet_[len_packet_++] = *(p+1);
}


int ptcl_packet_ansi::get_frame_seq()
{
	return frame_header_->frame_seq;
}

int ptcl_packet_ansi::parse_header()
{
	if (frame_header_->frame_start != 0xEE || frame_header_->frame_fill != 0x00)
		return -1;
	
	len_data_area_ = ntohs(frame_header_->len_data_area)-1; //Dxm modfiy,因为AFN也被当成头的一部分
	
	if (len_data_area_ < 0 || len_data_area_ > (MAX_PACKET_LENGTH - len_header_ - 2))
		return -1;

	len_packet_ = len_header_ + len_data_area_ + 2;
	

	return len_header_;
}

/*
解析buffer所在的包的报文
*/
int ptcl_packet_ansi::parse_header(unsigned char *buffer,int len_buffer)
{
	if (len_buffer < len_header_)
		return 0;
	reset(); 
	frame_header_ = (frame_header_struct *)buffer;
	return parse_header();
}

/* 苏合版的 parse_header*/
int ptcl_packet_ansi::parse_header_suhe(unsigned char *buffer,int len_buffer)
{
	if (len_buffer <= 0) { return 0; }

	len_packet_ = len_buffer;
	len_header_ = 0;
	len_data_area_ = len_packet_;
	buf_data_area_ = buf_packet_ + len_header_;
	memcpy(buf_packet_, buffer, len_buffer);
	return 1;
}

int ptcl_packet_ansi::parse_packet_suhe()
{
byte_t *p = buf_packet_;
  assert(0 == *p);
  ++p;
  uint16_t count = *( reinterpret_cast<uint16_t*>(p) );
  count = ptcl_utility::change_endian(count);
  if (len_packet_ < 1 + 2 + count + 1) { return -1; }
  return 1;
}
int ptcl_packet_ansi::parse_packet()
{
	int result = parse_header();
	if (result <= 0)
		return result;

	// 校验是包括头的，Dengxm
	//unsigned short int crc = get_check_sum_ansi(buf_packet_ + len_header_,frame_header_->len_data_area);
	unsigned short int crc = get_check_sum_ansi(buf_packet_ , len_header_+ len_data_area_);

	unsigned short int crc1 = 0;
	memcpy(&crc1,buf_packet_+ len_header_ + len_data_area_, 2);
	if (crc != crc1)
		return -1;

	return len_packet_;
}

unsigned char ptcl_packet_ansi::get_ctrl_code_area()
{
	return frame_header_->ctrl_code_area;
}

void ptcl_packet_ansi::set_ctrl_code_area(unsigned char ctrl)
{
	frame_header_->ctrl_code_area = ctrl;
}

unsigned char ptcl_packet_ansi::get_repeat_flag()
{
	return frame_header_->repeat_flag;
}

void ptcl_packet_ansi::set_repeat_flag(unsigned char flag)
{
	frame_header_->repeat_flag = flag;
}

unsigned char ptcl_packet_ansi::get_mult_first()
{
	return frame_header_->mult_first;
}

void ptcl_packet_ansi::set_mult_first(unsigned char first)
{
	frame_header_->mult_first = first;
}

unsigned char ptcl_packet_ansi::get_mult_flag()
{
	return frame_header_->mult_flag;
}

void ptcl_packet_ansi::set_mult_flag(unsigned char flag)
{
	frame_header_->mult_flag = flag;
}

unsigned char ptcl_packet_ansi::get_afn()
{
	return frame_header_->afn;
}

void ptcl_packet_ansi::set_afn(unsigned char afn)
{
	frame_header_->afn = afn;
}

void ptcl_packet_ansi::set_frame_seq(int seq)
{
	frame_header_->frame_seq = seq;
}



unsigned short int ptcl_packet_ansi::get_check_sum_ansi(unsigned char *buffer,int len)
{
        Crc_AT crc_func;
        crc_func.reset();
        crc_func = std::for_each(buffer, buffer+len , crc_func);
        unsigned short int crc = crc_func();
 
        return crc;
}
