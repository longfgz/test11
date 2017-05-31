#include <ptcl_codec/ptcl_module_ansi.h>
#include <ptcl_codec/ansi/ptcl.hpp>

ptcl_module_ansi *ptcl_module_ansi::instance_ = NULL;
ptcl_module_ansi::ptcl_module_ansi(void)
{
}

ptcl_module_ansi::~ptcl_module_ansi(void)
{
}

ptcl_module_ansi *ptcl_module_ansi::get_instance()
{
	if (instance_ == NULL)
		instance_ = new ptcl_module_ansi();
	return instance_;
}


void ptcl_module_ansi::fill_read_request(ptcl_packet_ansi &packet, uint16_t table_id, uint32_t offset, uint16_t read_bytes)
{
  ptcl_utility::ansi_c12::dot18::PReadOffset_CT sdu;
  sdu.init(table_id, offset, read_bytes);
  unsigned char *p = reinterpret_cast<unsigned char *>(&sdu);
  packet.parse_header_suhe(p, sizeof(ptcl_utility::ansi_c12::dot18::PReadOffset_CT));
}
// 40 00 07 00 0B 00 0A 00 07 10 06 14 11 24 30 01 5F 
void ptcl_module_ansi::fill_write_table_request(ptcl_packet_ansi &packet,uint16_t table_id,uint8_t *buffer,uint16_t len)
{
	byte_t *tmp = new byte_t[ len+6];
	tmp[0]=0x40;
	//表标志，2字节
	byte_t *t = (byte_t*) &table_id;
	tmp[1] = t[1];
	tmp[2] = t[0];
	//数据长度，2字节
	t = (byte_t*) &len;
	tmp[3] = t[1];
	tmp[4] = t[0];
	//数据 
	memcpy(tmp+5,buffer,len);
	//计算
	byte_t sum=0;
	for(int i=0;i<len;i++)
		sum += buffer[i];
	tmp[len+5] = -sum;
	packet.parse_header_suhe(tmp,len+6);
	delete tmp;
}

void ptcl_module_ansi::format_packet_read_data_history(ptcl_packet_ansi *packet)
{
	if (packet == NULL)
		return ;

	//控制域功能码	
	packet->set_ctrl_code_area(0);

	//应用层功能码
	//packet->set_afn(AFN_READ_DATA_HISTORY);


	//序列域
	unsigned int addr = 0;
	unsigned char seq = get_frame_seq(addr);
	packet->set_frame_seq(seq);
	
	

	//用户数据域
	int len_data = 0;
	unsigned char buffer[1024];
	memset(buffer,0,1024);

	//组织数据域
	

	packet->set_data_area_buffer(buffer,len_data);

	packet->format_packet();
}

void ptcl_module_ansi::format_packet_read_event(ptcl_packet_ansi *packet)
{
	if (packet == NULL)
		return ;

	//控制域功能码
	packet->set_ctrl_code_area(0);

	//应用层功能码
	packet->set_afn(0X3F);


	//序列域
	unsigned int addr = 0;
	unsigned char seq = get_frame_seq(addr);
	packet->set_frame_seq(seq);
	

	//用户数据域
	int len_data = 0;
	unsigned char buffer[1024];
	memset(buffer,0,1024);

	//组织数据域
	

	packet->set_data_area_buffer(buffer,len_data);

	packet->format_packet();
}

void ptcl_module_ansi::format_packet_get_param(ptcl_packet_ansi *packet)
{
	if (packet == NULL)
		return ;

	//控制域功能码
	packet->set_ctrl_code_area(0);

	//应用层功能码
	packet->set_afn(0x3F);


	//序列域
	unsigned int addr = 0;
	unsigned char seq = get_frame_seq(addr);
	packet->set_frame_seq(seq);
	
	

	//用户数据域
	int len_data = 0;
	unsigned char buffer[1024];
	memset(buffer,0,1024);

	//组织数据域
	

	packet->set_data_area_buffer(buffer,len_data);

	packet->format_packet();
}



void ptcl_module_ansi::format_packet_set_param(ptcl_packet_ansi *packet)
{
	if (packet == NULL)
		return ;

	//控制域功能码
	packet->set_ctrl_code_area(0);

	//应用层功能码
	packet->set_afn(0x4F);


	//序列域
	unsigned int addr = 0;
	unsigned char seq = get_frame_seq(addr);
	packet->set_frame_seq(seq);
	
	

	//用户数据域
	int len_data = 0;
	unsigned char buffer[1024];
	memset(buffer,0,1024);

	//组织数据域
	

	packet->set_data_area_buffer(buffer,len_data);

	packet->format_packet();
}