#pragma once

#include <ptcl_codec/ptcl_packet.h>

namespace ptcl_utility { namespace ansi_c12 { namespace dot18 {
	class FrameHead_CT;
}}}

class PTCL_CODEC_EXPORT ptcl_packet_ansi : public ptcl_packet
{
public:
	ptcl_packet_ansi(void) ;
	virtual ~ptcl_packet_ansi(void) ;

	virtual void reset();

	//组装报文，必须先set_packet_buffer
	virtual void format_packet();
	unsigned char* GetDataPtr(void);

	//解析报文 必须先set_packet_buffer
	//返回值 >0:解析报文成功 0:报文长度不够 <0:报文格式不对
	virtual int parse_packet();
	virtual int parse_packet_suhe();

	//返回值 >0:解析报文成功 0:报文长度不够 <0:报文格式不对
	virtual int parse_header();

	//返回值 >0:解析报文成功 0:报文长度不够 <0:报文格式不对
	virtual int parse_header(unsigned char *buffer,int len_buffer);
	virtual int parse_header_suhe(unsigned char *buffer,int len_buffer);

	//返回终端逻辑地址 按照报文顺序
	virtual unsigned int get_device_addr() { return 0; }; // 不需要实现

	//返回报文序列号
	virtual int get_frame_seq();
	void set_frame_seq(int seq);

	//计算CRC校验值
	unsigned short int get_check_sum_ansi(unsigned char *buffer,int len_buffer);

	//返回主站ID
	virtual unsigned char get_host_id() { return 0; }; // 不需要实现

	unsigned char get_ctrl_code_area();
	void set_ctrl_code_area(unsigned char ctrl);

	unsigned char get_repeat_flag();
	void set_repeat_flag(unsigned char flag);

	unsigned char get_mult_first();
	void set_mult_first(unsigned char first);

	unsigned char get_mult_flag();
	void set_mult_flag(unsigned char flag);

	unsigned char get_afn();
	void set_afn(unsigned char afn); 


protected:
	//报文头结构体
#pragma pack(push, 1)
	struct frame_header_struct
	{
		unsigned char frame_start;//0XEE
		unsigned char frame_fill;
		union 
		{
			struct
			{
				unsigned char	ctrl_fill		: 5 ,//D0--D4预留
								repeat_flag		: 1 ,//帧计数位，用于防止分组被重复接收。每当发送新分组时，该位取反；而对于重发的分组，该位状态不变。
								mult_first		: 1 ,//1-表示该分组是多分组传输的第一个分组  
								mult_flag		: 1 ;//1:表示该分组是多分组传输的一部分 
			};
			unsigned char ctrl_code_area;
		};
		unsigned char frame_seq;
		unsigned short int len_data_area;  
		unsigned char afn;
	};
#pragma pack(pop)

	frame_header_struct *frame_header_;
};