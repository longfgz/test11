#pragma once

#include <channel/channel_global.h>

#include <channel/message_block.h>

#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>


class CHANNEL_EXPORT base_channel_handler 
{
public:
	enum channel_handler_state 
	{
		chs_idle         =   0,
		chs_connected    =   1,
		chs_online       =   2,
		chs_offline      =   3,
		chs_dial		 =   4,
		chs_psw			 =   5,
		chs_link		 =   6,
		chs_login		 =   7,
	};


	base_channel_handler(unsigned int id);
	virtual ~base_channel_handler(void);

	virtual void put(message_block &mb) = 0;

	unsigned int get_id();
	void set_id(unsigned int id);
	channel_handler_state get_state();
	void set_state(channel_handler_state s);

protected:
	unsigned int id_;
	channel_handler_state state_;
};
