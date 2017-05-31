#include "log_message.h"



log_message::log_message(void)
{
	time(&message_time_);
	len_buffer_ = 0;
	memset(buffer_,0,MAX_BUFFER_LENGTH);
	packet_dir_ = 0;
	memset(message_id_,0,64);
}

log_message::~log_message(void)
{
}

unsigned char *log_message::get_buffer()
{
	return buffer_;
}

unsigned short int log_message::get_buffer_length()
{
	return len_buffer_;
}

void log_message::set_buffer(unsigned char *buffer,unsigned short int len_buffer)
{
	if (buffer == NULL)
		return ;

	if (len_buffer > MAX_BUFFER_LENGTH)
		len_buffer = MAX_BUFFER_LENGTH;

	memcpy(buffer_,buffer,len_buffer);
	len_buffer_ = len_buffer;
}

char *log_message::get_message_id()
{
	return message_id_;
}

int log_message::get_level()
{
	return level_;
}

void log_message::set_message_id(char *message_id)
{
	memset(message_id_,0,64);
	strcpy(message_id_,message_id);
}

void log_message::set_level(int level)
{
	level_ = level;
}

time_t log_message::get_message_time()
{
	return message_time_;
}

unsigned char log_message::get_packet_dir()
{
	return packet_dir_;
}

void log_message::set_packet_dir(unsigned char dir)
{
	packet_dir_ = dir;
}
