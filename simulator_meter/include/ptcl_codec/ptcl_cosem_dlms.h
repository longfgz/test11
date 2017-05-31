#pragma once

#include <ptcl_codec/asn_codec.h>

class  PTCL_CODEC_EXPORT ptcl_cosem_block
{
public:
	ptcl_cosem_block();
	~ptcl_cosem_block();
	int parse_block(unsigned char *buffer, int len, unsigned int root_tid=asn::ci_cosem_pdu);
	int parse_block_loop(unsigned char *buffer, int len, unsigned int root_tid=asn::ci_cosem_pdu);
	result_set &get_results() {return rs_;}
	bool extract_results(unsigned int tid, result_set &rs);
	bool extract_results(std::string des, result_set &rs);
	result_data get_item(std::string name);
	void clear_results(){rs_.clear();}
	int find_item(std::string des, int begin = 0);
private:
	result_set rs_;
};

class  PTCL_CODEC_EXPORT ptcl_cosem_request
{
public:
	typedef struct cosem_methoed_desc
	{
		int class_id;
		std::string obis;
		short attr_id;
		bool access_selector;
		unsigned int start_time;
		unsigned int end_time;
	};

public:
	ptcl_cosem_request();
	~ptcl_cosem_request(void);
	bool parse_block(unsigned char *buffer, int len);
	int get_request_type();//1:normal 2:datablock 3:list
	bool is_last_block();
	int request_block_number();
	std::string get_obis();
	int get_class_id();
	int get_attr_id();
	int is_access_selector();
	bool get_raw_data(unsigned char *buffer, int &len);
	bool get_normal_data(result_set &rs);
	int data_result_type(); //0:Êý¾Ý¿é 1:access code

	unsigned int get_start_time();
	unsigned int get_end_time();

	//with list
	bool get_list_data(std::vector<cosem_methoed_desc> & v_cosem_desc);

	ptcl_cosem_block &cosem_block(){return cosem_block_;}
private:
	ptcl_cosem_block cosem_block_;
};