#include <ptcl_codec/ptcl_module_in.h>

ptcl_module_in *ptcl_module_in::instance_ = NULL;

ptcl_module_in::ptcl_module_in(void)
{
}

ptcl_module_in::~ptcl_module_in(void)
{
}

ptcl_module_in *ptcl_module_in::get_instance()
{
	if (instance_ == NULL)
		instance_ = new ptcl_module_in();
	return instance_;
}

void ptcl_module_in::format_packet_connect_test(ptcl_packet_in *packet,
												unsigned int sender_id,unsigned char sender_type,
												unsigned int receiver_id,unsigned char receiver_type,
												event_sub_type est)
{
	if (packet == NULL)
		return ;

#pragma pack(push, 1)
	struct time_struct
	{
		unsigned char psw[3];
		unsigned char hour;
		unsigned char min;
		unsigned char sec;
	};
#pragma pack(pop)

	packet->set_sender_id(sender_id);
	packet->set_sender_type(sender_type);
	packet->set_protocol_type(0);
	packet->set_frame_seq(get_frame_seq(1));
	packet->set_event_type(et_subscription);
	packet->set_event_sub_type((unsigned short int)est);
	packet->set_transfer_id(0);
	packet->set_transfer_type(0);
	packet->set_receiver_id(receiver_id);
	packet->set_receiver_type(receiver_type);

	time_struct time_data;
	memset(&time_data,0,sizeof(time_struct));
	packet->set_data_area_buffer((unsigned char *)&time_data,sizeof(time_struct));

	packet->format_packet();
}

void ptcl_module_in::format_packet_route(ptcl_packet_in *packet,
										 unsigned int sender_id,unsigned char sender_type,
										 unsigned int transfer_id,unsigned char transfer_type,
										 unsigned int receiver_id,unsigned char receiver_type,
										 int ptcl_id,unsigned char seq,unsigned char *buffer,int len,
										 unsigned char event_sub_type)
{
	if (packet == NULL)
		return ;

	packet->set_sender_id(sender_id);
	packet->set_sender_type(sender_type);
	packet->set_protocol_type(ptcl_id);
	packet->set_frame_seq(seq);
	packet->set_event_type(et_route);
	packet->set_event_sub_type(event_sub_type);
	packet->set_transfer_id(transfer_id);
	packet->set_transfer_type(transfer_type);
	packet->set_receiver_id(receiver_id);
	packet->set_receiver_type(receiver_type);


	packet->set_data_area_buffer(buffer,len);

	packet->format_packet();
}


void ptcl_module_in::format_packet_dlms47_alarm(ptcl_packet_in *packet,
										 unsigned int sender_id,unsigned char sender_type,
										 unsigned int transfer_id,unsigned char transfer_type,
										 unsigned int receiver_id,unsigned char receiver_type,
										 int ptcl_id,unsigned char seq,unsigned char *buffer,int len)
{
	if (packet == NULL)
		return ;

	packet->set_sender_id(sender_id);
	packet->set_sender_type(sender_type);
	packet->set_protocol_type(ptcl_id);
	packet->set_frame_seq(seq);
	packet->set_event_type(et_route);
	packet->set_event_sub_type(est_alarm_active_req);
	packet->set_transfer_id(transfer_id);
	packet->set_transfer_type(transfer_type);
	packet->set_receiver_id(receiver_id);
	packet->set_receiver_type(receiver_type);

	packet->set_data_area_buffer(buffer,len);

	packet->format_packet();
}

void ptcl_module_in::format_packet_dlms47_alarm2(ptcl_packet_in *packet,
												unsigned int sender_id,unsigned char sender_type,
												unsigned int transfer_id,unsigned char transfer_type,
												unsigned int receiver_id,unsigned char receiver_type,
												int ptcl_id,unsigned char seq,unsigned char *buffer,int len)
{
	if (packet == NULL)
		return ;

	packet->set_sender_id(sender_id);
	packet->set_sender_type(sender_type);
	packet->set_protocol_type(ptcl_id);
	packet->set_frame_seq(seq);
	packet->set_event_type(et_route);
	packet->set_event_sub_type(est_alarm_active_req2);
	packet->set_transfer_id(transfer_id);
	packet->set_transfer_type(transfer_type);
	packet->set_receiver_id(receiver_id);
	packet->set_receiver_type(receiver_type);

	packet->set_data_area_buffer(buffer,len);

	packet->format_packet();
}


void ptcl_module_in::format_packet_dc_data(ptcl_packet_in *packet,
										   unsigned int sender_id,unsigned char sender_type,
										   unsigned char *buffer,int len)
{
	if (packet == NULL)
		return ;

	packet->set_sender_id(sender_id);
	packet->set_sender_type(sender_type);
	packet->set_protocol_type(0);
	packet->set_frame_seq(get_frame_seq(1));
	packet->set_event_type(et_route);
	packet->set_event_sub_type(est_dc_data_req);
	packet->set_transfer_id(0);
	packet->set_transfer_type(0);
	packet->set_receiver_id(0);
	packet->set_receiver_type(0);

	packet->set_data_area_buffer(buffer,len);

	packet->format_packet();
}

void ptcl_module_in::format_packet_rcs(ptcl_packet_in *packet,
										 unsigned int sender_id,unsigned char sender_type,
										 unsigned int transfer_id,unsigned char transfer_type,
										 unsigned int receiver_id,unsigned char receiver_type,
										 int ptcl_id,unsigned char seq,unsigned char *buffer,int len)
{
	if (packet == NULL)
		return ;

	packet->set_sender_id(sender_id);
	packet->set_sender_type(sender_type);
	packet->set_protocol_type(ptcl_id);
	packet->set_frame_seq(seq);
	packet->set_event_type(et_route);
	packet->set_event_sub_type(0xf7);
	packet->set_transfer_id(transfer_id);
	packet->set_transfer_type(transfer_type);
	packet->set_receiver_id(receiver_id);
	packet->set_receiver_type(receiver_type);

	packet->set_data_area_buffer(buffer,len);

	packet->format_packet();
}
