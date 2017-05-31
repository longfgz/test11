#include <logger/logger.h>
#include "log_message.h"

#include "sqlite_writer.h"
#include "logger_writer.h"
#include <stdarg.h>
#include <util/public.h>
#include <stdio.h>
#include <boost/filesystem.hpp>
#include <boost/tokenizer.hpp> 
#include <boost/algorithm/string.hpp>   

#if defined(WIN32)
	#include <windows.h>
#else

#endif


logger *logger::instance_ = NULL;

//Attention: the prefix len should not exceed the PACEKT_PREFIX_MAX_LEN
std::string logger::packet_prefixes[packet_type::packet_type_max] = 
{
	"UNDEFINE PACKET TYPE",			//undefine
	"receive from [Terminal]  ",	//packet_type_recv
	"send to [Terminal]  ",			//packet_type_send
	"receive error  ",				//packet_type_recv_error
	"send error  ",					//packet_type_send_error
	"receive from [Router]  ",		//packet_type_from_router
	"send to [Router]  ",			//packet_type_to_router
	"receive from [KMS]  ",			//packet_type_from_kms
	"send to [KMS]  ",				//packet_type_to_kms
	"receive from [Gateway]",		//packet_type_from_gateway
	"send to [Gateway]",			//packet_type_from_gateway
};

logger::logger(void)
{
	exit_ = 0;
	level_ = log_level::trace;

	log_to_sqlite_ = false;
	sq_writer_ = new sqlite_writer();
	logger_writer_ = new logger_writer();
	
}

logger::~logger(void)
{

}

logger *logger::get_instance()
{
	if (instance_ == NULL)
		instance_ = new logger();
	return instance_;
}

int logger::start(const char *root_dir)
{
	if (root_dir == NULL || strlen(root_dir) == 0)
		return 0;

	char dir[512];
	memset(dir,0,512);
	get_current_dir(dir,512);
	strcpy_s(root_name_,root_dir);
	strcpy(config_file_name,dir);
	strcat(config_file_name,"/log_config.ini");
	log_to_sqlite_ = get_log_dest();
	load_focus_config();

	strcat_s(dir,"/log/");
	strcat_s(dir,root_dir);
	if( log_to_sqlite_ )
		sq_writer_->start(dir);
	else
		logger_writer_->start(dir);
	
	return 0;
}

void logger::set_level(int level)
{
	assert( level>=none && level<=trace );
	const char* s[]={"None","Error","Warning","Info","Debug","Trace","Unknown"};
	LOG_INFO("system","Log Level is %s",s[level]);
	level_ = level;
}


int logger::join()
{
	if( log_to_sqlite_ )
		sq_writer_->join();
	else
		logger_writer_->join();

	return 0;
}

int logger::stop()
{
	if( log_to_sqlite_ )
		sq_writer_->stop();
	else
		logger_writer_->stop();

	return 0;
}


fpos_t get_file_size(std::string str_file)
{
	std::ifstream in(str_file.c_str(), std::ios::binary);
	in.seekg(0, std::ios::end);
	std::streampos ps = in.tellg();
	in.close();
	return ps.seekpos();
}
inline static std::string& trim(std::string& text)  
{  
	if(!text.empty())  
	{  
		text.erase(0, text.find_first_not_of(" \n\r\t;,"));  
		text.erase(text.find_last_not_of(" \n\r\t;,") + 1);  
	}  
	return text;  
}

bool logger::get_log_dest()
{
	char tmp[1024];
	DWORD dwRet = GetPrivateProfileString(root_name_,"log_dest","",tmp,1024,config_file_name);
	if( dwRet>0 )
	{
		std::string s = tmp;
		trim(s);
		if( s=="txt")
			return 0;
		else if( s=="sqlite")
			return 1;
	}
	
	WritePrivateProfileString(root_name_,"log_dest","txt",config_file_name);
	return 0;
}

void logger::load_focus_config()
{
	v_focus_terminal_.clear();

	char tmp[1024];
	DWORD dwRet = GetPrivateProfileString(root_name_,"focus","",tmp,1024,config_file_name);
	if( dwRet<=0 )
	{
		WritePrivateProfileString(root_name_,"focus",";\0",config_file_name);
		return;
	}

	 std::vector<std::string> vecSegTag;   
	 boost::split(vecSegTag, tmp, boost::is_any_of((";,\n"))); 
	 for(int i=0;i<vecSegTag.size();i++)
	 {
		 std::string s = vecSegTag[i];
		 trim(s);
		 int len = s.length();
		 
		 if( s.length()>2 || s.compare("0")==0 )
		 {
			 v_focus_terminal_.push_back(s);
		 }
	 }
}

void logger::logger_info(int level,char *message_id,const char * format, ...)
{
	//检查重点监控终端配置文件
	//减少cpu占用，每10次才读一次时间 
	static int s_count=0;
	static time_t s_tm_load_config;
	s_count++;
	if( (s_count%10)==1 )
	{
		time_t now = time(NULL);
		if( abs(now-s_tm_load_config)>=30 )
		{
			load_focus_config();
			s_tm_load_config = now;
		}
	}

	if (level_ < level )
	{
		bool find = false;
		for(int i=0;i<v_focus_terminal_.size();i++)
		{
			std::string s = v_focus_terminal_[i];
			if( s.compare(message_id)==0 || s=="0" )
			{
				find = true;
				break;
			}
		}
		if( !find )
			return ;
	}

	va_list arg;
	va_start(arg, format);
	char info[LOG_INFO_MAX_LEN];
	memset(info,0,LOG_INFO_MAX_LEN);
	vsnprintf(info,LOG_INFO_MAX_LEN-1,format,arg);
	va_end(arg);

#ifndef NDEBUG
	print_info(info);
#endif

	log_message message;
	message.set_level( level );
	message.set_message_id(message_id);
	message.set_packet_dir(0);
	message.set_buffer((unsigned char *)info,(unsigned short int)strlen(info));

	if( log_to_sqlite_ )
		sq_writer_->insert_message(message);
	else
		logger_writer_->insert_message(message);

}


//Dengxm adds 

void logger::logger_packet(int level,unsigned int id,unsigned char dir_type,
						   unsigned char *buffer,short int buf_size)
{
	char strid[64];
	sprintf_s(strid," %d",id);
	logger_packet(level,strid,dir_type,buffer,buf_size);
}

void logger::logger_packet(int level,char *message_id,unsigned char dir_type,
						   unsigned char *buffer,short int buf_size)
{
#ifndef NDEBUG
	print_packet(message_id, dir_type, buffer, buf_size);
#endif

	if (level_ < level )
	{
		bool find = false;
		for(int i=0;i<v_focus_terminal_.size();i++)
		{
			std::string s = v_focus_terminal_[i];
			if( s.compare(message_id)==0 || s=="0" )
			{
				find = true;
				break;
			}
		}
		if( !find )
			return ;
	}

	int max_size = log_message::MAX_BUFFER_LENGTH;
	int count = (buf_size / max_size) + 1;
	int cur_size = 0;
	for (int n=0;n<count;n++)
	{
		if (buf_size == 0)
			break;

		log_message message;
		message.set_level(level);
		message.set_message_id(message_id);
		message.set_packet_dir(dir_type);

		int print_size = buf_size;
		if (print_size > max_size)
			print_size = max_size;

		message.set_buffer(buffer+cur_size,print_size);
		cur_size = cur_size + print_size;
		buf_size = buf_size - print_size;

		if( log_to_sqlite_ )
			sq_writer_->insert_message(message);
		else
			logger_writer_->insert_message(message);

	}
}

//ADD by liugh 2016-8-23
void logger::logger_packet(char *message_id,unsigned char dir_type,unsigned char *buffer,short int buf_size)
{
	logger_packet(log_level::info,message_id,dir_type,buffer,buf_size);
}

void logger::logger_packet(unsigned int id,unsigned char dir_type,unsigned char *buffer,short int buf_size)
{
	//默认使用info 级别
	logger_packet(log_level::debug,id,dir_type,buffer,buf_size);
}


///----------------------------------------------
void logger::print_packet(	char *message_id,unsigned char dir_type,
							unsigned char *buffer,short int buf_size)
{
	print_mutex_.lock();

	if (buf_size > 2048)
		return ;

	time_t t;
	time(&t);
	tm *cur_tm = localtime(&t);
	char time_string[40];
	sprintf(time_string,"%04d-%02d-%02d %02d:%02d:%02d   ",cur_tm->tm_year+1900,cur_tm->tm_mon+1,cur_tm->tm_mday,cur_tm->tm_hour,cur_tm->tm_min,cur_tm->tm_sec);
	std::cout<<time_string;

	std::cout<<message_id;
	std::cout<<"   ";

	char packet[2048*4];
	memset(packet,0,2048*4);
	packet_to_string(dir_type,buffer,buf_size,packet);
	std::cout<<packet;

	std::cout<<std::endl;

	print_mutex_.unlock();
}

void logger::print_info(const char * format, ...)
{
	print_mutex_.lock();

	time_t t;
	time(&t);
	tm *cur_tm = localtime(&t);
	char time_string[40];
	sprintf(time_string,"%04d-%02d-%02d %02d:%02d:%02d   ",cur_tm->tm_year+1900,cur_tm->tm_mon+1,cur_tm->tm_mday,cur_tm->tm_hour,cur_tm->tm_min,cur_tm->tm_sec);
	std::cout<<time_string;

	va_list arg;
	va_start(arg, format);
	char info[512];
	memset(info,0,512);
	vsnprintf(info,511,format,arg);

	//std::cout<<info<<std::endl;
	puts(info);

	print_mutex_.unlock();
}

void logger::packet_to_string(	unsigned char dir_type,
							  unsigned char *buffer,short int buf_size,char *dst_buffer)
{
	//防止buffer长度越界
	if(buf_size >= log_message::MAX_BUFFER_LENGTH*4 - PACEKT_PREFIX_MAX_LEN)
	{
		strcat(dst_buffer,"packet length exceed the Max buffer length.");
		return;
	}
	
	//dir
	strcat(dst_buffer,packet_prefixes[dir_type].c_str());

	//区别于纯报文和kms交互的json消息，采用不同的打印策略
	if(dir_type == packet_type::packet_type_from_kms || 
		dir_type == packet_type::packet_type_to_kms)
	{
		strcat(dst_buffer,"\n");
		strcat(dst_buffer,(char*)buffer);
	}
	else
	{
		//size
		char len_string[40];
		sprintf(len_string,"%d bytes",buf_size);
		strcat(dst_buffer,len_string);
		strcat(dst_buffer,"\n");

		//packet
		for (int n=0;n<buf_size;n++)
		{
			char temp[4];
			sprintf(temp,"%02X ",buffer[n]);
			if (n!=0 && n % 32 == 0)
				strcat(dst_buffer,"\n");

			strcat(dst_buffer,temp);
		}
	}
}

/********************************************************************/
/*description: get the single file-name from the given filefullpath.*/
/*input:															*/
/*		filefullpath: file name with full path. it can be file name	*/
/*		with the path,"D:\\amr\\fee\\fee_dlms47\\tcp_channel.hpp"	*/
/*		eg. Also it can be single name like: "meter_handler.cpp".	*/
/*output:															*/
/*		single file name of the file.								*/
/********************************************************************/
char* logger::file_name(char* filefullpath)
{
	char c_filename[64]= {0};
	
	if(filefullpath != NULL)
	{
		std::string str_filefullpath = filefullpath;

		//if find, return the index of the '\\'; otherwise, return -1;	
		int nPos = str_filefullpath.find_last_of('\\');
		std::string filename = str_filefullpath.substr(nPos+1,str_filefullpath.length());
		return (filefullpath+nPos+1);
	}
	return NULL;
}

