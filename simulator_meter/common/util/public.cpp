
#include <util/public.h>
#include <boost/thread.hpp>
#include <boost/filesystem.hpp>
#include <iostream> 
#include <sstream>    
#include <string>    
#include <stdio.h>

#ifdef WIN32
#include <windows.h>
#include <io.h>
#include <direct.h>
#include <fstream>
#else
#include <sys/dir.h>
#endif


#if defined(WIN32)
#include <conio.h>
#else

#endif

void sleep_n(unsigned int nsecs)
{
	boost::xtime future;
	boost::xtime_get(&future, boost::TIME_UTC_);//boost 1-52
	//boost::xtime_get(&future, boost::TIME_UTC);//boost 1-43
	future.nsec += nsecs;
	boost::thread::sleep(future);
}

void sleep_m(unsigned int msecs)
{
	boost::xtime future;
	boost::xtime_get(&future, boost::TIME_UTC_);//boost 1-52
	//boost::xtime_get(&future, boost::TIME_UTC);//boost 1-43
	future.nsec += msecs * 1000000;
	boost::thread::sleep(future);
}

void sleep_s(unsigned int secs)
{
	boost::xtime future;
	boost::xtime_get(&future, boost::TIME_UTC_);//boost 1-52
	//boost::xtime_get(&future, boost::TIME_UTC);//boost 1-43
	future.sec += secs;
	boost::thread::sleep(future);
}

void get_current_dir(char *dir,int len_dir)
{
#if defined(WIN32)
	GetModuleFileName(NULL,dir,len_dir);
	(strrchr(dir,'\\'))[0] = 0;
#else
    readlink("/proc/self/exe", dir,len_dir);
    (strrchr(dir,'/'))[0] = 0;
#endif

}

bool check_dir(char *dir)
{
	try
	{
		/*if (boost::filesystem::is_directory(dir))
			return true;*/
		boost::filesystem::path full_path(boost::filesystem::initial_path());
		full_path = boost::filesystem::system_complete(boost::filesystem::path(dir, boost::filesystem::native));

		if (boost::filesystem::exists(full_path))
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


void scanf_password(char *password,int length)
{
#if defined(WIN32)
	for (int n=0;n<length;n++)
	{
		password[n] = _getch();
		if (password[n] == 0x0d)
		{
			password[n] = 0;
			break;
		}
	}
#else
    scanf("%32s",password);
#endif

}

/********************************************************************/
/*description: transform one double float value into one string.	*/
/*input:															*/
/*		value: the double data which need to be transformed.		*/
/*		decplaces: the num of the decimal digits,include the		*/
/*					decimal point.									*/
/*output:															*/
/*		value of the output string.									*/
/*example:															*/
/*		do_fraction(123456.4567,3); result is 123456.45				*/
/********************************************************************/
std::string do_fraction(long double value, int decplaces)
{
	using namespace std;
	 ostringstream out;
	 int prec= numeric_limits<double>::digits10; // 18
	 out.precision(prec);//覆盖默认精度
	 out<<value;
	 string str= out.str(); //从流中取出字符串

	 char DECIMAL_POINT = '.';
	 size_t n=str.find(DECIMAL_POINT);
	 if ((n!=string::npos)&& (str.size()> n+decplaces)) // (n!=string::npos)有小数点吗 (str.size()> n+decplaces)后面至少还有decplaces位吗
	 {	
		 str[n+decplaces]='\0';//覆盖第一个多余的数
	 }
	 str.swap(string(str.c_str()));//删除nul之后的多余字符
	 return str;
}
