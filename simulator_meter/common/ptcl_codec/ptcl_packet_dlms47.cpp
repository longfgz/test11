#include <ptcl_codec/ptcl_packet_dlms47.h>
#include <algorithm>

ptcl_packet_dlms47::ptcl_packet_dlms47(int packet_type)
{
	is_send_packet_ = packet_type;
	reset();
}

ptcl_packet_dlms47::ptcl_packet_dlms47(ptcl_packet_dlms47& packet)
{
	memcpy(buf_packet_,packet.get_packet_buffer(),packet.get_packet_length());
	len_data_area_ = packet.len_data_area_;
	len_packet_ = packet.len_packet_;
	len_header_ = packet.len_header_;
	buf_data_area_ = buf_packet_ + len_header_;
	
	is_send_packet_ = packet.is_send_packet_;
	frame_header_ = (frame_header_struct *)buf_packet_;
}

ptcl_packet_dlms47::~ptcl_packet_dlms47()
{

}

/*unsigned int ptcl_packet_dlms47::pack_to_gateway_data(std::string phy_add, unsigned char* output_data)
{
	unsigned short pos = 0;
	memcpy(output_data + pos, frame_header_, sizeof(frame_header_struct));
	pos += sizeof(frame_header_struct);

	if(phy_add.length() >= 255)
	{
		return 0;
	}

	unsigned char gateway_header[3] = {0xE6,0x0,phy_add.length()};
	memcpy(output_data + pos,gateway_header,3);
	pos += 3;

	memcpy(output_data + pos, phy_add.c_str(),phy_add.length());
	pos += phy_add.length();

	memcpy(output_data + pos, buf_data_area_,len_data_area_);
	pos += len_data_area_;

	//change the data area length.
	frame_header_struct* ptr = (frame_header_struct*)output_data;
	ptr->length = reverse_value(pos - sizeof(frame_header_struct));

	return pos;
}*/

unsigned int ptcl_packet_dlms47::pack_to_gateway_data(unsigned char* phy_add,unsigned char len, unsigned char* output_data)
{
	unsigned short pos = 0;
	memcpy(output_data + pos, frame_header_, sizeof(frame_header_struct));
	pos += sizeof(frame_header_struct);

	if(len >= 255)
	{
		return 0;
	}

	unsigned char gateway_header[3] = {0xE6,0x0,len};
	memcpy(output_data + pos,gateway_header,3);
	pos += 3;

	memcpy(output_data + pos, phy_add,len);
	pos += len;

	memcpy(output_data + pos, buf_data_area_,len_data_area_);
	pos += len_data_area_;

	//change the data area length.
	frame_header_struct* ptr = (frame_header_struct*)output_data;
	ptr->length = reverse_value(pos - sizeof(frame_header_struct));

	return pos;

}

void ptcl_packet_dlms47::reset()
{
	frame_header_ = (frame_header_struct *)buf_packet_;
	len_header_ = sizeof(frame_header_struct);
	buf_data_area_ = buf_packet_ + len_header_;
	len_packet_ = 0;
	len_data_area_ = 0;
	frame_header_->src_port = 0x0100;
	frame_header_->dest_port = 0x0100;
}


void ptcl_packet_dlms47::format_packet()
{
	frame_header_->version = 0x0100;
	len_packet_ = len_data_area_+ len_header_;
	frame_header_->length = reverse_value(len_data_area_);
}

int ptcl_packet_dlms47::parse_packet()
{
	int result = parse_header();
	if (result <= 0)
		return result;

	return len_packet_;
}

int ptcl_packet_dlms47::parse_header()
{
	if (frame_header_->version != 0x0100)
		return -1;

	len_data_area_ = reverse_value(frame_header_->length);
	len_packet_  = len_data_area_ + len_header_;
	return len_header_;
}

int ptcl_packet_dlms47::parse_header(unsigned char *buffer,int len_buffer)
{
	if (len_buffer < len_header_)
		return 0;

	frame_header_ = (frame_header_struct *)buffer;
	return parse_header();
}

unsigned int ptcl_packet_dlms47::get_device_addr()
{
	unsigned int dev_addr = 0;
	if (is_send_packet_ == 1)
	{
		dev_addr = 	reverse_value(frame_header_->dest_port);
	}
	else
	{
		dev_addr = 	reverse_value(frame_header_->src_port);
	}
	return dev_addr;

}

int ptcl_packet_dlms47::get_frame_seq()
{
	return 0;
}

unsigned char ptcl_packet_dlms47::get_host_id()
{
	if (is_send_packet_ == 1)
	{
		return (unsigned char)reverse_value(frame_header_->src_port);
	}
	else
	{
		return (unsigned char)reverse_value(frame_header_->dest_port);
	}
}

void ptcl_packet_dlms47::set_device_addr(unsigned short dev_addr)
{
	if (is_send_packet_ == 1)
	{
		frame_header_->dest_port = reverse_value(dev_addr);
	}
	else
	{
		frame_header_->src_port = reverse_value(dev_addr);
	}
}

void ptcl_packet_dlms47::set_src_port(unsigned short port)
{
	frame_header_->src_port = reverse_value(port);
}

void ptcl_packet_dlms47::set_dst_port(unsigned short port)
{
	frame_header_->dest_port = reverse_value(port);
}

void ptcl_packet_dlms47::set_host_id(unsigned char id)
{
	if (is_send_packet_ == 1)
	{
		frame_header_->src_port = reverse_value(id);
	}
	else
	{
		frame_header_->dest_port = reverse_value(id);
	}
}

void ptcl_packet_dlms47::set_frame_seq(unsigned char seq)
{

}
	
unsigned int ptcl_packet_dlms47::get_src_port()
{
	return reverse_value(frame_header_->src_port);
}

unsigned short ptcl_packet_dlms47::reverse_value(unsigned short value)
{
	unsigned char buf[2]={0};
	memcpy(buf, &value, 2);
	std::reverse(buf, buf+2);
	return *(unsigned short*)buf;
}