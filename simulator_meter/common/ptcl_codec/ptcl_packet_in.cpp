#include <ptcl_codec/ptcl_packet_in.h>
#include <algorithm>

ptcl_packet_in::ptcl_packet_in(void)
{
	reset();
}

ptcl_packet_in::~ptcl_packet_in(void)
{
}

void ptcl_packet_in::reset()
{
	frame_header_ = (frame_header_struct *)buf_packet_;
	len_header_ = 24;
	buf_data_area_ = buf_packet_ + len_header_;
	len_packet_ = 0;
	len_data_area_ = 0;
}

void ptcl_packet_in::format_packet()
{
	len_packet_ = 0;

	frame_header_->frame_start = 0x68;
	frame_header_->data_length = ntohs((unsigned short int)len_data_area_);

	len_packet_ = len_header_ + len_data_area_;

	frame_header_->frame_end = 0x16;
}

int ptcl_packet_in::parse_packet()
{
	int result = parse_header();
	if (result <= 0)
		return result;

	return len_packet_;
}

int ptcl_packet_in::parse_header()
{
	if (frame_header_->frame_start != 0x68 || frame_header_->frame_end != 0x16)
		return -1;

	len_data_area_ = (int)ntohs(frame_header_->data_length);
	if (len_data_area_ > (MAX_PACKET_LENGTH - len_header_))
		return -1;

	len_packet_ = len_header_ + len_data_area_;

	return len_header_;
}

int ptcl_packet_in::parse_header(unsigned char *buffer,int len_buffer)
{
	if (len_buffer < len_header_)
		return 0;

	frame_header_ = (frame_header_struct *)buffer;
	return parse_header();
}

unsigned int ptcl_packet_in::get_device_addr()
{
	return 0;
}

unsigned char ptcl_packet_in::get_host_id()
{
	return 0;
}

int ptcl_packet_in::get_frame_seq()
{
	return frame_header_->frame_seq;
}

unsigned int ptcl_packet_in::get_sender_id()
{
	return ntohl(frame_header_->sender_id);
}

void ptcl_packet_in::set_sender_id(unsigned int id)
{
	frame_header_->sender_id = ntohl(id);
}


unsigned char ptcl_packet_in::get_sender_type()
{
	return frame_header_->sender_type;
}

void ptcl_packet_in::set_sender_type(unsigned char type)
{
	frame_header_->sender_type = type;
}

unsigned char ptcl_packet_in::get_protocol_type()
{
	return frame_header_->protocol_type;
}

void ptcl_packet_in::set_protocol_type(unsigned char type)
{
	frame_header_->protocol_type = type;
}

void ptcl_packet_in::set_frame_seq(unsigned char seq)
{
	frame_header_->frame_seq = seq;
}

unsigned char ptcl_packet_in::get_event_type()
{
	return frame_header_->event_type;
}

void ptcl_packet_in::set_event_type(unsigned char type)
{
	frame_header_->event_type = type;
}

unsigned short int ptcl_packet_in::get_event_sub_type()
{
	return ntohs(frame_header_->event_sub_type);
}

void ptcl_packet_in::set_event_sub_type(unsigned short int type)
{
	frame_header_->event_sub_type = ntohs(type);
}

unsigned int ptcl_packet_in::get_transfer_id()
{
	return ntohl(frame_header_->transfer_id);
}

void ptcl_packet_in::set_transfer_id(unsigned int id)
{
	frame_header_->transfer_id = ntohl(id);
}

unsigned char ptcl_packet_in::get_transfer_type()
{
	return frame_header_->transfer_type;
}

void ptcl_packet_in::set_transfer_type(unsigned char type)
{
	frame_header_->transfer_type = type;
}

unsigned int ptcl_packet_in::get_receiver_id()
{
	return ntohl(frame_header_->receiver_id);
}

void ptcl_packet_in::set_receiver_id(unsigned int id)
{
	frame_header_->receiver_id = ntohl(id);
}

unsigned char ptcl_packet_in::get_receiver_type()
{
	return frame_header_->receiver_type;
}

void ptcl_packet_in::set_receiver_type(unsigned char type)
{
	frame_header_->receiver_type = type;
}

unsigned short int ptcl_packet_in::ntohs(unsigned short int src)
{
	unsigned short int dest = src;
	char *p = (char *)&dest;
	std::reverse(p,p+2);
	return dest;	
}

unsigned int ptcl_packet_in::ntohl(unsigned int src)
{
	unsigned int dest = src;
	char *p = (char *)&dest;
	std::reverse(p,p+4);
	return dest;	
}