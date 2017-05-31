/********************************************************************/
/*Filename:data_security.h											*/
/*Author: liuguanhai												*/
/*Data:2016-7-1														*/
/*Description: define the classes for data security.				*/
/********************************************************************/
#pragma once
#include "cryptopp\config.h"
#include "common.h"
#include <string>
#include <boost/thread.hpp>

using namespace Common;
using namespace Common::Security;

class data_security
{
public:
	data_security();
	~data_security();

//public:
//	static data_security *get_instance();

public:
	void set_security_control(byte sc);
	void set_init_vector(unsigned char* iv);
	void set_init_vector(unsigned char* systitle,unsigned int ic);
	void set_security_suite(Suite_ID id);

	/*输入明文，输出密文和|或校验码*/
	bool encrypt(std::string &plain_text, std::string &cipher_text, std::string &auth_tag, byte sc, unsigned char *in_iv);
	bool decrypt(std::string &cipher_text,std::string &authen_tag, std::string &plain_text,bool &authen_result, byte sc, unsigned char *in_iv);
	bool request_random_challenge(std::string& out_chall,const int chl_len = DEFAULT_AUTHEN_LEN);
	void reset(); //重置data_security.
	bool hls5authen(std::string &plain_text,std::string &auth_tag, byte sc, unsigned char *in_iv);

protected:
	byte security_control_;	//byte i.e 0x10
	unsigned char init_vector_[IV_LEN];	//bytes string,equals systitle + ic
	Suite_ID security_suite_id_; 
	unsigned char e_key_[EK_LEN];
	unsigned char a_key_[AK_LEN];
	
//	static data_security *instance_;
};
