#include "meter_handler.h"
#include <logger/logger.h>
#include <ptcl_codec/ptcl_packet_in.h>
#include <ptcl_codec/ptcl_packet_dlms47.h>
#include <ptcl_codec/ptcl_module_dlms47.h>
#include "common.h"
#include "data_security.h"
#include "object_transfer.h"

meter_handler::meter_handler(unsigned char* systitle,unsigned int meter_no,
	boost::shared_ptr<session_channel_handler> handler, boost::asio::io_service &ios):
strand_(ios),
id_(meter_no),
channel_handler_(handler),
invokeid_(0),
handler_state_(mhs_null),
removed_(0),
access_security_(NULL),
data_security_(NULL),
client_cur_use_ic_(0),
is_cipher_(false),
its_state_(image_trans_initiated)
{

	memcpy(server_systitle_,systitle,SYSTITLE_LEN);
	memset(client_systitle_,0,SYSTITLE_LEN);

	data_security_ = new data_security();

	access_security_ = new access_security(this);
}

meter_handler::~meter_handler()
{
	if(access_security_ != NULL)
	{
		delete access_security_;
	}

	if(data_security_ != NULL)
	{
		delete data_security_;
	}
	
}

void meter_handler::reset_state(meter_handler_state st)
{
	handler_state_ = st;
}

bool meter_handler::process_cosem_connect(ptcl_packet_dlms47 &packet)
{
	access_security_->proc_response(packet);

	ptcl_packet_dlms47 s_packet;
	s_packet.reset();
	s_packet.set_device_addr(client_host_id_);

	int ret = access_security_->proc_request(s_packet);
	if(ret == 1)
	{
		access_security_->reset_state();
		handler_state_ = mhs_data;
	}	
	else
		handler_state_ = mhs_cosem_connect;
	
	if (packet.get_packet_length() > 0)
	{
		send_packet(s_packet);
	}
	return true;
}

bool meter_handler::process_cosem_disconnect(ptcl_packet_dlms47 &packet)
{
	access_security_->proc_response(packet);

	ptcl_packet_dlms47 s_packet;
	s_packet.reset();
	s_packet.set_device_addr(client_host_id_);

	int ret = access_security_->proc_request(s_packet);
	if(ret <= 1)
	{
		access_security_->reset_state();
	}	

	if (packet.get_packet_length() > 0)
	{
		send_packet(s_packet);
	}
	//handler_state_ = mhs_null;
	return false;
	 
}

bool meter_handler::process_data(ptcl_packet_dlms47 &packet, message_block* mb)
{
	unsigned char *cosem_data = packet.get_data_area_buffer();

	
	int src_tag = cosem_data[0];
	
	unsigned int client_id = packet.get_src_port();

	ptcl_packet_dlms47 s_packet;
	switch (src_tag)
	{
	case Common::Dlms::GetRequest:
		{
			ptcl_cosem_request request;
			if (!request.parse_block(packet.get_data_area_buffer(), packet.get_data_area_length()))
				return false;

			invokeid_ = packet.get_data_area_buffer()[2];
			
			int request_type = packet.get_data_area_buffer()[1];
			if (request_type == 1)
			{
				int class_id = request.get_class_id();
				std::string obis = request.get_obis();
				int attr_id = request.get_attr_id();
				int is_selector = request.is_access_selector();
				unsigned int start_time = 0;
				unsigned int end_time = 0;
				std::vector<ptcl_module_dlms47::profile_unit> v_unit;
				ptcl_module_dlms47::get_data_result data_result ;
				if (is_selector > 0)
				{
					start_time = request.get_start_time();
					end_time =  request.get_end_time();
				}

				bool ret = object_transfer::get_instance()->format_get_normal_response(obis, class_id, attr_id, start_time, end_time, its_state_, data_result, v_unit);

				if (ret)
					if (v_unit.size() > 0)
						ptcl_module_dlms47::get_instance()->format_packet_get_response(&s_packet,client_id, v_unit, invokeid_);
					else
						ptcl_module_dlms47::get_instance()->format_packet_get_response(&s_packet,client_id, &data_result, invokeid_);
				else
					ptcl_module_dlms47::get_instance()->format_packet_get_response(&s_packet,client_id, NULL, invokeid_);
			}
			else if (request_type == 3)
			{
				std::vector<ptcl_cosem_request::cosem_methoed_desc> v_cosem_desc;
				request.get_list_data(v_cosem_desc);

				if (v_cosem_desc.size() == 0)
					return false;

				std::vector<std::vector<ptcl_module_dlms47::profile_unit >> v_v_unit;
				std::vector<ptcl_module_dlms47::get_data_result*> v_data_result;

				for (ptcl_cosem_request::cosem_methoed_desc cosem_desc : v_cosem_desc)
				{
					int class_id = cosem_desc.class_id;
					std::string obis = cosem_desc.obis;
					int attr_id = cosem_desc.attr_id;
					bool is_selector = cosem_desc.access_selector;
					unsigned int start_time = 0;
					unsigned int end_time = 0;
					std::vector<ptcl_module_dlms47::profile_unit> v_unit;
					ptcl_module_dlms47::get_data_result data_result ;
					if (is_selector)
					{
						start_time = cosem_desc.start_time;
						end_time =  cosem_desc.end_time;
					}

					bool ret = object_transfer::get_instance()->format_get_normal_response(obis, class_id, attr_id, start_time, end_time, its_state_, data_result, v_unit);

					if (ret)
						if (v_unit.size() > 0)
							v_v_unit.push_back(v_unit);
						else
							v_data_result.push_back(&data_result);
					else
						v_data_result.push_back(NULL);
				}
			
				if (v_v_unit.size() > 0)
					ptcl_module_dlms47::get_instance()->format_packet_get_response_with_list(&s_packet,client_id, v_v_unit, invokeid_);

				if (v_data_result.size() > 0)
					ptcl_module_dlms47::get_instance()->format_packet_get_response_with_list(&s_packet,client_id, v_data_result, invokeid_);
								
			}
		}	
		break;
	case Common::Dlms::ActionRequest:
		{
			ptcl_cosem_request request;
			if (!request.parse_block(packet.get_data_area_buffer(), packet.get_data_area_length()))
				return false;

			invokeid_ = packet.get_data_area_buffer()[2];

			int class_id = request.get_class_id();
			std::string obis = request.get_obis();
			int attr_id = request.get_attr_id();

			ptcl_module_dlms47::get_data_result data_result ;


			bool ret = object_transfer::get_instance()->format_action_normal_response(obis, class_id, attr_id, its_state_, data_result);

			if (ret)
			{
				ptcl_module_dlms47::get_instance()->format_packet_action_response(&s_packet,0, client_id, NULL, invokeid_);	
			}
			else
				ptcl_module_dlms47::get_instance()->format_packet_action_response(&s_packet,3, client_id, NULL, invokeid_);	
			
		}
		break;
	default:
		return false;
	}

	if (s_packet.get_packet_length() > 0)
	{
		message_block send_mb;
		send_mb.copy(s_packet.get_packet_buffer(), s_packet.get_packet_length());

		if (is_cipher_)
		{
			if(!call_encrypt(term_addr_,send_mb, send_mb))
			{
				//handler_state_ = mhs_null;
				return 0;
			}
		}

		channel_handler_->put(send_mb);
		return true;
	}

	return false;
}

unsigned char meter_handler::get_invokeid()
{
	return invokeid_;
}

bool meter_handler::handle_response(message_block* mb)
{

	ptcl_packet_dlms47 packet(0);
	packet.set_packet_buffer((unsigned char *)mb->rd_ptr(),(int)mb->length());
	if (packet.parse_packet() <= 0)
	{
		return false;
	}

	client_host_id_ = packet.get_host_id();

	is_cipher_ = false;
	int src_tag = packet.get_data_area_buffer()[0];
	if (src_tag >= Common::Dlms::glo_GetRequest && src_tag <=Common::Dlms::glo_ActionResponse && handler_state_ == mhs_data) 
	{
		ptcl_packet_dlms47 packPlain;
		message_block new_mb;
		//new_mb.copy(packet.get_packet_buffer(), packet.get_packet_length());
		is_cipher_ = true;
		//do the decrypt
		if(!call_decrypt(term_addr_,*mb,new_mb))
		{
			return false;
		}
		packPlain.set_src_port(packet.get_src_port());
		packPlain.set_data_area_buffer(new_mb.rd_ptr()+8, new_mb.length()-8);
		
		return process_data(packPlain, &new_mb);
	}
	else if (src_tag == Common::Dlms::AARQ)
	{
		return process_cosem_connect(packet);
	}
	else if (src_tag == Common::Dlms::RLRQ)
		return process_cosem_disconnect(packet);
	else
	{
		if (handler_state_ == mhs_data)
		{
			return process_data(packet, mb);
		}
		else if (handler_state_ == mhs_cosem_connect)
			return process_cosem_connect(packet);
		else
			return false;
		
	}
	

	return false;
}


unsigned int meter_handler::get_hostid(unsigned int seq)
{
	if (map_seq_hid.find(seq) == map_seq_hid.end())
		return 0;
	return map_seq_hid[seq];
}

void meter_handler::send_packet(ptcl_packet_dlms47 &packet)
{
	message_block mb;
	mb.copy(packet.get_packet_buffer(), packet.get_packet_length());

	channel_handler_->put(mb);
}


void meter_handler::stop()
{
	if (!removed_)
	{
		removed_ = 1;
		channel_handler_->release_session();
	}
}



unsigned char* meter_handler::get_client_systitle()
{
	return client_systitle_;
}

unsigned char* meter_handler::get_server_systitle()
{
	return server_systitle_;
}

void meter_handler::set_client_systitle(unsigned char* systitle)
{
	memcpy(client_systitle_,systitle,SYSTITLE_LEN);
}

void meter_handler::set_server_systitle(unsigned char* systitle)
{
	memcpy(server_systitle_,systitle,SYSTITLE_LEN);
}

unsigned int meter_handler::get_terminal_addr()
{
	return term_addr_;
}


unsigned int meter_handler::get_client_cur_ic()
{
	return client_cur_use_ic_;
}

unsigned int meter_handler::get_server_last_ic()
{
	return server_last_use_ic_;
}

void meter_handler::set_client_cur_ic(unsigned int ic)
{
	client_cur_use_ic_ = ic;
}

void meter_handler::set_server_last_ic(unsigned int ic)
{
	server_last_use_ic_ = ic;
}

bool meter_handler::check_ic_is_validate(unsigned int ic,unsigned int last_use_ic)
{
	if(ic >= std::pow(2,32) - 1)
	{
		handler_state_ = mhs_disc;
		puts("check,handler_state set to disc");
		return false;
	}

	//invocation counter必须比前一次使用counter要大.
	if(ic <= last_use_ic)
	{
		return false;
	}

	return true;	
}

unsigned int meter_handler::request_next_client_ic()
{
	//返回client_cur_use_ic_+1,并设置为新的cur_use_ic_;
	return ++client_cur_use_ic_;
}


access_security* meter_handler::get_access_security()
{
	return access_security_;
}

data_security* meter_handler::get_data_security()
{
	return data_security_;
}

/****************************************************************************/
/*description: encrypt one plain message block to cipher message.			*/
/*input:																	*/
/*		id: terminal id;													*/
/*		orgin_mb: orignal plain message block.								*/
/*output:																	*/
/*		out_mb: message block with ciphered message.						*/
/*		fucntion return:													*/
/*				true: encrypt success.										*/
/*				false: encrypt failed.										*/
/****************************************************************************/
bool meter_handler::call_encrypt(unsigned int id,message_block &orgin_mb,message_block &out_mb)
{
	//first check the input
	if(orgin_mb.length() <= 8)
	{
		return false;
	}

	if(!is_cipher_)
	{
		out_mb.copy(orgin_mb.base(),orgin_mb.length_base());
		return true;
	}

	unsigned int ic = request_next_client_ic();
	unsigned char* data = orgin_mb.rd_ptr() + 8;
	unsigned int data_len = orgin_mb.length() - 8;
	byte service_tag = data[0];

	//何毅确认后续数据将只使用Encryption+Authentication结合的方式。删除依据业务类型进行配置sc的逻辑。
	//by liuguanhai 2016-7-27

	byte sc = Common::Security::Encryption_FLAG | Common::Security::Authentication_FLAG;

	/*switch(service_tag)
	{
		case Common::Dlms::GetRequest:
		case Common::Dlms::glo_GetResponse:
		case Common::Dlms::GetResponse:
			sc = sc | Common::Security::Encryption_FLAG;
			break;

		case Common::Dlms::SetRequest:
		case Common::Dlms::glo_SetResponse:
		case Common::Dlms::SetResponse:
			sc = sc | Common::Security::Encryption_FLAG | Common::Security::Authentication_FLAG;
			break;

		case Common::Dlms::ActionRequest:
		case Common::Dlms::glo_ActionResponse:
		case Common::Dlms::ActionResponse:
			sc = sc | Common::Security::Authentication_FLAG;
			break;

		default:
			return false;
			break;
	}*/

	data_security_->set_security_control(sc);
	data_security_->set_init_vector(server_systitle_,ic);

	unsigned char iv[16] = {0};
	memcpy(iv,server_systitle_,SYSTITLE_LEN);
	unsigned int ic1 = Common::Util_tools::reverse_value(ic);
	memcpy(iv + SYSTITLE_LEN,&ic1,IC_LEN);

	std::string oct_data = Common::Util_tools::bytes_to_hex(data,data_len);
	std::string oct_cipher,oct_auth_tag;
	if(!data_security_->encrypt(oct_data,oct_cipher,oct_auth_tag,sc,iv))
	{
		return false;
	}
	
	//get the result and renew the out_mb;
	byte out_Tag = 0;
	switch (service_tag)
	{
	case Common::Dlms::GetRequest:
		out_Tag = Common::Dlms::glo_GetRequest;
		break;
	case Common::Dlms::SetRequest:
		out_Tag = Common::Dlms::glo_SetRequest;
		break;
	case Common::Dlms::ActionRequest:
		out_Tag = Common::Dlms::glo_ActionRequest;
		break;
	case Common::Dlms::GetResponse:
		out_Tag = Common::Dlms::glo_GetResponse;
		break;
	case Common::Dlms::SetResponse:
		out_Tag = Common::Dlms::glo_SetResponse;
		break;
	case Common::Dlms::ActionResponse:
		out_Tag = Common::Dlms::glo_ActionResponse;
		break;
	default:
		return false;
		break;
	}

	unsigned char bytes_ciphertext[1024] = {0};
	unsigned int cipher_len = 0;
	unsigned char bytes_authen_tag[1024] = {0};
	unsigned int authentag_len = 0;

	unsigned short content_len = 0;
	unsigned char content_data[1200] = {0};
	
	if( (sc & Common::Security::Authentication_FLAG)&& (sc & Common::Security::Encryption_FLAG))
	{
		Common::Util_tools::hex_to_bytes(oct_cipher,bytes_ciphertext,cipher_len);
		Common::Util_tools::hex_to_bytes(oct_auth_tag,bytes_authen_tag,authentag_len);

		memcpy(content_data,bytes_ciphertext,cipher_len);
		memcpy(content_data+cipher_len,bytes_authen_tag,authentag_len);
		content_len =  cipher_len + authentag_len;
	}
	else if(sc & Common::Security::Authentication_FLAG)
	{
		Common::Util_tools::hex_to_bytes(oct_auth_tag,bytes_authen_tag,authentag_len);

		memcpy(content_data,data,data_len);
		memcpy(content_data + data_len, bytes_authen_tag, authentag_len);
		content_len = data_len + authentag_len;
	}
	else if(sc & Common::Security::Encryption_FLAG)
	{
		Common::Util_tools::hex_to_bytes(oct_cipher,bytes_ciphertext,cipher_len);
		memcpy(content_data,bytes_ciphertext,cipher_len);
		content_len = cipher_len;
	}
	else
	{
		return false;
	}


	unsigned char buffer[1024] = {0};
	unsigned int pos = 0;
	memcpy(buffer,orgin_mb.rd_ptr(),8);
	pos = pos + 8;
	memcpy(buffer + pos,&out_Tag,sizeof(byte));
	pos = pos + 1;

	//以下是实现那个特别别扭的变长ANSI编码，太坑爹了。
	int len_pos = pos;
	if(content_len + 6 <= 127)
	{
		buffer[len_pos] = content_len + 5;
		pos = pos + 1;
	}
	else if(127 < content_len + 6  && content_len + 6 <= 255)
	{
		buffer[len_pos] = 0x81;
		buffer[len_pos + 1] = content_len + 5;
		pos = pos + 2;
	}
	else if( 255 < content_len + 6 && content_len + 6 <= 65535)
	{
		buffer[len_pos] = 0x82;
		unsigned short rev_apdu_len = Common::Util_tools::reverse_value((unsigned short)(content_len + 5));
		memcpy(buffer + len_pos + 1, &rev_apdu_len, sizeof(unsigned short));

		pos = pos + 3;
	}

	memcpy(buffer + pos,&sc,sizeof(byte));
	pos = pos + 1;
	unsigned int reverse_ic = Common::Util_tools::reverse_value(ic);
	memcpy(buffer + pos,&reverse_ic,sizeof(unsigned int));

	pos = pos + 4;

	memcpy(buffer + pos,content_data,content_len);
	pos = pos + content_len;

	//buffer[7] = pos - 8;
	//这段代码很别扭，主要是为了填写正确mb中的len。
	unsigned int rev_len = Common::Util_tools::reverse_value(pos - 8);
	byte *ptr = (byte*)(&rev_len) + 2; //翻转后，数据放在了后面两位。
	memcpy(buffer + 6, ptr, sizeof(unsigned short));

	//长度信息已经在前面填好了。
	//buffer[len_pos] = pos - len_pos - 1;

	//at last,put the buffer into the out_mb.
	out_mb.reset();
	out_mb.copy(buffer,pos);
	
	return true;
}

/****************************************************************************/
/*description: decrypt one ciphered message block to plain message.			*/
/*input:																	*/
/*		id: terminal id;													*/
/*		orig_mb: orignal message block,always a ciphered message.			*/
/*output:																	*/
/*		out_mb: message block with plain message.							*/
/*		fucntion return:													*/
/*				true: decrypt success.										*/
/*				false: decrypt failed.										*/
/****************************************************************************/
bool meter_handler::call_decrypt(unsigned int id,message_block &orig_mb,message_block &out_mb)
{
	//first check the input message block.
	if(orig_mb.length() <= 15)
	{
		return false;
	}

	//if has no data security require,return immediately
	if(!is_cipher_)
	{
		out_mb.copy(orig_mb.base(),orig_mb.length_base());
		return true;
	}

	//point to the complete ciphered apdu
	unsigned char* cpl_ciphered_apdu = orig_mb.rd_ptr() + 8; 

	int pos = 0;
	unsigned char tag = cpl_ciphered_apdu[pos];
	pos = pos + 1;

	//do the ASN decode
	unsigned int apdu_len = orig_mb.length() - 9;
	unsigned short len = 0;
	if(apdu_len <= 127)
	{
		len = cpl_ciphered_apdu[pos];
		pos = pos + 1;
	}
	else if(127 < apdu_len  && apdu_len <= 255 )
	{
		pos = pos + 1;
		len = cpl_ciphered_apdu[pos];
		pos = pos + 1;
	}
	else if( 255 < apdu_len && apdu_len <= 65535 )
	{
		pos = pos + 1;
		len = Common::Util_tools::reverse_value(*((unsigned short*)(cpl_ciphered_apdu + pos)));
		pos = pos + 2;
	}

	byte sc = cpl_ciphered_apdu[pos];
	pos = pos + 1;
	unsigned int rev_ic = *((unsigned int*)(cpl_ciphered_apdu + pos)); //该ic为网络字节序，大端排序。
	unsigned int ic = Common::Util_tools::reverse_value(rev_ic);  //该host_ic为主机字节序，用于存储，并比较判断
	pos = pos + 4;
	unsigned char* payload = cpl_ciphered_apdu + pos;

	//change the payload to octet string ,and parse them to 2 parts: cipher and authen tag.
	unsigned int payload_len = len - SC_LEN - IC_LEN;
	std::string oct_payload = Common::Util_tools::bytes_to_hex(payload,payload_len);
	std::string oct_cipher,oct_authtag;
	unsigned int tag_len = DEFAULT_TAG_LEN;
	if(sc & Encryption_FLAG && sc & Authentication_FLAG)
	{
		oct_cipher = oct_payload.substr(0,2*payload_len - 2*tag_len);
		oct_authtag = oct_payload.substr(2*payload_len - 2*tag_len);
	}
	else if(sc & Encryption_FLAG)
	{
		oct_cipher = oct_payload;
		oct_authtag = "";
	}
	else if(sc & Authentication_FLAG)
	{
		oct_cipher = oct_payload.substr(0,2*payload_len - 2*tag_len);
		oct_authtag = oct_payload.substr(2*payload_len - 2*tag_len);
	}
	
	//important!!! do the decrypt step
	data_security_->set_security_control(sc);
	data_security_->set_init_vector(client_systitle_,ic);

	unsigned char iv[16] = {0};
	memcpy(iv,client_systitle_,SYSTITLE_LEN);
	unsigned int ic1 = Common::Util_tools::reverse_value(ic);
	memcpy(iv + SYSTITLE_LEN,&ic1,IC_LEN);
	
	std::string octet_plain;
	bool auth_result = false;
	if(!data_security_->decrypt(oct_cipher,oct_authtag,octet_plain,auth_result,sc,iv))
	{
		return false;
	}

	if(auth_result)
	{
		unsigned char apdu[APDU_MAX_LEN];
		unsigned int out_len = 0;
		Common::Util_tools::hex_to_bytes(octet_plain,apdu,out_len);
		
		//如果大于最大设定长度，则记录日志并返回
		if(out_len > APDU_MAX_LEN)
		{
			return false;
		}

		//注意此处1536是message block的最大长度了
		unsigned char buffer[1536] = {0};
		memcpy(buffer,orig_mb.rd_ptr(),8);
		memcpy(buffer+8,apdu,out_len);
		
		//这段代码很别扭，主要是为了填写正确mb中的len。
		unsigned int rever_len = Common::Util_tools::reverse_value(out_len);
		byte *ptr = (byte*)(&rever_len) + 2; //翻转后，数据放在了后面两位。
		memcpy(buffer + 6, ptr, sizeof(unsigned short));

		out_mb.reset();
		out_mb.copy((unsigned char*)buffer, 8+out_len);
	}
	else
	{
		logger::get_instance()->logger_info(1,"system","decrypt fail.DATA = [%s]",orig_mb.rd_ptr());
		return false;
	}

	return true;
}
