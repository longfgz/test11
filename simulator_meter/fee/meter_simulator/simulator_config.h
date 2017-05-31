#pragma once

#include <vector>
#include <boost/unordered_map.hpp>

class simulator_config
{
protected:
	simulator_config(void);
	~simulator_config(void);

public:
	typedef struct capture_object
	{
		int class_id;
		std::string obis;
		int attr_id;
	};

	typedef struct profile
	{
		std::string obis;
		unsigned int period;
		std::string profile_type;
		std::vector<capture_object> v_objects;
	};

public:
	static simulator_config *get_instance();

	bool read_param();

	std::vector<capture_object> * get_capture_object_list_by_obis(const std::string & obis);

	int get_capture_period_by_obis(const std::string & obis);

	std::string get_profile_type_by_obis(const std::string & obis);

	std::string get_data_type(unsigned int class_id, const std::string & obis, unsigned int attr_id);

	short get_listen_port();

	std::string get_ak();

	std::string get_ek();

	std::string get_mk();

	std::string get_lls();

	std::string get_hls();

	std::string get_system_title();

	int get_latency_ms();

protected:
	class map_key
	{
	public:
		map_key(unsigned int class_id, const std::string & obis, unsigned int attr_id);
		~map_key();

		bool operator < (const map_key & key) const;
		bool operator == (const map_key & key) const;
		friend std::size_t hash_value(map_key const & key)
		{
			std::size_t seed = 0;
			boost::hash_combine(seed, key.class_id_);
			boost::hash_combine(seed, key.obis_);
			boost::hash_combine(seed, key.attr_id_);
			return seed;
		}
		unsigned int class_id_;
		std::string obis_;
		unsigned int attr_id_;
	};

protected:
	static simulator_config *instance_;

	unsigned short listen_port_;
	std::string ak_;
	std::string ek_;
	std::string mk_;
	std::string lls_;
	std::string hls_;
	std::string system_title_;
	int latency_ms_;

	boost::unordered_map<map_key, std::string> map_object_data_type_;

	boost::unordered_map<std::string, profile > map_profiles_;
};

