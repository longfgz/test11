#include <ptcl_codec/ptcl_packet_mbus.h>

ptcl_packet_mbus::ptcl_packet_mbus(int packet_type)
{
	packet_type_ = packet_type;
	reset();
}


ptcl_packet_mbus::~ptcl_packet_mbus(void)
{
}

void ptcl_packet_mbus::reset()
{
	if (packet_type_ == 0x7a)
	{
		frame_header_7a_ = (frame_header_struct_7a *)buf_packet_;
		len_header_ = 15;
		buf_data_area_ = buf_packet_ + len_header_;
		len_packet_ = 0;
		len_data_area_ = 0;
	}
	else
	{
		frame_header_72_ = (frame_header_struct_72 *)buf_packet_;
		len_header_ = 23;
		buf_data_area_ = buf_packet_ + len_header_;
		len_packet_ = 0;
		len_data_area_ = 0;
	}
}

int ptcl_packet_mbus::parse_packet()
{
	int result = parse_header();
	if (result < 0)
	{
		return result;
	}

	return len_packet_;
}
	
int ptcl_packet_mbus::parse_header()
{
	if (packet_type_ == 0x7a)
	{
		if (frame_header_7a_->c != 0x44 || frame_header_7a_->ci != 0x7a)
		{
			return -1;
		}
		len_data_area_ = frame_header_7a_->len - len_header_ + 1;
		if (len_data_area_ < 0 || len_data_area_ > (MAX_PACKET_LENGTH - len_header_))
			return -1;

		len_packet_ = len_header_ + len_data_area_;

		return len_header_;
	}
	else
	{
		if (frame_header_72_->c != 0x44 || frame_header_72_->ci != 0x72)
		{
			return -1;
		}
		len_data_area_ = frame_header_72_->len - len_header_ + 1;
		if (len_data_area_ < 0 || len_data_area_ > (MAX_PACKET_LENGTH - len_header_))
			return -1;

		len_packet_ = len_header_ + len_data_area_;

		return len_header_;
	}
}

int ptcl_packet_mbus::parse_header(unsigned char *buffer,int len_buffer)
{
	if (len_buffer < len_header_)
		return 0;

	if (packet_type_ == 0x7a)
	{
		frame_header_7a_ = (frame_header_struct_7a *)buffer;
	}
	else
		frame_header_72_ = (frame_header_struct_72 *)buffer;
	
	return parse_header();
}

unsigned int ptcl_packet_mbus::get_serial_number()
{
	if (packet_type_ == 0x7a)
	{
		return frame_header_7a_->s_number;
	}
	else
		return frame_header_72_->s_number;
}

unsigned short int ptcl_packet_mbus::get_macfacture()
{
	if (packet_type_ == 0x7a)
	{
		return frame_header_7a_->man_id;
	}
	else
		return frame_header_72_->man_id;
}

unsigned char ptcl_packet_mbus::get_version()
{
	if (packet_type_ == 0x7a)
	{
		return frame_header_7a_->version;
	}
	else
		return frame_header_72_->version;
}

unsigned char ptcl_packet_mbus::get_device_type()
{
	if (packet_type_ == 0x7a)
	{
		return frame_header_7a_->device_type;
	}
	else
		return frame_header_72_->device_type;
}

unsigned char ptcl_packet_mbus::get_ci()
{
	if (packet_type_ == 0x7a)
	{
		return frame_header_7a_->ci;
	}
	else
		return frame_header_72_->ci;
}

unsigned char ptcl_packet_mbus::get_access_no()
{
	if (packet_type_ == 0x7a)
	{
		return frame_header_7a_->access_no;
	}
	else
		return frame_header_72_->access_no;
}

unsigned char ptcl_packet_mbus::get_status()
{
	if (packet_type_ == 0x7a)
	{
		return frame_header_7a_->status;
	}
	else
		return frame_header_72_->status;
}

unsigned short int ptcl_packet_mbus::get_signature()
{
	if (packet_type_ == 0x7a)
	{
		return frame_header_7a_->signature;
	}
	else
		return frame_header_72_->signature;
}

unsigned int ptcl_packet_mbus::get_device_addr()
{
	return 0;
}

unsigned int ptcl_packet_mbus::get_ci_s_id()
{
	return frame_header_72_->ci_s_id;
}

unsigned char ptcl_packet_mbus::get_ci_version()
{
	return frame_header_72_->ci_version;
}

unsigned char ptcl_packet_mbus::get_ci_device_type()
{
	return frame_header_72_->ci_device_type;
}

void ptcl_packet_mbus::set_status(unsigned char status)
{
	if (packet_type_ == 0x7a)
		frame_header_7a_->status = status;
	else
		frame_header_72_->status = status;
}

//返回报文序列号
int ptcl_packet_mbus::get_frame_seq()
{
	return 0;
}

//返回主站ID
unsigned char ptcl_packet_mbus::get_host_id()
{
	return NULL;
}

void ptcl_packet_mbus::format_packet()
{

}