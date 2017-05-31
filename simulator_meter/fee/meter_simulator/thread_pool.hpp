#pragma once

#include <queue>
#include <boost/thread.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include "meter_handler.h"
#include <channel/message_block.h>

class thread_task
{

public:
	thread_task(meter_handler_ptr handler, message_block & mb)
		:handler_(handler),mb_(mb)
	{
	}

	~thread_task(){};

	void execute()
	{
		handler_->handle_response(&mb_);
	}

protected:
	meter_handler_ptr handler_;
	message_block mb_;
};

typedef boost::shared_ptr<thread_task> thread_task_ptr;

class thread_task_queue
{
public:
	void push_task(const thread_task_ptr task)
	{
		boost::unique_lock<boost::mutex> lock(task_queue_mutex_);
		task_queue_.push(task);
		cond_.notify_one();			
	}
	thread_task_ptr get_task()
	{
		boost::mutex::scoped_lock enter_lock(enter_mutex_);
		boost::unique_lock<boost::mutex> lock(task_queue_mutex_);
		if(task_queue_.size()==0)
		{
			cond_.wait(lock);
		}
		thread_task_ptr task(task_queue_.front());
		task_queue_.pop();
		return task;
	}
	int get_size()
	{
		return (int)task_queue_.size();
	}
private:
	std::queue<thread_task_ptr> task_queue_;
	boost::mutex task_queue_mutex_;
	boost::mutex enter_mutex_;
	boost::condition_variable_any cond_;
};

class thread_pool
{
public:
	thread_pool(int num):thread_num(num),is_run(false){}
	~thread_pool(){}
	void start()
	{
		is_run=true;
		if(thread_num<=0)
			return;
		for(int i=0;i<thread_num;++i)
		{
			thread_group_.add_thread(new boost::thread(boost::bind(&thread_pool::run,this)));
		}	
	}

	void stop()
	{
		is_run=false;
	}

	void put(const thread_task_ptr task)
	{
		task_queue_.push_task(task);
	}

	void join()
	{
		thread_group_.join_all();
	}

private:
	thread_task_queue task_queue_;
	boost::thread_group thread_group_;
	int thread_num;
	volatile bool is_run;
	void run()
	{
		while(is_run)
		{

			thread_task_ptr task=task_queue_.get_task();
			task->execute();
		}
	}
};