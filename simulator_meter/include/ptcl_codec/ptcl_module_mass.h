#pragma once

#include <ptcl_codec/ptcl_module.h>
#include <ptcl_codec/ptcl_packet_mass.h>
#include <ptcl_codec/ptcl_cosem_dlms.h>  

class PTCL_CODEC_EXPORT ptcl_module_mass : public ptcl_module
{
protected:
	ptcl_module_mass(void);
	virtual ~ptcl_module_mass(void);

public:
	struct get_data_result
	{
		unsigned char result_type;
		unsigned char buffer[1024];
		int length;
	};


	static ptcl_module_mass *get_instance();

	void format_packet_cosem(ptcl_packet_mass *packet, int invokeid_priority);

	void fill_packet_cosem_get(ptcl_packet_mass *packet, short obis_class, char* obis, unsigned char attr_id);

	void fill_packet_cosem_set(ptcl_packet_mass *packet, short obis_class, char* obis, unsigned char attr_id, 
		unsigned char data[], int data_len);

	void fill_packet_cosem_set_filter(ptcl_packet_mass *packet, unsigned int start_time, unsigned int end_time,
		unsigned int amount, unsigned int bitfield, unsigned char mode = 0x02);

	void fill_packet_cosem_next(ptcl_packet_mass *packet, unsigned int block_no);

protected:
	static ptcl_module_mass *instance_;
};