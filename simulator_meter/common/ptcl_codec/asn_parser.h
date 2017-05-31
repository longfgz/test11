#pragma once

#include <string>
#include <ptcl_codec/asn_codec.h>
#include <algorithm>
#include <boost/shared_ptr.hpp>

//================================================================================

class asn_parser;

typedef boost::shared_ptr<asn_parser> asn_parser_ptr;

class asn_factory
{
public:
	asn_parser_ptr create_parser(unsigned int tag,result_set  *results, std::string prefix = "");
};

class asn_parser
{
public:
	asn_parser(asn_factory *factory, result_set  *results)
	{
		factory_ = factory;
		results_ = results;
	}
	virtual ~asn_parser(){}
	virtual int parse_buf(unsigned char buf[], int len)=0;
protected:
	asn_factory *factory_;
	result_set  *results_;
};

//------------------------------------------------------

class sequence_parser : public asn_parser
{
public:
	sequence_parser(asn_factory *factory, result_set  *results, unsigned int vec_item[], int item_count, unsigned int option_mask = 0, std::string data_des = "")
		:asn_parser(factory, results), option_mask_(option_mask)
	{
		vec_item_.assign(vec_item, vec_item+item_count);
		if (data_des.size() > 0)
			data_des_ = data_des+"_";
	}
	virtual int parse_buf(unsigned char buf[], int len)
	{
		int pos =0;
		for (int i=0; i<(int)vec_item_.size(); i++)
		{
			unsigned int flag = (1 << i) & option_mask_;
			if (flag != 0)
			{
				pos++;
				if (buf[pos-1] == 0)
					continue;
			}
			asn_parser_ptr ap = factory_->create_parser(vec_item_[i], results_, data_des_);
			if (ap == NULL)
				return -1;
			int parse_len = ap->parse_buf(buf+pos, len-pos);
			if (parse_len < 0)
				return -1;
			pos += parse_len;
		}
		return pos;
	}
private:
	std::vector<unsigned int> vec_item_;
	unsigned int option_mask_;
	std::string  data_des_;
};

class choice_parser : public asn_parser
{
public:
	choice_parser(asn_factory *factory, result_set  *results, unsigned int vec_item[], int item_count, int data_typeid = asn::ci_null, std::string data_des = "")
		:asn_parser(factory, results),  data_typeid_(data_typeid), data_des_(data_des)
	{
		vec_item_.assign(vec_item, vec_item+item_count);
	}
	virtual int parse_buf(unsigned char buf[], int len)
	{
		unsigned int tag = buf[0];
		if (tag >= vec_item_.size())
			return -1;
		if (data_typeid_ != asn::ci_null)
		{
			result_item ri = {data_typeid_, "", 1, 0};
			strcpy(ri.des, data_des_.c_str());
			ri.buf[0] = tag;
			results_->push_back(ri);
		}
		asn_parser_ptr ap = factory_->create_parser(vec_item_[tag], results_);
		if (ap == NULL)
			return -1;
		int parse_len = ap->parse_buf(buf+1, len-1);
		if (parse_len < 0)
			return -1;
		return parse_len+1;
	}
private:
	std::vector<unsigned int> vec_item_;
	int data_typeid_;
	std::string  data_des_;
};

class sequence_of_data : public asn_parser
{
public:
	sequence_of_data(asn_factory *factory, result_set  *results, int data_typeid, std::string data_des)
		:asn_parser(factory, results), data_typeid_(data_typeid), data_des_(data_des){}
	virtual int parse_buf(unsigned char buf[], int len)
	{
		unsigned int info_len = 0;
		unsigned int item_count = 0;
		if (!parse_length_info(buf, info_len, item_count))
			return -1;
		result_item ri = {data_typeid_, "", 4, 0};
		strcpy(ri.des, data_des_.c_str());
		memcpy(ri.buf, &item_count, 4);
		std::reverse(ri.buf, ri.buf+4);
		results_->push_back(ri);
		int pos = info_len;
		for (unsigned int i=0; i<item_count && pos <len; i++)
		{
			asn_parser_ptr ap = factory_->create_parser(asn::ci_data, results_);
			int parse_len = ap->parse_buf(buf+pos, len-pos);
			pos += parse_len;
		}
		return pos;
	}
	bool parse_length_info(unsigned char buf[], unsigned int &parse_len, unsigned int &data_len)
	{
		data_len = 0;
		int flag = (buf[0]>>7);
		int count = (buf[0] & 0x7F);
		parse_len = 1;
		if (flag == 0)
		{
			data_len = count;
			return true;
		}
		if (count > 4)
			return false;
		for (int i=0; i< count; i++)
			data_len = data_len*256+buf[parse_len++];
		return true;
	}

private:
	std::string  data_des_;
	int data_typeid_;
};

class sequence_of_with_list : public asn_parser
{
public:
	sequence_of_with_list(asn_factory *factory, result_set  *results, int data_typeid, std::string data_des)
		:asn_parser(factory, results), data_typeid_(data_typeid), data_des_(data_des){}
	virtual int parse_buf(unsigned char buf[], int len)
	{
		unsigned int info_len = 0;
		unsigned int item_count = 0;
		if (!parse_length_info(buf, info_len, item_count))
			return -1;
		result_item ri = {data_typeid_, "", 1, 0};
		strcpy(ri.des, data_des_.c_str());
		memcpy(ri.buf, &item_count, 1);
		std::reverse(ri.buf, ri.buf+1);
		results_->push_back(ri);
		int pos = info_len;
		for (unsigned int i=0; i<item_count && pos <len; i++)
		{
			asn_parser_ptr ap = factory_->create_parser(asn::ci_cosem_attribute_descriptor, results_);
			int parse_len = ap->parse_buf(buf+pos, len-pos);
			pos += parse_len;

			result_item rid = {asn::ci_selective_access_descriptor, "", 1, 0};
			strcpy(rid.des, "selective_access_descriptor");
			rid.buf[0] = buf[pos];
			results_->push_back(rid);
			

			if (buf[pos] == 0x00) 
			{
				pos+=1;
			}
			else if (buf[pos] == 0x01)
			{
				ap = factory_->create_parser(asn::ci_data, results_);
				parse_len = ap->parse_buf(buf+pos, len-pos);
				pos += parse_len;
			}

		//	result_item rid = {asn::ci_selective_access_descriptor, "", 1, 0};
		//	strcpy(rid.des, "selective_access_descriptor");
		//	rid.buf[0] = 1;
		////	std::reverse(rid.buf, ri.buf+1);
		//	results_->push_back(rid);
		//	pos += 1;


		
		}
		return pos;
	}
	bool parse_length_info(unsigned char buf[], unsigned int &parse_len, unsigned int &data_len)
	{
		data_len = 0;
		int flag = (buf[0]>>7);
		int count = (buf[0] & 0x7F);
		parse_len = 1;
		if (flag == 0)
		{
			data_len = count;
			return true;
		}
		if (count > 4)
			return false;
		for (int i=0; i< count; i++)
			data_len = data_len*256+buf[parse_len++];
		return true;
	}

private:
	std::string  data_des_;
	int data_typeid_;
};

class ber_sequence_parser : public asn_parser
{
public:
	ber_sequence_parser(asn_factory *factory, result_set  *results, unsigned int vec_item[], int item_count)
		:asn_parser(factory, results)
	{
		vec_item_.assign(vec_item, vec_item+item_count);
	}
	virtual int parse_buf(unsigned char buf[], int len)
	{
		int data_len = buf[0];
		if (len < data_len+1)
			return -1;
		int pos =1;
		for (int i=0; i<(int)vec_item_.size(); i++)
		{
			int tag = buf[pos];
			if (tag >= (int)vec_item_.size())
				return -1;
			asn_parser_ptr ap = factory_->create_parser(vec_item_[tag], results_);
			if (ap == NULL)
				return -1;
			int parse_len = ap->parse_buf(buf+pos+1, len-pos-1);
			if (parse_len < 0)
				return -1;
			pos += (parse_len+1);
		}
		return pos;
	}
private:
	std::vector<unsigned int> vec_item_;
};

class ber_choice_parser : public asn_parser
{
public:
	ber_choice_parser(asn_factory *factory, result_set  *results, unsigned int vec_item[], int item_count)
		:asn_parser(factory, results)
	{
		vec_item_.assign(vec_item, vec_item+item_count);
	}
	virtual int parse_buf(unsigned char buf[], int len)
	{
		int data_len = buf[0];
		if (len < data_len+1)
			return -1;
		unsigned int tag = buf[1];
		if (tag >= vec_item_.size())
			return -1;
		asn_parser_ptr ap = factory_->create_parser(vec_item_[tag], results_);
		if (ap == NULL)
			return -1;
		int parse_len = ap->parse_buf(buf+2, len-2);
		if (parse_len < 0)
			return -1;
		return parse_len+2;
	}
private:
	std::vector<unsigned int> vec_item_;
};


//---------------------------------------------------------------------------------------------

class fixed_length_type : public asn_parser
{
public:
	fixed_length_type(asn_factory *factory, result_set  *results, int data_len, int data_typeid, std::string data_des)
		:asn_parser(factory, results), data_len_(data_len), data_typeid_(data_typeid), data_des_(data_des){}
	virtual int parse_buf(unsigned char buf[], int len)
	{
		if (len < data_len_)
			return -1;
		result_item ri = {data_typeid_, "", data_len_, 0};
		strcpy(ri.des, data_des_.c_str());
		memcpy(ri.buf, buf, data_len_);
		results_->push_back(ri);
		return data_len_;
	}
private:
	int data_len_;
	int data_typeid_;
	std::string  data_des_;
};

class unfixed_length_type : public asn_parser
{
public:
	unfixed_length_type(asn_factory *factory, result_set  *results, int data_typeid, std::string data_des)
		:asn_parser(factory, results),  data_typeid_(data_typeid), data_des_(data_des){}
	virtual int parse_buf(unsigned char buf[], int len)
	{
		unsigned int pos = 0;
		unsigned int data_len = 0;
		if (!parse_length_info(buf, pos, data_len))
			return -1;
		if ((unsigned int)len < data_len+pos)
			return -1;
		result_item ri = {data_typeid_, "", data_len, 0};
		char des_info[32] = {0};
		sprintf(des_info, "%s_%d", data_des_.c_str(), data_len);
		strcpy(ri.des, des_info);
		memcpy(ri.buf, buf+pos, data_len);
		results_->push_back(ri);
		return data_len+pos;
	}
	bool parse_length_info(unsigned char buf[], unsigned int &parse_len, unsigned int &data_len)
	{
		data_len = 0;
		int flag = (buf[0]>>7);
		int count = (buf[0] & 0x7F);
		parse_len = 1;
		if (flag == 0)
		{
			data_len = count;
			return true;
		}
		if (count > 4)
			return false;
		for (int i=0; i< count; i++)
			data_len = data_len*256+buf[parse_len++];
		return true;
	}
private:
	int data_typeid_;
	std::string  data_des_;
};
