#include <channel/base_channel_handler.h>
#include <channel/base_channel.h>

base_channel_handler::base_channel_handler(unsigned int id)
{
	id_ = id;
	state_ = chs_idle;
}

base_channel_handler::~base_channel_handler(void)
{
}

unsigned int base_channel_handler::get_id()
{
	return id_;
}

void base_channel_handler::set_id(unsigned int id)
{
	id_ = id;
}

base_channel_handler::channel_handler_state base_channel_handler::get_state()
{
	return state_;
}

void base_channel_handler::set_state(channel_handler_state s)
{
	state_ = s;
}