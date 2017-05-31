#pragma once

#include <ptcl_codec/ptcl_module_dlms47.h>
#include <channel/message_block.h>
#include "server_channel.hpp"

class ptcl_handler_serv
{
public:
	typedef boost::shared_ptr<server_channel_handler <ptcl_handler_serv> > server_handler_ptr;
	ptcl_handler_serv(void);
	~ptcl_handler_serv(void);
	void process_packet(frame_buffer &buffer, server_handler_ptr handler);
private:
	bool parse_response_frame(frame_buffer &buffer, server_handler_ptr handler);
	bool parse_link_frame(frame_buffer &buffer, server_handler_ptr handler);
	void handle_response_mb(server_handler_ptr handler, message_block &mb);

};
