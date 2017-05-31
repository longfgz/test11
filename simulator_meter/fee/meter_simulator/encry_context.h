#pragma once

#include <string>

class encry_context
{
public:
	encry_context(std::string cipher);
	~encry_context(void);

	std::string get_encry_key();
	std::string get_auth_key();
	std::string get_master_key();
	std::string get_title();
	std::string get_lls_secret();
	std::string get_hls_secret();
private:
	std::string master_key_;
	std::string encry_key_;
	std::string auth_key_;
	std::string title_;
	std::string lls_secret_;
	std::string hls_secret;
	void parse_cipher(std::string cipher);
	bool get_param(std::string str_info, std::string &str_name, std::string &str_value);
	std::string aes_decrypt(const char *cipherText);
};

