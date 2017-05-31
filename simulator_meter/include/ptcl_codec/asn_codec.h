#pragma once

#include <vector>
#include <string>
#include "ptcl_codec_global.h"
#include "time.h"
#include <algorithm>
#include <stdio.h>
#include <stdint.h>


class PTCL_CODEC_EXPORT asn_codec
{
public:
	asn_codec(void);
	~asn_codec(void);

	enum asn_type
	{
		asn_type_univeral_boolean	 		=	1,
		asn_type_univeral_integer	 		=	2,
		asn_type_univeral_bitString	 		=	3,
		asn_type_univeral_octetString	 	=	4,
		asn_type_univeral_null	 			=	5,
		asn_type_univeral_objectIdentifer	=	6,
		asn_type_univeral_objectDescriptor	=	7,
		asn_type_univeral_enumerated	 	=	10,
		asn_type_univeral_sequence		 	=	16,
		asn_type_univeral_set	 			=	17,
		//字符串类型
		asn_type_univeral_utc_time			=	23,
		asn_type_univeral_generalized_time	=	24,
		asn_type_univeral_graphic_string	=	25,
		asn_type_univeral_visible_string	=	26,

		//构造性
		asn_type_sequence					=	11,
		asn_type_sequenceOf					=	12,
		asn_type_choise						=	13,
	};

	enum asn_tagging_class
	{
		asn_tagging_class_universal			=	0,
		asn_tagging_class_application		=	1,
		asn_tagging_class_context			=	2,
		asn_tagging_class_private			=	3,
	};

	enum asn_tagging_model
	{
		asn_tagging_model_explicit		=	1,
		asn_tagging_model_ixplicit		=	2,
	};

	enum asn_tagging_codec_type
	{
		asn_tagging_codec_type_primary		=	0,
		asn_tagging_codec_type_construct	=	1,
	};

	static int format_ber_object_indentifier(int class_asn_tagging,int tagging_value,
			int* v_oi,int count_oi,unsigned char *buffer);
	static int format_ber_bit_string(int class_asn_tagging,int tagging_value,
			const char* bit_string,unsigned char *buffer);
	static int format_ber_graphic_string(int class_asn_tagging,int tagging_value,
			char* graphic_string,unsigned char *buffer);
	static int format_ber_choice(int class_asn_tagging,int tagging_value,
			unsigned char *context_area,int len_context_area,unsigned char *buffer);

	struct PDU
	{
		unsigned char context_area[1024];
		int len_context_area;
	};
	static int format_ber_seqence(int class_asn_tagging,int tagging_value,
		PDU* v_pdu,int count_pdu,unsigned char *buffer);

	static int format_tagging_area(int class_asn_tagging,int type_asn_tagging_codec,int tagging_value,
			unsigned char *buffer);
	static int parse_tagging_area(unsigned char *buffer,
			int & class_asn_tagging,int & type_asn_tagging_codec,int & tagging_value);

	static int format_length_area(int length,unsigned char *buffer);
	static int parse_length_area(unsigned char *buffer,int & length);

	//"0101110" --> 一个字节对一位 首字节填不足8位的
	static int format_context_area_bit_string(const char* bit_string,unsigned char *buffer);
	//"12345678" -> 0x31 0x32 0x33 0x34 0x35 0x36 0x37 0x38 直接asii转
	static int format_context_area_graphic_string(char* graphic_string,unsigned char *buffer);

	//"04FEFF00" -> 0x04 0xFE 0xFF 0x00
	static int format_context_area_octet_string(char* octet_string,unsigned char *buffer);
	//0x04 0xFE 0xFF 0x00 ->"04FEFF00"
	static int parse_context_area_octet_string(unsigned char *buffer, int buf_len, std::string &str_value);

	static int format_context_area_obis(const char* obis_string,unsigned char *buffer);
	static int format_context_area_short(short s,unsigned char *buffer);
	static int format_context_area_int(int i,unsigned char *buffer);
	static int format_context_area_int64(__int64 i,unsigned char *buffer);
	static int format_context_area_float(float value,unsigned char *buffer);
	static int format_context_area_object_indentifier(int * v_oi,int count_oi,unsigned char *buffer);
	static int parse_context_area_object_indentifier(unsigned char *buffer,int length,int * v_oi,int &count_oi);

	//0x07 0xDF 0x01 0x08 0x04 0x0B 0x09 0x39 0xFF 0x80 0x00 0x00 -> time_t
	static int parse_date_time(unsigned char *buffer,unsigned int & time);
	//time_t -> 0x07 0xDF 0x01 0x08 0x04 0x0B 0x09 0x39 0xFF 0x80 0x00 0x00
	static int  format_date_time(time_t dt, unsigned char *buffer);

	static int parse_unsigned_double_long(unsigned char *buf,unsigned int len, double & d,int count_radix_point);

	//组data类型的头部
	static int  format_basic_data(unsigned char tid, unsigned int length, unsigned char *buffer);
	static int  format_basic_data(unsigned char tid, unsigned char *buffer);
	static int  format_array_data(unsigned int item_size, unsigned char *buffer);
	static int  format_struct_data(unsigned int item_size, unsigned char *buffer);

	//1.0.8.0.0.255 -> 0100080000FF
	static int format_obis_string(std::string str_obis, unsigned char *buffer);
	//0100080000FF -> 1.0.8.0.0.255
	static int parse_obis_string(unsigned char *buffer, std::string &str_obis);


	//add by ly for mass memory
	static int format_unsigned_double_long(unsigned int value, unsigned char *buffer);

	static int format_unsigned(unsigned char value, unsigned char *buffer);

	static int format_date_time_extra(time_t dt, unsigned char *buffer);

private:
	static unsigned char bcd_to_bin(unsigned char bcd);
};

namespace asn
{
enum asn_type
{
	ci_null,
	ci_sequence_of_array,
	ci_sequence_of_struct,
	ci_boolean,
	ci_bit_string,
	ci_integer32,
	ci_unsigned32,
	ci_floating_point,			//IMPLICIT OCTET STRING(SIZE(4))33----
	ci_octet_string,
	ci_visible_string,
	ci_gen_time,				// IMPLICIT GeneralizedTime
	ci_bcd,
	ci_integer8,
	ci_integer16,
	ci_unsigned8,
	ci_unsigned16,
	ci_integer64,
	ci_unsigned64,
	ci_enumerated,
	ci_float32,
	ci_float64,
	ci_datetime,
	//------------------------------------
	ci_cosem_class_id,
	ci_cosem_object_instance_id,
	ci_cosem_object_attribute_id,
	ci_invoke_id_and_priority,
	ci_attribute_descriptor_list_qty,
	ci_raw_data,
	//------------------------------------
	ci_aarq,
	ci_aare,
	ci_application_context_name,
	ci_acse_requirements,
	ci_mechanism_name,
	//-------------------------------------
	ci_cosem_pdu,
	ci_get_data_result,
	ci_data_access_result,
	ci_data,
	ci_datablock_g,
	ci_result,
	ci_cosem_attribute_descriptor,
	ci_selective_access_descriptor,
	ci_attribute_descriptor_list,
	ci_get_request,
	ci_get_request_normal,
	ci_get_request_next,
	ci_get_request_with_list, //--
	ci_get_response,
	ci_get_response_normal,
	ci_get_response_with_datablock,
	ci_get_response_with_list, //--
	ci_set_request,
	ci_set_request_normal,
	ci_set_request_with_first_datablock, //--
	ci_set_request_with_datablock,//--
	ci_set_request_with_list, //--
	ci_set_request_with_list_and_first_datablock,//--
	ci_set_response,
	ci_set_response_normal,
	ci_set_response_datablock,//--
	ci_set_response_last_datablock,//--
	ci_set_response_last_datablock_with_list,//--
	ci_set_response_with_list,//--
	ci_action_request,
	ci_action_request_normal,
	ci_action_request_next_pblock,
	ci_action_request_with_list,
	ci_action_request_with_first_pblock,
	ci_action_request_with_list_and_first_pblock,
	ci_action_reqeust_with_pblock,
	ci_action_response,
	ci_action_response_normal,
	ci_action_response_with_pblock,
	ci_action_response_with_list,
	ci_action_response_next_pblock,
};

enum data_access_result
{
	ar_success,
	ar_hardware_fault,
	ar_temporary_failure,
	ar_read_write_denied,
	ar_object_undefined,
	ar_object_class_inconsistent,
	ar_object_unavailable,
	ar_type_unmatched,
	ar_scope_of_access_violated,
	ar_data_block_unavailable,
	ar_long_get_aborted,
	ar_no_long_get_in_progress,
	ar_long_set_aborted,
	ar_no_long_set_in_progress,
	ar_other_reason
};

}

struct result_item
{
	unsigned int tid;
	char des[32];
	unsigned short length;
	unsigned char buf[1024];
};

class result_data
{
public:
	result_data(result_item data)
		:data_(data){}
	result_item &data(){return data_;}
	bool to_bool(){return (data_.buf[0] == 0 ? false : true);}
	float to_float()
	{
		unsigned char tmp[4];
		memcpy(tmp, data_.buf, 4);
		std::reverse(tmp, tmp+4);
		return *(float*)tmp;
	}
	char* to_string()
	{
		data_.buf[data_.length] = 0;
		return (char*)data_.buf;
	}
	unsigned char to_uint8() {return data_.buf[0];}
	unsigned short to_uint16() {return data_.buf[0]*0x100+data_.buf[1];}
	unsigned int to_uint32(){return data_.buf[0]*0x1000000+data_.buf[1]*0x10000+data_.buf[2]*0x100+data_.buf[3];}
	time_t to_time()
	{
		time_t result = 0;
		struct tm value = {data_.buf[7], data_.buf[6], data_.buf[5], data_.buf[3], data_.buf[2]-1,
			data_.buf[0]*256+data_.buf[1]-1900};

		//根据clock status字节中的各bit位判断是否夏令时
		if(data_.buf[11] == 0xFF)
		{
			value.tm_isdst = -1;
		}
		else if(data_.buf[11] & 128 )
		{
			value.tm_isdst = 1;
		}
		else
		{
			value.tm_isdst = 0;
		}

		result = mktime(&value);

		/* 经过何毅测试，发现并不需要叠加偏移量。修改之后，只支持meter和server在同一时区的应用场景，其他情景可能会出现问题。
		//2016-7-6 by liuguanhai.

		short deviation_minutes; //偏移的分钟数;
		unsigned char *dev_Low = (unsigned char*)(&deviation_minutes);
		unsigned char *dev_High = (unsigned char*)(&deviation_minutes) + 1;
		*dev_Low = data_.buf[10];
		*dev_High = data_.buf[9];

		if(deviation_minutes <= 720 && deviation_minutes >= -720)
		{
			//返回标准UTC时间 = location_time - 偏移秒数
			result = mktime(&value) - deviation_minutes*60;
		}
		else
		{
			result = mktime(&value);
		}*/

		return result;
	}
	std::string to_hex_string()
	{
		std::string hex_string = "";
		for (int i=0; i<data_.length; i++)
		{
			char tmp[4]={0};
			sprintf(tmp, "%02X", data_.buf[i]);
			hex_string += tmp;
		}
		return hex_string;
	}
	std::string to_obis_string()
	{
		char obis_buf[32] = {0};
		sprintf(obis_buf, "%d.%d.%d.%d.%d.%d", data_.buf[0], data_.buf[1], data_.buf[2], data_.buf[3], data_.buf[4], data_.buf[5]);
		return obis_buf;
	}

	std::string to_standard_obis_string()
	{
		char obis_buf[32] = {0};
		sprintf(obis_buf, "%d-%d:%d.%d.%d.%d", data_.buf[0], data_.buf[1], data_.buf[2], data_.buf[3], data_.buf[4], data_.buf[5]);
		return obis_buf;
	}

	std::string to_formatted_obis()
	{
		char obis_buf[32] = {0};
		sprintf(obis_buf, "%02X%02X%02X%02X%02X%02X", data_.buf[0], data_.buf[1], data_.buf[2], data_.buf[3], data_.buf[4], data_.buf[5]);
		return obis_buf;
	}

	uint64_t to_unint64()
	{
		uint64_t obis = 0;
		unsigned char* ptr = (unsigned char*)(&obis);

		unsigned char ll_data[8] = {0,0,data_.buf[0],data_.buf[1],data_.buf[2],data_.buf[3], data_.buf[4], data_.buf[5]};
		memcpy(ptr,ll_data,sizeof(uint64_t));
		std::reverse(ptr,ptr+sizeof(uint64_t));

		return obis;
	}
private:
	result_item data_;
};

typedef std::vector<result_data> result_set;
