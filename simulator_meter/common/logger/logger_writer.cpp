#include "logger_writer.h"
#include "log_message.h"
#include <util/public.h>
#include <boost/filesystem.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <logger/logger.h>
#include <locale.h>
#include <stdio.h>
#include <iostream>
#include <fstream>

logger_writer::logger_writer()
{
	memset(root_dir_,0,512);
	exit_ = 0;
}

logger_writer::~logger_writer(void)
{
}

int logger_writer::start(char *root_dir)
{
	memset(root_dir_,0,512);
	strcpy(root_dir_,root_dir);

	thread_ptr_.reset(new boost::thread(boost::bind(&logger_writer::run, this)));

	return 0;
}

int logger_writer::join()
{
	if (thread_ptr_.get())
		thread_ptr_->join();

	return 0;
}

int logger_writer::stop()
{
	exit_ = 1;

	return 0;
}

int logger_writer::run()
{
	time_t last_clear_time = 0;
	while (!exit_)
	{
		if (time(NULL) - last_clear_time > 60*60) //一小时清理一次过时的日志
		{
			clear_log();
			last_clear_time = time(NULL);
		}

		write_message();

		sleep_m(50);
	}

	write_message();

	return 0;
}


void logger_writer::write_message()
{
	if( d_v_msg_.empty() )
		return;

	int write_count = 0;  //写若干个文件

	std::deque <V_MESSAGE> tmp_v;

	mutex_.lock();
	for(int i=0;i<10;i++)  //
	{
		if( d_v_msg_.empty() )
			break;
		
		tmp_v.push_back( d_v_msg_.front());
		d_v_msg_.pop_front();
		
	}
	mutex_.unlock();

	char frist_id[64];
	memset(frist_id,0,64);
	time_t frist_time = 0;

	for(auto it=tmp_v.begin();it!=tmp_v.end();it++)
	{
		V_MESSAGE v_msg = *it;
		log_message& message = v_msg.front();

		strcpy(frist_id,message.get_message_id());
		frist_time = message.get_message_time();
		write_message(frist_time,frist_id,v_msg);
		write_count++;
	//	printf("d=%d v=%d,w=%d\n",d_v_msg_.size(),v_msg.size(),write_count);
		v_msg.clear();
	}
	tmp_v.clear();
	
}

void logger_writer::write_message(time_t message_time,char *message_id,
								  std::vector <log_message> &v_messages)
{
	tm *message_tm = localtime(&message_time);
	if (message_tm == NULL)
		return ;

	char date_string[20];
	sprintf(date_string,"%04d-%02d-%02d",message_tm->tm_year+1900,message_tm->tm_mon+1,message_tm->tm_mday);

	char dir_name[512];
	sprintf(dir_name,"%s/%s",root_dir_,date_string);

	if (check_dir(dir_name) == false)
		return ;

	char file_name[512];
	sprintf(file_name,"%s/%s[%s].txt",dir_name,message_id,date_string);

	std::fstream file;
	file.open(file_name,std::ios_base::out|std::ios_base::app);

	for (size_t n=0;n<v_messages.size();n++)
		write_message(file,v_messages[n]);

	file.close();
}

void logger_writer::write_message(std::fstream & file,log_message & message)
{
	time_t message_time = message.get_message_time();
	unsigned char packet_dir = message.get_packet_dir();
	unsigned char *buffer = message.get_buffer();
	unsigned short int len_buffer = message.get_buffer_length();
	tm *message_tm = localtime(&message_time);
	if (message_tm == NULL)
		return ;

	char text[log_message::MAX_BUFFER_LENGTH*4];
	memset(text,0,log_message::MAX_BUFFER_LENGTH*4);

	char time_string[40];
	sprintf(time_string,"%04d-%02d-%02d %02d:%02d:%02d   ",message_tm->tm_year+1900,message_tm->tm_mon+1,message_tm->tm_mday,message_tm->tm_hour,message_tm->tm_min,message_tm->tm_sec);
	strcat(text,time_string);

	if (packet_dir > 0)
	{
		size_t pos = strlen(text);
		logger::packet_to_string(packet_dir,buffer,len_buffer,text+pos);
	}
	else
	{
		//将log的类型前缀加上
		switch(message.get_level())
		{
		case  logger::log_level::error:
			strcat(text,"[***ERROR***]\t");
			break;
		case logger::log_level::warning:
			strcat(text,"[**WARNING**]\t");
			break;
		case logger::log_level::info:
			strcat(text,"[***INFO****]\t");
			break;
		case logger::log_level::debug:
			//strcat(text,"[***DEBUG***]\t");
			break;
		case logger::log_level::trace:
			//trace don't add any prefix.
			break;
		default:
			break;
		}

		strcat(text,(char *)buffer);
	}

	strcat(text,"\n");

	file.write(text,strlen(text));
}

void logger_writer::insert_message(log_message & message)
{
	mutex_.lock();

	/*if (250000 >= (int)d_message_.size())
		d_message_.push_back(message);
		*/
	int len = d_v_msg_.size();
	char *msgid= message.get_message_id();
	bool find=false;
	for(int i=0;i<len;i++)
	{
		V_MESSAGE & v = d_v_msg_[i];
		if( strcmp(msgid,v.front().get_message_id())==0 )
		{
			v.push_back(message);
			find = true;
			break;
		}
	}
	if( !find && d_v_msg_.size()<50000 )  //5万块表
	{
		V_MESSAGE v;
		v.push_back(message);
		d_v_msg_.push_back(v);
	}
	mutex_.unlock();
}

bool logger_writer::check_dir(char *dir)
{
	try
	{
		if (boost::filesystem::is_directory(dir))
			return true;

		char up_dir[512];
		memset(up_dir,0,512);
		strcpy(up_dir,dir);

		char *p = strrchr(up_dir,'/');
		if (p == NULL)
			return false;

		p[0] = 0;
		if (check_dir(up_dir) == true)
		{
			if (boost::filesystem::create_directory(dir))
				return true;
		}
	}
	catch (...)
	{
		return false;
	}

	return false;
}


void logger_writer::clear_log()
{
	boost::gregorian::date day = boost::gregorian::day_clock::local_day() - boost::gregorian::days(10);
	char date_string[20];
	sprintf(date_string,"%04d-%02d-%02d",(int)day.year(),(int)day.month(),(int)day.day());

	try
	{
		boost::filesystem::path dir(root_dir_);
		boost::filesystem::directory_iterator end_it;
		for (boost::filesystem::directory_iterator it(dir);it != end_it;++it)
		{
			std::string dir_name = it->path().filename().string();////boost 1-52
			//std::string dir_name = it->leaf();////boost 1-43
			if (strcmp(date_string,dir_name.c_str()) > 0)
			{
				std::string delete_name = root_dir_;
				delete_name += "/";
				delete_name += dir_name;
				boost::filesystem::remove_all(delete_name);
			}
		}
	}
	catch (boost::filesystem::filesystem_error e)
	{

	}
}