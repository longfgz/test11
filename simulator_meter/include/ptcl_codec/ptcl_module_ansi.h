#pragma once

#include "ptcl_codec_global.h"
#include <boost/unordered_map.hpp>
#include <ptcl_codec/ptcl_packet_ansi.h>
#include <ptcl_codec/ptcl_module.h>

class PTCL_CODEC_EXPORT ptcl_module_ansi : public ptcl_module
{
public:
  ptcl_module_ansi(void);
  virtual ~ptcl_module_ansi(void);

  static ptcl_module_ansi *get_instance();

  static void fill_read_request(ptcl_packet_ansi &packet, uint16_t table_id, uint32_t offset, uint16_t read_bytes);
  static void fill_write_table_request(ptcl_packet_ansi &packet,uint16_t table_id,uint8_t *buffer,uint16_t len); //–¥’˚’≈±Ì
  void format_packet_read_data_history(	ptcl_packet_ansi *packet);

  void format_packet_read_event(ptcl_packet_ansi *packet);

  void format_packet_get_param(	ptcl_packet_ansi *packet);

  void format_packet_set_param(	ptcl_packet_ansi *packet);

protected:
	static ptcl_module_ansi *instance_;
};
