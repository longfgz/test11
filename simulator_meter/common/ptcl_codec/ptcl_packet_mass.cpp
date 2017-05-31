#include <ptcl_codec/ptcl_packet_mass.h>
#include <algorithm>

ptcl_packet_mass::ptcl_packet_mass(bool packet_type)
{
	is_get_packet_ = packet_type;
	reset();
}

ptcl_packet_mass::~ptcl_packet_mass()
{

}

void ptcl_packet_mass::reset()
{
	if (is_get_packet_) 
	{
		get_frame_header_ = (get_frame_header_struct *)buf_packet_;
		get_frame_header_->version = 0x0100;
		get_frame_header_->length = 0x0d00;
		len_header_ = 4;
		buf_data_area_ = buf_packet_ + len_header_;
		len_packet_ = 0;
		len_data_area_ = 0;
	}
	else
	{
		frame_header_ = (frame_header_struct *)buf_packet_;
		len_header_ = sizeof(frame_header_struct);
		buf_data_area_ = buf_packet_ + len_header_;
		len_packet_ = 0;
		len_data_area_ = 0;
		frame_header_->src_port = 0x0100;
		frame_header_->dest_port = 0x0100;
	}
}


void ptcl_packet_mass::format_packet()
{
	if (is_get_packet_) 
	{
		get_frame_header_->version = 0x0100;
		len_packet_ = len_data_area_ + len_header_ - 1;
		get_frame_header_->length = 0x0d00;
	}
	else
	{
		frame_header_->version = 0x0100;
		len_packet_ = len_data_area_+ len_header_;
		frame_header_->length = reverse_value(len_data_area_);
	}
	
}

int ptcl_packet_mass::parse_packet()
{
	int result = parse_header();
	if (result <= 0)
		return result;

	return len_packet_;
}

int ptcl_packet_mass::parse_header()
{
	if (is_get_packet_) 
	{
		return -1;
	}

	if (frame_header_->version != 0x0100)
		return -1;

	len_data_area_ = reverse_value(frame_header_->length);
	len_packet_  = len_data_area_ + len_header_;
	return len_header_;
}

int ptcl_packet_mass::parse_header(unsigned char *buffer,int len_buffer)
{
	if (len_buffer < len_header_)
		return 0;

	frame_header_ = (frame_header_struct *)buffer;
	return parse_header();
}

unsigned int ptcl_packet_mass::get_device_addr()
{
	unsigned int dev_addr = 0;

	dev_addr = 	reverse_value(frame_header_->src_port);

	return dev_addr;

}

int ptcl_packet_mass::get_frame_seq()
{
	return 0;
}

unsigned char ptcl_packet_mass::get_host_id()
{
	return (unsigned char)reverse_value(frame_header_->src_port);
}

void ptcl_packet_mass::set_device_addr(unsigned short dev_addr)
{
	if (is_get_packet_)
		return;

	frame_header_->src_port = reverse_value(dev_addr);
}


void ptcl_packet_mass::set_host_id(unsigned char id)
{
	if (is_get_packet_)
		return;

	frame_header_->src_port = reverse_value(id);
}

void ptcl_packet_mass::set_frame_seq(unsigned char seq)
{

}
	

unsigned short ptcl_packet_mass::reverse_value(unsigned short value)
{
	unsigned char buf[2]={0};
	memcpy(buf, &value, 2);
	std::reverse(buf, buf+2);
	return *(unsigned short*)buf;
}