#include "encry_context.h"
#include <cryptopp\aes.h>
#include <cryptopp\hex.h>
#include <cryptopp\gcm.h>


encry_context::encry_context(std::string cipher)
{
	parse_cipher(cipher);
}


encry_context::~encry_context(void)
{
}

std::string encry_context::get_encry_key()
{
	return encry_key_;
}

std::string encry_context::get_auth_key()
{
	return auth_key_;
}

std::string encry_context::get_master_key()
{
	return master_key_;
}

std::string encry_context::get_title()
{
	return title_;
}

std::string encry_context::get_lls_secret()
{
	return lls_secret_;
}

std::string encry_context::get_hls_secret()
{
	return hls_secret;
}

std::string encry_context::aes_decrypt(const char *cipherText)  
{  
    std::string outstr;  
  
    //Ìîkey   
	CryptoPP::SecByteBlock key(CryptoPP::AES::DEFAULT_KEYLENGTH);  
	key[0]  =  98; key[1]  = -110; key[2]  =  28; key[3]  =  24; 
	key[4]  = 102; key[5]  =   88; key[6]  = -63; key[7]  =  25;
	key[8]  = -22; key[9]  =  123; key[10] = 111; key[11] = -98;
	key[12] =  57; key[13] =  -87; key[14] =  23; key[15] = 101;  
      
    //Ìîiv   
	byte iv[CryptoPP::AES::BLOCKSIZE];  
	iv[0]  = 121; iv[1]  =  -10; iv[2]  = 122; iv[3]  = -112; 
	iv[4]  = -54; iv[5]  =   12; iv[6]  = -11; iv[7]  =   62;
	iv[8]  =  97; iv[9]  =  -53; iv[10] = 105; iv[11] =   73;
	iv[12] =   7; iv[13] =  116; iv[14] =  59; iv[15] =  -51;
  

    CryptoPP::CFB_Mode<CryptoPP::AES >::Decryption cfbDecryption((byte *)key, CryptoPP::AES::DEFAULT_KEYLENGTH, iv);  
      
    CryptoPP::HexDecoder decryptor(new CryptoPP::StreamTransformationFilter(cfbDecryption, new CryptoPP::StringSink(outstr)));  
    decryptor.Put((byte *)cipherText, strlen(cipherText));  
    decryptor.MessageEnd();  
  
    return outstr;  
}  

void encry_context::parse_cipher(std::string cipher)
{
	lls_secret_ = cipher;
	hls_secret  = cipher;
#if 0
	std::string str_plain = cipher;
	std::string str_plain = aes_decrypt(cipher.c_str());
	char *str = strtok((char*)str_plain.c_str(), ";");
	while (str)
	{
		std::string name;
		std::string value;
		get_param(str, name, value);
		if (name == "title")
			title_ = value;
		else if (name == "lls_secret")
			lls_secret_ = value;
		else if (name == "hls_secret")
			hls_secret = value;
		else if (name == "master")
			master_key_ = value;
		else if (name == "cipher")
			encry_key_ = value;
		else if (name == "auth")
			auth_key_ = value;
		str = strtok(NULL, ";");
	}
#endif
	title_ = "5753450000000001";
	encry_key_ = "000102030405060708090A0B0C0D0E0F";
	auth_key_ = "D0D1D2D3D4D5D6D7D8D9DADBDCDDDEDF";
}

bool encry_context::get_param(std::string str_info, std::string &str_name, std::string &str_value)
{
	int pos = str_info.find('=');
	if (pos == std::string::npos)
		return false;
	str_name = str_info.substr(0, pos);
	str_value = str_info.substr(pos+1);
	return true;
}