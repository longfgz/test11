#include "object_transfer.h"
#include "simulator_config.h"
#include <boost/algorithm/string.hpp>
#include "boost/date_time/gregorian/gregorian_types.hpp" 
#include "common.h"
object_transfer *object_transfer::instance_ = NULL;
object_transfer::object_transfer(void)
{

}


object_transfer::~object_transfer(void)
{
}


object_transfer *object_transfer::get_instance()
{
	if (instance_ == NULL)
	{
		instance_ = new object_transfer();
	}
	return instance_;
}

void object_transfer::calc_points(std::string obis, unsigned int start_time, unsigned int end_time, unsigned int & out_start_time, unsigned int & points)
{
	int period = simulator_config::get_instance()->get_capture_period_by_obis(obis);
	if (period == 0)
	{
		return;
	}

	int e_s = end_time - start_time;
	if (e_s < 0)
	{
		return;
	}

	points = e_s / period;

	if (points == 0 && period == 86400)
	{
		time_t now = time(NULL);
		tm *t_now = std::localtime(&now);
		t_now->tm_hour = 0;
		t_now->tm_min = 0;
		t_now->tm_sec = 0;
		if (start_time < std::mktime(t_now))
		{
			points = 1;
		}
	}

	if ((start_time%period == 0 || end_time%period == 0) && e_s > period)
	{
		points += 1;
	}
	

	out_start_time = get_next_period_time(start_time, period);

	if (points >= 32)
	{
		points = 32;
	}
}


bool object_transfer::format_profile_data_attr_2(std::string obis, unsigned int start_time, unsigned int end_time, std::vector<ptcl_module_dlms47::profile_unit>  & v_uints)
{
	std::vector<simulator_config::capture_object> *v_objects = simulator_config::get_instance()->get_capture_object_list_by_obis(obis);
	if (v_objects == NULL || v_objects->size() == 0)
	{
		return false;
	}

	int period = simulator_config::get_instance()->get_capture_period_by_obis(obis);

	unsigned int new_start_time  = 0;
	unsigned int points = 0;
	calc_points(obis, start_time, end_time, new_start_time, points);

	if(points == 0)
		return false;

	for (int i = 0; i < points; i++)
	{
		ptcl_module_dlms47::profile_unit uint;
		if (v_objects->at(0).obis.compare("0000010000FF") == 0 && v_objects->at(0).class_id == 8)
		{
			uint.dt = new_start_time + period*i;
		}
		else
			continue;;
		
		for (int j=1; j<v_objects->size();j++)
		{
			ptcl_module_dlms47::data_unit du;
			int t_id = get_ci_type(simulator_config::get_instance()->get_data_type(v_objects->at(j).class_id, v_objects->at(j).obis, v_objects->at(j).attr_id));

			if (t_id == ci_null || t_id == ci_dont_care)
			{
				continue;;
			}

			switch (t_id)
			{   						
			case ci_double_long_unsigned:
				du.tid = ci_double_long_unsigned;
				du.length = 4;
				memset(du.buffer, 0, sizeof(du.buffer));
				uint.values.push_back(du);
				break;		
			case ci_unsigned:
				du.tid = ci_unsigned;
				du.length = 1;
				memset(du.buffer, 0, sizeof(du.buffer));
				uint.values.push_back(du);
				break;		
			default:
				break;
			}
		}
		v_uints.push_back(uint);
	}
	if (v_uints.size() > 0)
	{
		return true;
	}
	else
		return false;
}

bool object_transfer::format_profile_attr_3(std::string obis, std::vector<ptcl_module_dlms47::profile_unit>  & v_uints)
{
	//判断是否抄profile的其他属性
	std::vector<simulator_config::capture_object> *v_objects = simulator_config::get_instance()->get_capture_object_list_by_obis(obis);
	if (v_objects == NULL)
		return false;

	for (simulator_config::capture_object object : *v_objects)
	{
		ptcl_module_dlms47::profile_unit unit;
		ptcl_module_dlms47::data_unit du;
		memset(du.buffer, 0 , sizeof(du.buffer));

		//class_id
		du.tid = ci_long_unsigned;
		du.length = 2;

		memcpy(du.buffer, &object.class_id, 2);
		std::reverse(du.buffer, du.buffer+2);
		unit.values.push_back(du);

		//obis
		du.tid = ci_octet_string;
		du.length = 6;
		unsigned int len;
		Common::Util_tools::hex_to_bytes(object.obis, du.buffer, len);

		unit.values.push_back(du);

		//attr
		du.tid = ci_integer;
		du.length = 1;
		memcpy(du.buffer, &object.attr_id, 1);
		unit.values.push_back(du);

		//data_index
		du.tid = ci_long_unsigned;
		du.length = 2;
		memset(du.buffer, 0 ,2);
		unit.values.push_back(du);	
		unit.dt = 0;
		v_uints.push_back(unit);
	}

	if (v_uints.size() > 0)
	{
		return true;
	}
	else
		return false;

}

bool object_transfer::format_profile_attr_4(std::string obis, ptcl_module_dlms47::get_data_result & result)
{
	int period = simulator_config::get_instance()->get_capture_period_by_obis(obis);
	result.result_type = ci_double_long_unsigned;
	result.length = 4;
	memcpy(result.buffer, &period, 4);
	std::reverse(result.buffer, result.buffer+4);
	return true;
}

bool object_transfer::format_profile_event_attr_3(std::string obis, std::vector<ptcl_module_dlms47::profile_unit>  & v_uints)
{
	//判断是否抄profile的其他属性
	std::vector<simulator_config::capture_object> *v_objects = simulator_config::get_instance()->get_capture_object_list_by_obis(obis);
	if (v_objects == NULL)
		return false;

	for (simulator_config::capture_object object : *v_objects)
	{
		ptcl_module_dlms47::profile_unit unit;
		ptcl_module_dlms47::data_unit du;
		memset(du.buffer, 0 , sizeof(du.buffer));

		//class_id
		du.tid = ci_long_unsigned;
		du.length = 2;

		memcpy(du.buffer, &object.class_id, 2);
		std::reverse(du.buffer, du.buffer+2);
		unit.values.push_back(du);

		//obis
		du.tid = ci_octet_string;
		du.length = 6;
		unsigned int len;
		Common::Util_tools::hex_to_bytes(object.obis, du.buffer, len);

		unit.values.push_back(du);

		//attr
		du.tid = ci_integer;
		du.length = 1;
		memcpy(du.buffer, &object.attr_id, 1);
		unit.values.push_back(du);

		//data_index
		du.tid = ci_long_unsigned;
		du.length = 2;
		memset(du.buffer, 0 ,2);
		unit.values.push_back(du);	
		unit.dt = 0;
		v_uints.push_back(unit);
	}
	if (v_uints.size() > 0)
	{
		return true;
	}
	else
		return false;
}

bool object_transfer::format_profile_event_attr_4(std::string obis, ptcl_module_dlms47::get_data_result & result)
{
	int period = simulator_config::get_instance()->get_capture_period_by_obis(obis);
	result.result_type = ci_double_long_unsigned;
	result.length = 4;
	memcpy(result.buffer, &period, 4);
	std::reverse(result.buffer, result.buffer+4);
	return true;
}

bool object_transfer::format_profile_event_attr_2(std::string obis, unsigned int start_time, unsigned int end_time, std::vector<ptcl_module_dlms47::profile_unit>  & v_uints)
{
	std::vector<simulator_config::capture_object> *v_objects = simulator_config::get_instance()->get_capture_object_list_by_obis(obis);
	if (v_objects == NULL || v_objects->size() == 0)
	{
		return false;
	}

	unsigned int points = 0;

	//随机取1-2个点产生事件
	points = 1 + rand() % 2;

	//得到随机时间范围
	int range = end_time - start_time + 1;

	if (points == 0)
		return false;

	for (int i = 0; i < points; i++)
	{
		ptcl_module_dlms47::profile_unit uint;
		if (v_objects->at(0).obis.compare("0000010000FF") == 0 && v_objects->at(0).class_id == 8)
		{
			//得到随机时间
			uint.dt = rand() % range + start_time;
		}
		else
			continue;

		ptcl_module_dlms47::data_unit du;
		du.tid = ci_enum;
		du.length = 1;
		//随机生成event_code
		byte event_code = get_event_code(obis);
		du.buffer[0] = event_code;
		uint.values.push_back(du);

		v_uints.push_back(uint);
	}
	if (v_uints.size() > 0)
	{
		return true;
	}
	else
		return false;
}

bool object_transfer::format_register_attr_2(std::string obis, int class_id, int attr_id, ptcl_module_dlms47::get_data_result & result)
{
	std::string s_data_type = simulator_config::get_instance()->get_data_type(class_id, obis, attr_id);
	int t_id = get_ci_type(s_data_type);

	switch (t_id)
	{   						
	case ci_double_long_unsigned:
		result.result_type = t_id;
		result.length = 4;
		memset(result.buffer, 0, sizeof(result.buffer));
		break;		
	case ci_unsigned:
		result.result_type = t_id;
		result.length = 1;
		memset(result.buffer, 0, sizeof(result.buffer));
		break;	
	case ci_enum:
		result.result_type = t_id;
		result.length = 1;
		memset(result.buffer, 0, sizeof(result.buffer));
		break;	
	case ci_datetime:
		result.result_type = t_id;
		result.length = 12;
		asn_codec::format_date_time(time(NULL), result.buffer);
		break;
	case ci_octet_string:
		if (class_id == 8)
		{
			result.result_type = t_id;
			result.length = 12;
			asn_codec::format_date_time(time(NULL), result.buffer);
		}
		break;
	default:
		return false;
	}
	return true;
}

bool object_transfer::format_event_code_attr_2(std::string obis, int class_id, int attr_id, ptcl_module_dlms47::get_data_result & result)
{
	if (obis.compare("0000600B00FF") == 0)
	{
		result.result_type = ci_enum;
		result.length = 1;
		//随机生成event_code
		byte event_code = 1 + rand() % 20;
		result.buffer[0] = event_code;
	}
	else
		return false;
	return true;
}

bool object_transfer::format_disconnect_control_attr(int attr_id, ptcl_module_dlms47::get_data_result & result)
{
	switch (attr_id)
	{
	case 2:
		result.result_type = ci_boolean;
		result.length = 1;
		//随机生成event_code
		result.buffer[0] = 1;
		break;
	case 3:
		result.result_type = ci_enum;
		result.length = 1;
		//随机生成event_code
		result.buffer[0] = 1;
		break;
	case 4:
		result.result_type = ci_enum;
		result.length = 1;
		//随机生成event_code
		result.buffer[0] = 3;
		break;
	default:
		return false;
	}
	return true;
}

bool object_transfer::format_disconnect_control_method(ptcl_module_dlms47::get_data_result & result)
{
	return false;
}

bool object_transfer::format_image_transfer_attr(int attr_id, int its_state, ptcl_module_dlms47::get_data_result & result)
{
	switch (attr_id)
	{
	case 2:
		{
			result.result_type = ci_double_long_unsigned;
			result.length = 4;
			int block_size = 192;
			memcpy(result.buffer, &block_size, 4);
			std::reverse(result.buffer, result.buffer+4);
		}	
		break;
	case 3:
		{
			result.result_type = ci_bit_string;
			result.length = 1;
			result.buffer[0] = 1;
		}	
		break;
	case 4:
		{
			result.result_type = ci_double_long_unsigned;
			result.length = 4;
			int block_size = 0xFFFFFF;
			memcpy(result.buffer, &block_size, 4);
			std::reverse(result.buffer, result.buffer+4);
		}		
		break;
	case 5:
		{
			result.result_type = ci_boolean;
			result.length = 1;
			result.buffer[0] = 1;
		}	
		break;
	case 6:
		{
			result.result_type = ci_enum;
			result.length = 1;
			result.buffer[0] = its_state;
		}	
		break;
	default:
		return false;
	}
	return true;
}

bool object_transfer::format_image_transfer_method(int attr_id, int &its_state, ptcl_module_dlms47::get_data_result & result)
{
	switch (attr_id)
	{
	case 1:
		its_state = 1;
		break;
	case 3:
		its_state = 3;
		break;
	case 4:
		its_state = 6;
		break;
	default:
		return false;
	}
	return true;
}

bool object_transfer::format_get_normal_response(std::string obis, int class_id, int attr_id, unsigned int start_time, unsigned int end_time, int its_state, ptcl_module_dlms47::get_data_result &result
											 ,std::vector<ptcl_module_dlms47::profile_unit> & v_uint)
{
	int ot_type = get_obis_type(class_id, obis);

	switch (ot_type)
	{
	case ot_null:
		return false;
	case ot_profile:
		if (attr_id == 2)
			return format_profile_data_attr_2(obis, start_time, end_time, v_uint);
		else if (attr_id == 3)
			return format_profile_attr_3(obis, v_uint);
		else if (attr_id == 4)
			return format_profile_attr_4(obis, result);
		else
			return false;
	case ot_event:
		if (attr_id == 2)
			return format_profile_event_attr_2(obis, start_time, end_time, v_uint);
		else if (attr_id == 3)
			return format_profile_attr_3(obis, v_uint);
		else if (attr_id == 4)
			return format_profile_attr_4(obis, result);
		else
			return false;		
	case ot_event_code:
		return format_event_code_attr_2(obis, class_id, attr_id, result);	
	case ot_register:
		return format_register_attr_2(obis, class_id, attr_id, result);
	case ot_firmware:
		return format_image_transfer_attr(attr_id, its_state, result);
	case ot_disconnect_control:
		return format_disconnect_control_attr(attr_id, result);
	default:
		return false;
	}
	return true;
}

bool object_transfer::format_action_normal_response(std::string obis, int class_id, int attr_id, int &its_state, ptcl_module_dlms47::get_data_result & result)
{

	int ot_type = get_obis_type(class_id, obis);

	switch (ot_type)
	{
	case ot_null:
		return false;
	case ot_firmware:
		return format_image_transfer_method(attr_id, its_state, result);
	case ot_disconnect_control:
		return format_disconnect_control_method(result);
	default:
		return false;
	}

	return true;
}

int object_transfer::get_obis_type(int class_id, std::string obis)
{
	switch (class_id)
	{
	//profile
	case 7:
		{
			std::string profile_type = simulator_config::get_instance()->get_profile_type_by_obis(obis);
			if (profile_type.compare("load profile") == 0)
				return ot_profile;
			else if (profile_type.compare("event profile") == 0)
			{
				return ot_event;
			}
			else
				return ot_null;
		}
		
	case 1:
	case 3:
		if (obis.compare("0000600B00FF") == 0)
		{
			return ot_event_code;
		}
		return ot_register;
	case 18:
		return ot_firmware;
	case 70:
		return ot_disconnect_control;
	default:
		return ot_null;
	}

}


int object_transfer::get_event_code(std::string obis)
{
	//100401 Standard Event Log
	if (obis.compare("0000636200FF") == 0)
	{
		return 1 + rand() % 25;;
	}
	//100402 Fraud Detection Log
	else if (obis.compare("0000636201FF") == 0)
	{
		return 40; 
	}
	//100403 Disconnect Control Log
	else if (obis.compare("0000636202FF") == 0)
	{
		return 59; 
	}
	//100404 Power Quality Event Profile
	else if (obis.compare("0000636204FF") == 0)
	{
		return 76; 
	}
	//100405 Communication Profile
	else if (obis.compare("0000636205FF") == 0)
	{
		return 119; 
	}
	//100406 Security
	else if (obis.compare("0000636207FF") == 0)
	{
		return 1; 
	}
	//100407 Access Log
	else if (obis.compare("0000636206FF") == 0)
	{
		return 26; 
	}
	//100408 M-Bus Event profile
	else if (obis.compare("0000636203FF") == 0)
	{
		return 38; 
	}
	//100409 M-Bus Control profile 1
	else if (obis.compare("0001180500FF") == 0)
	{
		return 30; 
	}
	//100410 M-Bus Control profile 2
	else if (obis.compare("0002180500FF") == 0)
	{
		return 32; 
	}
	//100411 M-Bus Control profile 3
	else if (obis.compare("0003180500FF") == 0)
	{
		return 34; 
	}
	//100412 M-Bus Control profile 4
	else if (obis.compare("0004180500FF") == 0)
	{
		return 36; 
	}
	//100415 Legally relevant
	else if (obis.compare("0100636300FF") == 0)
	{
		return 951; 
	}
	//100416 Image activation logbook
	else if (obis.compare("0100636208FF") == 0)
	{
		return 18; 
	}
	return 255;
}

int object_transfer::get_ci_type(std::string data_type)
{
	boost::algorithm::trim_left(data_type);
	boost::algorithm::trim_right(data_type);
//	boost::algorithm::to_lower(data_type);
	if (data_type.compare("NullData") == 0)
	{
		return ci_null;
	}
	else if (data_type.compare("Array") == 0)
	{
		return ci_sequence_of_array;
	}
	else if (data_type.compare("Structure") == 0)
	{
		return ci_sequence_of_struct;
	}
	else if (data_type.compare("Boolean") == 0)
	{
		return ci_boolean;
	}
	else if (data_type.compare("BitString") == 0)
	{
		return ci_bit_string;
	}
	else if (data_type.compare("DoubleLong") == 0)
	{
		return ci_double_long;
	}
	else if (data_type.compare("DoubleLongUnsigned") == 0)
	{
		return ci_double_long_unsigned;
	}
	else if (data_type.compare("FloatingPoint") == 0)
	{
		return ci_floating_point;
	}
	else if (data_type.compare("OctetString") == 0)
	{
		return ci_octet_string;
	}
	else if (data_type.compare("VisibleString") == 0)
	{
		return ci_visible_string;
	}
	else if (data_type.compare("Bcd") == 0)
	{
		return ci_bcd;
	}
	else if (data_type.compare("Integer") == 0)
	{
		return ci_integer;
	}
	else if (data_type.compare("Long") == 0)
	{
		return ci_long;
	}
	else if (data_type.compare("Unsigned") == 0)
	{
		return ci_unsigned;
	}
	else if (data_type.compare("LongUnsigned") == 0)
	{
		return ci_long_unsigned;
	}
	else if (data_type.compare("Long64") == 0)
	{
		return ci_long64;
	}
	else if (data_type.compare("Long64Unsigned") == 0)
	{
		return ci_long64_unsigned;
	}
	else if (data_type.compare("Enum") == 0)
	{
		return ci_enum;
	}
	else if (data_type.compare("Float32") == 0)
	{
		return ci_float32;
	}
	else if (data_type.compare("Float64") == 0)
	{
		return ci_float64;
	}
	else if (data_type.compare("DateTime") == 0)
	{
		return ci_datetime;
	}
	else if (data_type.compare("Date") == 0)
	{
		return ci_date;
	}
	else if (data_type.compare("Time") == 0)
	{
		return ci_time;
	}
	else if (data_type.compare("DontCare") == 0)
	{
		return ci_dont_care;
	}
	else
		return ci_null;

}

unsigned int object_transfer::get_next_period_time(time_t start_time, int period)
{
	if (start_time % period == 0)
	{
		return start_time;
	}

	if (period < 86400)
		return start_time/period*period + period;
	else if (period == 86400)
	{
		tm *t = std::localtime(&start_time);
		boost::gregorian::date day(t->tm_year + 1900,t->tm_mon + 1,t->tm_mday);
		boost::gregorian::date day_next = day + boost::gregorian::days(1);

		tm tm_next;
		tm_next.tm_year = day_next.year() - 1900;
		tm_next.tm_mon = day_next.month() - 1;

		tm_next.tm_mday = day_next.day();
		tm_next.tm_hour = 0;
		tm_next.tm_min = 0;
		tm_next.tm_sec = 0;
		tm_next.tm_isdst = -1;

		return std::mktime(&tm_next);
	}
	else if (period == 43200)
	{

		tm *t = std::localtime(&start_time);
		boost::gregorian::date day(t->tm_year + 1900,t->tm_mon + 1,t->tm_mday);
		boost::gregorian::date day_next = day + boost::gregorian::months(1);

		tm tm_next;
		tm_next.tm_year = day_next.year() - 1900;
		tm_next.tm_mon = day_next.month() - 1;

		tm_next.tm_mday = 1;
		tm_next.tm_hour = 0;
		tm_next.tm_min = 0;
		tm_next.tm_sec = 0;
		tm_next.tm_isdst = -1;

		return std::mktime(&tm_next);
	}
	else
		return start_time;
}