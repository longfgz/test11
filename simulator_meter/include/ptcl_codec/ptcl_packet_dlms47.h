#pragma once

#include <ptcl_codec/ptcl_packet.h>

class PTCL_CODEC_EXPORT ptcl_packet_dlms47 : public ptcl_packet
{
public:
	ptcl_packet_dlms47(int packet_type = 1);
	ptcl_packet_dlms47(ptcl_packet_dlms47& packet);
	virtual ~ptcl_packet_dlms47(void);
	//unsigned int pack_to_gateway_data(std::string phy_add, unsigned char* output_data);
	unsigned int pack_to_gateway_data(unsigned char* phy_add,unsigned char len, unsigned char* output_data);

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

	void set_src_port(unsigned short port);
	void set_dst_port(unsigned short port);

	unsigned int get_src_port();

	//主站地址
	void set_host_id(unsigned char id);

	//帧序号
	void set_frame_seq(unsigned char seq);

	int is_send_packet(){return is_send_packet_;}

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

protected:
	frame_header_struct *frame_header_;
	int is_send_packet_;
	unsigned short reverse_value(unsigned short value);
};
