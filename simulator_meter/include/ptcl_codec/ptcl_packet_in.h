#pragma once

#include <ptcl_codec/ptcl_packet.h>

class PTCL_CODEC_EXPORT ptcl_packet_in : public ptcl_packet
{
public:
	ptcl_packet_in(void);
	virtual ~ptcl_packet_in(void);

	virtual void reset();

	//组装报文，必须先set_packet_buffer
	virtual void format_packet();

	//解析报文 必须先set_packet_buffer
	//返回值 >0:解析报文成功 0:报文长度不够 <0:报文格式不对
	virtual int parse_packet();
	
	//返回值 >0:解析报文成功 0:报文长度不够 <0:报文格式不对
	virtual int parse_header();

	//返回值 >0:解析报文成功 0:报文长度不够 <0:报文格式不对
	virtual int parse_header(unsigned char *buffer,int len_buffer);

	//返回终端逻辑地址 按照报文顺序
	virtual unsigned int get_device_addr();

	//返回报文序列号
	virtual int get_frame_seq();
	//返回主站ID
	virtual unsigned char get_host_id();

	unsigned int get_sender_id();
	void set_sender_id(unsigned int id);

	unsigned char get_sender_type();
	void set_sender_type(unsigned char type);

	unsigned char get_protocol_type();
	void set_protocol_type(unsigned char type);

	void set_frame_seq(unsigned char seq);

	unsigned char get_event_type();
	void set_event_type(unsigned char type);

	unsigned short int get_event_sub_type();
	void set_event_sub_type(unsigned short int type);

	unsigned int get_transfer_id();
	void set_transfer_id(unsigned int id);

	unsigned char get_transfer_type();
	void set_transfer_type(unsigned char type);

	unsigned int get_receiver_id();
	void set_receiver_id(unsigned int id);

	unsigned char get_receiver_type();
	void set_receiver_type(unsigned char type);

protected:

	static unsigned short int ntohs(unsigned short int src);
	static unsigned int ntohl(unsigned int src);

#pragma pack(push, 1)
	struct frame_header_struct
	{
		unsigned char frame_start;
		unsigned int sender_id;
		unsigned char sender_type;
		unsigned char protocol_type;
		unsigned char frame_seq;
		unsigned char event_type;
		unsigned short int event_sub_type;
		unsigned int transfer_id;
		unsigned char transfer_type;
		unsigned int receiver_id;
		unsigned char receiver_type;
		unsigned short int data_length;
		unsigned char frame_end;
	};
#pragma pack(pop)

	frame_header_struct *frame_header_;
};
