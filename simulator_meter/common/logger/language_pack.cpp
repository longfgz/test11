#include "language_pack.h"
#include <fstream>

language_pack *language_pack::instance_ = NULL;
language_pack::language_pack(void)
{
}


language_pack::~language_pack(void)
{
}

language_pack *language_pack::get_instance()
{
	if (instance_ == NULL)
		instance_ = new language_pack;
	return instance_;
}

void language_pack::load(std::string file_path)
{
	std::ifstream file(file_path.c_str());
	if (!file.is_open())
		return;
	char buffer[2048];
	while (file.getline(buffer, 2048))
	{
		std::string str(buffer);
		int pos = str.find("=");
		if (pos == std::string::npos)
			continue;
		map_language_[str.substr(0, pos)] = str.substr(pos+1);
	}
}

const char* language_pack::get_value(const char *str_key)
{
	std::map<std::string, std::string>::iterator it = map_language_.find(str_key);
	if ( it == map_language_.end())
		return str_key;
	return (char*)it->second.c_str();
}


//---------------------------------------------------------------------------------
const char* lang(const char *str_key)
{
	return language_pack::get_instance()->get_value(str_key);
}
