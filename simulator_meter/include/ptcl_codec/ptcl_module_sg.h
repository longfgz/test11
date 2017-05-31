#pragma once

#include <ptcl_codec/ptcl_module.h>
#include <ptcl_codec/ptcl_packet_sg.h>

class PTCL_CODEC_EXPORT ptcl_module_sg : public ptcl_module
{
protected:
	ptcl_module_sg(void);
	virtual ~ptcl_module_sg(void);
public:
	//应用层功能码定义
	enum afn_code
	{
		AFN_ACK						=	0x00, //确认 否认
		AFN_RESET					=	0x01, //复位
		AFN_CONNECT_TEST			=	0x02, //链路接口检测(登陆,心跳,注销)
		AFN_RELAY_STATION_COMMAND	=	0x03, //中继站命令
		AFN_SET_PARAM				=	0x04, //设置参数
		AFN_COMMAND					=	0x05, //控制命令
		AFN_ID_PSW					=	0x06, //身份确认 和 密码协商
		AFN_READ_PARAM				=	0x0A, //读取参数
		AFN_READ_DATA_REALTIME		=	0x0C, //读1类数据（实时数据）
		AFN_READ_DATA_HISTORY		=	0x0D, //读2类数据（历史数据）
		AFN_READ_DATA_EVENT			=	0x0E, //读3类数据（事件数据）
		AFN_FILE_TRANS				=	0x0F, //文件传输
		AFN_DATA_TRANSMIT			=	0x10, //数据转发
	};

	//数据单元
	struct pnfn_data
	{
		int pn;
		int fn;
	};


	static int format_pnfn(pnfn_data *pnfn,unsigned char *buffer);
	static int parse_pnfn(unsigned char *buffer,pnfn_data *pnfn);

	//事件计数器
	struct ec_data
	{
		unsigned char important_ec;//重要事件计数器
		unsigned char generic_ec;//普通事件计数器
	};
	//事件计数器
	static int format_ec(ec_data *ec,unsigned char *buffer);
	static int parse_ec(unsigned char *buffer,ec_data *ec);

	//时间标识
	struct tp_data
	{
		unsigned char pfc;//启动帧帧序号计数器
		unsigned char sec;//秒
		unsigned char min;//分
		unsigned char hour;//时
		unsigned char day;//日
		unsigned char time_out;//超时时间
	};
	//时间标签
	static int format_tp(tp_data *tp,unsigned char *buffer);
	static int parse_tp(unsigned char *buffer,tp_data *tp);

	//数据单元结构
	struct pnfn_struct
	{
		unsigned char pn[2];
		unsigned char fn[2];
	};

	int get_aux_len();

	static ptcl_module_sg *get_instance();

	////读任务数据///////////////////////////////////////////////////////////////////////
	struct pnfn_data_uint
	{
		pnfn_data pnfn;
		unsigned char data_buffer[1024];
		int len_data;
	};
	void format_packet_read_data_history(	ptcl_packet_sg *packet,
											pnfn_data_uint* v_data_uint,int count_data_uint,
											tp_data *tp = NULL);
	/*
	packet: 只需要填写部分成员 终端地址
			 并且返回组装好的报文
	v_data_uint:数据单元集合
	count_data_uint:数据单元个数
	*/


	//确认帧//////////////////////////////////////////////////////////////////////////////
	void format_packet_confirm_frame(	ptcl_packet_sg *packet);
	/*
	packet: 只需要填写部分成员 终端地址
			 并且返回组装好的报文
	*/

	//对时//////////////////////////////////////////////////////////////////////////////
	void format_packet_set_param(	ptcl_packet_sg *packet,
									pnfn_data_uint* v_data_uint,
									int count_data_uint,
									unsigned char *psw,
									tp_data *tp = NULL
									);
	/*
	packet: 只需要填写部分成员 终端地址
			 并且返回组装好的报文
	*/
	/////写参数//////////////////////////////////////////////////////////////////////////////
	void format_packet_write_param(	ptcl_packet_sg *packet,
									pnfn_data_uint* v_data_uint,
									int count_data_uint,
									unsigned char *psw,
									tp_data *tp = NULL
									);
	/*
	packet: 只需要填写部分成员 终端地址
			 并且返回组装好的报文
	*/
	//数据转发//////////////////////////////////////////////////////////////////////////////
	void format_packet_transmit(	ptcl_packet_sg *packet,
									pnfn_data_uint &data_uint,
									unsigned char *psw,
									tp_data *tp = NULL
									);
	/*
	packet: 只需要填写部分成员 终端地址
			 并且返回组装好的报文
	*/
	//数据随抄//////////////////////////////////////////////////////////////////////////////
	void format_packet_rand_data(
									ptcl_packet_sg *packet, 
									pnfn_data_uint* v_data_uint,
									int count_data_uint,
									tp_data *tp = NULL);
	/*
	packet: 只需要填写部分成员 终端地址
			 并且返回组装好的报文
	*/
	
	//response for event push
	void format_packet_confirm_event_push(	ptcl_packet_sg *packet, 
											unsigned char afn, 
											unsigned char seq, 
											pnfn_data_uint &data_uint,
											tp_data *tp = NULL
											);
	

protected:
	static ptcl_module_sg *instance_;


};
