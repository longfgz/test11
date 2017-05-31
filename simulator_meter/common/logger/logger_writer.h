#pragma once

#include "log_message.h"
#include <logger/logger_global.h>
#include <vector>
#include <deque>

typedef std::vector <log_message> V_MESSAGE;

class logger_writer
{
public:
	logger_writer();
	~logger_writer(void);

	int start(char *root_dir);
	int join();
	int stop();

	void insert_message(log_message & message);

protected:
	int run();
	void write_message();
	void clear_log();
	void write_message(log_message & message);
	void write_message(std::fstream & file,log_message & message);

	void write_message(time_t message_time,char *message_id,std::vector <log_message> &v_messages);

	bool check_dir(char *dir);

	boost::shared_ptr<boost::thread> thread_ptr_;
	int exit_;
	char root_dir_[512];
	boost::mutex mutex_;
	//std::deque <log_message> d_message_;
	std::deque <V_MESSAGE> d_v_msg_;
};


