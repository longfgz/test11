#pragma once

#include <channel/channel_global.h>


class CHANNEL_EXPORT message_block
{
public:
	message_block(void);
	message_block(unsigned char* base, std::size_t length);
	~message_block(void);

	int copy(unsigned char* buf, std::size_t length);
	std::size_t size(void) const;
	std::size_t length(void) const;
	std::size_t length_base(void) const;
	std::size_t space(void) const;
	unsigned char* base(void);
	unsigned char* wr_ptr(void);
	void wr_ptr(std::size_t length);
	void wr_ptr(unsigned char *ptr);
	unsigned char* rd_ptr(void);
	void rd_ptr(std::size_t length);
	void rd_ptr(unsigned char *ptr);
	void reset(void);

	std::size_t get_rd_ptr();

private:
	enum { max_data_length = 1536 };

	unsigned char base_[max_data_length];
	std::size_t rd_ptr_;
	std::size_t wr_ptr_;

};

