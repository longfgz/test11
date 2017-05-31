#include <channel/message_block.h>
#include <string.h>

message_block::message_block(void)
: rd_ptr_(0)
, wr_ptr_(0)
{
}

message_block::message_block(unsigned char* base, std::size_t length)
: rd_ptr_(0)
, wr_ptr_(0)
{
	if (length > 0 && length < max_data_length)
	{
		memcpy(base_, base, length);
		wr_ptr_ += length;
	}
}

message_block::~message_block(void)
{
}

int message_block::copy(unsigned char* buf, std::size_t length)
{
	std::size_t len = space();

	if (len >= length)
	{
		memcpy(wr_ptr(), buf, length);
		wr_ptr(length);
		return 0;
	}
	else
		return -1;
}

std::size_t message_block::size(void) const
{
	return max_data_length;
}

std::size_t message_block::length(void) const
{
	return wr_ptr_ - rd_ptr_;
}

std::size_t message_block::length_base(void) const
{
	return wr_ptr_;
}

std::size_t message_block::space(void) const
{
	return max_data_length - (wr_ptr_ - rd_ptr_);
}

unsigned char* message_block::base(void)
{
	return base_;
}

unsigned char* message_block::wr_ptr(void)
{
	return base() + wr_ptr_;
}

void message_block::wr_ptr(std::size_t length)
{
	wr_ptr_ += length;
}

void message_block::wr_ptr(unsigned char *ptr)
{
	wr_ptr_ = ptr - base();
}

unsigned char* message_block::rd_ptr(void)
{
	return base() + rd_ptr_;
}

void message_block::rd_ptr(std::size_t length)
{
	rd_ptr_ += length;
}

void message_block::rd_ptr(unsigned char *ptr)
{
	rd_ptr_ = ptr - base();
}

void message_block::reset(void)
{
	rd_ptr_ = 0;
	wr_ptr_ = 0;
}

std::size_t message_block::get_rd_ptr()
{
	return rd_ptr_;
}