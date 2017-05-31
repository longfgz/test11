#include <ptcl_codec/ptcl_module.h>


ptcl_module::ptcl_module(void)
{
	host_id_ = 0;
}

ptcl_module::~ptcl_module(void)
{
}

void ptcl_module::set_host_id(unsigned char id)
{
	host_id_ = id;
}
unsigned char ptcl_module::get_host_id()
{
	return host_id_;
}

unsigned char ptcl_module::get_frame_seq(unsigned int addr)
{
	if (addr == 0)
		return 0;

	boost::unordered_map <unsigned int,unsigned char>::iterator it = map_seq_.find(addr);
	if (it == map_seq_.end())
	{
		map_seq_[addr] = 0;
		return 0;
	}
	else
	{
		unsigned char seq = map_seq_[addr];
		if (seq == 127)
			seq = 0;
		else
			seq ++;

		map_seq_[addr] = seq;
		return seq;
	}
	
}

void ptcl_module::reset_frame_seq(int addr)
{
	if (addr == 0)
		return ;

	map_seq_[addr] = 0;
}

unsigned char ptcl_module::bin_to_bcd(unsigned char bin)
{
	if (bin > 99)
		return 0;

	return ((bin/10)*16 + bin%10);
}

unsigned char ptcl_module::bcd_to_bin(unsigned char bcd)
{
	unsigned char b1 = bcd/16;
	if (b1 > 9)
		return 0;

	unsigned char b0 = bcd%16;
	if (b0 > 9)
		return 0;

	return (b1*10 + b0);
}

double ptcl_module::bcd_to_bin(unsigned char *buf,int len,int count_radix_point,unsigned char invalid)
{
	if (buf == NULL || len > 5 || count_radix_point > 10)
		return 0;

	if (is_invalid_data(buf,len,invalid) == true)
		return -10;

	unsigned int int_value = 0;
	for (int n=len-1;n>=0;n--)
	{
		unsigned int pow = 1;
		for (int m=0;m<n;m++)
			pow = pow * 100;

		int_value = int_value + (bcd_to_bin(buf[n]) * pow);
	}

	double pow = 1;
	if (count_radix_point >= 0)
	{
		for (int m=0;m<count_radix_point;m++)
			pow = pow * 10;

		return int_value / pow;
	}
	else
	{
		for (int m=0;m<-1 * count_radix_point;m++)
			pow = pow * 10;
		return int_value * pow;
	}

}

int ptcl_module::bcd_to_bin(unsigned char *buf,int len,unsigned char invalid)
{
	if (buf == NULL || len > 4)
		return 0;

	if (is_invalid_data(buf,len,invalid) == true)
		return -10;

	unsigned int int_value = 0;
	for (int n=len-1;n>=0;n--)
	{
		unsigned int pow = 1;
		for (int m=0;m<n;m++)
			pow = pow * 100;

		int_value = int_value + (bcd_to_bin(buf[n]) * pow);
	}

	return int_value;
}

unsigned int ptcl_module::bcd_to_byte(unsigned char *buf,int len,int pos)
{
	if ( buf == NULL )
		return 0;

	unsigned int int_value = 0;
	unsigned char flag = *buf;
	
	int_value = (flag>>pos)&0x01;

	return int_value;
}

int ptcl_module::bcd_to_time(unsigned char *buf,int len,int type,tm *t)
{
	if (buf == NULL || t == NULL)
		return 0;

	int len_parse = 0;
	if (type == time_type_ssmmhhddmmyy)
	{
		len_parse = 6;
		if (len < len_parse)
			return 0;
		t->tm_sec = bcd_to_bin(buf[0]);
		t->tm_min = bcd_to_bin(buf[1]);
		t->tm_hour = bcd_to_bin(buf[2]);
		t->tm_mday = bcd_to_bin(buf[3]);
		t->tm_mon = bcd_to_bin(buf[4]) - 1;
		t->tm_year = 100 + bcd_to_bin(buf[5]);
	}
	else if (type == time_type_ssmmhhddwwmmyy)
	{
		len_parse = 6;
		if (len < len_parse)
			return 0;
		t->tm_sec = bcd_to_bin(buf[0]);
		t->tm_min = bcd_to_bin(buf[1]);
		t->tm_hour = bcd_to_bin(buf[2]);
		t->tm_mday = bcd_to_bin(buf[3]);
		t->tm_mon = bcd_to_bin(buf[4] & 0x1F) - 1;
		t->tm_year = 100 + bcd_to_bin(buf[5]);
	}
	else if (type == time_type_mmhhddmmyy)
	{
		len_parse = 5;
		if (len < len_parse)
			return 0;
		t->tm_sec = 0;
		t->tm_min = bcd_to_bin(buf[0]);
		t->tm_hour = bcd_to_bin(buf[1]);
		t->tm_mday = bcd_to_bin(buf[2]);
		t->tm_mon = bcd_to_bin(buf[3]) - 1;
		t->tm_year = 100 + bcd_to_bin(buf[4]);
	}
	else if (type == time_type_ddmmyy)
	{
		len_parse = 3;
		if (len < len_parse)
			return 0;
		t->tm_sec = 0;
		t->tm_min = 0;
		t->tm_hour = 0;
		t->tm_mday = bcd_to_bin(buf[0]);
		t->tm_mon = bcd_to_bin(buf[1]) - 1;
		t->tm_year = 100 + bcd_to_bin(buf[2]);
	}
	else if (type == time_type_mmhhddmm)
	{
		len_parse = 4;
		if (len < len_parse)
			return 0;

		t->tm_sec = 0;
		t->tm_min = bcd_to_bin(buf[0]);
		t->tm_hour = bcd_to_bin(buf[1]);
		t->tm_mday = bcd_to_bin(buf[2]);
		t->tm_mon = bcd_to_bin(buf[3]) - 1;
		t->tm_year = 114;
	}
	else if (type == time_type_hhddmmyy)
	{
		len_parse = 4;
		if (len < len_parse)
			return 0;

		t->tm_sec = 0;
		t->tm_min = 0;
		t->tm_hour = bcd_to_bin(buf[0]);
		t->tm_mday = bcd_to_bin(buf[1]);
		t->tm_mon = bcd_to_bin(buf[2]) - 1;
		t->tm_year = 100 + bcd_to_bin(buf[3]);
	}
	else if (type == time_type_mmyy)
	{
		len_parse = 2;
		if (len < len_parse)
			return 0;

		t->tm_sec = 0;
		t->tm_min = 0;
		t->tm_hour = 0;
		t->tm_mday = 1;
		t->tm_mon = bcd_to_bin(buf[0]) - 1;
		t->tm_year = 100 + bcd_to_bin(buf[1]);
	}
	else if (type == time_type_yymmddhhmmss)
	{
		len_parse = 6;
		if (len < len_parse)
			return 0;

		t->tm_year = 100 + bcd_to_bin(buf[0]);
		t->tm_mon = bcd_to_bin(buf[1]) - 1;
		t->tm_mday = bcd_to_bin(buf[2]);
		t->tm_hour = bcd_to_bin(buf[3]);
		t->tm_min = bcd_to_bin(buf[4]);
		t->tm_sec = bcd_to_bin(buf[5]);
	}
	else if (type == time_type_yymmddhhmm)
	{
		len_parse = 5;
		if (len < len_parse)
			return 0;

		t->tm_year = 100 + bcd_to_bin(buf[0]);
		t->tm_mon = bcd_to_bin(buf[1]) - 1;
		t->tm_mday = bcd_to_bin(buf[2]);
		t->tm_hour = bcd_to_bin(buf[3]);
		t->tm_min = bcd_to_bin(buf[4]);
		t->tm_sec = 0;
	}
	else if (type == time_type_yymmdd)
	{
		len_parse = 3;
		if (len < len_parse)
			return 0;

		t->tm_year = 100 + bcd_to_bin(buf[0]);
		t->tm_mon = bcd_to_bin(buf[1]) - 1;
		t->tm_mday = bcd_to_bin(buf[2]);
		t->tm_hour = 0;
		t->tm_min = 0;
		t->tm_sec = 0;
	}
	else if (type == time_type_mmddhhmm)
	{
		len_parse = 4;
		if (len < len_parse)
			return 0;

		t->tm_year = 114;
		t->tm_mon = bcd_to_bin(buf[0]) - 1;
		t->tm_mday = bcd_to_bin(buf[1]);
		t->tm_hour = bcd_to_bin(buf[2]);
		t->tm_min = bcd_to_bin(buf[3]);
		t->tm_sec = 0;
	}
	else if (type == time_type_hhddmm)
	{
		len_parse = 3;
		if (len < len_parse)
			return 0;

		t->tm_year = 114;
		t->tm_hour = bcd_to_bin(buf[0]);
		t->tm_mday = bcd_to_bin(buf[1]);
		t->tm_mon = bcd_to_bin(buf[2]) - 1;
		t->tm_min = 0;
		t->tm_sec = 0;
	}
	else if (type == time_type_mmhhdd)
	{
		len_parse = 3;
		if(len < len_parse)
			return 0;

		t->tm_year = 114;
		t->tm_mon  = 1;
		t->tm_min  = bcd_to_bin(buf[0]);
		t->tm_hour = bcd_to_bin(buf[1]);
		t->tm_mday = bcd_to_bin(buf[2]);
		t->tm_sec = 0;
	}

	if (t->tm_mon < 0 || t->tm_mon > 11)
		t->tm_mon = 0;
	if (t->tm_mday < 1 || t->tm_mday > 31)
		t->tm_mday = 1;
	if (t->tm_hour < 0 || t->tm_hour > 23)
		t->tm_hour = 0;
	if (t->tm_min < 0 || t->tm_min > 59)
		t->tm_min = 0;
	if (t->tm_sec < 0 || t->tm_sec > 59)
		t->tm_sec = 0;

	return len_parse;
}

int ptcl_module::time_to_bcd(int type,tm *t,unsigned char *buf)
{
	if (buf == NULL || t == NULL)
		return 0;

	int len_format = 0;
	if (type == time_type_ssmmhhddmmyy)
	{
		len_format = 6;

		buf[0] = bin_to_bcd(t->tm_sec);
		buf[1] = bin_to_bcd(t->tm_min);
		buf[2] = bin_to_bcd(t->tm_hour);
		buf[3] = bin_to_bcd(t->tm_mday);
		buf[4] = bin_to_bcd(t->tm_mon+1);
		buf[5] = bin_to_bcd(t->tm_year%100);

	}
	else if (type == time_type_mmhhddmmyy)
	{
		len_format = 5;
		buf[0] = bin_to_bcd(t->tm_min);
		buf[1] = bin_to_bcd(t->tm_hour);
		buf[2] = bin_to_bcd(t->tm_mday);
		buf[3] = bin_to_bcd(t->tm_mon+1);
		buf[4] = bin_to_bcd(t->tm_year%100);

	}
	else if (type == time_type_ddmmyy)
	{
		len_format = 3;
		buf[0] = bin_to_bcd(t->tm_mday);
		buf[1] = bin_to_bcd(t->tm_mon+1);
		buf[2] = bin_to_bcd(t->tm_year%100);
	}
	else if (type == time_type_mmhhddmm)
	{
		len_format = 4;
		buf[0] = bin_to_bcd(t->tm_min);
		buf[1] = bin_to_bcd(t->tm_hour);
		buf[2] = bin_to_bcd(t->tm_mday);
		buf[3] = bin_to_bcd(t->tm_mon+1);

	}
	else if (type == time_type_hhddmmyy)
	{
		len_format = 4;
		buf[0] = bin_to_bcd(t->tm_hour);
		buf[1] = bin_to_bcd(t->tm_mday);
		buf[2] = bin_to_bcd(t->tm_mon+1);
		buf[3] = bin_to_bcd(t->tm_year%100);
	}
	else if (type == time_type_mmyy)
	{
		len_format = 2;
		buf[0] = bin_to_bcd(t->tm_mon+1);
		buf[1] = bin_to_bcd(t->tm_year%100);
	}
	else if (type == time_type_ssmmhhddwwmmyy)
	{
		len_format = 6;
		buf[0] = bin_to_bcd(t->tm_sec);
		buf[1] = bin_to_bcd(t->tm_min);
		buf[2] = bin_to_bcd(t->tm_hour);
		buf[3] = bin_to_bcd(t->tm_mday);
		int week = t->tm_wday;
		if (week == 0)
		 week = 7;
		buf[4] = ((t->tm_mon+1)%10)+(((t->tm_mon+1)/10)<<4)+((bin_to_bcd(t->tm_wday))<<5);
		buf[5] = bin_to_bcd(t->tm_year%100);
	}
	else if (type == time_type_yymmddhhmmss)
	{
		len_format = 6;

		buf[0] = bin_to_bcd(t->tm_year%100);
		buf[1] = bin_to_bcd(t->tm_mon+1);
		buf[2] = bin_to_bcd(t->tm_mday);
		buf[3] = bin_to_bcd(t->tm_hour);
		buf[4] = bin_to_bcd(t->tm_min);
		buf[5] = bin_to_bcd(t->tm_sec);
	}
	else if (type == time_type_yymmddhhmm)
	{
		len_format = 5;

		buf[0] = bin_to_bcd(t->tm_year%100);
		buf[1] = bin_to_bcd(t->tm_mon+1);
		buf[2] = bin_to_bcd(t->tm_mday);
		buf[3] = bin_to_bcd(t->tm_hour);
		buf[4] = bin_to_bcd(t->tm_min);
	}
	else if (type == time_type_yymmdd)
	{
		len_format = 3;

		buf[0] = bin_to_bcd(t->tm_year%100);
		buf[1] = bin_to_bcd(t->tm_mon+1);
		buf[2] = bin_to_bcd(t->tm_mday);

	}
	else if (type == time_type_mmddhhmm)
	{
		len_format = 4;

		buf[0] = bin_to_bcd(t->tm_mon+1);
		buf[1] = bin_to_bcd(t->tm_mday);
		buf[2] = bin_to_bcd(t->tm_hour);
		buf[3] = bin_to_bcd(t->tm_min);
	}

	else if (type == time_type_mmddhhmmss)
	{
		len_format = 5;

		buf[0] = bin_to_bcd(t->tm_mon+1);
		buf[1] = bin_to_bcd(t->tm_mday);
		buf[2] = bin_to_bcd(t->tm_hour);
		buf[3] = bin_to_bcd(t->tm_min);
		buf[4] = bin_to_bcd(t->tm_sec);
	}

	return len_format;
}

bool ptcl_module::is_invalid_data(unsigned char *buffer,int len,unsigned char invalid)
{
	for (int n=0;n<len;n++)
	{
		if (buffer[n] == invalid || buffer[n] == 0xee)
			return true;
	}
	return false;
}

int ptcl_module::format_td_c(tm *t,int m,int n,unsigned char *buffer)
{
	int len = format_sg_15(t,buffer);
	if (len <= 0)
		return 0;

	buffer[len++] = m;
	buffer[len++] = n;

	return len;
}

int ptcl_module::parse_td_c(unsigned char *buffer,int len,tm *t,int &m,int &n)
{
	int len_parse = parse_sg_15(buffer,len,t);
	if (len_parse <= 0)
		return 0;

	m = buffer[len_parse++];
	n = buffer[len_parse++];

	return len_parse;
}

int ptcl_module::format_td_d(tm *t,unsigned char *buffer)
{
	return format_sg_20(t,buffer);
}

int ptcl_module::parse_td_d(unsigned char *buffer,int len,tm *t)
{
	return parse_sg_20(buffer,len,t);
}

int ptcl_module::format_td_m(tm *t,unsigned char *buffer)
{
	return format_sg_21(t,buffer);
}

int ptcl_module::parse_td_m(unsigned char *buffer,int len,tm *t)
{
	return parse_sg_21(buffer,len,t);
}

int ptcl_module::format_td_time(tm *t, unsigned char *buffer)
{
	return format_sg_31(t,buffer);
}

int ptcl_module::parse_sg_1(unsigned char *buffer,int len,tm *t)
{
	return bcd_to_time(buffer,len,ptcl_module::time_type_ssmmhhddwwmmyy,t);
}

int ptcl_module::parse_sg_2(unsigned char *buffer,int len,double & d)
{
	if (len < 2)
		return 0;

	if (is_invalid_data(buffer,2,0xee) == true)
	{
		d = -10;
		return 2;
	}

	//unsigned char temp[2];
	//memset(temp,0,len);
	//memcpy(temp,buffer,len);
	//temp[1] = (temp[1] >> 4);

	if (check_bcd(buffer,2,4) == false)
		return 0;

	unsigned char tmp[2];
	memcpy(tmp,buffer,2);
	unsigned char s = (tmp[1] & 0x10) >> 4;
	unsigned char g = (tmp[1] & 0xe0) >> 5;
	tmp[1] = tmp[1] & 0x0f;
	int count_radix_point = -4 + g;
	d = bcd_to_bin(tmp,2,count_radix_point,0xee);
	if (s == 1)
		d = 0 - d;

	return 2;
}

int ptcl_module::format_sg_3(int value,unsigned char *buffer,unsigned int offset)
{
	int s = 0;
	int g = 0;

	if(value < 0)
	{
		s = 1;
		value = 0 - value;
	}

	if((value % 1000) == 0 && value != 0)
	{
		g = 1;
		value = value / 1000;
	}

	int n = 0;
	while (value > 0)
	{
		buffer[offset+n] = bin_to_bcd(value % 100);
			
		value = value / 100;
		n++;
	}

	buffer[offset+3] = buffer[offset+3] | (s<<4);
	buffer[offset+3] = buffer[offset+3] | (g<<6);

	return 4;
}

int ptcl_module::parse_sg_3(unsigned char *buffer,int len,double &d)
{
	if (len < 4)
		return 0;

	if (is_invalid_data(buffer,4,0xee) == true)
	{
		d = -10;
		return 4;
	}

	if (check_bcd(buffer,4) == false)
		return 0;

	unsigned char tmp[4];
	memcpy(tmp,buffer,4);
	unsigned char s = (tmp[3] & 0x10) >> 4;
	//unsigned char g = (tmp[3] & 0x40) >> 6;
	tmp[3] = tmp[3] & 0x0f;
	d = bcd_to_bin(tmp,4,0,0xee);
	if (s == 1)
		d = 0 - d;
	return 4;
}

int ptcl_module::parse_sg_4(unsigned char *buffer,int len,double & d)
{
	if (len < 2)
		return 0;

		if (is_invalid_data(buffer,2,0xee) == true)
	{
		d = -10;
		return 2;
	}

	if (check_bcd(buffer,2) == false)
		return 0;

	unsigned char tmp[2];
	memcpy(tmp,buffer,2);
	unsigned char s = (tmp[1] & 0x10) >> 7;
	tmp[1] = tmp[1] & 0x7f;
	d = bcd_to_bin(tmp,2,0,0xee);
	if (s == 1)
		d = 0 - d;

	return 2;
}


int ptcl_module::parse_sg_5(unsigned char *buffer,int len,double & d)
{
	if (len < 2)
		return 0;

	if (is_invalid_data(buffer,2,0xee) == true)
	{
		d = -10;
		return 2;
	}

	if (check_bcd(buffer,2) == false)
		return 0;

	unsigned char tmp[2];
	memcpy(tmp,buffer,2);
	unsigned char s = tmp[1] >> 7;
	tmp[1] = tmp[1] & 0x7f;
	d = bcd_to_bin(tmp,2,1,0xee);
	if (s == 1)
		d = 0 - d;

	return 2;
}

int ptcl_module::parse_sg_6(unsigned char *buffer,int len,double & d)
{
	if (len < 2)
		return 0;


	if (is_invalid_data(buffer,2,0xee) == true)
	{
		d = -10;
		return 2;
	}

	if (check_bcd(buffer,2) == false)
		return 0;


	unsigned char tmp[2];
	memcpy(tmp,buffer,2);
	unsigned char s = tmp[1] >> 7;
	tmp[1] = tmp[1] & 0x7f;
	d = bcd_to_bin(tmp,2,2,0xee);
	if (s == 1)
		d = 0 - d;

	return 2;
}

int ptcl_module::parse_sg_7(unsigned char *buffer,int len,double & d)
{
	if (len < 2)
		return 0;

	d = bcd_to_bin(buffer,2,1,0xee);

	if (check_bcd(buffer,2) == false)
		return 0;

	return 2;
}

int ptcl_module::parse_sg_8(unsigned char *buffer,int len,double & d)
{
	if (len < 2)
		return 0;

	d = bcd_to_bin(buffer,2,0,0xee);

	if (check_bcd(buffer,2) == false)
		return 0;

	return 2;
}

int ptcl_module::parse_sg_9(unsigned char *buffer,int len,double & d)
{
	if (len < 3)
		return 0;


	if (is_invalid_data(buffer,3,0xee) == true)
	{
		d = -10;
		return 3;
	}

	if (check_bcd(buffer,3) == false)
		return 0;

	unsigned char tmp[3];
	memcpy(tmp,buffer,3);
	unsigned char s = tmp[2] >> 7;
	tmp[2] = tmp[2] & 0x7f;
	d = bcd_to_bin(tmp,3,4,0xee);
	if (s == 1)
		d = 0 - d;

	return 3;
}

int ptcl_module::parse_sg_10(unsigned char *buffer,int len,double & d)
{
	if (len < 3)
		return 0;

	d = bcd_to_bin(buffer,3,0,0xee);

	if (check_bcd(buffer,3) == false)
		return 0;

	return 3;
}

int ptcl_module::parse_sg_11(unsigned char *buffer,int len,double & d)
{
	if (len < 4)
		return 0;

	d = bcd_to_bin(buffer,4,2,0xee);

	if (check_bcd(buffer,4) == false)
		return 0;

	return 4;
}

int ptcl_module::parse_sg_12(unsigned char *buffer,int len,double & d)
{
	if (len < 6)
		return 0;

	d = bcd_to_bin(buffer,6,0,0xee);

	if (check_bcd(buffer,6) == false)
		return 0;

	return 6;
}

int ptcl_module::parse_sg_13(unsigned char *buffer,int len,double & d)
{
	if (len < 4)
		return 0;

	d = bcd_to_bin(buffer,4,4,0xee);

	if (check_bcd(buffer,4) == false)
		return 0;

	return 4;
}

int ptcl_module::parse_sg_14(unsigned char *buffer,int len,double & d)
{
	if (len < 5)
		return 0;

	d = bcd_to_bin(buffer,5,4,0xee);

	if (check_bcd(buffer,5) == false)
		return 0;

	return 5;
}

int ptcl_module::parse_sg_22(unsigned char *buffer,int len,double & d)
{
	if (len < 1)
		return 0;

	d = bcd_to_bin(buffer,1,1,0xee);

	if (check_bcd(buffer,1) == false)
		return 0;

	return 1;
}

int ptcl_module::parse_sg_23(unsigned char *buffer,int len,double & d)
{
	if (len < 3)
		return 0;

	d = bcd_to_bin(buffer,3,4,0xee);

	if (check_bcd(buffer,3) == false)
		return 0;

	return 3;
}

int ptcl_module::parse_sg_25(unsigned char *buffer,int len,double & d)
{
	if (len < 3)
		return 0;

	//d = bcd_to_bin(buffer,3,3,0xee);

	if (check_bcd(buffer,3) == false)
		return 0;

	unsigned char tmp[3];
	memcpy(tmp,buffer,3);
	unsigned char s = tmp[2] >> 7;
	tmp[2] = tmp[2] & 0x7f;
	d = bcd_to_bin(tmp,3,3,0xee);

	if(fabs(d-(-10)) < 0.000001)
	{
		return 3;
	}

	if (s == 1)
		d = 0 - d;

	return 3;
}

int ptcl_module::format_sg_15(tm *t,unsigned char *buffer)
{
	return time_to_bcd(ptcl_module::time_type_mmhhddmmyy,t,buffer);
}

int ptcl_module::parse_sg_15(unsigned char *buffer,int len,tm *t)
{
	return bcd_to_time(buffer,len,ptcl_module::time_type_mmhhddmmyy,t);
}

int ptcl_module::parse_sg_17(unsigned char *buffer,int len,tm *t)
{
	return bcd_to_time(buffer,len,ptcl_module::time_type_mmhhddmm,t);
}

int ptcl_module::parse_sg_18(unsigned char *buffer,int len,tm *t)
{
	return bcd_to_time(buffer,len,ptcl_module::time_type_mmhhdd,t);
}

int ptcl_module::format_sg_20(tm *t,unsigned char *buffer)
{
	return time_to_bcd(ptcl_module::time_type_ddmmyy,t,buffer);
}

int ptcl_module::parse_sg_20(unsigned char *buffer,int len,tm *t)
{
	return bcd_to_time(buffer,len,ptcl_module::time_type_ddmmyy,t);
}

int ptcl_module::format_sg_21(tm *t,unsigned char *buffer)
{
	return time_to_bcd(ptcl_module::time_type_mmyy,t,buffer);
}

int ptcl_module::parse_sg_21(unsigned char *buffer,int len,tm *t)
{
	return bcd_to_time(buffer,len,ptcl_module::time_type_mmyy,t);
}

int ptcl_module::format_sg_31(tm *t,unsigned char *buffer)
{
	return time_to_bcd(ptcl_module::time_type_ssmmhhddwwmmyy,t,buffer);
}

int ptcl_module::parse_sg_24(unsigned char *buffer,int len,double & d,tm *t)
{
	if (len < 7)
		return 0;

	d = bcd_to_bin(buffer,4,2,0xee);

	bcd_to_time( buffer+4, 3, ptcl_module::time_type_hhddmm, t);

	if (check_bcd(buffer,7) == false)
		return 0;

	return 7;
}

int ptcl_module::parse_sg_32(unsigned char *buffer,int len,double & d,tm *t)
{
	if (len < 7)
		return 0;


	d = bcd_to_bin(buffer,4,2,0xee);

	bcd_to_time(buffer+4, 3, ptcl_module::time_type_ddmmyy, t);

	if (check_bcd(buffer,7) == false)
		return 0;

	return 7;
}

int ptcl_module::parse_sg_33(unsigned char *buffer,int len,double & d,tm *t)
{
	if(len < 7)
		return 0;

	d = bcd_to_bin(buffer,4,2,0xee);

	bcd_to_time(buffer+4, 2, ptcl_module::time_type_mmyy, t);

	if (check_bcd(buffer,6) == false)
		return 0;

	return 6;
}

int ptcl_module::format_sgdy04_td_d(tm *t,int day_num,int meter_no,unsigned char *buffer)
{
	buffer[0] = meter_no%256;
	buffer[1] = meter_no/256;
	buffer[2] = day_num;
	return time_to_bcd(ptcl_module::time_type_ddmmyy,t,&buffer[3])+3;
}

int ptcl_module::format_sgdy04_td_m(tm *t,int mon_num,int meter_no,unsigned char *buffer)
{
	buffer[0] = meter_no%256;
	buffer[1] = meter_no/256;
	buffer[2] = mon_num;
	return time_to_bcd(ptcl_module::time_type_mmyy,t,&buffer[3])+3;
}

int ptcl_module::format_sgdy04_td_c(tm *t,int day_num,int meter_no,unsigned char *buffer)
{
	buffer[0] = meter_no%256;
	buffer[1] = meter_no/256;
	buffer[2] = day_num;
	return time_to_bcd(ptcl_module::time_type_ddmmyy,t,&buffer[3])+3;
}

bool ptcl_module::check_bcd(unsigned char *buffer, int len, unsigned char point)
{
	unsigned char tmp[256];
	memset(tmp,0,256);
	memcpy(tmp,buffer,len);
	if (point > 8)
	{
		return false;
	}
	else
	{
		tmp[len-1] = (tmp[len-1] << point);
	}
	for (int i=0;i<len;i++)
	{
		unsigned char b1 = tmp[i]/16;
		unsigned char b0 = tmp[i]%16;
		if (b0 == 0x0e && b1 == 0x0e)
			continue;
		if (b0 > 9 || b1 > 9)
			return false;
	}
	return true;
}