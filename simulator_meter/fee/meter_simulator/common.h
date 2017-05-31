#pragma once
#include <cryptopp/hex.h>
#include <string>

namespace Common
{
	namespace Util_tools
	{

#define MIN(a,b) ((a)>(b)? (b) : (a))
#define MAX(a,b) ((a)>(b)? (a) : (b))

		std::string bytes_to_hex(byte * s,unsigned int s_len); 
		//std::string int_to_hex(byte* data);
		bool hex_to_bytes(std::string encoded,byte *d,unsigned int &d_len);  
		unsigned int reverse_value(unsigned int value);
		unsigned short reverse_value(unsigned short value);
		std::string descrypt_password(std::string context);
		char* trim(char* str); //È¥³ý×Ö·û´®Î²µÄ¿Õ¸ñ
	}
	
	namespace Dlms
	{

#define wPort_No_station		0x0000
#define wPort_Client_MP			0x0001
#define wPort_Public_Client		0x0010
#define wPort_M_Logical_Device	0x0001
#define wPort_All_Station		0x007F

#define OBIS_LEN 6

		enum
		{
			InitiateRequest =	1,		//0x01
			ReadRequest		=	5,		//0x05
			WriteRequest	=	6,		//0x06
			InitiateResponse=	8,		//0x08
			GetRequest		=	192,	//0xC0
			SetRequest		=	193,	//0xC1
			ActionRequest	=	195,	//0xC3
			GetResponse		=	196,	//0xC4
			SetResponse		=	197,	//0xC5
			ActionResponse	=	199,	//0xC7

			glo_InitiateRequest =	33,	//0x21
			glo_ReadRequest		=	37,	//0x25
			glo_WriteRequest	=	38,	//0x26
			glo_InitiateResponse=	40,	//0x28
			glo_GetRequest	=	200,	//0xC8
			glo_SetRequest	=	201,	//0xC9
			glo_ActionRequest	=	203,//0xCB
			glo_GetResponse		=	204,//0xCC
			glo_SetResponse		=	205,//0xCD
			glo_ActionResponse	=	207,//0xCF

			AARQ				=	96,
			AARE				=	97,
			RLRQ				=	98,
			RLRE				=	99,
		};
	}

	namespace Security
	{

#define SYSTITLE_LEN 8
#define IC_LEN 4
#define IV_LEN (SYSTITLE_LEN + IC_LEN)
#define EK_LEN 16
#define AK_LEN 16
#define SC_LEN 1
#define DEFAULT_AUTHEN_LEN 16
#define MAX_AUTHEN_LEN 64
#define DEFAULT_TAG_LEN 12

		enum
		{
			Authentication_FLAG = 16,
			Encryption_FLAG = 32,
			Key_Broadcast_FLAG = 64,
			Compress_FLAG = 128,
		};

		enum Suite_ID
		{
			AES_GCM_128 = 0,
			EE_AES_GCM_128_SHA_256 = 1,
			EE_AES_GCM_256_SHA_384 = 2,
		};

		//Green Manual Book page 256
		enum Authen_Mechanism
		{
			LOWEST_LS_MECH = 0,
			LLS_MECH = 1,
			HLS_MECH = 2,
			HLS_MECH_MD5 = 3,
			HLS_MECH_SHA_1 = 4,
			HLS_MECH_GMAC = 5,
			HLS_MECH_SHA_256 = 6,
			HLS_MECH_SHA_ECDSA = 7,
		};


		std::string get_sec_control_description(byte sc);
		std::string get_sec_suite_description(Suite_ID suite);

		bool gcm_aes_encrypt(const std::string &key,const std::string &iv,const std::string &plain,
									const std::string &associated_data,std::string &cipher,std::string &authen_tag,
									const int tag_size = 12);
		bool gcm_aes_decrypt(const std::string &key,const std::string &iv,const std::string &cipher,
									const std::string &associated_data,std::string &plain_text, const int tag_size = 12);
	}

}