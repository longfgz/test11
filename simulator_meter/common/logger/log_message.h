#pragma once

#include <deque>
#include <boost/thread.hpp>
#include <logger/logger_global.h>

class log_message
{
	friend class sqlite_writer;
public:
	enum { MAX_BUFFER_LENGTH = 512 };

	log_message(void);
	~log_message(void);

	unsigned char *get_buffer();
	unsigned short int get_buffer_length();
	void set_buffer(unsigned char *buffer,unsigned short int len_buffer);
	
	char *get_message_id();
	void set_message_id(char *message_id);

	time_t get_message_time();
	
	unsigned char get_packet_dir();
	void set_packet_dir(unsigned char dir);
	int get_level();
	void set_level(int level);
protected:
	
	unsigned char buffer_[MAX_BUFFER_LENGTH];
	unsigned short int len_buffer_;
	time_t message_time_;
	unsigned char packet_dir_;
	char message_id_[64];
	int level_;
};