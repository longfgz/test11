#pragma once

#include <string>
#include <map>

class language_pack
{
public:
	language_pack(void);
	~language_pack(void);
	static language_pack *get_instance();
	void load(std::string file_path);
	const char* get_value(const char *str_key);
private:
	static language_pack *instance_;
	std::map<std::string, std::string> map_language_;
};

const char* lang(const char *str_key);

