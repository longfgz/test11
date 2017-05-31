#pragma once
#include <channel/base_channel.h>
#include <boost/unordered_map.hpp>
#include <channel/base_channel_handler.h>
#include <logger/logger.h>

template <class channel_handler>
class CHANNEL_EXPORT event_channel : public base_channel
{
public:
	typedef typename boost::shared_ptr<channel_handler> handler_ptr;

protected:
	std::deque<message_block> d_r_mb_;
	boost::recursive_mutex mutex_r_mb_;
	boost::unordered_map <unsigned int,handler_ptr> map_handlers_;
	typedef typename boost::unordered_map<unsigned int, handler_ptr>::iterator  map_handlers_iterator;
	boost::shared_mutex mutex_handler_;

public:
	event_channel(unsigned int id,unsigned char type,unsigned char transport_proto,int io_thread_count,
		int ptcl_class_id,std::string ip,unsigned short int port)
		:base_channel(id,type,transport_proto,io_thread_count,ptcl_class_id,ip,port)
	{
	}

	virtual ~event_channel(void)
	{
	}

	virtual int put(unsigned int id,message_block &mb)
	{
		boost::shared_lock<boost::shared_mutex> shared_lock(mutex_handler_);

		int result = -1;
		if (id == 0)
		{
			map_handlers_iterator it;
			for (it = map_handlers_.begin();it != map_handlers_.end();++it)
			{
				//if (it->second->get_state() == base_channel_handler::chs_online)
				//{
					it->second->put(mb);
					result = 0;
				//}
			}
		}
		else
		{
			map_handlers_iterator it = map_handlers_.find(id);
			if (it != map_handlers_.end())
			{
				if (it->second->get_state() >= base_channel_handler::chs_online)
				{
					it->second->put(mb);
					result = 0;
				}
			}
		}

		return result;
	}

	virtual int get(message_block & mb)
	{
		if (!d_r_mb_.empty())
		{
			boost::recursive_mutex::scoped_lock scoped_lock(mutex_r_mb_);
			if (!d_r_mb_.empty())
			{
				mb = d_r_mb_.front();
				d_r_mb_.pop_front();
				return 0;
			}
			return -1;
		}
		return -1;
	}

	void buffer_event(message_block & mb)
	{
		boost::recursive_mutex::scoped_lock scoped_lock(mutex_r_mb_);
		d_r_mb_.push_back(mb);
	}

	void insert_handler(handler_ptr handler)
	{
		boost::unique_lock<boost::shared_mutex> shared_lock(mutex_handler_);

		unsigned int id = handler->get_id();
		map_handlers_iterator it = map_handlers_.find(id);
		if (it != map_handlers_.end())
		{
			LOG_WARNING("system","The handler id=[%d] is already in the channel map. insert handler failed!",id);
			return ;
		}

		map_handlers_[id] = handler;

		if (transport_proto_ == 1)
		{
			//LOG_DEBUG("system","tcp channel port(%d): id(%08x) online ++ %d",port_,id,map_handlers_.size());
		}
		else
		{
			//LOG_DEBUG("system","udp channel port(%d): id(%08x) online ++ %d",port_,id,map_handlers_.size());
		}
	}

	void remove_handler(unsigned int id)
	{
		boost::unique_lock<boost::shared_mutex> shared_lock(mutex_handler_);
		map_handlers_iterator it = map_handlers_.find(id);
		if (it != map_handlers_.end())
		{
			map_handlers_.erase(it);

			if (transport_proto_ == 1)
			{
			//	LOG_DEBUG("system","tcp channel port(%d): id(%08x) offline -- %d",port_,id,map_handlers_.size());
			}
			else
			{
			//	LOG_DEBUG("system","udp channel port(%d): id(%08x) offline -- %d",port_,id,map_handlers_.size());
			}

		}
		else
		{
			LOG_WARNING("system","The handler id=[%d] is not in the channel map,remove handler failed!",id);
		}
	}

	bool find_handler(unsigned int id,handler_ptr & handler)
	{
		boost::shared_lock<boost::shared_mutex> shared_lock(mutex_handler_);

		map_handlers_iterator it = map_handlers_.find(id);
		if (it != map_handlers_.end())
		{
			handler = it->second;
			return true;
		}

		return false;
	}
};
