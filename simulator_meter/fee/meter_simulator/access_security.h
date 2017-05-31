#pragma once

#include "encry_context.h"
#include <ptcl_codec/ptcl_packet_dlms47.h>
#include <cryptopp\config.h>
#include "common.h"
#include "data_security.h"


#define AA_COMP_HEADER_LEN 4
typedef struct aa_component_header
{

	enum TAG 
	{
		AARQ_Application_Context_Name	= 0xA1,
		AARQ_Called_AP_Title			= 0xA2,
		AARQ_Called_AE_Qualifier		= 0xA3,
		AARQ_Called_AP_Invocation_ID	= 0xA4,
		AARQ_Called_AE_Invocation_ID	= 0xA5,
		AARQ_Calling_AP_Title			= 0xA6,
		AARQ_Calling_AE_Qualifier		= 0xA7,
		AARQ_Calling_AP_Invocation_ID	= 0xA8,
		AARQ_Calling_AE_Invocation_ID	= 0xA9,
		AARQ_Sender_ACSE_Requirements	= 0x8A,
		AARQ_Mechanism_Name				= 0x8B,
		AARQ_Calling_Authen_Value		= 0xAC,
		AARQ_Implemenation_Information	= 0x9D,
		AARQ_User_Information			= 0xBE,

	};

	byte tag;
	byte comp_len;
	byte encode_choice;
	byte obj_value_len;

}AA_COMP_HEADER;

class meter_handler;
class access_security
{
public:
	enum State
	{
		/*when use role and ic,following six and the next four states are used*/
		INIT = 0,
		PRE_AARQ = 1,
		PRE_AARE = 2,
		PRE_REQUEST_IC = 3,
		PRE_RESPONSE_IC = 4,
		PRE_RLRQ = 5,
		PRE_RLRE = 6,

		/*standard hls security use following four states*/
		PASS_1 = 7,
		PASS_2 = 8,
		PASS_3 = 9,
		PASS_4 = 10,

		/*standard lls security use following two statues*/
		LLS_AARQ = 11,
		LLS_AARE = 12,

		RLRQ = 13,
		RLRE = 14,
	};
public:
	access_security(meter_handler *mh);
	virtual ~access_security(void);

	unsigned int build_init_response(unsigned char* out_irq);

	int proc_response(ptcl_packet_dlms47 &packet);

	int proc_request(ptcl_packet_dlms47 &packet);

	void reset_state(State st = INIT);

	int get_state();

protected:
	bool parse_aarq(ptcl_packet_dlms47 &packet,bool& associate_result,bool &cipher_flag,
		byte &mechanism_id,unsigned char* req_ap_title,unsigned int &ap_title_len,
		unsigned char* req_authen_value,unsigned int &authen_len);

	bool parse_rlrq(ptcl_packet_dlms47 &packet);

	bool get_lls_aare(ptcl_packet_dlms47 &packet);

	bool get_hls_aare_pass2(ptcl_packet_dlms47 &packet);

	bool get_hls_aare_pass4(ptcl_packet_dlms47 &packet);

	bool get_rlre(ptcl_packet_dlms47 &packet);

	std::string request_random_challenge(const int chl_len);

protected:
	State state_;
	bool cipher_flag_;
	Authen_Mechanism authen_mech_id_;
	meter_handler *meter_handler_;
	byte sc_;
	unsigned int ic_;
	bool association_result_;
	std::string challenge_stoc_;
	std::string challenge_ctos_;
	std::string fctos_;
	std::string fstoc_;
	int client_id_;
	int invoke_id_;
	data_security *data_security_;
};
