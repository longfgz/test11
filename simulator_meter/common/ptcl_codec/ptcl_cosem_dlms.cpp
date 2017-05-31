//#include "stdafx.h"
#include <ptcl_codec/ptcl_cosem_dlms.h>
#include "asn_parser.h"

ptcl_cosem_block::ptcl_cosem_block()
{
}

ptcl_cosem_block::~ptcl_cosem_block()
{
}

int ptcl_cosem_block::parse_block(unsigned char *buffer, int len, unsigned int root_tid)
{
	asn_factory af;
	asn_parser_ptr ap = af.create_parser(root_tid, &rs_);
	return ap->parse_buf(buffer, len);
}

int ptcl_cosem_block::parse_block_loop(unsigned char *buffer, int len, unsigned int root_tid)
{
	int pos = 0;
	while (pos < len)
	{
		int parse_len = parse_block(buffer+pos, len-pos, root_tid);
		if (parse_len <= 0)
			break;
		pos += parse_len;
	}
	return pos;
}

bool ptcl_cosem_block::extract_results(unsigned int tid, result_set &rs)
{
	rs.clear();
	for (int i=0; i<(int)rs_.size(); i++)
	{
		if (rs_[i].data().tid == tid)
			rs.push_back(rs_[i]);
	}
	if (rs.size() > 0)
		return true;
	return false;
}

bool ptcl_cosem_block::extract_results(std::string des, result_set &rs)
{
	rs.clear();
	for (int i=0; i<(int)rs_.size(); i++)
	{
		std::string des_info =  rs_[i].data().des;
		if (des_info.find(des) != std::string::npos)
			rs.push_back(rs_[i]);
	}
	if (rs.size() > 0)
		return true;
	return false;
}

result_data ptcl_cosem_block::get_item(std::string name)
{
	result_item ri = {0};
	result_set rs;
	if (!extract_results(name, rs))
		return result_data(ri);
	return rs[0];
}

int ptcl_cosem_block::find_item(std::string des, int begin)
{
	for (int i=begin; i<(int)rs_.size(); i++)
	{
		std::string des_info =  rs_[i].data().des;
		if (des_info.find(des) != std::string::npos)
			return i;
	}
	return -1;
}

//---------------------------------------------------------------------------
ptcl_cosem_request::ptcl_cosem_request(void)
{
}

ptcl_cosem_request::~ptcl_cosem_request(void)
{
}

bool ptcl_cosem_request::parse_block(unsigned char *buffer, int len)
{
	cosem_block_.clear_results();
	if (cosem_block_.parse_block(buffer, len) < len)
		return false;
	return true;
}

int ptcl_cosem_request::get_request_type()
{
	result_set rs;
	if (!cosem_block_.extract_results(asn::ci_get_request, rs))
		return -1;
	return rs[0].to_uint8();
}

bool ptcl_cosem_request::is_last_block()
{
	result_set rs;
	if (!cosem_block_.extract_results("datablock_g_boolean", rs))
		return true;
	return rs[0].to_bool();
}

int ptcl_cosem_request::request_block_number()
{
	result_set rs;
	if (!cosem_block_.extract_results("datablock_g_unsigned32", rs))
		return -1;
	return rs[0].to_uint32();
}


bool ptcl_cosem_request::get_raw_data(unsigned char *buffer, int &len)
{
	len = 0;
	result_set rs;
	if (!cosem_block_.extract_results(asn::ci_raw_data, rs))
		return false;
	memcpy(buffer, rs[0].data().buf, rs[0].data().length);
	len = rs[0].data().length;
	return true;
}

bool ptcl_cosem_request::get_normal_data(result_set &rs)
{
	if (data_result_type() != 0)
		return false;
	int pos = cosem_block_.find_item("get_data_result");
	if (pos == -1)
		return false;
	result_set &cosem_rs = cosem_block_.get_results();
	rs.assign(cosem_rs.begin()+pos+1, cosem_rs.end());
	return true;
}

int ptcl_cosem_request::data_result_type()
{
	result_set rs;
	if (!cosem_block_.extract_results(asn::ci_get_data_result, rs))
		return -1;
	return rs[0].to_uint8();
}

std::string ptcl_cosem_request::get_obis()
{
	result_set rs;
	if (!cosem_block_.extract_results(asn::ci_cosem_object_instance_id, rs))
		return "0";

	return rs[0].to_formatted_obis();
}

int ptcl_cosem_request::get_class_id()
{
	result_set rs;
	if (!cosem_block_.extract_results(asn::ci_cosem_class_id, rs))
		return 0;
	return rs[0].to_uint16();
}

int ptcl_cosem_request::get_attr_id()
{
	result_set rs;
	if (!cosem_block_.extract_results(asn::ci_cosem_object_attribute_id, rs))
		return 0;
	return rs[0].to_uint8();
}

int ptcl_cosem_request::is_access_selector()
{
	int pos = cosem_block_.find_item("unsigned8");
	if (pos == -1)
		return false;
	result_set &cosem_rs = cosem_block_.get_results();
	if (cosem_rs[pos].data().tid == asn::ci_unsigned8)
	{
		return cosem_rs[pos].to_uint8();
	}
	return 0;
}

unsigned int ptcl_cosem_request::get_start_time()
{
	int pos = cosem_block_.find_item("unsigned8");
	if (pos == -1)
		return false;
	result_set &cosem_rs = cosem_block_.get_results();
	if (cosem_rs[pos+7].data().tid == asn::ci_octet_string && cosem_rs[pos+7].data().length == 12)
	{
		return (unsigned int)cosem_rs[pos+7].to_time();
	}
	else if (cosem_rs[pos+7].data().tid == asn::ci_datetime)
		return (unsigned int)cosem_rs[pos+7].to_time();
	return 0;
}

unsigned int ptcl_cosem_request::get_end_time()
{
	int pos = cosem_block_.find_item("unsigned8");
	if (pos == -1)
		return false;
	result_set &cosem_rs = cosem_block_.get_results();
	if (cosem_rs[pos+8].data().tid == asn::ci_octet_string && cosem_rs[pos+8].data().length == 12)
	{
		return (unsigned int)cosem_rs[pos+8].to_time();
	}
	else if (cosem_rs[pos+8].data().tid == asn::ci_datetime)
		return (unsigned int)cosem_rs[pos+8].to_time();
	return 0;
}

bool ptcl_cosem_request::get_list_data(std::vector<cosem_methoed_desc> & v_cosem_desc)
{
	int pos = cosem_block_.find_item("attribute_descriptor_list");
	if (pos == -1)
		return false;
	result_set &cosem_rs = cosem_block_.get_results();

	int list_count = cosem_rs[pos++].to_uint8();

	for (int i=0; i<list_count; i++)
	{
		cosem_methoed_desc desc;
		desc.class_id = cosem_rs[pos++].to_uint16();
		desc.obis = cosem_rs[pos++].to_formatted_obis();
		desc.attr_id = cosem_rs[pos++].to_uint8();
		desc.access_selector = cosem_rs[pos++].to_bool();
		if (desc.access_selector == true)
		{
			pos+=7;
			if (cosem_rs[pos].data().tid == asn::ci_octet_string && cosem_rs[pos].data().length == 12
			&& cosem_rs[pos+1].data().tid == asn::ci_octet_string && cosem_rs[pos+1].data().length == 12)
			{
				desc.start_time = (unsigned int)cosem_rs[pos++].to_time();
				desc.end_time = (unsigned int)cosem_rs[pos++].to_time();
			}
			else if (cosem_rs[pos].data().tid == asn::ci_datetime)
			{
				desc.start_time = (unsigned int)cosem_rs[pos++].to_time();
				desc.end_time = (unsigned int)cosem_rs[pos++].to_time();
			}
			else
				return false;
			pos++;
		
		}

		v_cosem_desc.push_back(desc);
		
	}
	return true;
}