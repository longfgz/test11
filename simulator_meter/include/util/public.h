#pragma once

#define VERSION "amr_4_00_20131010"
#include <string>

void sleep_n(unsigned int nsecs);
void sleep_m(unsigned int msecs);
void sleep_s(unsigned int secs);
void get_current_dir(char *dir,int len_dir);
bool check_dir(char *dir);
void scanf_password(char *password,int length);
std::string do_fraction(long double value, int decplaces=4); //将浮点型数据转换为指定小数位数的字符串；
