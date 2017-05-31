#pragma once

#include <ptcl_codec/ptcl_module.h>
#include <ptcl_codec/ptcl_packet_hdlc.h>
#include <ptcl_codec/ptcl_packet_dlms47.h>

class PTCL_CODEC_EXPORT ptcl_module_hdlc : public ptcl_module
{
protected:
	ptcl_module_hdlc(void);
	virtual ~ptcl_module_hdlc(void);

public:
	enum ctrl_code 
	{
		ctrl_code_snrm			= 0x93,//µÇÂ½Ö¡SNRM
		ctrl_code_disc          = 0x53,//¶Ï¿ªÖ¡SNRM
		ctrl_code_ua            = 0x73,//µÇÂ½È·ÈÏÖ¡UA
	};

	static ptcl_module_hdlc *get_instance();

	void format_packet_hdlc_connect(ptcl_packet_hdlc *packet,
		short int max_send_bytes = 1024,short int max_recv_bytes = 1024,
		int send_window = 1,int recv_window = 7);

	void format_packet_hdlc_disconnect(ptcl_packet_hdlc *packet);
	
	/*contextID = 11 
	  senderAcseRequirements = 1 */
	void format_packet_cosem_connect(ptcl_packet_hdlc *packet,char* psw);
	
	void format_packet_cosem_disConnect(ptcl_packet_hdlc *packet);

	void dlms2hdlc(ptcl_packet_hdlc *packet, ptcl_packet_dlms47 *packet47, int recv_seq, int send_seq);

	void hdlc2dlms(ptcl_packet_hdlc *packet, ptcl_packet_dlms47 *packet47);

protected:
	static ptcl_module_hdlc *instance_;
};