#include <ptcl_codec/asn_codec.h>
#include <algorithm>
#include <time.h>
#include <stdio.h>

asn_codec::asn_codec(void)
{
}

asn_codec::~asn_codec(void)
{
}

int asn_codec::format_ber_object_indentifier(int class_asn_tagging,int tagging_value,
								 int* v_oi,int count_oi,unsigned char *buffer)
{
	int length = 0;
	int len_tagging_area = format_tagging_area(class_asn_tagging,asn_tagging_codec_type_primary,tagging_value,buffer+length);
	length = length + len_tagging_area;

	unsigned char context_area[1024];
	int len_context_area = format_context_area_object_indentifier(v_oi,count_oi,context_area);

	int len_length_area = format_length_area(len_context_area,buffer+length);
	length = length + len_length_area;

	memcpy(buffer+length,context_area,len_context_area);
	length = length + len_context_area;


	return length;
}

int asn_codec::format_ber_bit_string(int class_asn_tagging,int tagging_value,
						 const char* bit_string,unsigned char *buffer)
{
	int length = 0;
	int len_tagging_area = format_tagging_area(class_asn_tagging,asn_tagging_codec_type_primary,tagging_value,buffer+length);
	length = length + len_tagging_area;

	unsigned char context_area[1024];
	int len_context_area = format_context_area_bit_string(bit_string,context_area);

	int len_length_area = format_length_area(len_context_area,buffer+length);
	length = length + len_length_area;

	memcpy(buffer+length,context_area,len_context_area);
	length = length + len_context_area;

	return length;
}

int asn_codec::format_ber_graphic_string(int class_asn_tagging,int tagging_value,
							  char* graphic_string,unsigned char *buffer)
{
	int length = 0;
	int len_tagging_area = format_tagging_area(class_asn_tagging,asn_tagging_codec_type_primary,tagging_value,buffer+length);
	length = length + len_tagging_area;

	unsigned char context_area[1024];
	int len_context_area = format_context_area_graphic_string(graphic_string,context_area);

	int len_length_area = format_length_area(len_context_area,buffer+length);
	length = length + len_length_area;

	memcpy(buffer+length,context_area,len_context_area);
	length = length + len_context_area;

	return length;
}

int asn_codec::format_ber_choice(int class_asn_tagging,int tagging_value,
					  unsigned char *context_area,int len_context_area,unsigned char *buffer)
{
	int length = 0;
	int len_tagging_area = format_tagging_area(class_asn_tagging,asn_tagging_codec_type_construct,tagging_value,buffer+length);
	length = length + len_tagging_area;

	int len_length_area = format_length_area(len_context_area,buffer+length);
	length = length + len_length_area;

	memcpy(buffer+length,context_area,len_context_area);
	length = length + len_context_area;

	return length;
}

int asn_codec::format_ber_seqence(int class_asn_tagging,int tagging_value,
					 PDU* v_pdu,int count_pdu,unsigned char *buffer)
{
	int length = 0;

	int len_tagging_area = format_tagging_area(class_asn_tagging,asn_tagging_codec_type_construct,tagging_value,buffer+length);
	length = length + len_tagging_area;

	unsigned char context_area[1024];
	int len_context_area = 0;
	for (int n=0;n<count_pdu;n++)
	{
		memcpy(context_area+len_context_area,v_pdu[n].context_area,v_pdu[n].len_context_area);
		len_context_area = len_context_area + v_pdu[n].len_context_area;
	}

	int len_length_area = format_length_area(len_context_area,buffer+length);
	length = length + len_length_area;

	memcpy(buffer+length,context_area,len_context_area);
	length = length + len_context_area;

	return length;
}


int asn_codec::format_tagging_area(int class_asn_tagging,int type_asn_tagging_codec,int tagging_value,
						unsigned char *buffer)
{
	if (tagging_value >= 0 && tagging_value <= 30)
	{
		buffer[0] = (tagging_value & 0x1f) | ((type_asn_tagging_codec & 0x01) << 5) | ((class_asn_tagging & 0x03) << 6);
		return 1;
	}
	else if (tagging_value >= 31 && tagging_value <= 127)
	{
		buffer[0] = (0x1f & 0x1f) | ((type_asn_tagging_codec & 0x01) << 5) | ((class_asn_tagging & 0x03) << 6);
		buffer[1] = (tagging_value & 0x7f) | ((0 & 0x01) << 7);
		return 2;
	}

	return 0;
}

int asn_codec::parse_tagging_area(unsigned char *buffer,
					   int & class_asn_tagging,int & type_asn_tagging_codec,int & tagging_value)
{
	tagging_value = buffer[0] & 0x1f;
	type_asn_tagging_codec = (buffer[0] >> 5) & 0x01;
	class_asn_tagging = (buffer[0] >> 6) & 0x03;

	if (tagging_value >= 0 && tagging_value <= 30)
	{
		return 1;
	}
	else
	{
		int end = (buffer[1] >> 7) & 0x01;
		if (end == 0)
		{
			tagging_value = buffer[1] & 0x7f;
			return 2;
		}
	}

	return 0;

}

int asn_codec::format_length_area(int length,unsigned char *buffer)
{
	int buf_len = 0;
	if (length <=127)
	{
		buffer[buf_len++] = length;
	}
	else if (length <=255)
	{
		buffer[buf_len++] = 0x81;
		buffer[buf_len++] = length;
	}
	else if (length <=65535)
	{
		buffer[buf_len++] = 0x82;
		buffer[buf_len++] = length/256;
		buffer[buf_len++] = length%256;
	}
	return buf_len;
}

int asn_codec::parse_length_area(unsigned char *buffer,int & length)
{
	int flag = (buffer[0]>>7);
	int count = (buffer[0] & 0x7F);
	int parse_len = 1;
	if (flag == 0)
	{
		length = count;
		return parse_len;
	}
	if (count > 4)
		return 0;
	for (int i=0; i< count; i++)
		length = length*256+buffer[parse_len++];
	return parse_len;
}

//"0101110" --> 一个字节对一位 首字节填不足8位的
int asn_codec::format_context_area_bit_string(const char* bit_string,unsigned char *buffer)
{
	int length = (int)strlen(bit_string);
	buffer[0] = 8 - (length % 8);

	for (int n=0;n<length;n++)
	{
		buffer[1+(n/8)] = (bit_string[n]-'0') << (8-(n%8)-1);
	}

	return 1+(length/8)+1;
}

//"12345678" -> 0x31 0x32 0x33 0x34 0x35 0x36 0x37 0x38 直接asii转
int asn_codec::format_context_area_graphic_string(char* graphic_string,unsigned char *buffer)
{
	int length = (int)strlen(graphic_string);
	for (int n=0;n<length;n++)
	{
		buffer[n] = graphic_string[n];
	}

	return length;
}

//"04FEFF00" -> 0x04 0xFE 0xFF 0x00
int asn_codec::format_context_area_octet_string(char* octet_string,unsigned char *buffer)
{
	if (octet_string == NULL)
		return 0;

	int length = (int)strlen(octet_string) / 2;
	for (int n=0;n<length;n++)
	{
		sscanf(octet_string+2*n,"%02X",&buffer[n]);
	}

	return length;
}

int asn_codec::parse_context_area_octet_string(unsigned char *buffer, int buf_len, std::string &str_value)
{
	str_value = "";
	for (int i=0; i<buf_len; i++)
	{
		char str[8] = {0};
		sprintf(str, "%02X", buffer[i]);
		str_value += str;
	}
	return buf_len;
}

int asn_codec::format_context_area_obis(const char* obis_string,unsigned char *buffer)
{
	if (obis_string == NULL)// || strlen(obis_string) != 12)
		return 0;

	int tmp_value = 0;
	for (size_t n=0;n<strlen(obis_string)/2;n++)
	{
		sscanf(obis_string+2*n,"%02X",&tmp_value);
		buffer[n] = tmp_value;
	}


	return (int)strlen(obis_string)/2;
}

int asn_codec::format_context_area_short(short s,unsigned char *buffer)
{
	memcpy(buffer,&s,2);
	std::reverse(buffer,buffer+2);
	return 2;
}


int asn_codec::format_context_area_int(int i,unsigned char *buffer)
{
	memcpy(buffer,&i,4);
	std::reverse(buffer,buffer+4);
	return 4;
}

int asn_codec::format_context_area_int64(__int64 i,unsigned char *buffer)
{
	memcpy(buffer,&i,8);
	std::reverse(buffer,buffer+8);
	return 8;
}

int asn_codec::format_context_area_float(float value,unsigned char *buffer)
{
	memcpy(buffer,&value,4);
	std::reverse(buffer,buffer+4);
	return 4;
}

int asn_codec::format_context_area_object_indentifier(int * v_oi,int count_oi,unsigned char *buffer)
{
	if (v_oi == NULL || count_oi < 5)
		return 0;

	buffer[0] = v_oi[0] * 40 + v_oi[1];

	int value = v_oi[2];
	buffer[1] = ((value >> 7) & 0x7f) | ((1 & 0x01) << 7);
	buffer[2] = (value & 0x7f) | ((0 & 0x01) << 7);

	int length = 3;
	for (;length<count_oi;length++)
	{
		buffer[length] = v_oi[length];
	}
	return length;
}


int asn_codec::parse_context_area_object_indentifier(unsigned char *buffer,int length,int * v_oi,int &count_oi)
{
	v_oi[0] = buffer[0] / 40;
	v_oi[1] = buffer[0] % 40;
	v_oi[2] = ((buffer[1] & 0x7f) << 7) | (buffer[2] & 0x7f);

	for (int n=3;n<length;n++)
	{
		v_oi[n] = buffer[n];
	}

	return length;
}

int asn_codec::parse_date_time(unsigned char *buffer,unsigned int & time)
{
	tm t;
	unsigned short year = 0;
	year = buffer[0]*16*16 + buffer[1];
	t.tm_year = year - 1900;
	t.tm_mon = buffer[2]-1;
	t.tm_mday = buffer[3];
	t.tm_hour = buffer[5];
	t.tm_min = buffer[6];
	t.tm_sec = buffer[7];
	t.tm_isdst = -1;

	time = (unsigned int)mktime(&t);
	return 13;
}

int  asn_codec::format_date_time(time_t dt, unsigned char *buffer)
{
	tm *t = localtime(&dt);
	char time_s[64]={0};
	sprintf(time_s,"%04x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",t->tm_year+1900,t->tm_mon+1,t->tm_mday,t->tm_wday,t->tm_hour,t->tm_min,t->tm_sec,0,0x80,0,0);
	return asn_codec::format_context_area_octet_string(time_s, buffer);
}

int asn_codec::parse_unsigned_double_long(unsigned char *buf,unsigned int len,double & d,int count_radix_point)
{
	if (buf == NULL || len > 5 || count_radix_point > 10)
		return 0;


	unsigned int int_value = 0;
	for (int n=0;n<(int)len;n++)
	{
		unsigned int pow = 1;
		for (int m=len-1;m>0;m--)
			pow = pow * 100;

		int_value = int_value + (bcd_to_bin(buf[n]) * pow);
	}

	double pow = 1;
	if (count_radix_point >= 0)
	{
		for (int m=0;m<count_radix_point;m++)
			pow = pow * 10;

		d = int_value / pow;
	}
	else
	{
		for (int m=0;m<-1 * count_radix_point;m++)
			pow = pow * 10;
		d = int_value * pow;
	}

	return len;
}

unsigned char asn_codec::bcd_to_bin(unsigned char bcd)
{
	unsigned char b1 = bcd/16;
	if (b1 > 9)
		return 0;

	unsigned char b0 = bcd%16;
	if (b0 > 9)
		return 0;

	return (b1*10 + b0);
}

int  asn_codec::format_basic_data(unsigned char tid, unsigned int length, unsigned char *buffer)
{
	int buf_len = 0;
	buffer[buf_len++] = tid;
	buf_len += format_length_area(length, buffer+buf_len);
	return buf_len;
}

int  asn_codec::format_basic_data(unsigned char tid, unsigned char *buffer)
{
	int buf_len = 0;
	buffer[buf_len++] = tid;
	return buf_len;
}

int  asn_codec::format_array_data(unsigned int item_size, unsigned char *buffer)
{
	int buf_len = 0;
	buffer[buf_len++] = 0x01;
	buffer[buf_len++] = item_size;
	return buf_len;
}

int  asn_codec::format_struct_data(unsigned int item_size, unsigned char *buffer)
{
	int buf_len = 0;
	buffer[buf_len++] = 0x02;
	buffer[buf_len++] = item_size;
	return buf_len;
}

int asn_codec::format_obis_string(std::string str_obis, unsigned char *buffer)
{
	int values[6];
	sscanf(str_obis.c_str(), "%d.%d.%d.%d.%d.%d", values, values+1, values+2, values+3, values+4, values+5);
	for (int i=0; i<6; i++)
	{
		buffer[i] = values[i];
	}
	return 6;
}

int asn_codec::parse_obis_string(unsigned char *buffer, std::string &str_obis)
{
	char obis_buf[32] = {0};
	sprintf(obis_buf, "%d.%d.%d.%d.%d.%d", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);
	str_obis =  obis_buf;
	return 6;
}

int asn_codec::format_unsigned_double_long(unsigned int value, unsigned char *buffer) 
{
	if (buffer == nullptr)
		return 0;

	int buf_len = 0;
	buffer[buf_len++] = 0x06;
	int len = format_context_area_int(value, buffer+buf_len);
	return buf_len+len;
}

int asn_codec::format_unsigned(unsigned char value, unsigned char *buffer) 
{
	if (buffer == nullptr)
		return 0;

	buffer[0] = 0x16;
	buffer[1] = value;
	return 2;
}

int asn_codec::format_date_time_extra(time_t dt, unsigned char *buffer)
{
	if (buffer == nullptr)
		return 0;
	int buf_len = 0;
	buffer[buf_len++] = 0x09;
	buffer[buf_len++] = 0x0c;

	int len = 0;
	if (dt == 0) {
		len = 12;
	}
	else 
	{
		tm *t = localtime(&dt);
		char time_s[64]={0};
		sprintf(time_s,"%04x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",t->tm_year+1900,t->tm_mon+1,t->tm_mday,t->tm_wday,t->tm_hour,t->tm_min,t->tm_sec,0,0x80,0,0);
		len = asn_codec::format_context_area_octet_string(time_s, buffer+buf_len);
	}
	
	return buf_len + len;
}