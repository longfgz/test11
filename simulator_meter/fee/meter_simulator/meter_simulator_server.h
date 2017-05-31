#pragma once

#include <channel/base_channel.h>
class meter_simulator_server
{
public:
	meter_simulator_server(void);
	~meter_simulator_server(void);

	void run();
	void exit();

protected:
	base_channel *channel_;

};

