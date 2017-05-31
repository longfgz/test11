#pragma once

#include <string>
#include <ptcl_codec/ptcl_module_dlms47.h>

class object_transfer
{
protected:
	enum data_type
	{
		ci_null							=	0,
		ci_sequence_of_array			=	1,
		ci_sequence_of_struct           =	2,
		ci_boolean						=	3,
		ci_bit_string					=	4,
		ci_double_long					=	5,
		ci_double_long_unsigned			=	6,
		ci_floating_point				=	7,			//IMPLICIT OCTET STRING(SIZE(4))33----
		ci_octet_string					=	9,
		ci_visible_string				=	10,
		ci_bcd							=	13,
		ci_integer						=	15,
		ci_long							=	16,
		ci_unsigned						=	17,
		ci_long_unsigned				=	18,			
		ci_long64						=	20,
		ci_long64_unsigned				=	21,
		ci_enum							=	22,
		ci_float32						=	23,
		ci_float64						=	24,
		ci_datetime						=	25,
		ci_date							=	26,
		ci_time							=	27,
		ci_dont_care					=	255,
	};

	enum obis_type
	{
		ot_null,
		ot_event,
		ot_event_code,
		ot_profile,
		ot_register,
		ot_firmware,	
		ot_disconnect_control,
	};

protected:
	object_transfer(void);
	~object_transfer(void);

public:
	static object_transfer *get_instance();

	bool format_get_normal_response(std::string obis, int class_id, int attr_id, unsigned int start_time, unsigned int end_time, int its_state, ptcl_module_dlms47::get_data_result & result,
		std::vector<ptcl_module_dlms47::profile_unit> & v_uint);

	bool format_action_normal_response(std::string obis, int class_id, int attr_id, int &its_state, ptcl_module_dlms47::get_data_result & result);

protected:
	int get_ci_type(std::string data_type);

	int get_obis_type(int class_id, std::string obis);

	void calc_points(std::string obis, unsigned int start_time, unsigned int end_time, unsigned int & out_start_time, unsigned int & points);

	bool format_profile_data_attr_2(std::string obis, unsigned int start_time, unsigned int end_time, std::vector<ptcl_module_dlms47::profile_unit>  & v_uints);

	bool format_profile_attr_3(std::string obis, std::vector<ptcl_module_dlms47::profile_unit>  & v_uints);

	bool format_profile_attr_4(std::string obis, ptcl_module_dlms47::get_data_result & result);

	bool format_profile_event_attr_2(std::string obis, unsigned int start_time, unsigned int end_time, std::vector<ptcl_module_dlms47::profile_unit>  & v_uints);

	bool format_profile_event_attr_3(std::string obis, std::vector<ptcl_module_dlms47::profile_unit>  & v_uints);

	bool format_profile_event_attr_4(std::string obis, ptcl_module_dlms47::get_data_result & result);

	bool format_register_attr_2(std::string obis, int class_id, int attr_id, ptcl_module_dlms47::get_data_result & result);

	bool format_event_code_attr_2(std::string obis, int class_id, int attr_id, ptcl_module_dlms47::get_data_result & result);

	bool format_disconnect_control_attr(int attr_id, ptcl_module_dlms47::get_data_result & result);

	bool format_disconnect_control_method(ptcl_module_dlms47::get_data_result & result);

	bool format_image_transfer_attr(int attr_id, int its_state, ptcl_module_dlms47::get_data_result & result);

	bool format_image_transfer_method(int attr_id, int &its_state, ptcl_module_dlms47::get_data_result & result);

	int get_event_code(std::string obis);

	unsigned int get_next_period_time(time_t start_time, int period);

protected:
	static object_transfer *instance_;
};

