#pragma once

#include <vector>
#include <string>
#include <channel/message_block.h>
#include <boost/thread.hpp>

class frame_buffer
{
public:
	frame_buffer();
	virtual ~frame_buffer();
	void add(unsigned char *buf,int len);
	int  find(int begin,unsigned char value);
	int  size();
	unsigned char operator[](int pos);
	bool get_value(int pos,unsigned char &value);
	void remove(int begin,int length);
	void clear();
	bool get_block(int begin, int len, message_block &mb);
	bool extract_block(int begin, int len, message_block &mb);
private:
    std::basic_string<unsigned char> buffer_;
	boost::recursive_mutex mutex_buffer_;
};

