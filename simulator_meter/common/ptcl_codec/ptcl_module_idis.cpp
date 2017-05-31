#include <ptcl_codec/ptcl_module_idis.h>
#include <ptcl_codec\asn_codec.h>

ptcl_module_idis *ptcl_module_idis::instance_ = NULL;
ptcl_module_idis::ptcl_module_idis(void)
{
}

ptcl_module_idis::~ptcl_module_idis(void)
{
}

ptcl_module_idis *ptcl_module_idis::get_instance()
{
	if (instance_ == NULL)
	{
		instance_ = new ptcl_module_idis();
	}

	return instance_;
}

int ptcl_module_idis::format_data_unit_param(data_unit_param &dup, unsigned char *buffer)
{
	int length = 0;
	memcpy(buffer+length, dup.meter_asset_no, 16);
	length += 16;
	memcpy(buffer+length, &dup.os.class_id, 2);
	length += 2;
	memcpy(buffer+length, dup.os.obis_code, 6);
	length += 6;
	buffer[length++] = dup.os.attribute_id;
	memcpy(buffer+length, &dup.length, 2);
	length += 2;
	memcpy(buffer+length, dup.buffer, dup.length);
	length += dup.length;
	return length;
}

int ptcl_module_idis::parse_data_unit_param(data_unit_param &dup, unsigned char *buffer)
{
	int length = 0;
	memcpy(dup.meter_asset_no, buffer+length, 16);
	length += 16;
	memcpy(&dup.os.class_id, buffer+length, 2);
	length += 2;
	memcpy(dup.os.obis_code, buffer+length, 6);
	length += 6;
	dup.os.attribute_id = buffer[length++];
	memcpy(&dup.length, buffer+length, 2);
	length += 2;
	memcpy(dup.buffer, buffer+length, dup.length);
	length += dup.length;
	return length;
}

void ptcl_module_idis::format_packet_error_ack(ptcl_packet_sg *packet, int flag)
{
	packet->set_ctrl_code_area(0);
	packet->set_frm(1);
	packet->set_fcv(0);
	packet->set_fcv_acd(0);
	packet->set_dir(1);

	packet->set_group_addr(0);
	packet->set_afn(AFN_ERROR_ACK);

	packet->set_tpv(0);
	packet->set_fri(1);
	packet->set_fin(1);
	packet->set_con(0);

	int len_data = 0;
	unsigned char buffer[10];
	memset(buffer,0,10);

	memcpy(buffer, &flag, 4);
	len_data += 4;

	packet->set_data_area_buffer(buffer, len_data);
	packet->format_packet();
}

int ptcl_module_idis::format_packet_with_param(ptcl_packet_sg *packet, int afn, std::vector<data_unit_param> &vec_item)
{
	if (vec_item.size() == 0)
		return 1;
	if (vec_item[0].length > 1024)
		return 1;
	packet->set_ctrl_code_area(0);
	packet->set_frm(1);
	packet->set_fcv(0);
	packet->set_fcv_acd(0);
	packet->set_dir(0);

	packet->set_group_addr(0);
	
	packet->set_afn(afn);

	packet->set_tpv(0);
	packet->set_fri(1);
	packet->set_fin(1);
	packet->set_con(0);

	unsigned int addr = packet->get_device_addr();
	unsigned char seq = get_frame_seq(addr);
	packet->set_frame_seq(seq);

	int len_data = 0;
	unsigned char buffer[10240]={0};

	buffer[len_data++] = vec_item.size();
	for (int i=0; i<vec_item.size(); i++)
	{
		len_data += format_data_unit_param(vec_item[i], buffer+len_data);
	}

	if (len_data > 1400)
	{
		printf("Error:data area buffer(%d) out of 1400\n", len_data);
		return 1;
	}

	packet->set_data_area_buffer(buffer, len_data);
	packet->format_packet();
	return 0;
}

void ptcl_module_idis::parse_packet_with_param(ptcl_packet_sg *packet, std::vector<data_unit_param> &vec_item)
{
	unsigned char *data_buffer = packet->get_data_area_buffer();
	int data_len = 0;
	int item_size = data_buffer[data_len++];
	if (item_size == 0xff)
		return;
	for (int i=0; i<item_size; i++)
	{
		data_unit_param du;
		data_len += parse_data_unit_param(du, data_buffer+data_len);
		vec_item.push_back(du);
	}
}	

void ptcl_module_idis::format_packet_history_data(ptcl_packet_sg *packet, std::vector<req_history_data> &vec_item)
{
	std::vector<data_unit_param> vec_dup;
	for (int i=0; i<vec_item.size(); i++)
	{
		data_unit_param dup;
		memcpy(&dup, &vec_item[i].du, sizeof(data_unit));
		int data_len = 0;
		data_len += asn_codec::format_struct_data(2, dup.buffer+data_len);

		data_len += asn_codec::format_basic_data(9, 12, dup.buffer+data_len);
		data_len += asn_codec::format_date_time(vec_item[i].start_time, dup.buffer+data_len);

		data_len += asn_codec::format_basic_data(9, 12, dup.buffer+data_len);
		data_len += asn_codec::format_date_time(vec_item[i].end_time, dup.buffer+data_len);

		dup.length = data_len;
		vec_dup.push_back(dup);
	}
	format_packet_with_param(packet, AFN_READ_DATA_HISTORY, vec_dup);
}

