#include "access_security.h"
#include "encry_context.h"
#include <ptcl_codec/ptcl_module_dlms47.h>
#include <cryptopp/osrng.h>  
#include <cryptopp/hex.h>
#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1  
#include <cryptopp/md5.h>
#include <cryptopp/aes.h>
#include <cryptopp/gcm.h>
#include <logger\logger.h>
#include "common.h"
#include "meter_handler.h"
#include "simulator_config.h"

using namespace CryptoPP;



access_security::access_security(meter_handler* mh)
	:meter_handler_(mh),
	
	state_(INIT),
	cipher_flag_(false), 
	authen_mech_id_(LOWEST_LS_MECH),
	sc_(0),
	association_result_(true),
	client_id_(1),
	invoke_id_(0xff)
{
	data_security_ = mh->get_data_security();
}

access_security::~access_security(void)
{
}

void access_security::reset_state(State st)
{
	state_ = st;
}

/****************************************************************************/
/*description:根据cipher_flag生成加密/不加密的init_response字符串					*/
/*input:																	*/
/*		none. access_security object.										*/
/*output:																	*/
/*		out_irq，output init_response message	,alway an array[256]			*/
/*		function return: length of the output init_rsp_len					*/
/****************************************************************************/
unsigned int access_security::build_init_response(unsigned char* out_irq)
{
	//orig init_request
	unsigned char init_response[] = {
		0x08,0x00,0x06,
		0x5f,0x1f,0x04,0x00,0x00,
		0x50,0x1F,0x01,0xF4, 0x00, 0x07}; //14 bytes with no dedicated-key
	unsigned int init_rsp_len = 14;

	//not with cipher data
	if(cipher_flag_ == false)
	{
		memcpy(out_irq,init_response,init_rsp_len);
		return init_rsp_len;
	}

	//with cipher data
	std::string oct_ori_irq = Common::Util_tools::bytes_to_hex(init_response,init_rsp_len);
	std::string oct_cipher_irq;
	std::string oct_authen_tag;

	//authen and encryption
	//byte sc = Security::Authentication_FLAG | Security::Encryption_FLAG;
	unsigned int ic = meter_handler_->request_next_client_ic();
	data_security_->set_security_control(sc_);
	data_security_->set_init_vector(meter_handler_->get_server_systitle(),ic);

	unsigned char iv[16] = {0};
	memcpy(iv,meter_handler_->get_server_systitle(),SYSTITLE_LEN);
	unsigned int ic1 = Common::Util_tools::reverse_value(ic);
	memcpy(iv + SYSTITLE_LEN,&ic1,IC_LEN);

	if(! data_security_->encrypt(oct_ori_irq,oct_cipher_irq,oct_authen_tag,sc_,iv))
	{
		return 0;
	}

	//output bytes string cipher init request
	unsigned char TAG = Common::Dlms::glo_InitiateResponse;
	unsigned char bytes_cipher_irq[256] = {0};
	unsigned int c_len = 0;
	unsigned char bytes_authen_tag[64] = {0};
	unsigned int tag_len = 0;
	Common::Util_tools::hex_to_bytes(oct_cipher_irq,bytes_cipher_irq,c_len);
	Common::Util_tools::hex_to_bytes(oct_authen_tag,bytes_authen_tag,tag_len);
	unsigned int rev_ic = Common::Util_tools::reverse_value(ic);

	unsigned int pos = 0;
	out_irq[pos++] = TAG;
	out_irq[pos++] = SC_LEN + IC_LEN + c_len + tag_len;
	out_irq[pos++] = sc_;
	memcpy(out_irq + pos,&(rev_ic),IC_LEN);
	pos = pos + 4;
	memcpy(out_irq + pos,bytes_cipher_irq,c_len);
	pos = pos + c_len;
	memcpy(out_irq + pos,bytes_authen_tag,tag_len);
	pos = pos + tag_len;
	unsigned int total_len = pos;

	return total_len;
}


bool access_security::parse_aarq(ptcl_packet_dlms47 &packet,bool& associate_result,bool &cipher_flag, 
								 byte &mechanism_id,unsigned char* req_ap_title,unsigned int &ap_title_len, 
								 unsigned char* req_authen_value,unsigned int &authen_len)
{

	unsigned char *cosem_data = packet.get_data_area_buffer();

	//提取AARQ消息中的数据
	unsigned int pos = 0;
	if(cosem_data[pos++] != 0x60)
	{
		return false;
	}
	unsigned int aarq_content_len = cosem_data[pos++];
	if(aarq_content_len < 29) 
	{
		return false;
	}

	unsigned char *component_start = cosem_data + pos; 
	pos = 0;
	unsigned int read_len = 0;
	byte tag;
	byte comp_len;
	byte encode_choice;
	byte obj_value_len;

	//output results
	byte context_id;
	unsigned char* obj_buffer = NULL;

	unsigned char *component_ptr = component_start;
	while(aarq_content_len - read_len > 0)
	{
		aa_component_header* comp_ptr = (aa_component_header*)(component_ptr);
		obj_buffer = (unsigned char*)comp_ptr + AA_COMP_HEADER_LEN;

		switch (comp_ptr->tag)
		{
		case AA_COMP_HEADER::AARQ_Application_Context_Name:
			/********************************************************************/
			/* COSEM_Application_Context_Name ::={joint-iso-ccitt(2) country(16)*/
			/* country-name(756) identified-organisation(5) DLMS-UA(8) 			*/
			/* application-context(1)context_id(x)}								*/
			/* BER Encoding value like:{0x60,0x85,0x74,0x05,0x08,0x01,0x01}		*/
			/* Green Manual Book page 398										*/
			/********************************************************************/
			context_id = *(obj_buffer  + comp_ptr->obj_value_len - 1);
			if(context_id == 3)
			{
				cipher_flag = true;
				cipher_flag_ = true;
			}
			else
			{
				cipher_flag = false;
				cipher_flag_ = false;
			}
			break;
		case AA_COMP_HEADER::AARQ_Mechanism_Name:
			/********************************************************************/
			/* COSEM_Authentication_Mechanism_Name ::={joint-iso-ccitt(2)		*/
			/* country(16)country-name(756)identified-organization(5) DLMS-UA(8)*/
			/* authentication_mechanism_name(2) mechanism_id(x)}				*/
			/* BER Encoding value like:{0x60,0x85,0x74,0x05,0x08,0x02,0x01}		*/
			/* Green Manual Book page 255										*/
			/********************************************************************/
			mechanism_id = *(obj_buffer + 4);
			break;

		case AA_COMP_HEADER::AARQ_Calling_AP_Title:
			ap_title_len = comp_ptr->obj_value_len;
			if(ap_title_len != SYSTITLE_LEN)
			{
				return false;
			}
			memcpy(req_ap_title,obj_buffer,ap_title_len);
			break;

		case AA_COMP_HEADER::AARQ_Calling_Authen_Value:
			authen_len = comp_ptr->obj_value_len;
			if(authen_len > MAX_AUTHEN_LEN)
			{
				return false;
			}
			memcpy(req_authen_value,obj_buffer,authen_len);
			break;

		case AA_COMP_HEADER::AARQ_User_Information:
			if (*(obj_buffer) == 0x21)
			{
				sc_ = *(obj_buffer + 2);
				memcpy(&ic_, obj_buffer + 3, 4);
			}
			break;
		case AA_COMP_HEADER::AARQ_Sender_ACSE_Requirements:
		case AA_COMP_HEADER::AARQ_Called_AP_Title:
		case AA_COMP_HEADER::AARQ_Called_AE_Invocation_ID:
		case AA_COMP_HEADER::AARQ_Called_AE_Qualifier:
		case AA_COMP_HEADER::AARQ_Called_AP_Invocation_ID:
		case AA_COMP_HEADER::AARQ_Calling_AE_Invocation_ID:
		case AA_COMP_HEADER::AARQ_Calling_AE_Qualifier:
		case AA_COMP_HEADER::AARQ_Calling_AP_Invocation_ID:
		case AA_COMP_HEADER::AARQ_Implemenation_Information:
			break;

		default:
			return false;
			break;
		}


		read_len += sizeof(tag) + sizeof(comp_len) + comp_ptr->comp_len;
		component_ptr = component_start + read_len;
	}

	return true;
}

bool access_security::parse_rlrq(ptcl_packet_dlms47 &packet) 
{
	return true;
}

int access_security::proc_response(ptcl_packet_dlms47 &packet)
{
	bool associate_result = false;
	bool cipher_flag = false;
	byte mech_id = 0;
	unsigned char rsp_ap_title[SYSTITLE_LEN] = {0};
	unsigned int ap_title_len = 0;
	unsigned char rsp_authen_value[64] = {0};
	unsigned int authen_len = 0;

	client_id_ = packet.get_src_port();
	byte tag = packet.get_data_area_buffer()[0];

	switch (tag)
	{
	case Common::Dlms::AARQ:
	{
		if (!parse_aarq(packet, associate_result, cipher_flag, mech_id, rsp_ap_title, ap_title_len, rsp_authen_value, authen_len))
			return 1;

		authen_mech_id_ = Authen_Mechanism(mech_id);

		meter_handler_->set_client_systitle(rsp_ap_title);

		challenge_ctos_ = Common::Util_tools::bytes_to_hex(rsp_authen_value,authen_len);


		switch (mech_id)
		{
		case Common::Security::LOWEST_LS_MECH:		
		case Common::Security::LLS_MECH:
			state_ = LLS_AARE;
			break;
		case Common::Security::HLS_MECH:
		case Common::Security::HLS_MECH_MD5:
		case Common::Security::HLS_MECH_SHA_1:
		case Common::Security::HLS_MECH_GMAC:
		case Common::Security::HLS_MECH_SHA_256:
		case Common::Security::HLS_MECH_SHA_ECDSA:	
			if (state_ == INIT)
			{	
				std::string hex_challenge = Common::Util_tools::bytes_to_hex(rsp_authen_value,authen_len);
	
				//store the system_title of server
				meter_handler_->set_client_systitle(rsp_ap_title);

				//build authentication tag of hex_challenge 
				std::string oct_cipher_challenge;
				std::string oct_authen_stoc;
				byte sc = Security::Authentication_FLAG;
				unsigned ic = meter_handler_->request_next_client_ic();

				data_security_->set_security_control(sc);
				data_security_->set_init_vector(meter_handler_->get_server_systitle(),ic);
	
				unsigned char iv[16] = {0};
				memcpy(iv,meter_handler_->get_server_systitle(),SYSTITLE_LEN);
				unsigned int ic1 = Common::Util_tools::reverse_value(ic);
				memcpy(iv + SYSTITLE_LEN,&ic1,IC_LEN);

				//very important step
				/*bool en_ret = data_security_->encrypt(hex_challenge,oct_cipher_challenge,oct_authen_stoc);*/
				bool hls5authen_ret = data_security_->hls5authen(hex_challenge,oct_authen_stoc,sc,iv);

				unsigned int rev_ic = Common::Util_tools::reverse_value(ic);
				std::string oct_rev_ic = Common::Util_tools::bytes_to_hex((byte*)&rev_ic,sizeof(unsigned int));
	
				//fstoc = SC || IC || AUTHEN_TAG 
				fctos_ = "10"+ oct_rev_ic + oct_authen_stoc;


				state_ = PASS_2;
			}
			break;
		default:
			break;
		}	
	}
	break;
	case Common::Dlms::RLRQ:
	{
		if (!parse_rlrq(packet))
			return 0;
		state_ = RLRE;
	}
	break;
	case Common::Dlms::glo_ActionRequest:
	{
		unsigned char *cosem = packet.get_data_area_buffer();
		byte fstoc_len = cosem[1]; 
		unsigned char *fstoc = cosem + 7;


		unsigned int pos = 2;
		byte sc = cosem[pos++];
		unsigned int rev_ic = *(unsigned int*)(cosem + pos);
		unsigned int ic = Common::Util_tools::reverse_value(rev_ic);
		pos = pos + 4;

		//获取得到octet string的authenticated challenge.
		std::string oct_ori_chll_ctos = Common::Util_tools::bytes_to_hex(fstoc,fstoc_len-5-12);
		std::string oct_authen_chll_stoc = Common::Util_tools::bytes_to_hex(fstoc+fstoc_len-12-5, 12);

		data_security_->set_security_control(sc);
		data_security_->set_init_vector(meter_handler_->get_client_systitle(),ic);


		unsigned char iv[16] = {0};
		memcpy(iv,meter_handler_->get_client_systitle(),SYSTITLE_LEN);
		unsigned int ic1 = Common::Util_tools::reverse_value(ic);
		memcpy(iv + SYSTITLE_LEN,&ic1,IC_LEN);

		std::string oct_plain_text;
		bool check_result = false;
		if(!data_security_->decrypt(oct_ori_chll_ctos,oct_authen_chll_stoc,oct_plain_text,check_result,sc,iv))
		{
			associate_result = false;
		}

		unsigned char new_cosem[1024] = {0};
		unsigned int new_cosem_len = 0;
		Common::Util_tools::hex_to_bytes(oct_plain_text,new_cosem, new_cosem_len);

		invoke_id_ = new_cosem[2];
		client_id_ = packet.get_src_port();

		state_ = PASS_4;
	}
	break;
	case Common::Dlms::ActionRequest:
	{
		unsigned char *cosem = packet.get_data_area_buffer();
		if (cosem[0] != Common::Dlms::ActionRequest || cosem[3] != 0x00)
		{
			associate_result = false;
		}
		invoke_id_ = cosem[2];
		//如果足够严谨，应该使用特定的解析方法来读取后文中的各项数据。因为本数据长度目前来分析，不会超过127bytes
		//特别注意，如果长度大于127，那么计算方法就不是这样的，所以说这里存在隐患！！！ 2016-7-7 by liugh
		byte fstoc_len = cosem[7]; 
		unsigned char *fstoc = cosem + 8;

		unsigned int pos = 0;
		byte sc = fstoc[pos++];
		unsigned int rev_ic = *(unsigned int*)(fstoc + pos);
		unsigned int ic = Common::Util_tools::reverse_value(rev_ic);
		pos = pos + 4;

		//获取得到octet string的authenticated challenge.
		std::string oct_ori_chll_ctos = Common::Util_tools::bytes_to_hex((byte*)challenge_stoc_.c_str(),challenge_stoc_.length());
		std::string oct_authen_chll_stoc = Common::Util_tools::bytes_to_hex(fstoc + pos, fstoc_len - pos);

		data_security_->set_security_control(sc);
		data_security_->set_init_vector(meter_handler_->get_client_systitle(),ic);

		unsigned char iv[16] = {0};
		memcpy(iv,meter_handler_->get_client_systitle(),SYSTITLE_LEN);
		unsigned int ic1 = Common::Util_tools::reverse_value(ic);
		memcpy(iv + SYSTITLE_LEN,&ic1,IC_LEN);

		std::string oct_plain_text;
		bool check_result = false;
		if(!data_security_->decrypt(oct_ori_chll_ctos,oct_authen_chll_stoc,oct_plain_text,check_result,sc,iv))
		{
			associate_result = true;
		}

		state_ = PASS_4;	
	}
	break;
	default:
		return -1;
	}


	return 0;
}

int access_security::proc_request(ptcl_packet_dlms47 &packet)
{
	switch (state_)
	{
	case access_security::INIT:
		break;
	case access_security::PASS_2:
		get_hls_aare_pass2(packet);
		return 2;
	case access_security::PASS_4:
		get_hls_aare_pass4(packet);
		break;
	case access_security::LLS_AARE:
		get_lls_aare(packet);
		break;
	case access_security::RLRE:
		get_rlre(packet);
		break;
	default:
		break;
	}

	return 1;
}

bool access_security::get_lls_aare(ptcl_packet_dlms47 &packet)
{
	unsigned char init_response[256] = {0};
	unsigned int irq_len = build_init_response(init_response); 
	unsigned char *bytes_sys_title = meter_handler_->get_server_systitle();

	challenge_ctos_;
	std::string lls_pass = simulator_config::get_instance()->get_lls();

	unsigned char result = association_result_ ? 0 : 1;

	ptcl_module_dlms47::get_instance()->format_packet_lls_aare_ln(&packet,
		client_id_,
		cipher_flag_,
		authen_mech_id_,
		bytes_sys_title,
		SYSTITLE_LEN,
		init_response,irq_len, 
		result);
	state_ = RLRE;

	return true;
}

bool access_security::get_hls_aare_pass2(ptcl_packet_dlms47 &packet)
{
	bool req_ret = data_security_->request_random_challenge(challenge_stoc_);
	if(!req_ret)
	{
		return false;
	}

	unsigned char *bytes_systitle = meter_handler_->get_server_systitle();

	//build the init response
	unsigned char init_request[256] = {0};
	unsigned int irq_len = 0;
	irq_len = build_init_response(init_request);
	if(irq_len == 0)
	{
		return false;
	}

	unsigned char result = association_result_ ? 0 : 1;

	//build the aare
	ptcl_module_dlms47::get_instance()->format_packet_hls_aare_ln(&packet,
		client_id_,
		cipher_flag_,
		authen_mech_id_,
		bytes_systitle,
		SYSTITLE_LEN,
		init_request,
		irq_len,
		result,
		(char*)challenge_stoc_.c_str());

	return true;
}

bool access_security::get_hls_aare_pass4(ptcl_packet_dlms47 &packet)
{
	//unsigned char code = 0;
	//if (association_result_ == false)
	//{
	//	code = 2;
	//}
	//ptcl_module_dlms47::get_instance()->format_packet_action_response(&packet, client_id_, code, fctos_);

	//if (cipher_flag_)
	//{
	//	message_block in_mb;
	//	in_mb.copy(packet.get_packet_buffer(), packet.get_packet_length());
	//	message_block out_mb;
	//	meter_handler_->call_encrypt(1, in_mb, out_mb);
	//	packet.set_packet_buffer(out_mb.rd_ptr(), out_mb.length());
	//}

	ptcl_packet_dlms47 action_request;
	ptcl_module_dlms47::get_instance()->format_packet_pass4(&action_request,client_id_, 0, invoke_id_, fctos_);

	//do encrypt and authenticate,using sc = 0x30. and send the glo_ActionRequest to the meter.
	std::string oct_cipher_action_request;
	std::string oct_authen_tag;

	unsigned char buf_data_area[1024];
	memcpy(buf_data_area,
		action_request.get_data_area_buffer(),
		action_request.get_data_area_length());

	std::string plain_action_request = Common::Util_tools::bytes_to_hex(buf_data_area,
		action_request.get_data_area_length());

	byte sc = Security::Authentication_FLAG | Security::Encryption_FLAG;
	unsigned ic = meter_handler_->request_next_client_ic();
	data_security_->set_security_control(sc_);
	data_security_->set_init_vector(meter_handler_->get_server_systitle(),ic);

	unsigned char iv[16] = {0};
	memcpy(iv,meter_handler_->get_server_systitle(),SYSTITLE_LEN);
	unsigned int ic1 = Common::Util_tools::reverse_value(ic);
	memcpy(iv + SYSTITLE_LEN,&ic1,IC_LEN);
	
	bool enauth_ret = data_security_->encrypt(plain_action_request,oct_cipher_action_request,oct_authen_tag,sc,iv);


	//Auth and Encrypt APDU = TAG || LEN || SC-AE || IC || C || T 
	//SC-AE || IC || C || T in octet is as below
	//std::string auth_en_data = "30"+ oct_rev_ic + oct_cipher_action_request + oct_authen_tag;
	//gloActionRequest
	std::string auth_en_ActionRequest = oct_cipher_action_request + oct_authen_tag;
	ptcl_module_dlms47::get_instance()->format_packet_glo(&packet,Dlms::glo_ActionResponse,0x30,ic,auth_en_ActionRequest,client_id_);

	return true;
}

bool access_security::get_rlre(ptcl_packet_dlms47 &packet)
{
	unsigned char init_rsp[256];
	unsigned int init_rsp_len = 0;
	init_rsp_len = build_init_response(init_rsp);

	int use_flags = ptcl_module_dlms47::REASON_FIELD | ptcl_module_dlms47::USER_INFO_RLRE ;
	ptcl_module_dlms47::get_instance()->format_packet_rlre(&packet,
		client_id_,
		use_flags,
		0,			//reason is normal
		init_rsp,
		init_rsp_len);

	state_ = INIT;


	return true;
}


std::string access_security::request_random_challenge(const int chl_len)
{
	char rndword[128] = {0};
	CryptoPP::AutoSeededRandomPool rnd;
	//sprintf(rndword, "%08u", rnd.GenerateWord32(10000000, 99999999));
	for(int i = 0;i < chl_len; i++)
	{
		rndword[i] = rnd.GenerateByte();
		while( rndword[i] == 0 || rndword[i] == 255)
		{
			rndword[i] = rnd.GenerateByte();
		}
	}

	std::string result(rndword);

	return result;
}

int access_security::get_state()
{
	return state_;
}