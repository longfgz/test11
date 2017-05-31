#pragma once

#include <boost/unordered_map.hpp>
#include <ptcl_codec/ptcl_module_dlms47.h>
#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <channel/message_block.h>
#include <channel/base_channel_handler.h>
#include "encry_context.h"
#include "access_security.h"
#include "data_security.h"

class session_channel_handler : public base_channel_handler
{
public:
	session_channel_handler(unsigned int id)
		:base_channel_handler(id)
	{
	}
	virtual void release_session() = 0;
};

class meter_handler : public boost::enable_shared_from_this<meter_handler>
{
public:
	enum meter_handler_state
	{
		mhs_null,
		mhs_cosem_connect,
		mhs_data,
		mhs_disc,
		mhs_over
	};

	enum image_transfer_state
	{
		image_trans_initiated	=	1,
		image_trans_verify		=	3,
		image_trans_active		=	6,

	};

public:
	meter_handler(unsigned char* systitle,unsigned int meter_no, 
					boost::shared_ptr<session_channel_handler> handler, boost::asio::io_service &ios);
	virtual ~meter_handler(void);
	meter_handler_state get_state(){return handler_state_;}
	void reset_state(meter_handler_state st = mhs_null);

public:
	virtual bool handle_response(message_block* mb);
	void start();
	void stop();

	bool call_encrypt(unsigned int id,message_block &orig_mb,message_block &out_mb);
	bool call_decrypt(unsigned int id,message_block &orig_mb,message_block &out_mb);

	unsigned int get_hostid(unsigned int seq);

	unsigned char* get_client_systitle();
	unsigned char* get_server_systitle();
	void set_client_systitle(unsigned char* systitle);
	void set_server_systitle(unsigned char* systitle);

	unsigned int get_id() { return id_; }
	unsigned int get_terminal_addr();
	unsigned int get_client_cur_ic();
	unsigned int get_server_last_ic();
	unsigned int request_next_client_ic();
	
	void set_client_cur_ic(unsigned int ic);
	void set_server_last_ic(unsigned int ic);
	bool check_ic_is_validate(unsigned int ic,unsigned int last_use_ic);

	access_security* get_access_security();
	data_security* get_data_security();

protected:
	//以下函数沿用原有meter_handler方法
	bool process_cosem_connect(ptcl_packet_dlms47 &packet);
	bool process_cosem_disconnect(ptcl_packet_dlms47 &packet);
	void send_packet(ptcl_packet_dlms47 &packet);
	bool process_data(ptcl_packet_dlms47 &packet, message_block* mb);
	unsigned char get_invokeid();

protected:
	access_security* access_security_; //设定为抽象类
	data_security *data_security_;

	unsigned int id_;
	unsigned int term_addr_;
	boost::shared_ptr<session_channel_handler> channel_handler_;
	meter_handler_state handler_state_;
	message_block save_mb_;
	unsigned int invokeid_;
	ptcl_packet_dlms47 packet_; 

	boost::unordered_map<unsigned int, unsigned int> map_seq_hid;
	boost::asio::strand strand_;
	volatile int removed_;

	unsigned char client_systitle_[SYSTITLE_LEN]; //bytes string ie. {0x4D,0x4D,0x4D,0x00,0x00,0xBC,0x61,0x4E}
	unsigned char server_systitle_[SYSTITLE_LEN]; //bytes string ie. {0x4D,0x4D,0x4D,0x00,0x00,0x00,0x00,0x01}
	unsigned int client_cur_use_ic_;  
	unsigned int server_last_use_ic_; 

	unsigned int client_host_id_;
	bool is_cipher_;
	int its_state_;

};

typedef  boost::shared_ptr<meter_handler> meter_handler_ptr;
