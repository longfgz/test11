#include <ptcl_codec/ptcl_packet_gw_dlms47.h>
#include <algorithm>

ptcl_packet_gw_dlms47::ptcl_packet_gw_dlms47(int packet_type)
	:ptcl_packet_dlms47(packet_type)
{
	reset();
}

ptcl_packet_gw_dlms47::ptcl_packet_gw_dlms47(ptcl_packet_dlms47& packet,std::string meter_no)
	:ptcl_packet_dlms47(packet.is_send_packet())
{
	memcpy(frame_header_,packet.get_packet_buffer(),packet.get_header_len());
	gateway_header_->header_flag = is_send_packet_ ? 0xE6 : 0xE7;
	gateway_header_->network_id = 0;
	gateway_header_->addr_len = meter_no.length();
	memcpy(gateway_header_ + sizeof(gateway_header_struct), meter_no.c_str(),meter_no.length());

	memcpy(gateway_header_ + sizeof(gateway_header_struct) + gateway_header_->addr_len, 
		packet.get_data_area_buffer(),packet.get_data_area_length());
	
	len_data_area_ = gateway_header_->addr_len + packet.get_data_area_length(); 
	len_packet_ = len_header_ + len_data_area_;
}

ptcl_packet_gw_dlms47::~ptcl_packet_gw_dlms47()
{

}

void ptcl_packet_gw_dlms47::reset()
{
	frame_header_ = (frame_header_struct *)buf_packet_;
	gateway_header_ = (gateway_header_struct*)((char*)frame_header_ + sizeof(frame_header_struct));
	len_header_ = sizeof(frame_header_struct) + sizeof(gateway_header_struct);

	buf_data_area_ = buf_packet_ + len_header_;
	len_packet_ = 0;
	len_data_area_ = 0;
	frame_header_->src_port = 0x0100;
	frame_header_->dest_port = 0x0100;
}

void ptcl_packet_gw_dlms47::format_packet()
{
	frame_header_->version = 0x0100;
	len_packet_ = len_data_area_+ len_header_;
	frame_header_->length = reverse_value(len_data_area_);
	gateway_header_->header_flag = is_send_packet_ ? 0xE6 : 0xE7;
	gateway_header_->network_id = 0;
}

int ptcl_packet_gw_dlms47::parse_header()
{
	if (frame_header_->version != 0x0100)
		return -1;

	if (gateway_header_->header_flag != 0xE6 && gateway_header_->header_flag != 0xE7)
		return -2;

	len_data_area_ = reverse_value(frame_header_->length);
	len_packet_  = len_data_area_ + len_header_;
	return len_header_;
}

int ptcl_packet_gw_dlms47::parse_header(unsigned char *buffer,int len_buffer)
{
	if (len_buffer < len_header_)
		return 0;

	frame_header_ = (frame_header_struct *)buffer;
	gateway_header_ = (gateway_header_struct*)((char*)frame_header_ + sizeof(frame_header_struct));

	return parse_header();
}

std::string ptcl_packet_gw_dlms47::get_physical_addr()
{
	char out_address[MAX_GATEWAY_LENGTH] = {0};
	char address_len = gateway_header_->addr_len;
	
	if(MAX_GATEWAY_LENGTH < address_len)
	{
		return NULL;
	}

	char* add_ptr = (char*)gateway_header_ + sizeof(gateway_header_struct);

	memcpy(out_address,add_ptr,address_len);
	return out_address;
}

unsigned int ptcl_packet_gw_dlms47::unpack_to_std_dlms47_data(unsigned char* output_data)
{
	unsigned char buffer[MAX_PACKET_LENGTH] = {0};
	unsigned short pos = 0;
	memcpy(buffer+pos,frame_header_,sizeof(frame_header_struct));
	pos += sizeof(frame_header_struct);

	unsigned len = len_packet_ - len_header_ - gateway_header_->addr_len;
	memcpy(buffer+pos,buf_data_area_+ gateway_header_->addr_len,len);
	pos += len;

	//change the data area length.
	frame_header_struct* ptr = (frame_header_struct*)buffer;
	ptr->length = reverse_value(pos - sizeof(frame_header_struct));

	//set the output data
	memcpy(output_data,buffer,pos);
	return pos;
}