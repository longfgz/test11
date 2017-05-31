#include "ptcl_handler_serv.h"
#include <logger/logger.h>
#include <ptcl_codec/ptcl_packet_in.h>
#include <ptcl_codec/ptcl_module_in.h>
#include "meter_handler.h"

ptcl_handler_serv::ptcl_handler_serv(void)
{

}

ptcl_handler_serv::~ptcl_handler_serv(void)
{

}

void ptcl_handler_serv::process_packet(frame_buffer &buffer, server_handler_ptr handler)
{
	while (buffer.size() > 8)
	{ 
		if (buffer[0] != 0x00 || buffer[1] != 0x01)
		{
			message_block mb;
			buffer.extract_block(0, buffer.size(), mb);
		}
		if (!parse_response_frame(buffer, handler))
			break;
	}
}

bool ptcl_handler_serv::parse_response_frame(frame_buffer &buffer, server_handler_ptr handler)
{
	ptcl_packet_dlms47 packet(0);
	message_block mb;
	if (!buffer.get_block(0, packet.get_header_len(), mb))
		return false;
	if (packet.parse_header(mb.rd_ptr(), mb.length()) <= 0)
		return false;
	if (!buffer.extract_block(0, packet.get_packet_length(), mb))
		return false;
	handle_response_mb(handler, mb);
	return true;
}

void ptcl_handler_serv::handle_response_mb(server_handler_ptr handler, message_block &mb)
{
	ptcl_packet_dlms47 packet(0);
	packet.set_packet_buffer((unsigned char *)mb.rd_ptr(),(int)mb.length());
	if (packet.parse_packet() <= 0)
		return;

	meter_handler_ptr meter = handler->get_meter_handler();
	if (!meter)
		return;

	thread_task_ptr task_ptr(new thread_task(meter, mb));
	handler->get_channel()->get_thread_pool()->put(task_ptr);

}
