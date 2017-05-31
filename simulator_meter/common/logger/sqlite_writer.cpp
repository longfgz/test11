#include "sqlite_writer.h"
#include <boost/filesystem/operations.hpp>
#include <util/public.h>
#include <boost/timer.hpp>
#include <logger/logger.h>

#define FILE_SIZE_LIMIT (uintmax_t)1024*1024*1024*4

sqlite_writer::sqlite_writer(void)
{
	db_ = nullptr;
	memset(root_dir_,0,512);
	memset(file_name_,0,512);
	file_seq_ = 1;
	exit_ = 0;
	msg_full_ = false;
	cur_insert_d_msg_ = 1;

	p_d_msg_write_db_ = p_d_msg_insert_ = nullptr;
}

sqlite_writer::~sqlite_writer(void)
{
}

int sqlite_writer::start(char *root_dir)
{
	boost::filesystem::path dir(root_dir); //test

	memset(root_dir_,0,512);
	strcpy(root_dir_,root_dir);

	thread_ptr_.reset(new boost::thread(boost::bind(&sqlite_writer::run, this)));
	
	p_d_msg_insert_ = &d_message_1_;
	p_d_msg_write_db_ = &d_message_2_;
	open_db( time(NULL) );
	//test();
	return 0;
}

int sqlite_writer::join()
{
	if (thread_ptr_.get())
		thread_ptr_->join();

	return 0;
}

int sqlite_writer::stop()
{
	exit_ = 1;

	return 0;
}

void sqlite_writer::close()
{
	if( db_ )
	{
		sqlite3_close( db_ );
		db_ = nullptr;
	}
}

int sqlite_writer::run()
{
	time_t last_clear_time = 0;
	time_t last_check_size_time = 0;
	while (!exit_)
	{
		time_t now = time(NULL);
		if ( now- last_clear_time > 60*60) //一小时清理一次过时的日志
		{
			clear_log();
			last_clear_time = now;
		}

		 //10分钟查一次文件大小
		if( now-last_check_size_time >60*10 )
		{
			boost::system::error_code ec;
			uintmax_t size = boost::filesystem::file_size( file_name_,ec );
			if( ec==0 )  //check ok
			{
				last_check_size_time = now;
				if( size>=FILE_SIZE_LIMIT  ) 
				{
					file_seq_ ++;
					open_db( now );
				}
			}
		}

		write_message();	
		sleep_m(50);
	}

	while( !p_d_msg_write_db_->empty() && !p_d_msg_insert_->empty())
		write_message();
	
	return 0;
}

int sqlite_writer::test(void)
{
	puts("begin test...");
	time_t tm1 = time(NULL);
	char * errmsg = NULL;
	int result = sqlite3_exec(db_, "BEGIN;", 0, 0, &errmsg); 
	if( result!=0 )
		return -1;
	////


    const char* insertSQL = "INSERT INTO log VALUES(?,?,?,?)";
     sqlite3_stmt* stmt3 = NULL;
     if (sqlite3_prepare_v2(db_,insertSQL,strlen(insertSQL),&stmt3,NULL) != SQLITE_OK) 
	 {
         if (stmt3)
             sqlite3_finalize(stmt3);
         sqlite3_close(db_);
         return -1;
     }
     int insertCount = 200000;
     char strData[] = "This is a test. GOOD MORNING....A..ASDFASDFAFA 12334556677890";
     //7. 基于已有的SQL语句，迭代的绑定不同的变量数据
     for (int i = 0; i < insertCount; ++i) 
	 {
         //在绑定时，最左面的变量索引值是1。
         sqlite3_bind_int(stmt3,1,i);
         sqlite3_bind_int(stmt3,2,i*i);
		 strData[i%3]='#'+i%10;
         sqlite3_bind_text(stmt3,3,strData,strlen(strData),SQLITE_TRANSIENT);
		 strData[21]='@'+i%10;
		 sqlite3_bind_text(stmt3,4,strData,strlen(strData),SQLITE_TRANSIENT);
         if (sqlite3_step(stmt3) != SQLITE_DONE) 
		 {
             sqlite3_finalize(stmt3);
             sqlite3_close(db_);
             return -1;
         }
         //重新初始化该sqlite3_stmt对象绑定的变量。
         sqlite3_reset(stmt3);
       //  printf("Insert Succeed.\n");
     }
     sqlite3_finalize(stmt3);

	 result = sqlite3_exec(db_, "COMMIT;", 0, 0, &errmsg); 
		if( result!=0 )
			return -1;
  
     //8. 提交之前的事物。
   /*  const char* commitSQL = "COMMIT";
     sqlite3_stmt* stmt4 = NULL;
     if (sqlite3_prepare_v2(db_,commitSQL,strlen(commitSQL),&stmt4,NULL) != SQLITE_OK) 
	 {
         if (stmt4)
             sqlite3_finalize(stmt4);
         sqlite3_close(db_);
         return -1;
     }
     if (sqlite3_step(stmt4) != SQLITE_DONE) 
	 {
         sqlite3_finalize(stmt4);
         sqlite3_close(db_);
         return -1;
     }
     sqlite3_finalize(stmt4);
	 */
	 time_t tv = time(NULL)-tm1;
	// insert row
	 tv = tv;

	 result = sqlite3_exec(db_, "BEGIN;", 0, 0, &errmsg); 
	if( result!=0 )
		return -1;

	tm1 =  time(NULL);
	
	int i=0;
	char cmd[300];
		
	for(i=0;i<200000;i++)
	{
		sprintf(cmd,"insert into log values(%d,%d,'%sPP%d','%d%s%d')",
			65537+i,1000-i,"GPRS0010020030",i%1000,i*i,"GOOD MORNING....A..ASDFASDFAFA 12334556677890",3*i);
		
		result = sqlite3_exec(db_,cmd,0,0,&errmsg);
		if( result!=0 )
			return -1;
	}

	result = sqlite3_exec(db_, "COMMIT;", 0, 0, &errmsg); 
		if( result!=0 )
			return -1;

	time_t tm2 = time(NULL);
	printf("time used:%d \t",tm2-tm1);
	return 0;
}


/*


*/

int sqlite_writer::open_db(time_t message_time)
{
	
	int result;
	char * errmsg = NULL;
	// create file

	tm *message_tm = localtime(&message_time);
	if (message_tm == NULL)
		return -1;
	
	char date_string[20];
	sprintf(date_string,"%04d-%02d-%02d",message_tm->tm_year+1900,message_tm->tm_mon+1,message_tm->tm_mday);

	char dir_name[512];
	sprintf(dir_name,"%s/%s",root_dir_,date_string);

	if (check_dir(dir_name) == false)
		return -1;

	sprintf(file_name_,"%s/runlog_%d.db3",dir_name,file_seq_);

	//记录下这一天的开始时间
	message_tm->tm_hour = message_tm->tm_min = message_tm->tm_sec = 0;
	tm_begin_ = mktime( message_tm );

	// close db
	if( db_ )
		close();

	// check file size
	while(true)
	{
		boost::system::error_code ec;
		if( boost::filesystem::exists(file_name_,ec) )
		{
			uintmax_t size = boost::filesystem::file_size(file_name_,ec);
			if( ec==0 && size>= FILE_SIZE_LIMIT )
			{
				file_seq_++;
				sprintf(file_name_,"%s/runlog_%d.db3",dir_name,file_seq_);
				continue;
			}
		}
		break;
	}

	result = sqlite3_open( file_name_, &db_ );
	if( result != SQLITE_OK )
	{
		//数据库打开失败
		return -1;
	}
	
	// rowid 是隐藏的列，不需要定义
	result = sqlite3_exec( db_, "CREATE TABLE log(TV int,LEVEL int,ID TEXT,MSG text);", NULL, NULL, &errmsg );
	if(result != SQLITE_OK )
	{
		if( !strstr(errmsg,"already exists") )
			printf( "create table failed，errcode:%d，result:%s\n", result, errmsg );
	}

	result = sqlite3_exec( db_, "CREATE INDEX index1 on log(id)", NULL, NULL, &errmsg );
	if(result != SQLITE_OK )
	{
		if( !strstr(errmsg,"already exists") )
			printf( "create index failed，errcode:%d，result:%s\n", result, errmsg );
	}

	result = sqlite3_exec( db_, "CREATE INDEX index2 on log(tv)", NULL, NULL, &errmsg );
	if(result != SQLITE_OK )
	{
		if( !strstr(errmsg,"already exists") )
			printf( "create index failed，errcode:%d，result:%s\n", result, errmsg );
	}

	result = sqlite3_exec( db_, "CREATE INDEX index3 on log(level)", NULL, NULL, &errmsg );
	if(result != SQLITE_OK )
	{
		if( !strstr(errmsg,"already exists") )
			printf( "create index failed，errcode:%d，result:%s\n", result, errmsg );
	}

	return 0;
}

void sqlite_writer::packet_to_string(	unsigned char dir_type,
							  unsigned char *buffer,short int buf_size,char *dst_buffer)
{
	//dir
	if (dir_type == packet_type_recv)
		strcat(dst_buffer,"receive  ");
	else if (dir_type == packet_type_send)
		strcat(dst_buffer,"send  ");
	else if (dir_type == packet_type_recv_error)
		strcat(dst_buffer,"receive error  ");
	else if (dir_type == packet_type_send_error)
		strcat(dst_buffer,"send error ");

	//size
	char len_string[40];
	sprintf(len_string,"%dbyte ",buf_size);
	strcat(dst_buffer,len_string);
	strcat(dst_buffer,"\r\n");

	//packet
	for (int n=0;n<buf_size;n++)
	{
		char temp[4];
		sprintf(temp,"%02X ",buffer[n]);
		if (n!=0 && n % 32 == 0)
			strcat(dst_buffer," \r\n");  //以文本输出

		strcat(dst_buffer,temp);
	}
}


void sqlite_writer::insert_message(log_message & message)
{

	if (message.packet_dir_ > 0)
	{
		char text[log_message::MAX_BUFFER_LENGTH*4];
		text[0]=0;
		packet_to_string(message.packet_dir_,message.buffer_,message.len_buffer_,text);
		message.set_buffer( (uint8_t*)text,strlen(text));
	}

	mutex_.lock();
	
	if (210000 >= (int) p_d_msg_insert_->size())
	{
		p_d_msg_insert_->push_back(message);

		if ( msg_full_ && 10000 < (int)p_d_msg_insert_->size())
		{
			// 从消息满状态中恢复
			msg_full_ = false;
		}
	}
	else
	{
		if( !msg_full_ )
		{ 
			//只记录第一次消息过满
			msg_full_ = true;
			log_message msg;
			msg.set_level(1);
			msg.set_message_id("0");
			message.set_packet_dir(0);
			char info[]="deque is full,message will be droped";
			msg.set_buffer( (unsigned char *)info,strlen(info));
			p_d_msg_insert_->push_front(message);  //插到最前面
		}
	}
	mutex_.unlock();
}


void sqlite_writer::write_message()
{
	boost::timer t;
	t.restart();
	mutex_.lock();
	
	if( p_d_msg_insert_->empty() && p_d_msg_write_db_->empty() )
	{
		mutex_.unlock();
		return;
	}

	if( !p_d_msg_insert_->empty() && p_d_msg_write_db_->empty() )
	{
		//切换
		auto tmp = p_d_msg_insert_;
		p_d_msg_insert_ = p_d_msg_write_db_;
		p_d_msg_write_db_ = tmp;
	}

	double f1 = t.elapsed();
	log_message msg = p_d_msg_write_db_->front();
	time_t msg_time =  msg.get_message_time();
	int size = p_d_msg_write_db_->size();
	if( msg_time< tm_begin_ || msg_time>= tm_begin_+ 24*60*60 )
	{
		//更换db了
		file_seq_ = 1;
		open_db( msg_time );
	}
	mutex_.unlock();
	
	double f2 = t.elapsed();
	write_db( * p_d_msg_write_db_  );
	double f3 = t.elapsed();
	if(f3>0.7 )
		printf("%f %f %f %d\n",f1,f2,f3,size);
	int k=0;

}

int sqlite_writer::write_db( std::deque <log_message>& d_msg)
{
	char * errmsg = NULL;

	// insert row
	time_t tm1 = time(NULL);
	int result = sqlite3_exec(db_, "BEGIN;", 0, 0, &errmsg); 
	if( result!=0 )
	{
		puts("Error in write sqlite，begin!");
		result = sqlite3_exec(db_, "COMMIT;", 0, 0, &errmsg); 
		return -1 ;
	}
	
	const char* insertSQL = "INSERT INTO log VALUES(?,?,?,?)";
     sqlite3_stmt* stmt = NULL;
     if (sqlite3_prepare_v2(db_,insertSQL,strlen(insertSQL),&stmt,NULL) != SQLITE_OK) 
	 {
         if (stmt)
             sqlite3_finalize(stmt);
		 puts("Error in write sqlite,insert!");
		 result = sqlite3_exec(db_, "COMMIT;", 0, 0, &errmsg); 
         return -1;
     }
	
	 int size = d_msg.size();
	 if( size>10000)
		 size = 10000;
	 
	 for(int i=0;i<size;i++)  //每次提交条数
	{
		log_message &msg = d_msg[i];

		 sqlite3_bind_int(stmt,1,msg.message_time_);
		 sqlite3_bind_int(stmt,2,msg.level_ );
         sqlite3_bind_text(stmt,3,msg.message_id_,strlen(msg.message_id_),SQLITE_TRANSIENT);
		 sqlite3_bind_text(stmt,4,(char*)msg.buffer_,msg.len_buffer_,SQLITE_TRANSIENT);
         if (sqlite3_step(stmt) != SQLITE_DONE) 
		 {
             sqlite3_finalize(stmt);
			//不要回滚，而是将已经写的提交
			 puts("Error in write sqlite,bind!");
			 result = sqlite3_exec(db_, "ROLLBACK;", 0, 0, &errmsg); 
			 result = sqlite3_exec(db_, "COMMIT;", 0, 0, &errmsg); 
             return -1;
         }
         //重新初始化该sqlite3_stmt对象绑定的变量。
         sqlite3_reset(stmt);
	}
	sqlite3_finalize(stmt);
	result = sqlite3_exec(db_, "COMMIT;", 0, 0, &errmsg); 
	if( result!=0 )
	{
		puts("Error in write sqlite,commit!");
		sleep_m(500);
		result = sqlite3_exec(db_, "COMMIT;", 0, 0, &errmsg); 
		if( result!=0 )
		{
			puts("Error in write sqlite,commit 2!");
			sleep_m(1000);
			result = sqlite3_exec(db_, "COMMIT;", 0, 0, &errmsg); 
			if( result!=0 )
			{
				puts("Error in write sqlite,commit 3!");
				result = sqlite3_exec(db_, "ROLLBACK;", 0, 0, &errmsg); 
				return -1;
			}
		}
		// retry?
	}

	//只从前面删除，这样效率最高，如果改成clear或者erase，则会慢许多，从0.3秒变成1秒左右。
	//原因可能是deque的规律性
	for(int i=0;i<size;i++)
		d_msg.pop_front();
	
	return result;
}

void sqlite_writer::clear_log()
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


bool sqlite_writer::check_dir(char *dir)
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
