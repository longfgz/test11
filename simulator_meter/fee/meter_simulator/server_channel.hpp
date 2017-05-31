#pragma once

#include <channel/event_channel.hpp>
#include <boost/asio.hpp>

#include <channel/base_channel_handler.h>
#include <boost/enable_shared_from_this.hpp>
#include <logger/logger.h>
#include "frame_buffer.h"
#include <ptcl_codec/ptcl_packet_dlms47.h>
#include "meter_handler.h"
#include "simulator_config.h"
#include "thread_pool.hpp"

template <class ptcl_handler>
class server_channel;



template <class ptcl_handler>
class server_channel_handler : public session_channel_handler,
	public boost::enable_shared_from_this<server_channel_handler<ptcl_handler> >
{
public:
	server_channel<ptcl_handler> *channel_;
	boost::asio::strand strand_;
	boost::asio::ip::tcp::socket socket_;
	message_block r_mb_;
	std::deque<message_block> d_w_mb_;
	boost::recursive_mutex mutex_w_mb_;
	ptcl_handler ptcl_handler_;
	volatile unsigned int w_op_counts_;
	volatile int removed_;
	boost::asio::ip::tcp::endpoint remote_endpoint_;
	meter_handler_ptr meter_handler_;
	int latency_ms_;
	boost::asio::deadline_timer latency_timer_;
	byte *sys_title_;

private:
	frame_buffer recv_buf_;

public:
	server_channel_handler(server_channel<ptcl_handler> *channel)
		:session_channel_handler(0)
		,socket_(channel->get_io_service())
		,strand_(channel->get_io_service())
		,latency_timer_(channel->get_io_service())
	{
		channel_ = channel;

		w_op_counts_ = 0;
		removed_ = 0;
		latency_ms_ = simulator_config::get_instance()->get_latency_ms();
	}

	~server_channel_handler(void)
	{
		release_session();
	}

	void start(unsigned int meter_id, byte *sys_title)
	{
		boost::asio::ip::tcp::no_delay tcp_nodelay(true);
		socket_.set_option(tcp_nodelay);
		boost::asio::socket_base::receive_buffer_size rbs(256 * 1024);
		socket_.set_option(rbs);

		remote_endpoint_ = socket_.remote_endpoint();
		id_ = meter_id;
		channel_->insert_handler(boost::enable_shared_from_this<server_channel_handler<ptcl_handler> >::shared_from_this());
		start_read(1024);
		sys_title_ = sys_title;
//		start_login_timer();
	}

	server_channel<ptcl_handler> *get_channel()
	{
		return channel_;
	}

	virtual void release_session()
	{
		if (meter_handler_)
			meter_handler_.reset();
	}

	void put(message_block &mb)
	{
		boost::recursive_mutex::scoped_lock scoped_lock(mutex_w_mb_);

		d_w_mb_.push_back(mb);
			

		if (w_op_counts_ == 0 && !d_w_mb_.empty() && state_ == chs_online)
			start_write();
			//start_latency_timer();

	}

	void start_latency_timer()
	{
		int latency = rand() % latency_ms_ + latency_ms_/2;
		latency_timer_.expires_from_now(boost::posix_time::seconds(latency/1000));

		latency_timer_.async_wait(
			strand_.wrap(boost::bind(
			&server_channel_handler<ptcl_handler>::handle_latency_timer,
			server_channel_handler<ptcl_handler>::shared_from_this(),
			boost::asio::placeholders::error)));
	}

	void handle_latency_timer(const boost::system::error_code& e)
	{
		if (!e && !removed_)
		{
			start_write();
			latency_timer_.cancel();
		}
	}

	meter_handler_ptr create_meter_handler()
	{
		meter_handler_ptr mh_ptr;

		unsigned int meter_id = 1;
		

		std::string lls = simulator_config::get_instance()->get_lls();
		std::string hls = simulator_config::get_instance()->get_hls();
		std::string ek = simulator_config::get_instance()->get_ek();
		std::string ak = simulator_config::get_instance()->get_ak();

		mh_ptr.reset(new meter_handler(sys_title_,meter_id,
			boost::enable_shared_from_this<server_channel_handler<ptcl_handler> >::shared_from_this(),
			channel_->get_io_service()));

		return mh_ptr;
	}

	meter_handler_ptr get_meter_handler()
	{
		return meter_handler_;
	}

	boost::asio::ip::tcp::socket & get_socket()
	{
		return socket_;
	}

protected:
	void stop()
	{
		if (!removed_)
		{

			socket_.close();
//			stop_heart_beat_timer();
			if (state_ != chs_idle)
			{
				state_ = chs_offline;

				boost::shared_ptr<server_channel_handler<ptcl_handler> > handler;
				channel_->find_handler(id_,handler);
				if (handler == server_channel_handler<ptcl_handler>::shared_from_this())
				{
					channel_->remove_handler(id_);
				}
			}

			removed_ = 1;
		}
	}


	void start_write()
	{
		message_block mb = d_w_mb_.front();
		std::size_t bytes_to_write = mb.length();

		socket_.async_write_some(
			boost::asio::buffer(mb.rd_ptr(), bytes_to_write),
			strand_.wrap(boost::bind(
			&server_channel_handler<ptcl_handler>::handle_write,
			server_channel_handler<ptcl_handler>::shared_from_this(),
			bytes_to_write,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred)));

		++w_op_counts_;
	}

	

	void handle_write(std::size_t bytes_to_write,
		const boost::system::error_code& e,
		std::size_t bytes_transferred)
	{
		if (1)
		{
			boost::recursive_mutex::scoped_lock scoped_lock(mutex_w_mb_);

			--w_op_counts_;

			if (!e && !removed_)
			{
				//flux_table::get_instance()->update_down_total_bytes(id_, bytes_transferred)
				
				if (bytes_to_write == bytes_transferred)
				{
					d_w_mb_.pop_front();
				}
				else
				{
					d_w_mb_.front().rd_ptr(bytes_transferred);
				}

				if (w_op_counts_ == 0 && !d_w_mb_.empty())
					start_write();

				return;
			}
		}

		logger::get_instance()->print_info("%X Failed to write data", id_);
		stop();
	}

	void start_read(std::size_t bytes_to_read)
	{
		socket_.async_read_some(
			boost::asio::buffer(r_mb_.wr_ptr(), bytes_to_read),
			strand_.wrap(boost::bind(
			&server_channel_handler<ptcl_handler>::handle_read,
			server_channel_handler<ptcl_handler>::shared_from_this(), bytes_to_read,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred)));
	}

	void handle_read(std::size_t bytes_to_read,
		const boost::system::error_code& e,
		std::size_t bytes_transferred)
	{

		if (!e && !removed_)
		{
			if (!meter_handler_)
			{
				meter_handler_ = create_meter_handler();
			}
			state_ = chs_online;

			r_mb_.wr_ptr(bytes_transferred);
				
			recv_buf_.add(r_mb_.rd_ptr(), r_mb_.length());	
			ptcl_handler_.process_packet(recv_buf_, server_channel_handler<ptcl_handler>::shared_from_this());
			recv_buf_.clear();


			r_mb_.reset();
			start_read(1024);
			return;
		}

		stop();
	}

};


//--------------------------------------------------------------------------------

template<class ptcl_handler>
class server_channel : public event_channel<server_channel_handler<ptcl_handler> >
{
protected:
	boost::asio::ip::tcp::acceptor acceptor_;
	boost::asio::strand strand_;
	base_channel *router_channel_;
	unsigned int meter_id_;
	byte sys_title_[16];
	thread_pool thread_pool_;

public:
	server_channel(unsigned int id,unsigned char type,int io_thread_count,
		int ptcl_class_id,std::string ip,unsigned short int port,base_channel *router_channel, int thread_pool_count)
		:event_channel<server_channel_handler<ptcl_handler> >(id,type,1,io_thread_count,ptcl_class_id,ip,port)
		,acceptor_(this->io_service_,boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(),port))
		,strand_(this->io_service_)
		,thread_pool_(thread_pool_count)
	{
		router_channel_ = router_channel;
		meter_id_ = 1;
		memset(sys_title_,0,sizeof(sys_title_));
		transfer_systitle(sys_title_);
	}

	virtual ~server_channel(void)
	{
	}

	virtual void start()
	{
		thread_pool_.start();
		start_accept();
		base_channel::start();
	}

	virtual void join()
	{
		thread_pool_.join();
	}

	virtual void stop()
	{
		thread_pool_.stop();
	}

	unsigned int get_next_meter_id()
	{
		if (meter_id_ > 100000)
		{
			meter_id_ = 1;
		}
		return ++meter_id_;
	}

	thread_pool *get_thread_pool()
	{
		return &thread_pool_;
	}


protected:
	void start_accept()
	{
		boost::shared_ptr<server_channel_handler<ptcl_handler> > handler(
			new server_channel_handler<ptcl_handler>(this));

		acceptor_.async_accept(	handler->get_socket(),
								strand_.wrap(boost::bind(
								&server_channel<ptcl_handler>::handle_accept,
								this, handler, boost::asio::placeholders::error)));
	}

	void handle_accept(	boost::shared_ptr<server_channel_handler<ptcl_handler> > handler,
						const boost::system::error_code& e)
	{
		if (!e)
		{
			handler->start(get_next_meter_id(),sys_title_);
			start_accept();
		}
	}

	void transfer_systitle(byte *sys_title)
	{
		std::string systitle = simulator_config::get_instance()->get_system_title();
		//byte sys_byte[16];
		unsigned int lenth;
	//	Common::
	//	= {0x4d,0x4d,0x4d,0x00,0x00,0xBC,0x61,0x4e,};
		Common::Util_tools::hex_to_bytes(systitle, sys_title, lenth);
	}
};
