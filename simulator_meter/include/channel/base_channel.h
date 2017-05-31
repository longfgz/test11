#pragma once

#include <channel/channel_global.h>
#include <channel/message_block.h>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/asio.hpp>

class CHANNEL_EXPORT base_channel
{
public:

	enum channel_type
	{
		channel_type_s_tcp =	1,
		channel_type_s_udp =	3,
		channel_type_c_tcp = 	8,
		channel_type_c_tel =    2,		//电话拨号
		channel_type_ct	   =	5,		//拨号网络双通道
	};

	base_channel(unsigned int id,unsigned char type,unsigned char transport_proto,int io_thread_count,
		int ptcl_class_id,std::string ip,unsigned short int port);
	
	virtual ~base_channel(void);

	virtual void start();
	virtual void join();
	virtual void stop();
	virtual int put(unsigned int id,message_block &mb) = 0;
	virtual int get(message_block & mb) = 0;

	boost::asio::io_service & get_io_service();
	unsigned int get_id();
	unsigned char get_object_type();		//服务器类型
	unsigned char get_ransport_proto();
	int get_ptcl_class_id();
	std::string get_ip();
	unsigned short int get_port();

	unsigned char get_channel_type();
	void set_channel_type(unsigned char comm_type);


	
protected:
	unsigned int id_;
	unsigned char type_;
	unsigned char transport_proto_;
	int ptcl_class_id_;
	boost::thread_group thread_pool_;
	boost::asio::io_service io_service_;
	int io_thread_count_;
	std::string ip_;
	unsigned short int port_;
	unsigned char channel_type_;
	
};
