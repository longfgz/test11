#include "simulator_config.h"
#include <boost/property_tree/xml_parser.hpp>
#include <util/public.h>
#include <logger/logger.h>

simulator_config *simulator_config::instance_ = NULL;
simulator_config::simulator_config(void):
	listen_port_(6001),
	latency_ms_(0)
{
}


simulator_config::~simulator_config(void)
{
}

simulator_config *simulator_config::get_instance()
{
	if (instance_ == NULL)
		instance_ = new simulator_config();
	return instance_;
}

bool simulator_config::read_param()
{
	boost::property_tree::ptree param;
	try
	{
		char file_name[512];
		memset(file_name,0,512);
		get_current_dir(file_name,512);
		strcat(file_name,"/simulator_config.xml");
		read_xml(file_name, param);

		listen_port_ = param.get_child("config").get<unsigned int>("comm.listen_port");
		ak_ = param.get_child("config").get<std::string>("comm.authentication_key");
		ek_ = param.get_child("config").get<std::string>("comm.encrption_key");
		mk_ = param.get_child("config").get<std::string>("comm.master_key");
		lls_ = param.get_child("config").get<std::string>("comm.lls_secret");
		hls_ = param.get_child("config").get<std::string>("comm.hls_secret");
		system_title_ = param.get_child("config").get<std::string>("comm.system_title");
		latency_ms_ = param.get_child("config").get<int>("comm.latency_ms");

		boost::property_tree::ptree::iterator it;
		for(it = param.get_child("config.profiles").begin();
			it != param.get_child("config.profiles").end(); ++it)
		{
			profile po;
			std::string obis = it->second.get<std::string>("obis");
			for (auto it2 = it->second.get_child("capture_object_list").begin();
				it2 != it->second.get_child("capture_object_list").end(); ++it2)
			{
				capture_object co;
				co.class_id = it2->second.get<unsigned int>("class_id");
				co.obis = it2->second.get<std::string>("obis");
				co.attr_id = it2->second.get<unsigned int>("attribute");
				po.v_objects.push_back(co);
			}
			po.obis = obis;
			po.period = it->second.get<unsigned int>("period");
			po.profile_type = it->second.get<std::string>("profile_type");
			map_profiles_.insert(std::make_pair(obis, po));
		
		}

		for (it = param.get_child("config.clocks").begin();
			it != param.get_child("config.clocks").end(); ++it)
		{
			unsigned int class_id = it->second.get<unsigned int>("class_id");
			std::string obis = it->second.get<std::string>("obis");
			unsigned int attr_id = it->second.get<unsigned int>("attribute");
			std::string data_type = it->second.get<std::string>("data_type");
			map_key key(class_id, obis, attr_id);
			map_object_data_type_.insert(std::make_pair(key, data_type));
		}

		for (it = param.get_child("config.registers").begin();
			it != param.get_child("config.registers").end(); ++it)
		{
			unsigned int class_id = it->second.get<unsigned int>("class_id");
			std::string obis = it->second.get<std::string>("obis");
			unsigned int attr_id = it->second.get<unsigned int>("attribute");
			std::string data_type = it->second.get<std::string>("data_type");
			map_key key(class_id, obis, attr_id);
			map_object_data_type_.insert(std::make_pair(key, data_type));
		}

		for (it = param.get_child("config.datas").begin();
			it != param.get_child("config.datas").end(); ++it)
		{
			unsigned int class_id = it->second.get<unsigned int>("class_id");
			std::string obis = it->second.get<std::string>("obis");
			unsigned int attr_id = it->second.get<unsigned int>("attribute");
			std::string data_type = it->second.get<std::string>("data_type");
			map_key key(class_id, obis, attr_id);
			map_object_data_type_.insert(std::make_pair(key, data_type));
		}
	}
	catch(std::exception& e)
	{
		LOG_ERROR("system", "load xml is failed, %s", e.what());
		return false;
	}
	return true;
}

std::vector<simulator_config::capture_object> * simulator_config::get_capture_object_list_by_obis(const std::string & obis)
{	
	auto it = map_profiles_.find(obis);
	if (it != map_profiles_.end())
		return &it->second.v_objects;

	return NULL;
}

int simulator_config::get_capture_period_by_obis(const std::string & obis)
{	
	auto it = map_profiles_.find(obis);
	if (it != map_profiles_.end())
		return it->second.period;

	return 0;
}

std::string simulator_config::get_data_type(unsigned int class_id, const std::string & obis, unsigned int attr_id)
{
	map_key key(class_id, obis, attr_id);
	auto it = map_object_data_type_.find(key);
	if (it != map_object_data_type_.end())
		return it->second;

	return "";
}

std::string simulator_config::get_profile_type_by_obis(const std::string & obis)
{
	auto it = map_profiles_.find(obis);
	if (it != map_profiles_.end())
		return it->second.profile_type;

	return "";
}

int simulator_config::get_latency_ms()
{
	return latency_ms_;
}

short simulator_config::get_listen_port()
{
	return listen_port_;
}

std::string simulator_config::get_ak()
{
	return ak_;
}

std::string simulator_config::get_ek()
{
	return ek_;
}

std::string simulator_config::get_mk()
{
	return mk_;
}

std::string simulator_config::get_lls()
{
	return lls_;
}

std::string simulator_config::get_hls()
{
	return hls_;
}

std::string simulator_config::get_system_title()
{
	return system_title_;
}

simulator_config::map_key::map_key(unsigned int class_id, const std::string & obis, unsigned int attr_id)
{
	class_id_ = class_id;
	obis_ = obis;
	attr_id_ = attr_id;
}

simulator_config::map_key::~map_key(void)
{

}

bool simulator_config::map_key::operator < (const map_key & key) const
{
	if (class_id_ < key.class_id_)
		return true;
	if (class_id_ < key.class_id_ && obis_.compare(key.obis_) < 0)
		return true;
	if (class_id_ == key.class_id_ && obis_.compare(key.obis_) < 0 && attr_id_ < key.attr_id_)
		return true;
	return false;
}

bool simulator_config::map_key::operator == (const map_key & key) const
{
	if (class_id_ == key.class_id_ && obis_.compare(key.obis_) == 0 && attr_id_ == key.attr_id_)
		return true;

	return false;
}
