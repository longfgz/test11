#pragma once

#include "ptcl_codec_global.h"
#include <boost/unordered_map.hpp>


class PTCL_CODEC_EXPORT ptcl_module
{
public:
	ptcl_module(void);
	virtual ~ptcl_module(void);

	void set_host_id(unsigned char id);
	unsigned char get_host_id();

	//45 --> 0x45
	static unsigned char bin_to_bcd(unsigned char bin);

	//0x45 --> 45
	static unsigned char bcd_to_bin(unsigned char bcd);

	//报文0x12 0x34 0x56 0x78 (NNNNNN.NN) --> 785634.12
	//len最大为4 count_radix_point最大为8
	static double bcd_to_bin(unsigned char *buf,int len,int count_radix_point,unsigned char invalid = 0xff);

	//报文0x12 0x34 0x56 0x78 (NNNNNNNN) --> 78563412
	//len最大为4
	static int bcd_to_bin(unsigned char *buf,int len,unsigned char invalid = 0xff);

	//len可以为0
	static unsigned int bcd_to_byte(unsigned char *buf,int len,int pos = 0);

	enum time_type
	{
		//反序 [0]秒 [1]分 [2]时 [3]日 [4]月 [5]年
		time_type_ssmmhhddmmyy  = 1,
		time_type_mmhhddmmyy    = 2,
		time_type_ddmmyy		= 3,
		time_type_mmhhddmm      = 4,
		time_type_hhddmmyy      = 5,
		time_type_mmyy          = 6,
		time_type_hhddmm		= 7,
		time_type_ssmmhhddwwmmyy = 8,
		time_type_mmhhdd        = 9,  
		
		//正序 [0]年 [1]月 [2]日 [3]时 [4]分 [5]秒
		time_type_yymmddhhmmss	= 21,
		time_type_yymmddhhmm	= 22,
		time_type_yymmdd		= 23,
		time_type_mmddhhmm		= 24,
		time_type_mmddhhmmss    = 25,
	};
	static int bcd_to_time(unsigned char *buf,int len,int type,tm *t);
	static int time_to_bcd(int type,tm *t,unsigned char *buf);

	static bool is_invalid_data(unsigned char *buffer,int len,unsigned char invalid = 0xff);

	static bool check_bcd(unsigned char *buffer, int len, unsigned char point = 0);

	//国电规约常用解析
	//采集时标
	//td_c
	static int format_td_c(tm *t,int m,int n,unsigned char *buffer);
	static int parse_td_c(unsigned char *buffer,int len,tm *t,int &m,int &n);
	//td_d
	static int format_td_d(tm *t,unsigned char *buffer);
	static int parse_td_d(unsigned char *buffer,int len,tm *t);
	//td_m
	static int format_td_m(tm *t,unsigned char *buffer);
	static int parse_td_m(unsigned char *buffer,int len,tm *t);

	//td_time对时时间格式
	static int format_td_time(tm *t, unsigned char *buffer);

	//标准格式数据
	static int parse_sg_1(unsigned char *buffer,int len,tm *t);

	static int parse_sg_2(unsigned char *buffer,int len,double & d);

	static int format_sg_3(int value,unsigned char *buffer,unsigned int offset);

	static int parse_sg_3(unsigned char *buffer,int len,double &d);

	static int parse_sg_4(unsigned char *buffer,int len,double & d);

	static int parse_sg_5(unsigned char *buffer,int len,double & d);
	
	static int parse_sg_6(unsigned char *buffer,int len,double & d);

	static int parse_sg_8(unsigned char *buffer,int len,double & d);

	static int parse_sg_7(unsigned char *buffer,int len,double & d);

	static int parse_sg_9(unsigned char *buffer,int len,double & d);

	static int parse_sg_10(unsigned char *buffer,int len,double & d);

	static int parse_sg_11(unsigned char *buffer,int len,double & d);

	static int parse_sg_12(unsigned char *buffer,int len,double & d);

	static int parse_sg_13(unsigned char *buffer,int len,double & d);

	static int parse_sg_14(unsigned char *buffer,int len,double & d);

	static int parse_sg_17(unsigned char *buffer,int len,tm *t);

	static int parse_sg_18(unsigned char *buffer,int len,tm *t);

	static int parse_sg_22(unsigned char *buffer,int len,double & d);

	static int parse_sg_23(unsigned char *buffer,int len,double & d);

	static int parse_sg_25(unsigned char *buffer,int len,double & d);

	static int format_sg_15(tm *t,unsigned char *buffer);
	static int parse_sg_15(unsigned char *buffer,int len,tm *t);

	static int format_sg_20(tm *t,unsigned char *buffer);
	static int parse_sg_20(unsigned char *buffer,int len,tm *t);

	static int format_sg_21(tm *t,unsigned char *buffer);
	static int parse_sg_21(unsigned char *buffer,int len,tm *t);

	static int parse_sg_24(unsigned char *buffer,int len,double & d,tm *t);

	static int parse_sg_32(unsigned char *buffer,int len,double & d,tm *t);

	static int parse_sg_33(unsigned char *buffer,int len,double & d,tm *t);

	static int format_sgdy04_td_c(tm *t,int day_num,int meter_no,unsigned char *buffer);

	static int format_sgdy04_td_d(tm *t,int day_num,int meter_no,unsigned char *buffer);

	static int format_sgdy04_td_m(tm *t,int mon_num,int meter_no,unsigned char *buffer);

	static int format_sg_31(tm *t,unsigned char *buffer);

protected:
	unsigned char host_id_;

	boost::unordered_map <unsigned int,unsigned char> map_seq_;

	virtual unsigned char get_frame_seq(unsigned int addr);
	virtual void reset_frame_seq(int addr);
};
