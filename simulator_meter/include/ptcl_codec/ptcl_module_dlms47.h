#pragma once

#include <ptcl_codec/ptcl_module.h>
#include <ptcl_codec/ptcl_packet_dlms47.h>
#include <ptcl_codec/ptcl_cosem_dlms.h>
#define APDU_MAX_LEN 1024
#define WRAP_APDU_MAX_LEN (APDU_MAX_LEN + 24)  //实际为APDU_MAX_LEN + 8 + 9,为了预留设大些

class PTCL_CODEC_EXPORT ptcl_module_dlms47 : public ptcl_module
{
protected:
	ptcl_module_dlms47(void);
	virtual ~ptcl_module_dlms47(void);

public:

	enum class_id
	{
		ci_data		= 1,//数据接口类
		ci_register = 3,//寄存器接口类
		ci_extended = 4,//扩展寄存器接口类
		ci_demand   = 5,//需量寄存器接口类
		ci_profile	= 7,//通用集接口类
		ci_clock	= 8,//时钟接口类
	};

	enum RLRQ_COMPON_FLAGS
	{
		REASON_FIELD = 1,
		USER_INFO_RLRQ = 2,
		USER_INFO_RLRE = 3,
	};

	enum AARQ_COMPON_FLAGS
	{
		PROTOCOL_VERSION =	1,
		APP_CONTEXT_NAME =	2,
		CALLING_AP_TITLE =	4,
		SENDER_ACSE_REQ =	8,
		MECHANISM_NAME =	16,
		CALLING_AHTHEN_VALUE = 32,
		USER_INFO_AARQ =	64,
	};

	struct get_data_result
	{
		unsigned char result_type;
		unsigned char buffer[1024];
		int length;
	};

	struct data_unit
	{
		unsigned char tid;
		unsigned char length;
		unsigned char buffer[256];
	};

	struct profile_unit
	{
		time_t dt;
		std::vector<data_unit> values;
	};

	static ptcl_module_dlms47 *get_instance();

	void format_packet_pure_aarq(ptcl_packet_dlms47 *packet,unsigned short host_id);

	void format_packet_pure_rlrq(ptcl_packet_dlms47 *packet,unsigned short host_id);

	void format_packet_cosem_connect(ptcl_packet_dlms47 *packet, int mech, char* psw);

	void format_packet_cosem_connect_with_security(ptcl_packet_dlms47 *packet, int mech, 
													unsigned char* sys_title,int len,char* psw);

	//LN情形下统一使用此接口创建AARQ消息 2016-7-18
	void format_packet_cosem_connect_LN(ptcl_packet_dlms47 *packet, unsigned short host_id,
													bool b_ciphering,int mech_id,unsigned char* sys_title,int len_1, 
													char* authen_value,unsigned char* init_request,int len_2);

	//SN情形下统一使用此接口,暂未实现
	void format_packet_cosem_connect_SN(ptcl_packet_dlms47 *packet, unsigned short host_id,
													bool b_ciphering,int mech_id,unsigned char* sys_title,int len_1, 
													char* authen_value,unsigned char* init_request,int len_2);

	void format_packet_cosem_disc(ptcl_packet_dlms47 *packet);

	//LN情形下统一使用此接口创建AARE消息 2016-7-18
	void format_packet_cosem_disc_LN(ptcl_packet_dlms47 *packet,unsigned short host_id,int use_flags,
													 unsigned char req_resson,unsigned char* init_request,int len);

	void format_packet_cosem(ptcl_packet_dlms47 *packet, int invokeid_priority);

	void fill_packet_cosem_get(ptcl_packet_dlms47 *packet, short obis_class, char* obis, unsigned char attr_id);

	void fill_packet_cosem_get(ptcl_packet_dlms47 *packet, char* obis, time_t start, time_t end);

	void fill_packet_cosem_get(ptcl_packet_dlms47 *packet, char* obis, time_t start, time_t end, unsigned char type);

	void fill_packet_cosem_next(ptcl_packet_dlms47 *packet, unsigned int block_no);

	void fill_packet_cosem_set(ptcl_packet_dlms47 *packet, 
		short obis_class, 
		char* obis, 
		unsigned char attr_id, 
		unsigned char data[], 
		int data_len, 
		int data_type);

	void fill_packet_cosem_action(ptcl_packet_dlms47 *packet, short obis_class, char* obis, unsigned char method_id, 
		unsigned char data[], unsigned int data_len);

	void fill_packet_cosem_set(ptcl_packet_dlms47 *packet, short obis_class, char* obis, unsigned char attr_id, 
		unsigned char data[], int data_len);

	void format_packet_glo(ptcl_packet_dlms47 *packet, 
		unsigned char tag, 
		unsigned char sc, 
		unsigned int fc, 
		std::string str_data,
		unsigned int client_id);

	void format_packet_action_request(ptcl_packet_dlms47 *packet, std::string str_data);

	void format_packet_get_response(ptcl_packet_dlms47 *packet,unsigned int client_id, get_data_result *result, int invoke_id);

	void format_packet_get_response(ptcl_packet_dlms47 *packet,unsigned int client_id, std::vector<profile_unit> vec_unit, int invoke_id);

	void format_packet_get_response_with_list(ptcl_packet_dlms47 *packet,
															  unsigned int client_id, 
															  std::vector<get_data_result *> vec_result,
															  int invoke_id);

	void format_packet_get_response_with_list(ptcl_packet_dlms47 *packet,
															  unsigned int client_id, 
															  std::vector<std::vector<profile_unit >> vec_vec_unit,
															  int invoke_id);

	void fill_packet_cosem_set_filter(ptcl_packet_dlms47 *packet, unsigned int start_time, unsigned int end_time,
		unsigned int amount, unsigned int bitfield, char *obis, unsigned char mode = 2);

	void format_packet_lls_aare_ln(ptcl_packet_dlms47 *packet,unsigned int client_id,
		bool b_ciphering,unsigned char mech_id,unsigned char* sys_title,int sys_title_len, 
		unsigned char* init_response,int init_response_len, unsigned char association_result);

	void format_packet_hls_aare_ln(ptcl_packet_dlms47 *packet,unsigned int client_id,
		bool b_ciphering,unsigned char mech_id,unsigned char* sys_title,int sys_title_len, 
		unsigned char* init_response,int init_response_len, unsigned char association_result,
		char *authen_value);

	void format_packet_action_response(ptcl_packet_dlms47 *packet,int error_code, unsigned int client_id, get_data_result *result, int invoke_id);

	void format_packet_rlre(ptcl_packet_dlms47 *packet,unsigned int client_id,int use_flags,
		unsigned char rsp_resson,unsigned char* init_response,int len);

	void format_packet_pass4(ptcl_packet_dlms47 *packet,unsigned int client_id, unsigned char error_code, int invoke_id, std::string str_data);

protected:
	static ptcl_module_dlms47 *instance_;
};