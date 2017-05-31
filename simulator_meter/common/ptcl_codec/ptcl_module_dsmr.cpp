#include <ptcl_codec/ptcl_module_dsmr.h>

ptcl_module_dsmr *ptcl_module_dsmr::instance_ = NULL;

ptcl_module_dsmr::ptcl_module_dsmr(void)
{
}

ptcl_module_dsmr::~ptcl_module_dsmr(void)
{
}

ptcl_module_dsmr *ptcl_module_dsmr::get_instance()
{
	if (instance_ == NULL)
		instance_ = new ptcl_module_dsmr();
	return instance_;
}

void ptcl_module_dsmr::format_create_history_data_request(ptcl_packet_sg *packet, std::vector<data_area_struct> & v_das)
{
	packet->set_ctrl_code_area(0);
	packet->set_frm(1);
	packet->set_fcv(0);
	packet->set_fcv_acd(0);
	packet->set_dir(0);

	packet->set_group_addr(0);
	
	packet->set_afn(AFN_READ_DATA_HISTORY);

	packet->set_tpv(0);
	packet->set_fri(1);
	packet->set_fin(1);
	packet->set_con(0);

	unsigned int addr = packet->get_device_addr();
	unsigned char seq = get_frame_seq(addr);
	packet->set_frame_seq(seq);

	int len_data = 0;
	unsigned char buffer[1024];
	memset(buffer,0,1024);

	//buffer[len_data++] = v_das.size();
	for (int i=0; i<v_das.size(); i++)
	{
		int len = sizeof(v_das[i].meter_asset_no) + sizeof(obis_struct) + v_das[i].len_data;
		memcpy(buffer+len_data, &v_das[i], len);
		len_data += len;
	}

	packet->set_data_area_buffer(buffer, len_data);
	packet->format_packet();
}

void ptcl_module_dsmr::format_create_response(ptcl_packet_sg *packet, char *resource_id)
{
	packet->set_ctrl_code_area(0);
	packet->set_frm(1);
	packet->set_fcv(0);
	packet->set_fcv_acd(0);
	packet->set_dir(1);

	packet->set_group_addr(0);
	packet->set_afn(AFN_CREATE_GET_RESPONSE);

	packet->set_tpv(0);
	packet->set_fri(1);
	packet->set_fin(1);
	packet->set_con(0);

	int len_data = 0;
	unsigned char buffer[1024];
	memset(buffer,0,1024);

	memcpy(buffer, resource_id, 32);
	len_data += 32;

	packet->set_data_area_buffer(buffer, len_data);
	packet->format_packet();
}

void ptcl_module_dsmr::format_get_request(ptcl_packet_sg *packet, char *resource_id)
{
	packet->set_ctrl_code_area(0);
	packet->set_frm(1);
	packet->set_fcv(0);
	packet->set_fcv_acd(0);
	packet->set_dir(0);

	packet->set_group_addr(0);
	packet->set_afn(AFN_GET_REQUEST);

	packet->set_tpv(0);
	packet->set_fri(1);
	packet->set_fin(1);
	packet->set_con(0);

	unsigned int addr = packet->get_device_addr();
	unsigned char seq = get_frame_seq(addr);
	packet->set_frame_seq(seq);

	int len_data = 0;
	unsigned char buffer[1024];
	memset(buffer,0,1024);

	memcpy(buffer, resource_id, 32);
	len_data += 32;

	packet->set_data_area_buffer(buffer, len_data);
	packet->format_packet();
}

void ptcl_module_dsmr::format_get_response(ptcl_packet_sg *packet, std::vector<data_area_struct> & v_das)
{
	packet->set_ctrl_code_area(0);
	packet->set_frm(1);
	packet->set_fcv(0);
	packet->set_fcv_acd(0);
	packet->set_dir(1);

	packet->set_group_addr(0);
	packet->set_afn(AFN_CREATE_GET_RESPONSE);

	packet->set_tpv(0);
	packet->set_fri(1);
	packet->set_fin(1);
	packet->set_con(0);

	int len_data = 0;
	unsigned char buffer[1024];
	memset(buffer,0,1024);

	//buffer[len_data++] = v_das.size();
	for (int i=0; i<v_das.size(); i++)
	{
		int len = sizeof(v_das[i].meter_asset_no) + sizeof(obis_struct) + v_das[i].len_data;
		memcpy(buffer, &v_das[i], len);
		len_data += len;
	}

	packet->set_data_area_buffer(buffer, len_data);
	packet->format_packet();
}

void ptcl_module_dsmr::format_confirm(ptcl_packet_sg *packet, int flag)
{
	packet->set_ctrl_code_area(0);
	packet->set_frm(1);
	packet->set_fcv(0);
	packet->set_fcv_acd(0);
	packet->set_dir(1);

	packet->set_group_addr(0);
	packet->set_afn(AFN_CONFIRM);

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

void ptcl_module_dsmr::format_del_request(ptcl_packet_sg *packet, char *resource_id)
{
	packet->set_ctrl_code_area(0);
	packet->set_frm(1);
	packet->set_fcv(0);
	packet->set_fcv_acd(0);
	packet->set_dir(0);

	packet->set_group_addr(0);
	packet->set_afn(AFN_DEL_REQUEST);

	packet->set_tpv(0);
	packet->set_fri(1);
	packet->set_fin(1);
	packet->set_con(0);

	unsigned int addr = packet->get_device_addr();
	unsigned char seq = get_frame_seq(addr);
	packet->set_frame_seq(seq);

	int len_data = 0;
	unsigned char buffer[1024];
	memset(buffer,0,1024);

	memcpy(buffer, resource_id, 32);
	len_data += 32;

	packet->set_data_area_buffer(buffer, len_data);

	packet->format_packet();
}