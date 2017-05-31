#include <ptcl_codec/ptcl_module_mass.h>
#include <ptcl_codec/asn_codec.h>
#include <time.h>
#include <stdio.h>

ptcl_module_mass *ptcl_module_mass::instance_ = NULL;
ptcl_module_mass::ptcl_module_mass(void)
{

}

ptcl_module_mass::~ptcl_module_mass(void)
{

}

ptcl_module_mass *ptcl_module_mass::get_instance()
{
	if (instance_ == NULL)
	{
		instance_ = new ptcl_module_mass;
	}
	return instance_;
}




void ptcl_module_mass::format_packet_cosem(ptcl_packet_mass *packet, int invokeid_priority)
{
	//主站ID
	packet->set_host_id(host_id_);

	unsigned char *cosem_data = packet->get_data_area_buffer();
	cosem_data[2] = (unsigned char)invokeid_priority;

	packet->format_packet();
}

void ptcl_module_mass::fill_packet_cosem_get(ptcl_packet_mass *packet, short obis_class, char* obis, unsigned char attr_id)
{
	//用户数据域
	unsigned char buf_data_area[1024];
	int length = 0;

	//APDU
	buf_data_area[length++] = 0xC0;
	buf_data_area[length++] = 0x01;
	buf_data_area[length++] = 0xFF;

	//class id
	int len = asn_codec::format_context_area_short(obis_class,buf_data_area+length);
	length = length + len;

	//obis
	len = asn_codec::format_context_area_obis(obis,buf_data_area+length);
	length = length + len;
	buf_data_area[length++] = attr_id;//buffer  属性的值
	buf_data_area[length++] = 0x00;//access_selector is null

	packet->set_data_area_buffer(buf_data_area,length);

	packet->format_packet();
}

void ptcl_module_mass::fill_packet_cosem_set(ptcl_packet_mass *packet, short obis_class, char* obis, unsigned char attr_id, 
		unsigned char data[], int data_len)
{
	//用户数据域
	unsigned char buf_data_area[1024];
	int length = 0;

	//APDU
	buf_data_area[length++] = 0xC1;
	buf_data_area[length++] = 0x01;
	buf_data_area[length++] = 0xFF;

	//class id
	int len = asn_codec::format_context_area_short(obis_class,buf_data_area+length);
	length = length + len;

	//obis
	len = asn_codec::format_context_area_obis(obis,buf_data_area+length);
	length = length + len;
	buf_data_area[length++] = attr_id;//buffer  属性的值
	buf_data_area[length++] = 0x00;//access_selector is null

	memcpy(buf_data_area + length, data, data_len);
	length += data_len;

	packet->set_data_area_buffer(buf_data_area,length);

	packet->format_packet();
}

void ptcl_module_mass::fill_packet_cosem_set_filter(ptcl_packet_mass *packet, unsigned int start_time, unsigned int end_time,
		unsigned int amount, unsigned int bitfield, unsigned char mode)
{
	//static final obis value mass memory filter
	short obis_class = 1;
	std::string obis_value = "0000411002FF";
	unsigned char attribute = 2;

	unsigned char buf[1024];
	int length = 0;

	int len = asn_codec::format_struct_data(5, buf+length);
	length += len;

	len = asn_codec::format_unsigned(mode, buf+length);
	length += len;

	len = asn_codec::format_date_time_extra(start_time, buf+length);
	length += len;

	len = asn_codec::format_date_time_extra(end_time, buf+length);
	length += len;

	len = asn_codec::format_unsigned_double_long(amount, buf+length);
	length += len;

	len = asn_codec::format_unsigned_double_long(bitfield, buf+length);
	length += len;

	fill_packet_cosem_set(packet, obis_class,(char *)obis_value.c_str(), attribute, buf, length);
}

void ptcl_module_mass::fill_packet_cosem_next(ptcl_packet_mass *packet, unsigned int block_no)
{
	//用户数据域
	unsigned char buf_data_area[1024];
	int length = 0;

	//APDU
	buf_data_area[length++] = 0xC0;//A-XDR编码get-request [192] IMPLICIT
	buf_data_area[length++] = 0x02;//[1] IMPLICIT Get-Request-Normal,
	buf_data_area[length++] = 0xFF;

	//class id
	int len = asn_codec::format_context_area_int(block_no,buf_data_area+length);
	length = length + len;

	packet->set_data_area_buffer(buf_data_area,length);

	packet->format_packet();
}