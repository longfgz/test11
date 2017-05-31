#include "common.h"
#include <cryptopp/modes.h>
#include <cryptopp/des.h>
#include <cryptopp/base64.h>
#include <windows.h>
#include <cryptopp\aes.h>
#include <cryptopp\hex.h>
#include <cryptopp\gcm.h>
#include <cryptopp\files.h>
using namespace std;
using namespace CryptoPP;
using namespace Common;


std::string Common::Util_tools::bytes_to_hex(byte * s,unsigned int s_len)
{  
	std::string encoded;  
      
	CryptoPP::StringSource ss(s, s_len, true,  
		new CryptoPP::HexEncoder(new CryptoPP::StringSink(encoded))  
	);    
	return encoded;  
}

bool Common::Util_tools::hex_to_bytes(std::string encoded,byte *d,unsigned int &d_len)  
{  
	d_len = 0;
	CryptoPP::HexDecoder decoder;  
	decoder.Put( (byte*)encoded.data(), encoded.size() );  
	decoder.MessageEnd();  
      
	size_t size = (size_t) decoder.MaxRetrievable();  
	if(size > 0)  
	{  
		decoder.Get(d, size);  
		d_len = size;  
		return true;  
	}  
	return false;  
} 

unsigned short Common::Util_tools::reverse_value(unsigned short value)
{
	unsigned char buf[sizeof(unsigned short)]={0};
	memcpy(buf, &value, sizeof(unsigned short));
	std::reverse(buf, buf+sizeof(unsigned short));
	return *(unsigned short*)buf;
}

unsigned int Common::Util_tools::reverse_value(unsigned int value)
{
	unsigned char buf[sizeof(unsigned int)]={0};
	memcpy(buf, &value, sizeof(unsigned int));
	std::reverse(buf, buf + sizeof(unsigned int));
	return *(unsigned int*)buf;
}

/********************************************************************/
/*description: descrypt the ciphered text using public fixed KEY	*/
/*input:															*/
/*		context: ciphered text,alway a password.					*/
/*output:															*/
/*		plain password.												*/
/*attention: the algorithm is same as func in param_mgr::des_decrypt*/
/*		reference it in file param_mgr.cpp							*/
/********************************************************************/
std::string Common::Util_tools::descrypt_password(std::string context)
{
	//reference the algorithm param_mgr::des_decrypt(std::string context) 
	//in file param_mgr.cpp. by liugh

	using namespace CryptoPP;
	std::string recovered;  
	byte key[DES_EDE3::KEYLENGTH]; 
	key[0]  = 0x80; key[1]  = 0x2f; key[2]  =  0x98; key[3]  =  0xcd; 
	key[4]  = 0x66; key[5]  = 0x1f; key[6]  =  0x91; key[7]  =  0xbc;
	key[8]  = 0x80; key[9]  = 0x2f; key[10] =  0x98; key[11] =  0xcd;
	key[12] = 0x66; key[13] = 0x1f; key[14] =  0x91; key[15] =  0xbc;  
	key[16] = 0x80; key[17] = 0x2f; key[18] =  0x98; key[19] =  0xcd;
	key[20] = 0x66; key[21] = 0x1f; key[22] =  0x91; key[23] =  0xbc; 

	ECB_Mode<DES_EDE3>::Decryption e;  
	e.SetKey((const unsigned char *) key, DES_EDE3::KEYLENGTH);  

	//unsigned char * mi = new unsigned char[strlen(context.c_str())*8];  
	// base64_de((unsigned char*) context.c_str(), mi);  
	//context = (char*) mi;  
    // The StreamTransformationFilter adds padding  
    //  as required. ECB and CBC Mode must be padded  
    //  to the block size of the cipher.  
	StringSource(context, true,  new Base64Decoder(
            new StreamTransformationFilter(e, new StringSink(recovered),
			BlockPaddingSchemeDef::NO_PADDING,true) // StreamTransformationFilter  
                   ) );// StringSource  

	//注意，测试发现所得recovered的方法会出现多余的空格，使用data()方法去除尾部的空格。
	char* data = (char*)(recovered.c_str());
	return trim(data);
}

/********************************************************************/
/*description: 去除字符串尾部的空格										*/
/*input:															*/
/*		str: source input string. i.e. "1234567   "					*/
/*output:															*/
/*		string without blank. i.e. "1234567"						*/
/********************************************************************/
char* Common::Util_tools::trim(char *str)
{
	int n = strlen(str) - 1; 
	while(*(str + n ) == ' ')
	{
		*(str + n-- ) = '\0'; 	
	}
	return str;
}

/********************************************************************/
/*description: encrypt the plain text to ciphered text.			 	*/
/*input:															*/
/*		key: global block cipher key. short for EK.[octet string]	*/
/*		iv:	init vector,type is [octet string]. 					*/
/*		plain: plaintext [octet string]								*/
/*		associated_data: SC_AK or SC_AK_APDU [octet string]			*/
/*		tag_size: default is 12.									*/
/*output:															*/
/*		cipher:	ciphered text,short for 'C',type is[octet string]	*/
/*		authen_tag:	authenticate tag,short for 'T',[octet string]	*/
/*attention: reference Green Manual Book page 199.					*/
/********************************************************************/
bool Common::Security::gcm_aes_encrypt(const std::string &key,const std::string &iv,const std::string &plain,
							const std::string &associated_data,std::string &cipher,std::string &authen_tag,
							const int tag_size)
{
	byte tmp_buf[1024]={0};
	byte tmp_buf_iv[1024]={0};
	unsigned int length,length_iv=0;
	std::string cipher_all;
	bool ret = true;
	try
	{
		GCM< AES >::Encryption e;
		Util_tools::hex_to_bytes(key, tmp_buf, length);
		Util_tools::hex_to_bytes(iv, tmp_buf_iv, length_iv);
		e.SetKeyWithIV(tmp_buf,AES::DEFAULT_KEYLENGTH,tmp_buf_iv,12);
		AuthenticatedEncryptionFilter ef(e,
			new StringSink(cipher_all),
			false,
			tag_size);//AuthenticatedEncryptionFilter

		memset(tmp_buf, 0, sizeof(tmp_buf));
		Util_tools::hex_to_bytes(associated_data, tmp_buf, length);
		ef.ChannelPut(AAD_CHANNEL,tmp_buf,length);
		ef.ChannelMessageEnd(AAD_CHANNEL);

		memset(tmp_buf, 0, sizeof(tmp_buf));
		Util_tools::hex_to_bytes(plain, tmp_buf, length); 
		ef.ChannelPut( DEFAULT_CHANNEL, (byte *)tmp_buf, length); 
		ef.ChannelMessageEnd("");  

		cipher_all = Util_tools::bytes_to_hex((byte*)cipher_all.data(), cipher_all.size());

		//substr the authen_tag and cipher text.
		authen_tag = cipher_all.substr(cipher_all.size()- 2*tag_size);
		cipher = cipher_all.substr(0, cipher_all.size()-2*tag_size);
	}
	catch(const CryptoPP::Exception &e)
	{
		cerr<< e.what()<<endl;
		ret = false;
	}

	return ret;
}

/********************************************************************/
/*description: decrypt the ciphered text to plain text.all params is*/
/*				octet string.										*/
/*input:															*/
/*		key: global block cipher key. short for EK.					*/
/*		iv:	init vector,type is bytes string.						*/
/*		cipher:	ciphered text and | or authen_tag					*/
/*		associated_data: SC_AK or SC_AK_APDU						*/
/*		tag_size: default is 12.									*/
/*output:															*/
/*		plain_text: plaintext										*/
/*attention: reference Green Manual Book page 199.					*/
/*		本函数只能用于解密plain_text，无法对authen进行验证					*/
/********************************************************************/
bool Common::Security::gcm_aes_decrypt(const std::string &key,const std::string &iv,const std::string &cipher,
							const std::string &associated_data,std::string &plain_text, const int tag_size)
{
	byte tmp_buf[1024]={0};
	byte tmp_buf_iv[1024]={0};
	byte tmp_buf_mac[1024] = {0};
	byte tmp_buf_enc[1024] = {0};
	byte tmp_buf_auth[1024] = {0};
	unsigned int length,length_iv,length_mac,length_enc,length_auth,length_plain = 0;
	bool rt = true;

	try
	{
		Util_tools::hex_to_bytes(key, tmp_buf, length);
		Util_tools::hex_to_bytes(iv, tmp_buf_iv, length_iv);

		GCM< AES >::Decryption de;
		de.SetKeyWithIV(tmp_buf,AES::DEFAULT_KEYLENGTH,tmp_buf_iv,12);
		
		// Break the cipher text out into it's
		//  components: Encrypted Data and MAC Value

		string enc = cipher.substr( 0, cipher.length() - tag_size*2 );
		string mac = cipher.substr( cipher.length()- tag_size*2 );

		Util_tools::hex_to_bytes(mac,tmp_buf_mac,length_mac);
		Util_tools::hex_to_bytes(enc,tmp_buf_enc,length_enc);
		Util_tools::hex_to_bytes(associated_data,tmp_buf_auth,length_auth);
		
		// Object will not throw an exception
		//  during decryption\verification _if_
		//  verification fails.
		//AuthenticatedDecryptionFilter df( d, NULL,
		// AuthenticatedDecryptionFilter::MAC_AT_BEGIN );

		AuthenticatedDecryptionFilter df( de, NULL,
			AuthenticatedDecryptionFilter::MAC_AT_BEGIN |
			AuthenticatedDecryptionFilter::THROW_EXCEPTION, tag_size );

		// The order of the following calls are important
		df.ChannelPut( DEFAULT_CHANNEL, tmp_buf_mac, length_mac );
		df.ChannelPut( AAD_CHANNEL, tmp_buf_auth, length_auth); 
		df.ChannelPut( DEFAULT_CHANNEL, tmp_buf_enc, length_enc );

		// If the object throws, it will most likely occur  
		//   during ChannelMessageEnd()  
		df.ChannelMessageEnd( AAD_CHANNEL );  
		df.ChannelMessageEnd( DEFAULT_CHANNEL );  

		// If the object does not throw, here's the only  
		//  opportunity to check the data's integrity  
		if(! df.GetLastResult() )  
			return false;  

		// Remove data from channel  
		string retrieved;  
		size_t n = (size_t)-1;  

		// Plain text recovered from enc.data()  
		df.SetRetrievalChannel( DEFAULT_CHANNEL );  
		n = (size_t)df.MaxRetrievable();  
		retrieved.resize( n );  

		if( n > 0 ) { df.Get( (byte*)retrieved.data(), n ); }  
		plain_text = retrieved;  
		
		plain_text = Util_tools::bytes_to_hex((byte*)plain_text.c_str(),plain_text.length());
	}
	catch(const CryptoPP::Exception &e)
	{
		cerr<<"decrypt failed!"<< e.what()<<endl;
		rt = false;
	}
	return rt;
}

std::string Common::Security::get_sec_control_description(byte sc)
{
	std::string str_sc;

	if((Authentication_FLAG & sc)  && 
		(Encryption_FLAG & sc))
	{
		str_sc = "AUTHENTICATE_AND_ENCRYPT";
	}
	else if(Authentication_FLAG & sc)
	{
		str_sc = "AUTHENTICATE";
	}
	else if(Encryption_FLAG & sc)
	{
		str_sc = "ENCRYPT";
	}
	
	return str_sc;
}

std::string Common::Security::get_sec_suite_description(Suite_ID suite)
{
	std::string str_suite;
	switch(suite)
	{
	case AES_GCM_128:
		str_suite = "SYM_SUITE_0";
		break;
	case  EE_AES_GCM_128_SHA_256:
		str_suite = "SYM_SUITE_1";
		break;
	case EE_AES_GCM_256_SHA_384:
		str_suite = "SYM_SUITE_2";
		break;

	default:
		str_suite = "";
		break;
	}

	return str_suite;
}
