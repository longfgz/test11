#pragma once

#include <channel/channel_global.h>

class CHANNEL_EXPORT channel_monitor
{
public:
	channel_monitor(void) {};
	virtual ~channel_monitor(void) {};

	enum channel_event_type
	{
		cet_id_change  =   1
	};
	virtual void on_event(int event_type,char *event_buffer,char *sub_event_buffer) = 0;
};
