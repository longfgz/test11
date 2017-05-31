#include "data_security.h"
#include "simulator_config.h"
#include <logger/logger.h>
#include <cryptopp/osrng.h>  
//
//data_security *data_security::instance_ = NULL;
data_security::data_security()
{
	security_control_ = 0x0;
	memset(init_vector_,0,IV_LEN);
	security_suite_id_ = AES_GCM_128;
	memset(e_key_,0,EK_LEN);
	memset(a_key_,0,AK_LEN);

	unsigned char ek[32] = {0};
	unsigned char ak[32] = {0};
	unsigned int k_len = 0;
	Common::Util_tools::hex_to_bytes(simulator_config::get_instance()->get_ak(), ak, k_len);
	Common::Util_tools::hex_to_bytes(simulator_config::get_instance()->get_ek(), ek, k_len);

	memcpy(this->e_key_,ek,EK_LEN);
	memcpy(this->a_key_,ak,AK_LEN);
}


data_security::~data_security()
{

}

//data_security *data_security::get_instance()
//{
//	if (instance_ == NULL)
//	{
//		instance_ = new data_security();
//	}
//
//	return instance_;
//}

void data_security::reset()
{
	memset(init_vector_,0,IV_LEN);
	security_control_ = 0x0;
}

bool data_security::request_random_challenge(std::string& out_chall,const int chl_len)
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

	out_chall = rndword;
	
	return true;
}


void data_security::set_security_control(byte sc)
{
	security_control_ = sc;
}

void data_security::set_init_vector(unsigned char* iv)
{
	memcpy(init_vector_,iv,IV_LEN);
}

/************************************************************************/
/*input:																*/
/*		systitle: type is [bytes string],length is SYSTITLE_LEN			*/
/*		ic: type is Little Endian,which equal host value				*/
/************************************************************************/
void data_security::set_init_vector(unsigned char* systitle,unsigned int ic)
{
	memcpy(init_vector_,systitle,SYSTITLE_LEN);
	unsigned int rev_ic = Common::Util_tools::reverse_value(ic);
	memcpy(init_vector_ + SYSTITLE_LEN,&rev_ic,IC_LEN);
}

void data_security::set_security_suite(Suite_ID id)
{
	security_suite_id_ = id;
}

/********************************************************************************/
/*description: encrypt or|and authenticate the plain text.						*/
/*input:																		*/
/*		plain_text: type is [octet string]										*/
/*output:																		*/
/*		cipher_text: type is [octet string]										*/
/*		auth_tag: type is [octet string]										*/
/********************************************************************************/
bool data_security::encrypt(std::string &plain_text, std::string &cipher_text, std::string &auth_tag, byte sc, unsigned char *in_iv)
{
	std::string oct_ek = Util_tools::bytes_to_hex(e_key_,EK_LEN);
	std::string oct_iv = Util_tools::bytes_to_hex(in_iv,IV_LEN);
	std::string associate_data;
	bool ret = false;
	
	//Attendtion: if else 的顺序不能变化！！！！！
	if(sc & Security::Authentication_FLAG  && security_control_ & Security::Encryption_FLAG)
	{
		//SC || AK
		associate_data = Util_tools::bytes_to_hex(&sc,sizeof(byte)) + Util_tools::bytes_to_hex(a_key_,AK_LEN);
		ret = Security::gcm_aes_encrypt(oct_ek,oct_iv,plain_text,associate_data,cipher_text,auth_tag);
	}
	else if(sc & Security::Authentication_FLAG)
	{
		//SC || AK || APDU
		associate_data = Util_tools::bytes_to_hex(&sc,sizeof(byte)) + Util_tools::bytes_to_hex(a_key_,AK_LEN) + plain_text;
		ret = Security::gcm_aes_encrypt(oct_ek,oct_iv,"",associate_data,cipher_text,auth_tag);
	}
	else if(sc & Security::Encryption_FLAG)
	{
		//null
		associate_data = "";
		ret = Security::gcm_aes_encrypt(oct_ek,oct_iv,plain_text,"",cipher_text,auth_tag);
	}
	else
	{
		ret = false;
	}
	
	//重置加密机
	reset();
	return ret;
}


bool data_security::hls5authen(std::string &plain_text,std::string &auth_tag, byte sc, unsigned char *in_iv)
{
	std::string oct_ek = Util_tools::bytes_to_hex(e_key_,EK_LEN);
	std::string oct_iv = Util_tools::bytes_to_hex(in_iv,IV_LEN);
	std::string associate_data;
	std::string cipher_text;
	bool ret = false;
	
	//SC || AK || APDU
	associate_data = Util_tools::bytes_to_hex(&sc,sizeof(byte)) + Util_tools::bytes_to_hex(a_key_,AK_LEN) + plain_text;
	ret = Security::gcm_aes_encrypt(oct_ek,oct_iv,"",associate_data,cipher_text,auth_tag);

	//重置加密机
	reset();
	return ret;
}

/********************************************************************************/
/*description: decrypt or|and de-authenticate the cipher text or authen tag.	*/
/*input:																		*/
/*		plain_text: type is [octet string]										*/
/*output:																		*/
/*		cipher_text: type is [octet string]										*/
/*		auth_tag: type is [octet string]										*/
/*对于只做认证，那么，传入的cipher_text为ori-apdu,即未加密报文,authen_tag填入真实的校验码	*/
/********************************************************************************/
bool data_security::decrypt(std::string &cipher_text,std::string &authen_tag, std::string &plain_text,bool &authen_result, byte sc, unsigned char *in_iv)
{
	std::string oct_ek = Util_tools::bytes_to_hex(e_key_,EK_LEN);
	std::string oct_iv = Util_tools::bytes_to_hex(in_iv,IV_LEN);
	std::string associate_data;
	std::string expect_authen_tag;
	std::string expect_cipher_text;
	std::string payload;
	bool ret = true;

	//Attendtion: if else 的顺序不能变化！！！！！
	if(sc & Security::Authentication_FLAG  && sc & Security::Encryption_FLAG)
	{
		//SC || AK
		associate_data = Util_tools::bytes_to_hex(&sc,sizeof(byte)) + Util_tools::bytes_to_hex(a_key_,AK_LEN);
		
		//cipher_text || authen_tag
		payload = cipher_text + authen_tag;
		ret = Security::gcm_aes_decrypt(oct_ek,oct_iv,payload,associate_data,plain_text,12);
		authen_result = true;
	}
	else if(sc & Security::Authentication_FLAG)
	{
		//SC || AK || APDU
		associate_data = Util_tools::bytes_to_hex(&sc,sizeof(byte)) + Util_tools::bytes_to_hex(a_key_,AK_LEN) + cipher_text;
		ret = Security::gcm_aes_encrypt(oct_ek,oct_iv,"",associate_data,expect_cipher_text,expect_authen_tag);
		
		if(expect_authen_tag.compare(authen_tag) == 0)
		{
			authen_result = true;
			plain_text = cipher_text;
		}
		else
		{
			authen_result = false;
			plain_text = "";
		}
	}
	else if(sc & Security::Encryption_FLAG)
	{
		//tag_size = 0;associate_data = "";
		authen_result = true;
		ret = Security::gcm_aes_decrypt(oct_ek,oct_iv,cipher_text,"",plain_text,0);
	}
	else
	{
		ret = false;
	}

	//重置解密机
	reset();
	return ret;
}

