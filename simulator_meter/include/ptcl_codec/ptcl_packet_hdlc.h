#pragma once

#include <ptcl_codec/ptcl_packet.h>

class PTCL_CODEC_EXPORT ptcl_packet_hdlc : public ptcl_packet
{
public:
	ptcl_packet_hdlc(int packet_type = 1);
	virtual ~ptcl_packet_hdlc(void);

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


	unsigned char get_ctrl_code_area();
	void set_ctrl_code_area(unsigned char code);
	
	unsigned char get_cca_recv_seq();
	void set_cca_recv_seq(unsigned char seq);
	
	unsigned char get_cca_pf();
	void set_cca_pf(unsigned char pf);
	
	unsigned char get_cca_send_seq();
	void set_cca_send_seq(unsigned char seq);
	
	unsigned char get_cca_end();
	void set_cca_end(unsigned char end);

protected:
	//报文头结构体
#pragma pack(push, 1)
	struct frame_header_struct
	{
		unsigned char frame_start;//0x7E
		/*union 
		{
			struct
			{
				unsigned short int    len_frame	    :	 11 ,//D0--D10长度（除开头标记，尾标记）
									  fta_separator	:	 1 ,//D11 分段标记
									  frame_type	:	 4 ;//D12--D15 幁格式1010:格式3
			};//需要转换网络字节序
			unsigned short int frame_type_area;  
		};*/
		unsigned short int frame_type_area;  
		union 
		{
			struct
			{
				unsigned char	device_addr_dst[4];
				unsigned char   host_id_src;
			};
			struct
			{
				unsigned char	host_id_dst;	
				unsigned char	device_addr_src[4];
			};
		};
		union 
		{
			struct
			{
				unsigned char cca_end	 	:	 1 ,//D7 I时候0 其他时候1
							  cca_send_seq	:	 3 ,//D4--D6 发送序号
							  cca_pf		:	 1 ,//D3 查询/结束位
							  cca_recv_seq	:	 3 ;//D0--D2 接收序号
								 
								 
								 
			};
			unsigned char ctrl_code_area;
		};
		unsigned short int hcs;
	};
#pragma pack(pop)

	frame_header_struct *frame_header_;
	int is_send_packet_;

	unsigned short int get_crc(unsigned char* buffer,int length);
};
/*
class PTCL_CODEC_EXPORT ptcl_packet_hdlc : public ptcl_packet
{
public:
	ptcl_packet_hdlc(void);
	virtual ~ptcl_packet_hdlc(void);

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
};*/
