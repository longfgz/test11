//#include "stdafx.h"
#include "asn_parser.h"
using namespace asn;

asn_parser_ptr asn_factory::create_parser(unsigned int tag,result_set  *results, std::string prefix)
{
	asn_parser_ptr ap;
	switch (tag)
	{
	case ci_cosem_attribute_descriptor:
		{
			unsigned int vec_item[]={ci_cosem_class_id, ci_cosem_object_instance_id, ci_cosem_object_attribute_id};
			ap.reset(new sequence_parser(this, results, vec_item, sizeof(vec_item)/4));
		}
		break;
	case ci_invoke_id_and_priority:
		ap.reset(new fixed_length_type(this, results, 1, ci_invoke_id_and_priority, "invoke_id_and_priority"));
		break;
	case ci_cosem_object_instance_id:
		ap.reset(new fixed_length_type(this, results, 6, ci_cosem_object_instance_id, "cosem_object_instance_id"));
		break;
	case ci_get_request_normal:
		{
			unsigned int vec_item[]={ci_invoke_id_and_priority, ci_cosem_attribute_descriptor, ci_selective_access_descriptor};
			ap.reset(new sequence_parser(this, results, vec_item, sizeof(vec_item)/4, 4));
		}
		break;
	case ci_get_request_next:
		{
			unsigned int vec_item[]={ci_invoke_id_and_priority, ci_unsigned32};
			ap.reset(new sequence_parser(this, results, vec_item, sizeof(vec_item)/4, 4));
		}
		break;
	case ci_get_request_with_list:
		{
			unsigned int vec_item[]={ci_invoke_id_and_priority, ci_attribute_descriptor_list};
			ap.reset(new sequence_parser(this, results, vec_item, sizeof(vec_item)/4, 4));
		}
		break;
	case ci_attribute_descriptor_list:
		{
			ap.reset(new sequence_of_with_list(this, results, ci_attribute_descriptor_list, "attribute_descriptor_list"));
		}
		break;
	case ci_cosem_class_id:
		ap.reset(new fixed_length_type(this, results, 2, ci_cosem_class_id, "cosem_class_id"));
		break;
	case ci_cosem_object_attribute_id:
		ap.reset(new fixed_length_type(this, results, 1, ci_cosem_object_attribute_id, "cosem_object_attribute_id"));
		break;
	case ci_cosem_pdu:
		{
			unsigned int vec_item[256]={ci_null};
			vec_item[192] = ci_get_request;
			vec_item[196] = ci_get_response;
			vec_item[193] = ci_set_request;
			vec_item[197] = ci_set_response;
			vec_item[195] = ci_action_request;
			ap.reset(new choice_parser(this, results, vec_item, sizeof(vec_item)/4, ci_cosem_pdu, "cosem_pdu"));
		}
		break;
	case ci_get_response:
		{
			unsigned int vec_item[]={ci_null, ci_get_response_normal, ci_get_response_with_datablock, ci_get_response_with_list};
			ap.reset(new choice_parser(this, results, vec_item, sizeof(vec_item)/4, ci_get_response, "get_response"));
		}
		break;
	case ci_get_response_normal:
		{
			unsigned int vec_item[]={ci_invoke_id_and_priority, ci_get_data_result};
			ap.reset(new sequence_parser(this, results, vec_item, sizeof(vec_item)/4));
		}
		break;
	case ci_get_response_with_datablock:
		{
			unsigned int vec_item[]={ci_invoke_id_and_priority, ci_datablock_g};
			ap.reset(new sequence_parser(this, results, vec_item, sizeof(vec_item)/4));
		}
		break;
	case ci_get_data_result:
		{
			unsigned int vec_item[]={ci_data, ci_data_access_result};
			ap.reset(new choice_parser(this, results, vec_item, sizeof(vec_item)/4, ci_get_data_result, "get_data_result"));
		}
		break;
	case ci_data_access_result:
		ap.reset(new fixed_length_type(this, results, 1, ci_data_access_result, "data_access_result"));
		break;
	case ci_data:
		{
			unsigned int vec_item[256]={ci_null, ci_sequence_of_array, ci_sequence_of_struct, ci_boolean, ci_bit_string,	
				ci_integer32, ci_unsigned32, ci_floating_point, ci_null, ci_octet_string, ci_visible_string, ci_gen_time, 
				ci_null, ci_bcd, ci_null, ci_integer8, ci_integer16, ci_unsigned8, ci_unsigned16, ci_null, ci_integer64, 
				ci_unsigned64, ci_enumerated, ci_float32, ci_float64, ci_datetime, ci_null};
			ap.reset(new choice_parser(this, results, vec_item, sizeof(vec_item)/4));
		}
		break;
	case ci_null:
		ap.reset(new fixed_length_type(this, results, 0, ci_null, prefix+"null_data"));
		break;
	case ci_sequence_of_array:
		ap.reset(new sequence_of_data(this, results, ci_sequence_of_array, "sequence_of_array"));
		break;
	case ci_sequence_of_struct:
		ap.reset(new sequence_of_data(this, results, ci_sequence_of_struct, "sequence_of_struct"));
		break;
	case ci_boolean:
		ap.reset(new fixed_length_type(this, results, 1, ci_boolean, prefix+"boolean"));
		break;
	case ci_bit_string:	
		ap.reset(new unfixed_length_type(this, results, ci_bit_string, prefix+"bit_string"));
		break;
	case ci_integer32:
		ap.reset(new fixed_length_type(this, results, 4, ci_integer32, prefix+"integer32"));
		break;
	case ci_unsigned32:
		ap.reset(new fixed_length_type(this, results, 4, ci_unsigned32, prefix+"unsigned32"));
		break;
	case ci_floating_point:
		ap.reset(new unfixed_length_type(this, results, ci_floating_point, prefix+"floating_point"));
		break;
	case ci_float32:
		ap.reset(new fixed_length_type(this, results, 4, ci_float32, prefix+"float32"));
		break;
	case ci_float64:
		ap.reset(new fixed_length_type(this, results, 8, ci_float64, prefix+"float64"));
		break;
	case ci_octet_string:
		ap.reset(new unfixed_length_type(this, results, ci_octet_string, "octet_string"));
		break;
	case ci_visible_string:
		ap.reset(new unfixed_length_type(this, results, ci_visible_string, prefix+"visible_string"));
		break;
	case ci_gen_time:
		ap.reset(new fixed_length_type(this, results, 6, ci_gen_time, prefix+"gen_time"));
		break;
	case ci_bcd:
		ap.reset(new fixed_length_type(this, results, 1, ci_bcd, prefix+"bcd"));
		break;
	case ci_integer8:
		ap.reset(new fixed_length_type(this, results, 1, ci_integer8, prefix+"integer8"));
		break;
	case ci_integer16:
		ap.reset(new fixed_length_type(this, results, 2, ci_integer16, prefix+"integer16"));
		break;
	case ci_unsigned8:
		ap.reset(new fixed_length_type(this, results, 1, ci_unsigned8, prefix+"unsigned8"));
		break;
	case ci_unsigned16:
		ap.reset(new fixed_length_type(this, results, 2, ci_unsigned16, prefix+"unsigned16"));
		break;
	case ci_integer64:
		ap.reset(new fixed_length_type(this, results, 8, ci_integer64, prefix+"integer64"));
		break;
	case ci_unsigned64:
		ap.reset(new fixed_length_type(this, results, 8, ci_unsigned64, prefix+"unsigned64"));
		break;
	case ci_datetime:
		ap.reset(new fixed_length_type(this, results, 12, ci_datetime, prefix+"datetime"));
		break;
	case ci_enumerated:
		ap.reset(new fixed_length_type(this, results, 1, ci_enumerated, prefix+"enumerated"));
		break;
	case ci_raw_data:
		ap.reset(new unfixed_length_type(this, results, ci_raw_data, prefix+"raw_data"));
		break;
	case ci_get_request:
		{
			unsigned int vec_item[]={ci_null, ci_get_request_normal, ci_get_request_next, ci_get_request_with_list};
			ap.reset(new choice_parser(this, results, vec_item, sizeof(vec_item)/4));
		}
		break;
	case ci_set_request:
		{
			unsigned int vec_item[]={ci_null, ci_set_request_normal,ci_set_request_with_first_datablock, ci_set_request_with_datablock,
				ci_set_request_with_list, ci_set_request_with_list_and_first_datablock,};
			ap.reset(new choice_parser(this, results, vec_item, sizeof(vec_item)/4));
		}
		break;
	case ci_set_response:
		{
			unsigned int vec_item[]={ci_null, ci_set_response_normal, ci_set_response_datablock, ci_set_response_last_datablock,//--
				ci_set_response_last_datablock_with_list, ci_set_response_with_list};
			ap.reset(new choice_parser(this, results, vec_item, sizeof(vec_item)/4));
		}
		break;
	case ci_set_request_normal:
		{
			unsigned int vec_item[]={ci_invoke_id_and_priority, ci_cosem_attribute_descriptor, ci_selective_access_descriptor, ci_data};
			ap.reset(new sequence_parser(this, results, vec_item, sizeof(vec_item)/4, 4));
		}
		break;
	case ci_set_response_normal:
		{
			unsigned int vec_item[]={ci_invoke_id_and_priority, ci_data_access_result};
			ap.reset(new sequence_parser(this, results, vec_item, sizeof(vec_item)/4));
		}
		break;
	case ci_action_request:
		{
			unsigned int vec_item[]={ci_null, ci_action_request_normal,ci_action_request_next_pblock,
										ci_action_request_with_list, ci_action_request_with_first_pblock,
										ci_action_request_with_list_and_first_pblock, ci_action_reqeust_with_pblock,};
			ap.reset(new choice_parser(this, results, vec_item, sizeof(vec_item)/4));
		}
		break;
	case ci_action_response:
		{
			unsigned int vec_item[]={ci_null, ci_action_response_normal,ci_action_response_with_pblock,
									 ci_action_response_with_list, ci_action_response_next_pblock};
			ap.reset(new choice_parser(this, results, vec_item, sizeof(vec_item)/4));
		}
		break;
	case ci_action_request_normal:
		{
			unsigned int vec_item[]={ci_invoke_id_and_priority, ci_cosem_attribute_descriptor, ci_data};
			ap.reset(new sequence_parser(this, results, vec_item, sizeof(vec_item)/4, 4));
		}
		break;
	case ci_selective_access_descriptor:
		{
			unsigned int vec_item[]={ci_unsigned8, ci_data};
			ap.reset(new sequence_parser(this, results, vec_item, sizeof(vec_item)/4,4));
		}
		break;
	case ci_aarq:
		{
			unsigned int vec_item[]={ci_unsigned8, ci_data};
			ap.reset(new ber_sequence_parser(this, results, vec_item, sizeof(vec_item)/4));
		}
		break;
	case ci_datablock_g:
		{
			unsigned int vec_item[]={ci_boolean, ci_unsigned32, ci_result};
			ap.reset(new sequence_parser(this, results, vec_item, sizeof(vec_item)/4, 0, "datablock_g"));
		}
		break;
	case ci_result:
		{
			unsigned int vec_item[]={ci_raw_data, ci_data_access_result};
			ap.reset(new choice_parser(this, results, vec_item, sizeof(vec_item)/4));
		}
		break;
	}
	if (ap == NULL)
	{
		int i=0;
	}
	return ap;
}
