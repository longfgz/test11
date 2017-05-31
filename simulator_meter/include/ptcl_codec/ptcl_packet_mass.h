#pragma once

#include <ptcl_codec/ptcl_packet.h>

class PTCL_CODEC_EXPORT ptcl_packet_mass : public ptcl_packet
{
public:
	ptcl_packet_mass(bool packet_type = false);
	virtual ~ptcl_packet_mass(void);

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

	//逻辑地址
	void set_device_addr(unsigned short dev_addr);

	//主站地址
	void set_host_id(unsigned char id);

	//帧序号
	void set_frame_seq(unsigned char seq);

protected:
	//固定报文头，get专用
	struct get_frame_header_struct
	{
		unsigned short version;
		unsigned short length;
	};

	//报文头结构体
#pragma pack(push, 1)
	struct frame_header_struct
	{
		unsigned short version;
		unsigned short src_port;
		unsigned short dest_port;
		unsigned short length;
	};
#pragma pack(pop)

	frame_header_struct *frame_header_;
	get_frame_header_struct *get_frame_header_;
	bool is_get_packet_;
	unsigned short reverse_value(unsigned short value);
};
