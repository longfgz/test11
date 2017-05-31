#include "frame_buffer.h"

frame_buffer::frame_buffer()
{
}

frame_buffer::~frame_buffer()
{
}

void frame_buffer::add(unsigned char *buf,int len)
{
	boost::recursive_mutex::scoped_lock scoped_lock(mutex_buffer_);
	buffer_.append(buf, buf+len);
}

int  frame_buffer::find(int begin,unsigned char value)
{
	boost::recursive_mutex::scoped_lock scoped_lock(mutex_buffer_);
	return buffer_.find(value, begin);
}

int  frame_buffer::size()
{
	boost::recursive_mutex::scoped_lock scoped_lock(mutex_buffer_);
	return buffer_.size();
}

unsigned char frame_buffer::operator[](int pos)
{
	boost::recursive_mutex::scoped_lock scoped_lock(mutex_buffer_);
	if (pos>=buffer_.size())
		return 0;
	return buffer_[pos];
}

bool frame_buffer::get_value(int pos,unsigned char&value)
{
	boost::recursive_mutex::scoped_lock scoped_lock(mutex_buffer_);
	if (pos >= buffer_.size())
		return false;
	value = buffer_[pos];
	return true;
}

void frame_buffer::remove(int begin,int length)
{
	boost::recursive_mutex::scoped_lock scoped_lock(mutex_buffer_);
	buffer_.erase(buffer_.begin()+begin, buffer_.begin()+begin+length);
}

void frame_buffer::clear()
{
	boost::recursive_mutex::scoped_lock scoped_lock(mutex_buffer_);
	buffer_.clear();
}

bool frame_buffer::get_block(int begin, int len, message_block &mb)
{
	boost::recursive_mutex::scoped_lock scoped_lock(mutex_buffer_);
	if (buffer_.size() < begin+len)
		return false;
	mb.reset();
	mb.copy((unsigned char *)buffer_.data()+begin, len);
	return true;
}

bool frame_buffer::extract_block(int begin, int len, message_block &mb)
{
	boost::recursive_mutex::scoped_lock scoped_lock(mutex_buffer_);
	if (buffer_.size() < begin+len)
		return false;
	mb.reset();
	mb.copy((unsigned char *)buffer_.data()+begin, len);
	remove(begin, len);
	return true;
}