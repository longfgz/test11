#pragma once

#include <logger/logger_global.h>
#include <boost/thread/mutex.hpp>
#include <boost/thread.hpp>
#include <util/public.h>

#define PACEKT_PREFIX_MAX_LEN 32
#define LOG_INFO_MAX_LEN 512

class LOGGER_EXPORT logger
{
protected:
	logger(void);
	~logger(void);
public:
	static logger *get_instance();

	int start(const char *root_dir);

	void set_level(int level = 2);
	void set_log_dest(int dest=1); //0:none,1:file,2,sqlite
	int join();
	int stop();

	enum packet_type
	{
		packet_type_undefine     = 0,
		packet_type_recv		 = 1,	//从meter收电表数据时使用
		packet_type_send		 = 2,	//发电表数据到meter时使用
		packet_type_recv_error   = 3,
		packet_type_send_error   = 4,
		packet_type_from_router  = 5,	//从router收数据时使用
		packet_type_to_router	 = 6,	//向router发数据时使用
		packet_type_from_kms     = 7,	//向KMS发送数据时使用
		packet_type_to_kms       = 8,	//向KMS收送数据时使用
		packet_type_from_gw		 = 9,	//向gateway发数据时使用
		packet_type_to_gw        = 10,	//向gateway收数据时使用
		packet_type_max          = 24,
	};

	static std::string packet_prefixes[packet_type_max];

	//type 见packet_type
	enum log_level
	{
		 none = 0,
		 error=1,
		 warning=2,
		 info=3,
		 debug=4,
		 trace=5,
	};

	//外部接口1 logger_info 
	//log_level需要设定,建议在记录Error、Warning、Info、运行Status等事件信息时统一使用此接口
	void logger_info(int level,char *message_id,const char * format, ...);

	//外部接口2 logger_packet 
	//默认log_level为info,建议在打印发送/接收数据报文时统一使用此接口
	void logger_packet(char *message_id,unsigned char dir_type,unsigned char *buffer,short int buf_size);
	void logger_packet(unsigned int id,unsigned char dir_type,unsigned char *buffer,short int buf_size);
	void logger_packet(	int level,char *message_id,unsigned char dir_type,
						unsigned char *buffer,short int buf_size);
	void logger_packet(int level,unsigned int id,unsigned char dir_type,
						   unsigned char *buffer,short int buf_size);

	//耗时，不可频繁打印
	//type 见packet_type
	void print_packet(	char *message_id,unsigned char dir_type,
						unsigned char *buffer,short int buf_size);

	//耗时，不可频繁打印
	void print_info(const char * format, ...);
	static void packet_to_string(unsigned char dir_type,unsigned char *buffer,short int buf_size,char *dst_buffer);

public:
	char* file_name(char* filefullpath);	// just for get the single filename from one full path filename.

protected:
	void load_focus_config();
	bool get_log_dest();
	std::vector<std::string> v_focus_terminal_;
	char config_file_name[512];
	char root_name_[120];  //eg: fee_dlms47
	static logger *instance_;
	int exit_;
	int level_;
	boost::mutex print_mutex_;

	class sqlite_writer *sq_writer_;
	class logger_writer *logger_writer_;
	bool log_to_sqlite_;

};

typedef logger::log_level LOG_LEVEL;

#define FILE_NAME(path) logger::get_instance()->file_name(path)

#define PRINT_LOG(level,out_file,fmt,...) \
	logger::get_instance()->logger_info(level,out_file,"%s %d\n"##fmt,FILE_NAME(__FILE__), __LINE__,##__VA_ARGS__)

#define PRINT_RAW_LOG(level,out_file,fmt,...) \
	logger::get_instance()->logger_info(level,out_file,fmt,##__VA_ARGS__)

/*---------------------------------------standard interfaces for print log----------------------------------------*/
#define LOG_ERROR(out_file,fmt,...)\
	logger::get_instance()->logger_info(LOG_LEVEL::error,out_file,fmt##"\t\t%s %d",##__VA_ARGS__,FILE_NAME(__FILE__), __LINE__)

#define LOG_WARNING(out_file,fmt,...)\
	logger::get_instance()->logger_info(LOG_LEVEL::warning,out_file,fmt##"\t\t%s %d",##__VA_ARGS__,FILE_NAME(__FILE__), __LINE__)

#define LOG_INFO(out_file,fmt,...)\
	logger::get_instance()->logger_info(LOG_LEVEL::info,out_file,fmt##"\t\t%s %d",##__VA_ARGS__,FILE_NAME(__FILE__), __LINE__)

#define LOG_DEBUG(out_file,fmt,...)\
	logger::get_instance()->logger_info(LOG_LEVEL::debug,out_file,fmt##"\t\t%s %d",##__VA_ARGS__,FILE_NAME(__FILE__), __LINE__)

#define LOG_TRACE(out_file,fmt,...)\
	logger::get_instance()->logger_info(LOG_LEVEL::trace,out_file,fmt##"\t\t%s %d",##__VA_ARGS__,FILE_NAME(__FILE__), __LINE__)

#define LOG_PACKET	logger::get_instance()->logger_packet

/*---------------------------------------standard interfaces for print log----------------------------------------*/

/*---------------------------------------Log interfaces for AMR---------------------------------------------------*/
#ifndef NDEBUG  //debug
#define AMR_PRINT_RAW_LOG PRINT_RAW_LOG
#define AMR_LOG_ERROR LOG_ERROR
#define AMR_LOG_WARNING LOG_WARNING
#define AMR_LOG_INFO LOG_INFO
#define AMR_LOG_DEBUG LOG_DEBUG
#define AMR_LOG_TRACE LOG_TRACE
#else    //release
#define AMR_PRINT_RAW_LOG(level,out_file,fmt,...)\
	logger::get_instance()->logger_info(level,out_file,fmt,##__VA_ARGS__);\
	PRINT_RAW_LOG(level,out_file,fmt,##__VA_ARGS__)
#define AMR_LOG_ERROR(out_file,fmt,...)\
	logger::get_instance()->print_info(fmt,##__VA_ARGS__);\
	LOG_ERROR(out_file,fmt,##__VA_ARGS__)
#define AMR_LOG_WARNING(out_file,fmt,...)\
	logger::get_instance()->print_info(fmt,##__VA_ARGS__);\
	LOG_WARNING(out_file,fmt,##__VA_ARGS__)
#define AMR_LOG_INFO(out_file,fmt,...)\
	logger::get_instance()->print_info(fmt,##__VA_ARGS__);\
	LOG_INFO(out_file,fmt,##__VA_ARGS__)
#define AMR_LOG_DEBUG(out_file,fmt,...)\
	logger::get_instance()->print_info(fmt,##__VA_ARGS__);\
	LOG_DEBUG(out_file,fmt,##__VA_ARGS__)
#define AMR_LOG_TRACE(out_file,fmt,...)\
	logger::get_instance()->print_info(fmt,##__VA_ARGS__);\
	LOG_TRACE(out_file,fmt,##__VA_ARGS__)
#endif
/*---------------------------------------Log interfaces for AMR---------------------------------------------------*/