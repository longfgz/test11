#pragma once

#include <ptcl_codec/ptcl_packet.h>



class PTCL_CODEC_EXPORT ptcl_packet_dlms : public ptcl_packet
{
public:
	ptcl_packet_dlms(void);
	virtual ~ptcl_packet_dlms(void);

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

	void set_device_addr(unsigned int addr);

	unsigned char get_ctrl_code();
	void set_ctrl_code(unsigned char code);

	unsigned char get_header_start();

	unsigned short get_check_sum(unsigned char *buffer,int len,unsigned short fcs = 0xffff);

	void set_fcs_tab();

	void set_host_id(unsigned char host_id);

	unsigned char get_frame_header();

	unsigned char get_send_seq();
	void set_send_seq(unsigned char seq);

	unsigned char get_recv_seq();
	void set_recv_seq(unsigned char seq);

	void set_ctrl_flag(unsigned char flag);
	void set_ctrl_pf(unsigned char pf);

protected:
	//报文头结构体
#pragma pack(push, 1)

	struct frame_header_struct
	{
		unsigned char frame_start;//0x7e
		union
		{
			struct
			{
				unsigned short	len			:	11,			//帧类型 dlms里恒为0x0a
								s			:	1,			//s标志位,说明帧是否被分割
								frame_type	:	4;			//帧长度(除帧头帧尾外)
			};
			unsigned short int len_data_area;//帧类型与帧长
		};
		
		unsigned char device_addr[4];//目的地址域	高位地址默认为1 测试时
		unsigned char host_id;	//源地址域
		union
		{
			struct  
			{
				unsigned char	flag		: 1,		//发送帧计数
								sss			: 3,		//P/F
								p_and_f		: 1,		//接受帧计数
								rrr			: 3;		//特征
			};
			unsigned char ctrl_code_area; //控制域
		};
		
	};

#pragma pack(pop)

	frame_header_struct *frame_header_;
	unsigned short fcs_tab_[256];
};
