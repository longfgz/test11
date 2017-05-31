#pragma once

#include <ptcl_codec/ptcl_packet.h>

class PTCL_CODEC_EXPORT ptcl_packet_sg : public ptcl_packet
{
public:
	ptcl_packet_sg(void);
	virtual ~ptcl_packet_sg(void);

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
	void set_device_addr(unsigned int addr);

	//主站地址域
	unsigned char get_host_id_area();
	void set_host_id_area(unsigned char area);

	//主站地址
	void set_host_id(unsigned char id);

	//帧序号
	void set_frame_seq(unsigned char seq);

	//控制域功能码
	unsigned char get_afn_ctrl();
	void set_afn_ctrl(unsigned char afn);

	//帧计数有效位
	unsigned char get_fcv();
	void set_fcv(unsigned char fcv);

	//帧计数位
	unsigned char get_fcv_acd();
	void set_fcv_acd(unsigned char fcv_acd);

	//启动标志位
	unsigned char get_frm();
	void set_frm(unsigned char frm);

	//传输方向
	unsigned char get_dir();
	void set_dir(unsigned char dir);

	//功能码域
	unsigned char get_ctrl_code_area();
	void set_ctrl_code_area(unsigned char area);

	//终端组地址
	unsigned char get_group_addr();
	void set_group_addr(unsigned char group_addr);

	//应用层功能码
	unsigned char get_afn();
	void set_afn(unsigned char afn);
	
	//请求确认标志位
	unsigned char get_con();
	void set_con(unsigned char con);

	//结束帧标志
	unsigned char get_fin();
	void set_fin(unsigned char fin);

	//首帧标志
	unsigned char get_fri();
	void set_fri(unsigned char fri);

	//帧时间标签有效位
	unsigned char get_tpv();
	void set_tpv(unsigned char tpv);

	//桢序列字段
	unsigned char get_seq_area();
	void set_seq_area(unsigned char area);


	void set_ptcl_flag(unsigned char flag);

	unsigned char get_ptcl_flag();
	////数据单元
	//struct pnfn_data
	//{
	//	int pn;
	//	int fn;
	//};


	//static int format_pnfn(pnfn_data *pnfn,unsigned char *buffer,bool is_sg04 = false);
	//static int parse_pnfn(unsigned char *buffer,pnfn_data *pnfn,bool is_sg04 = false);

	////事件计数器
	//struct ec_data
	//{
	//	unsigned char important_ec;//重要事件计数器
	//	unsigned char generic_ec;//普通事件计数器
	//};
	////事件计数器
	//static int format_ec(ec_data *ec,unsigned char *buffer);
	//static int parse_ec(unsigned char *buffer,ec_data *ec);

	////时间标识
	//struct tp_data
	//{
	//	unsigned char pfc;//启动帧帧序号计数器
	//	unsigned char sec;//秒
	//	unsigned char min;//分
	//	unsigned char hour;//时
	//	unsigned char day;//日
	//	unsigned char time_out;//超时时间
	//};
	////时间标签
	//static int format_tp(tp_data *tp,unsigned char *buffer);
	//static int parse_tp(unsigned char *buffer,tp_data *tp);

	//int get_aux_len();

protected:
	//报文头结构体
#pragma pack(push, 1)
	struct frame_header_struct
	{
		unsigned char frame_start;//0x68
		union 
		{
			struct
			{
				unsigned short int    ptcl_flag		:	 2 ,//D0--D1规约标示
									  len_data_area	:	 14 ;//D2--D15长度（控制域、地址域、链路用户数据长度总数）
			};
			unsigned short int len_area;  
		};
		unsigned short int len_area_ex;
		unsigned char data_area_start;//0x68
		union 
		{
			struct
			{
				unsigned char	afn_ctrl : 4 ,//D0--D3控制域功能码
								fcv		 : 1 ,//D4帧计数有效位
								fcb_acd	 : 1 ,//D5帧计数位 或者 请求访问位
								frm		 : 1 ,//D6启动标志位
								dir		 : 1 ;//D7传输方向位
			};
			unsigned char ctrl_code_area;
		};
		unsigned char device_addr[4];//终端逻辑地址
		union 
		{
			struct
			{
				unsigned char   group_addr	:	 1 ,//D0 终端组地址
								host_id		:	 7 ;//D1-D7主站地址
			};
			unsigned char host_id_area;
		};
		unsigned char afn;
		union 
		{
			struct
			{
				unsigned char    frame_seq	:	 4 ,//D0--D3帧序号
								 con		:	 1 ,//D4请求确认标志位
								 fin		:	 1 ,//D5结束帧标志
								 fri		:	 1 ,//D6首帧标志
								 tpv		:	 1 ;//D7帧时间标签有效位
			};
			unsigned char seq_area;
		};
	};

	////数据单元结构
	//struct pnfn_struct
	//{
	//	unsigned char pn[2];
	//	unsigned char fn[2];
	//};

#pragma pack(pop)

	frame_header_struct *frame_header_;
};
