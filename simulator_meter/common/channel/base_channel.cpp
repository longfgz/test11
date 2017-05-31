#include <channel/base_channel.h>

base_channel::base_channel(unsigned int id,unsigned char type,unsigned char transport_proto,int io_thread_count,
						   int ptcl_class_id,std::string ip,unsigned short int port)
{
	id_ = id;
	type_ = type;
	transport_proto_ = transport_proto;
	io_thread_count_ = io_thread_count;
	ptcl_class_id_ = ptcl_class_id;
	ip_ = ip;
	port_ = port;
	channel_type_ = 0;
}

base_channel::~base_channel(void)
{
}


void base_channel::start()
{
	for (int i=0;i<io_thread_count_;i++)
	{
		thread_pool_.create_thread(boost::bind(&boost::asio::io_service::run, &io_service_));
	}
}

void base_channel::join()
{
	thread_pool_.join_all();
}


void base_channel::stop()
{
	io_service_.stop();
}

boost::asio::io_service & base_channel::get_io_service()
{
	return io_service_;
}

unsigned int base_channel::get_id()
{
	return id_;
}

unsigned char base_channel::get_object_type()
{
	return type_;
}

int base_channel::get_ptcl_class_id()
{
	return ptcl_class_id_;
}

std::string base_channel::get_ip()
{
	return ip_;
}

unsigned short int base_channel::get_port()
{
	return port_;
}

void base_channel::set_channel_type(unsigned char comm_type)
{
	channel_type_ = comm_type;
}

unsigned char base_channel::get_channel_type()
{
	return channel_type_;
}