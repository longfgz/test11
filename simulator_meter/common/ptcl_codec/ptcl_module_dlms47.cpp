#include <ptcl_codec/ptcl_module_dlms47.h>
#include <ptcl_codec/asn_codec.h>
#include <time.h>
#include <stdio.h>

ptcl_module_dlms47 *ptcl_module_dlms47::instance_ = NULL;
ptcl_module_dlms47::ptcl_module_dlms47(void)
{

}

ptcl_module_dlms47::~ptcl_module_dlms47(void)
{

}

ptcl_module_dlms47 *ptcl_module_dlms47::get_instance()
{
	if (instance_ == NULL)
	{
		instance_ = new ptcl_module_dlms47;
	}
	return instance_;
}

/****************************************************************************/
/*description: packet one pure AARQ message.								*/
/*input :																	*/
/*		host_id: host id use in the wrapper header,always called wPort. 	*/
/*output:																	*/
/*		pakcet: one pure AARQ message in ptcl_packet_dlms47 format.			*/
/****************************************************************************/
void ptcl_module_dlms47::format_packet_pure_aarq(ptcl_packet_dlms47 *packet,unsigned short host_id)
{
	//主站ID
	packet->set_src_port(host_id);

	//用户数据域
	unsigned char buf_data_area[1024];
	int length = 0;

	asn_codec::PDU aarq[2];

	//application-context-name
	unsigned char acn[] = {
		0xa1,//Context 1
		0x09,//长度
		0x06,//OBJECT IDENTIFIER 类型标识 06
		0x07,//长度
		0x60,0x85,0x74,0x05,0x08,0x01,0x01};//内容（2 16 756 5 8 1 1）
	memcpy(aarq[0].context_area,acn,11);
	aarq[0].len_context_area = 11;

	//user-information
	unsigned short max_apdu_length = APDU_MAX_LEN;
	unsigned char L_byte = *(unsigned char*)(&max_apdu_length);
	unsigned char H_byte = *((unsigned char*)(&max_apdu_length) + 1);
	unsigned char ui[]  = {
		0xbe,//Context 30
		0x10,//长度
		0x04,//OTCET String 类型标识 04
		0x0e,//长度
		0x01,0x00,0x00,0x00,0x06,
		0x5f,0x1f,0x04,0x00,0x00,
		0x7E,0x1F,H_byte,L_byte};
	memcpy(aarq[1].context_area,ui,18);
	aarq[1].len_context_area = 18;

	//AARQ
	int len_aarq = asn_codec::format_ber_seqence(asn_codec::asn_tagging_class_application,0,aarq,2,buf_data_area+length);
	length = length + len_aarq;

	packet->set_data_area_buffer(buf_data_area,length);

	packet->format_packet();
}

/****************************************************************************/
/*description: packet one pure RLRQ message.								*/
/*input :																	*/
/*		host_id: host id use in the wrapper header,always called wPort. 	*/
/*output:																	*/
/*		pakcet: one pure RLRQ message in ptcl_packet_dlms47 format.			*/
/****************************************************************************/
void ptcl_module_dlms47::format_packet_pure_rlrq(ptcl_packet_dlms47 *packet,unsigned short host_id)
{
	//设置src_wport
	packet->set_src_port(host_id);
	
	//用户数据域
	unsigned char buf_data_area[512] = {0};
	
	//设置相关数据
	int pos = 0;
	buf_data_area[pos++] = 0x62;
	int len_pos = pos++;

	//RLRQ's  reason field

	unsigned char reason_field[3] = {0x80,0x01,0x00}; //reason set to normal
	memcpy(buf_data_area+pos,reason_field,3);
	pos = pos + 3; 

	//RLRQ's user_information field
	unsigned short max_apdu_length = APDU_MAX_LEN;
	unsigned char L_byte = *(unsigned char*)(&max_apdu_length);
	unsigned char H_byte = *((unsigned char*)(&max_apdu_length) + 1);
	unsigned char ui[]  = {
		0xbe,//Context 30
		0x10,//长度
		0x04,//OTCET String 类型标识 04
		0x0e,//长度
		0x01,0x00,0x00,0x00,0x06,
		0x5f,0x1f,0x04,0x00,0x00,
		0x7E,0x1F,H_byte,L_byte};
	memcpy(buf_data_area + pos,ui,18);
	pos = pos + 18; 
	
	buf_data_area[len_pos] = pos - 2; //减去2得到length of the RLRQ's contents field

	//将数据填进packet
	packet->set_data_area_buffer(buf_data_area,pos); //此处用pos，是整个rlrq消息的长度

	packet->format_packet();

}

void ptcl_module_dlms47::format_packet_cosem_connect(ptcl_packet_dlms47 *packet, int mech, char* psw)
{
	//主站ID
	packet->set_host_id(host_id_);

	//用户数据域
	unsigned char buf_data_area[1024];
	int length = 0;

	asn_codec::PDU aarq[5];

	//application-context-name
	unsigned char acn[] = {
		0xa1,//Context 1
		0x09,//长度
		0x06,//OBJECT IDENTIFIER 类型标识 06
		0x07,//长度
		0x60,0x85,0x74,0x05,0x08,0x01,0x01};//内容（2 16 756 5 8 1 1）
	memcpy(aarq[0].context_area,acn,11);
	aarq[0].len_context_area = 11;

	//sender-acse-requirements
	aarq[1].len_context_area = asn_codec::format_ber_bit_string(asn_codec::asn_tagging_class_context,10,"1",aarq[1].context_area);

	//mechanism-name 2 16 756 5 8 2 X(1)
	int oi[] = {2,16,756,5,8,2,mech};
	aarq[2].len_context_area = asn_codec::format_ber_object_indentifier(asn_codec::asn_tagging_class_context,11, oi,7,aarq[2].context_area);

	//calling-authentication-value
	unsigned char sub_cav[1024];
	int len_sub_cav = asn_codec::format_ber_graphic_string(asn_codec::asn_tagging_class_context,0, psw, sub_cav);
	aarq[3].len_context_area = asn_codec::format_ber_choice(asn_codec::asn_tagging_class_context,12,sub_cav,len_sub_cav,aarq[3].context_area);

	unsigned short max_apdu_length = APDU_MAX_LEN;
	unsigned char L_byte = *(unsigned char*)(&max_apdu_length);
	unsigned char H_byte = *((unsigned char*)(&max_apdu_length) + 1);

	//user-information
	unsigned char ui[]  = {
		0xbe,//Context 30
		0x10,//长度
		0x04,//OTCET String 类型标识 04
		0x0e,//长度
		0x01,0x00,0x00,0x00,0x06,
		0x5f,0x1f,0x04,0x00,0x00,
		0x7E,0x1F,H_byte,L_byte};
	memcpy(aarq[4].context_area,ui,18);
	aarq[4].len_context_area = 18;

	//AARQ
	int len_aarq = asn_codec::format_ber_seqence(asn_codec::asn_tagging_class_application,0,aarq,5,buf_data_area+length);
	length = length + len_aarq;

	packet->set_data_area_buffer(buf_data_area,length);

	packet->format_packet();
}

void ptcl_module_dlms47::format_packet_cosem_connect_with_security(ptcl_packet_dlms47 *packet, int mech, 
																   unsigned char* sys_title,int len,char* psw)
{
	//主站ID
	packet->set_host_id(host_id_);

	//用户数据域
	unsigned char buf_data_area[1024];
	int length = 0;

	asn_codec::PDU aarq[6];

	//application-context-name
	unsigned char acn[] = {
		0xa1,//Context 1
		0x09,//长度
		0x06,//OBJECT IDENTIFIER 类型标识 06
		0x07,//长度
		0x60,0x85,0x74,0x05,0x08,0x01,0x03};//内容（2 16 756 5 8 1 1）
	memcpy(aarq[0].context_area,acn,11);
	aarq[0].len_context_area = 11;

	//sender-acse-requirements
	aarq[1].len_context_area = asn_codec::format_ber_bit_string(asn_codec::asn_tagging_class_context,10,"1",aarq[1].context_area);

	//mechanism-name 2 16 756 5 8 2 X(1)
	int oi[] = {2,16,756,5,8,2,mech};
	aarq[2].len_context_area = asn_codec::format_ber_object_indentifier(asn_codec::asn_tagging_class_context,11, oi,7,aarq[2].context_area);

	//calling-ap-title
	unsigned char cat[64] = {0xa6,0x0a,0x04};

	cat[3] = len;
	memcpy(cat+4,sys_title,len);
	memcpy(aarq[3].context_area,cat,4+len);
	aarq[3].len_context_area = 4 + len;

/*	unsigned char cat[] = {
		0xa6,
		0x0a,
		0x04,
		0x08,
		0x4D,0x4D,0x4D,0x00,0x00,0xBC,0x61,0x4E};
	memcpy(aarq[3].context_area,cat,12);
	aarq[3].len_context_area = 12;*/

	//calling-authentication-value
	unsigned char sub_cav[1024];
	int len_sub_cav = asn_codec::format_ber_graphic_string(asn_codec::asn_tagging_class_context,0, psw, sub_cav);
	aarq[4].len_context_area = asn_codec::format_ber_choice(asn_codec::asn_tagging_class_context,12,sub_cav,len_sub_cav,aarq[4].context_area);

	unsigned short max_apdu_length = APDU_MAX_LEN;
	unsigned char L_byte = *(unsigned char*)(&max_apdu_length);
	unsigned char H_byte = *((unsigned char*)(&max_apdu_length) + 1);

	//user-information
	unsigned char ui[]  = {
		0xbe,//Context 30
		0x10,//长度
		0x04,//OTCET String 类型标识 04
		0x0e,//长度
		0x01,0x00,0x00,0x00,0x06,
		0x5f,0x1f,0x04,0x00,0x00,
		0x7E,0x1F,H_byte,L_byte};
	memcpy(aarq[5].context_area,ui,18);
	aarq[5].len_context_area = 18;

	//AARQ
	int len_aarq = asn_codec::format_ber_seqence(asn_codec::asn_tagging_class_application,0,aarq,6,buf_data_area+length);
	length = length + len_aarq;

	packet->set_data_area_buffer(buf_data_area,length);

	packet->format_packet();
}

/****************************************************************************/
/*description: format the AARQ packet in LN mode.							*/
/*input :																	*/
/*		host_id: host id use in the wrapper header,always is src wPort. 	*/
/*		b_ciphering: flag of with ciphering init_request or not				*/
/*		mech_id: mechanism id in the mechanism-name							*/
/*		sys_title: systen title,type is should be [bytes string]			*/
/*					i.e {0x4D,0x4D,0x4D,0x00,0x00,0x00,0x00,0x01}			*/
/*		len_1: lenght of the bytes sys_title								*/
/*		authen_value: password or challenge string,type is [bytes string]	*/
/*					i.e "kemakema" 、"K56iVagY"								*/
/*		init_request: ciphered or no ciphered init_request,not cipher in 	*/
/*					this function.type is [bytes string]					*/
/*		len_2: length of bytes init_request									*/
/*output:																	*/
/*		pakcet: AARQ message in ptcl_packet_dlms47 format.					*/
/****************************************************************************/
void ptcl_module_dlms47::format_packet_cosem_connect_LN(ptcl_packet_dlms47 *packet, unsigned short host_id,
														bool b_ciphering,int mech_id,unsigned char* sys_title,int len_1, 
														char* authen_value,unsigned char* init_request,int len_2)
{
	if(len_1 >= 255 || len_2 >= 255)
	{
		return;
	}

	//主站ID
	packet->set_src_port(host_id);
	 
	//用户数据域
	unsigned char buf_data_area[1024];
	int length = 0;

	asn_codec::PDU aarq[6];

	//application-context-name
	unsigned char context_id = b_ciphering ? 3 : 1;
	unsigned char acn[] = {
		0xa1,//Context 1
		0x09,//长度
		0x06,//OBJECT IDENTIFIER 类型标识 06
		0x07,//长度
		0x60,0x85,0x74,0x05,0x08,0x01,context_id};//内容（2 16 756 5 8 1 context_id）
	memcpy(aarq[0].context_area,acn,11);
	aarq[0].len_context_area = 11;

	//calling-ap-title
	unsigned char cat[64] = {0xa6,0x0a,0x04};

	cat[3] = len_1;
	memcpy(cat+4,sys_title,len_1);
	memcpy(aarq[1].context_area,cat,4+len_1);
	aarq[1].len_context_area = 4 + len_1;

	//sender-acse-requirements
	aarq[2].len_context_area = asn_codec::format_ber_bit_string(asn_codec::asn_tagging_class_context,10,"1",aarq[2].context_area);

	//mechanism-name 2 16 756 5 8 2 X(mech_id)
	int oi[] = {2,16,756,5,8,2,mech_id};
	aarq[3].len_context_area = asn_codec::format_ber_object_indentifier(asn_codec::asn_tagging_class_context,11, oi,7,aarq[3].context_area);

	//calling-authentication-value
	unsigned char sub_cav[1024];
	int len_sub_cav = asn_codec::format_ber_graphic_string(asn_codec::asn_tagging_class_context,0, authen_value, sub_cav);
	aarq[4].len_context_area = asn_codec::format_ber_choice(asn_codec::asn_tagging_class_context,12,sub_cav,len_sub_cav,aarq[4].context_area);

	//user-information
	unsigned char tag_component_len = len_2 + 2;	//length of the tagged component's value field.
	unsigned char ui_len = tag_component_len + 2;	//length of user information.
	unsigned char ui[255] = {
		0xbe,//Context 30
		tag_component_len,//长度
		0x04,//OTCET String 类型标识 04
		len_2//长度
	};
	memcpy(ui + 4,init_request,len_2); //init_request
	memcpy(aarq[5].context_area,ui,ui_len);
	aarq[5].len_context_area = ui_len;

	//AARQ
	int len_aarq = asn_codec::format_ber_seqence(asn_codec::asn_tagging_class_application,0,aarq,6,buf_data_area+length);
	length = length + len_aarq;

	packet->set_data_area_buffer(buf_data_area,length);

	packet->format_packet();
}

void ptcl_module_dlms47::format_packet_cosem_disc(ptcl_packet_dlms47 *packet)
{
	//主站ID
	packet->set_host_id(host_id_);

	//用户数据域
	unsigned char buf_data_area[2]={0x62, 0x00};

	packet->set_data_area_buffer(buf_data_area,sizeof(buf_data_area));

	packet->format_packet();
}

/****************************************************************************/
/*description: format the RLRQ packet in LN mode.							*/
/*input :																	*/
/*		use_flags:flags of the used components,see RLRQ_COMPON_FLAGS enum.	*/
/*		host_id: host id use in the wrapper header,always is src wPort. 	*/
/*		req_reason: request reason, normal|urgent|user-defined				*/
/*		init_request: initial request										*/
/*		len: length of the initial request									*/
/*output:																	*/
/*		pakcet: RLRQ message in ptcl_packet_dlms47 format.					*/
/****************************************************************************/
void ptcl_module_dlms47::format_packet_cosem_disc_LN(ptcl_packet_dlms47 *packet,unsigned short host_id,int use_flags,
													 unsigned char req_resson,unsigned char* init_request,int len)
{
	//设置src_wport
	packet->set_src_port(host_id);
	
	//用户数据域
	unsigned char buf_data_area[512] = {0};
	
	//设置相关数据
	int pos = 0;
	buf_data_area[pos++] = 0x62;
	int len_pos = pos++;

	//RLRQ's  reason field
	if(use_flags & RLRQ_COMPON_FLAGS::REASON_FIELD)
	{
		unsigned char reason_field[3] = {0x80,0x01,req_resson};
		memcpy(buf_data_area+pos,reason_field,3);
		pos = pos + 3; 
	}

	//RLRQ's user_information field
	if(use_flags & RLRQ_COMPON_FLAGS::USER_INFO_RLRQ)
	{
		if(len > 255)
		{
			return;
		}

		int tpos = 0;
		unsigned char user_data[256] = {0};
		user_data[tpos++] = 0xBE;		//tag ([30])
		user_data[tpos++] = len + 2;	//length of the user information
		user_data[tpos++] = 0x04,		//choice for user-information (OCTET STRING) 
		user_data[tpos++] = len;		//length of the OCTET STRING
		memcpy(user_data + tpos,init_request,len);
		tpos = tpos + len;

		memcpy(buf_data_area + pos,user_data,tpos);
		pos = pos + tpos; 
	}

	buf_data_area[len_pos] = pos - 2; //减去2得到length of the RLRQ's contents field

	//将数据填进packet
	packet->set_data_area_buffer(buf_data_area,pos); //此处用pos，是整个rlrq消息的长度

	packet->format_packet();
}

void ptcl_module_dlms47::format_packet_cosem(ptcl_packet_dlms47 *packet, int invokeid_priority)
{
	//主站ID
	packet->set_host_id(host_id_);

	unsigned char *cosem_data = packet->get_data_area_buffer();
	cosem_data[2] = (unsigned char)invokeid_priority;

	packet->format_packet();
}

void ptcl_module_dlms47::fill_packet_cosem_get(ptcl_packet_dlms47 *packet, short obis_class, char* obis, unsigned char attr_id)
{
	//用户数据域
	unsigned char buf_data_area[1024];
	int length = 0;

	//APDU
	buf_data_area[length++] = 0xC0;//A-XDR编码get-request [192] IMPLICIT
	buf_data_area[length++] = 0x01;//[1] IMPLICIT Get-Request-Normal,
	buf_data_area[length++] = 0xFF;

	//class id
	int len = asn_codec::format_context_area_short(obis_class,buf_data_area+length);
	length = length + len;

	//obis
	len = asn_codec::format_context_area_obis(obis,buf_data_area+length);
	length = length + len;
	buf_data_area[length++] = attr_id;//buffer  属性的值
	buf_data_area[length++] = 0x00;//access_selector is null

	packet->set_data_area_buffer(buf_data_area,length);

	packet->format_packet();
}

void ptcl_module_dlms47::fill_packet_cosem_get(ptcl_packet_dlms47 *packet, char* obis, time_t start, time_t end)
{
	//用户数据域
	unsigned char buf_data_area[1024];
	int length = 0;

	//APDU
	buf_data_area[length++] = 0xC0;//A-XDR编码get-request [192] IMPLICIT
	buf_data_area[length++] = 0x01;//[1] IMPLICIT Get-Request-Normal,
	buf_data_area[length++] = 0xFF;

	//class id
	int len = asn_codec::format_context_area_short(ptcl_module_dlms47::ci_profile,buf_data_area+length);
	length = length + len;

	//obis
	len = asn_codec::format_context_area_obis(obis,buf_data_area+length);
	length = length + len;
	buf_data_area[length++] = 0x02;//buffer  属性的值

	buf_data_area[length++] = 0x01;//access_selector
	buf_data_area[length++] = 0x01; //访问选择的值（range_descriptor）
	buf_data_area[length++] = 0x02;	//structure
	buf_data_area[length++] = 0x04;	//length
	buf_data_area[length++] = 0x02;	//structure  restricting_object  capture_object_definition
	buf_data_area[length++] = 0x04; //length

	//class id
	buf_data_area[length++] = 0x12;
	len = asn_codec::format_context_area_short(ptcl_module_dlms47::ci_clock,buf_data_area+length);
	length = length + len;

	//logic_name
	buf_data_area[length++] = 0x09;
	buf_data_area[length++] = 0x06;
	len = asn_codec::format_context_area_obis("0000010000ff",buf_data_area+length);
	length = length + len;

	//attribute_index
	buf_data_area[length++] = 0x0f;
	buf_data_area[length++] = 0x02;

	//data_index
	buf_data_area[length++] = 0x12;
	buf_data_area[length++] = 0x00;
	buf_data_area[length++] = 0x00;

	//instance_specific  date_time
	char time_s[32];
	memset(time_s,0,16);
	tm *t = localtime(&start);
	//buf_data_area[length++] = 0x19;
	buf_data_area[length++] = 0x09;
	buf_data_area[length++] = 0x0C;
	sprintf(time_s,"%04x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",t->tm_year+1900,t->tm_mon+1,t->tm_mday,t->tm_wday,t->tm_hour,t->tm_min,t->tm_sec,0,0x80,0,0);
	len = asn_codec::format_context_area_obis(time_s,buf_data_area+length);
	length = length + len;

	// instance_specific  date_time
	//char time_e[16] = {'\0'};
	t = localtime(&end);
	//buf_data_area[length++] = 0x19;
	buf_data_area[length++] = 0x09;
	buf_data_area[length++] = 0x0C;
	sprintf(time_s,"%04x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",t->tm_year+1900,t->tm_mon+1,t->tm_mday,t->tm_wday,t->tm_hour,t->tm_min,t->tm_sec,0,0x80,0,0);
	len = asn_codec::format_context_area_obis(time_s,buf_data_area+length);
	length = length + len;

	// array capture_object_definition
	buf_data_area[length++] = 0x01;
	buf_data_area[length++] = 0x00;

	packet->set_data_area_buffer(buf_data_area,length);

	packet->format_packet();

}

void ptcl_module_dlms47::fill_packet_cosem_get(ptcl_packet_dlms47 *packet, char* obis, time_t start, time_t end, unsigned char type)
{
	//用户数据域
	unsigned char buf_data_area[1024];
	int length = 0;

	//APDU
	buf_data_area[length++] = 0xC0;//A-XDR编码get-request [192] IMPLICIT
	buf_data_area[length++] = 0x01;//[1] IMPLICIT Get-Request-Normal,
	buf_data_area[length++] = 0xFF;

	//class id
	int len = asn_codec::format_context_area_short(ptcl_module_dlms47::ci_profile,buf_data_area+length);
	length = length + len;

	//obis
	len = asn_codec::format_context_area_obis(obis,buf_data_area+length);
	length = length + len;
	buf_data_area[length++] = 0x02;//buffer  属性的值

	buf_data_area[length++] = 0x01;//access_selector
	buf_data_area[length++] = 0x01; //访问选择的值（range_descriptor）
	buf_data_area[length++] = 0x02;	//structure
	buf_data_area[length++] = 0x04;	//length
	buf_data_area[length++] = 0x02;	//structure  restricting_object  capture_object_definition
	buf_data_area[length++] = 0x04; //length

	//class id
	buf_data_area[length++] = 0x12;
	len = asn_codec::format_context_area_short(ptcl_module_dlms47::ci_clock,buf_data_area+length);
	length = length + len;

	//logic_name
	buf_data_area[length++] = 0x09;
	buf_data_area[length++] = 0x06;
	len = asn_codec::format_context_area_obis("0000010000ff",buf_data_area+length);
	length = length + len;

	//attribute_index
	buf_data_area[length++] = 0x0f;
	buf_data_area[length++] = 0x02;

	//data_index
	buf_data_area[length++] = 0x12;
	buf_data_area[length++] = 0x00;
	buf_data_area[length++] = 0x00;

	//instance_specific  date_time
	char time_s[32];
	memset(time_s,0,16);
	tm *t = localtime(&start);
	if (type == 0)
	{
		buf_data_area[length++] = 0x19;
	}
	else
	{
		buf_data_area[length++] = 0x09;
		buf_data_area[length++] = 0x0C;
	}
	sprintf(time_s,"%04x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",t->tm_year+1900,t->tm_mon+1,t->tm_mday,t->tm_wday,t->tm_hour,t->tm_min,t->tm_sec,0,0x80,0,0);
	len = asn_codec::format_context_area_obis(time_s,buf_data_area+length);
	length = length + len;

	// instance_specific  date_time
	//char time_e[16] = {'\0'};
	t = localtime(&end);
	if (type == 0)
	{
		buf_data_area[length++] = 0x19;
	}
	else
	{
		buf_data_area[length++] = 0x09;
		buf_data_area[length++] = 0x0C;
	}
	sprintf(time_s,"%04x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",t->tm_year+1900,t->tm_mon+1,t->tm_mday,t->tm_wday,t->tm_hour,t->tm_min,t->tm_sec,0,0x80,0,0);
	len = asn_codec::format_context_area_obis(time_s,buf_data_area+length);
	length = length + len;

	// array capture_object_definition
	buf_data_area[length++] = 0x01;
	buf_data_area[length++] = 0x00;

	packet->set_data_area_buffer(buf_data_area,length);

	packet->format_packet();

}

void ptcl_module_dlms47::fill_packet_cosem_next(ptcl_packet_dlms47 *packet, unsigned int block_no)
{
	//用户数据域
	unsigned char buf_data_area[1024];
	int length = 0;

	//APDU
	buf_data_area[length++] = 0xC0;//A-XDR编码get-request [192] IMPLICIT
	buf_data_area[length++] = 0x02;//[1] IMPLICIT Get-Request-Normal,
	buf_data_area[length++] = 0xFF;

	//class id
	int len = asn_codec::format_context_area_int(block_no,buf_data_area+length);
	length = length + len;

	packet->set_data_area_buffer(buf_data_area,length);

	packet->format_packet();
}

void ptcl_module_dlms47::fill_packet_cosem_set(ptcl_packet_dlms47 *packet, 
											   short obis_class, 
											   char* obis, 
											   unsigned char attr_id, 
											   unsigned char data[], 
											   int data_len, 
											   int data_type)
{
	//用户数据域
	unsigned char buf_data_area[1024];
	int length = 0;

	//APDU
	buf_data_area[length++] = 0xC1;
	buf_data_area[length++] = 0x01;
	buf_data_area[length++] = 0xFF;

	//class id
	int len = asn_codec::format_context_area_short(obis_class,buf_data_area+length);
	length = length + len;

	//obis
	len = asn_codec::format_context_area_obis(obis,buf_data_area+length);
	length = length + len;
	buf_data_area[length++] = attr_id;//buffer  属性的值
	buf_data_area[length++] = 0x00;//access_selector is null
	buf_data_area[length++] = (unsigned char)data_type;
	buf_data_area[length++] = data_len;
	memcpy(buf_data_area+length, data, data_len);
	length += data_len;

	packet->set_data_area_buffer(buf_data_area,length);

	packet->format_packet();
}

void ptcl_module_dlms47::fill_packet_cosem_action(ptcl_packet_dlms47 *packet, short obis_class, char* obis, unsigned char method_id, 
		unsigned char data[], unsigned int data_len)
{
	//用户数据域
	unsigned char buf_data_area[1024];
	int length = 0;

	//APDU
	buf_data_area[length++] = 0xC3;
	buf_data_area[length++] = 0x01;
	buf_data_area[length++] = 0xFF;

	//class id
	int len = asn_codec::format_context_area_short(obis_class,buf_data_area+length);
	length = length + len;
	//obis
	len = asn_codec::format_context_area_obis(obis,buf_data_area+length);
	length = length + len;
	//MethodId
	buf_data_area[length++] = method_id;

	//Data OPTIONAL
	if (data == NULL || data_len == 0)
	{
		buf_data_area[length++] = 0;
	}
	else
	{
		buf_data_area[length++] = 1;
		memcpy(buf_data_area + length, data, data_len);
		length += data_len;
	}

	packet->set_data_area_buffer(buf_data_area,length);

	packet->format_packet();
}

void ptcl_module_dlms47::fill_packet_cosem_set(ptcl_packet_dlms47 *packet, short obis_class, char* obis, unsigned char attr_id, 
		unsigned char data[], int data_len)
{
	//用户数据域
	unsigned char buf_data_area[1024];
	int length = 0;

	//APDU
	buf_data_area[length++] = 0xC1;
	buf_data_area[length++] = 0x01;
	buf_data_area[length++] = 0xFF;

	//class id
	int len = asn_codec::format_context_area_short(obis_class,buf_data_area+length);
	length = length + len;

	//obis
	len = asn_codec::format_context_area_obis(obis,buf_data_area+length);
	length = length + len;
	buf_data_area[length++] = attr_id;//buffer  属性的值
	buf_data_area[length++] = 0x00;//access_selector is null

	memcpy(buf_data_area + length, data, data_len);
	length += data_len;

	packet->set_data_area_buffer(buf_data_area,length);

	packet->format_packet();
}

void ptcl_module_dlms47::format_packet_glo(ptcl_packet_dlms47 *packet, unsigned char tag, unsigned char sc, unsigned int fc, std::string str_data, unsigned int client_id)
{
	//主站ID
	packet->set_host_id(host_id_);
	packet->set_dst_port(client_id);

	//用户数据域
	unsigned char buf_data_area[1024];
	int length = 0;

	//APDU
	buf_data_area[length++] = tag;
	length += asn_codec::format_length_area(str_data.size()/2+5, buf_data_area+length);
	buf_data_area[length++] = sc;
	length += asn_codec::format_context_area_int(fc, buf_data_area+length);
	length += asn_codec::format_context_area_octet_string((char*)str_data.c_str(), buf_data_area+length);

	packet->set_data_area_buffer(buf_data_area,length);

	packet->format_packet();
}

void ptcl_module_dlms47::format_packet_action_request(ptcl_packet_dlms47 *packet, std::string str_data)
{
	//主站ID
	packet->set_host_id(host_id_);

	//用户数据域
	unsigned char buf_data_area[1024];
	int length = 0;

	buf_data_area[length++] = 0xC3; // ActionRequest 
	buf_data_area[length++] = 0x01; //ActionRequestNormal
	buf_data_area[length++] = 0xFF; //InvokeIdAndPriority

	//MethodDescriptor
	unsigned char mdes[] = {
		0x00, 0x0F, 
		0x00,0x00,0x28,0x00,0x00,0xFF,
		0x01
	};

	memcpy(buf_data_area+length, mdes, sizeof(mdes));
	length += sizeof(mdes);

	buf_data_area[length++] = 0x01;
	buf_data_area[length++] = 0x09;
	buf_data_area[length++] = str_data.size()/2;
	length += asn_codec::format_context_area_octet_string((char*)str_data.c_str(), buf_data_area+length);

	packet->set_data_area_buffer(buf_data_area,length);

	packet->format_packet();
}

void ptcl_module_dlms47::format_packet_get_response(ptcl_packet_dlms47 *packet,unsigned int client_id, get_data_result *result, int invoke_id)
{
	//主站ID
	packet->set_host_id(host_id_);
	packet->set_dst_port(client_id);

	//用户数据域
	unsigned char buf_data_area[1024];
	memset(buf_data_area, 0, 1024);
	int length = 0;

	buf_data_area[length++] = 0xC4; // ActionRequest 
	buf_data_area[length++] = 0x01; //ActionRequestNormal
	buf_data_area[length++] = invoke_id; //InvokeIdAndPriority

	if (result == NULL)
	{
		buf_data_area[length++] = 0x01;
		buf_data_area[length++] = 0x03;
	}
	else 
	{
		buf_data_area[length++] = 0x00;
		buf_data_area[length++] = result->result_type;
		if ( result->result_type == 4 || result->result_type == 9 ||  result->result_type == 10 ||  result->result_type == 12)
				buf_data_area[length++] =  result->length;
		memcpy(buf_data_area+length, result->buffer, result->length);
		length += result->length;
	}
	

	packet->set_data_area_buffer(buf_data_area,length);

	packet->format_packet();
}

void ptcl_module_dlms47::format_packet_get_response(ptcl_packet_dlms47 *packet,unsigned int client_id, std::vector<profile_unit> vec_unit, int invoke_id)
{
		//主站ID
	packet->set_host_id(host_id_);
	packet->set_dst_port(client_id);

	//用户数据域
	unsigned char buf_data_area[1024];
	memset(buf_data_area, 0, 1024);
	int length = 0;

	buf_data_area[length++] = 0xC4; // ActionRequest 
	buf_data_area[length++] = 0x01; //ActionRequestNormal
	buf_data_area[length++] = invoke_id; //InvokeIdAndPriority

	buf_data_area[length++] = 0x00;
	buf_data_area[length++] = 0x01;
	buf_data_area[length++] = vec_unit.size();
	

	for (int i=0; i<vec_unit.size(); i++)
	{
		buf_data_area[length++] = 0x02;
		profile_unit & unit = vec_unit[i];

		if (unit.dt != 0)
		{
			buf_data_area[length++] = unit.values.size()+1;
			buf_data_area[length++] = 0x09;
			buf_data_area[length++] = 0x0C;
			length += asn_codec::format_date_time(unit.dt, buf_data_area+length); 
		}
		else
			buf_data_area[length++] = unit.values.size();
		
		for (int k=0; k<unit.values.size(); k++)
		{
			buf_data_area[length++] = unit.values[k].tid;
			if (unit.values[k].tid == 4 ||unit.values[k].tid == 9 || unit.values[k].tid == 10 || unit.values[k].tid == 12)
				buf_data_area[length++] = unit.values[k].length;
			memcpy(buf_data_area+length, unit.values[k].buffer, unit.values[k].length);
			length += unit.values[k].length;
		}
	}

	packet->set_data_area_buffer(buf_data_area,length);

	packet->format_packet();
}

void ptcl_module_dlms47::format_packet_get_response_with_list(ptcl_packet_dlms47 *packet,
													unsigned int client_id, 
													std::vector<get_data_result *> vec_result,
													int invoke_id)
{
	//主站ID
	packet->set_host_id(host_id_);
	packet->set_dst_port(client_id);

	//用户数据域
	unsigned char buf_data_area[1024];
	memset(buf_data_area, 0, 1024);
	int length = 0;

	buf_data_area[length++] = 0xC4; // ActionRequest 
	buf_data_area[length++] = 0x03; //ActionRequestNormal
	buf_data_area[length++] = invoke_id; //InvokeIdAndPriority
	buf_data_area[length++] = vec_result.size();

	for (get_data_result *result : vec_result)
	{
		if (result == NULL)
		{
			buf_data_area[length++] = 0x01;
			buf_data_area[length++] = 0x03;
		}
		else 
		{
			buf_data_area[length++] = 0x00;
			buf_data_area[length++] = result->result_type;
			if ( result->result_type == 4 || result->result_type == 9 ||  result->result_type == 10 ||  result->result_type == 12)
					buf_data_area[length++] =  result->length;
			memcpy(buf_data_area+length, result->buffer, result->length);
			length += result->length;
		}
	
	}
	packet->set_data_area_buffer(buf_data_area,length);

	packet->format_packet();
}

void ptcl_module_dlms47::format_packet_get_response_with_list(ptcl_packet_dlms47 *packet,
															  unsigned int client_id, 
															  std::vector<std::vector<profile_unit >> vec_vec_unit,
															  int invoke_id)
{
		//主站ID
	packet->set_host_id(host_id_);
	packet->set_dst_port(client_id);

	//用户数据域
	unsigned char buf_data_area[1024];
	memset(buf_data_area, 0, 1024);
	int length = 0;

	buf_data_area[length++] = 0xC4; // ActionRequest 
	buf_data_area[length++] = 0x03; //ActionRequestNormal
	buf_data_area[length++] = invoke_id; //InvokeIdAndPriority
	
	buf_data_area[length++] = vec_vec_unit.size();

	for (std::vector<profile_unit > vec_unit : vec_vec_unit)
	{
		buf_data_area[length++] = 0x00;
		buf_data_area[length++] = 0x01;
		buf_data_area[length++] = vec_unit.size();
	

		for (int i=0; i<vec_unit.size(); i++)
		{
			if (length > 1000)
				break;

			buf_data_area[length++] = 0x02;
			profile_unit & unit = vec_unit[i];

			if (unit.dt != 0)
			{
				buf_data_area[length++] = unit.values.size()+1;
				buf_data_area[length++] = 0x09;
				buf_data_area[length++] = 0x0C;
				length += asn_codec::format_date_time(unit.dt, buf_data_area+length); 
			}
			else
				buf_data_area[length++] = unit.values.size();
		
			for (int k=0; k<unit.values.size(); k++)
			{
				buf_data_area[length++] = unit.values[k].tid;
				if (unit.values[k].tid == 4 ||unit.values[k].tid == 9 || unit.values[k].tid == 10 || unit.values[k].tid == 12)
					buf_data_area[length++] = unit.values[k].length;
				memcpy(buf_data_area+length, unit.values[k].buffer, unit.values[k].length);
				length += unit.values[k].length;
			}
		}
	}

	

	packet->set_data_area_buffer(buf_data_area,length);

	packet->format_packet();
}

void ptcl_module_dlms47::fill_packet_cosem_set_filter(ptcl_packet_dlms47 *packet, unsigned int start_time, unsigned int end_time,
													unsigned int amount, unsigned int bitfield, char *obis, unsigned char mode)
{
	//static final obis value mass memory filter
	short obis_class = 1;
	//std::string obis_value = "0000411002FF";
	unsigned char attribute = 2;

	if (obis == NULL)
		return;

	unsigned char buf[1024] = {0};
	int length = 0;

	int len = asn_codec::format_struct_data(5, buf+length);
	length += len;

	len = asn_codec::format_unsigned(mode, buf+length);
	length += len;

	len = asn_codec::format_date_time_extra(start_time, buf+length);
	length += len;

	len = asn_codec::format_date_time_extra(end_time, buf+length);
	length += len;

	len = asn_codec::format_unsigned_double_long(amount, buf+length);
	length += len;

	len = asn_codec::format_unsigned_double_long(bitfield, buf+length);
	length += len;

	fill_packet_cosem_set(packet, obis_class,obis, attribute, buf, length);
}


void ptcl_module_dlms47::format_packet_lls_aare_ln(ptcl_packet_dlms47 *packet,unsigned int client_id,
												   bool b_ciphering,unsigned char mech_id,unsigned char* sys_title,int sys_title_len, 
												   unsigned char* init_response,int init_response_len, unsigned char aassociation_result)
{
	if(sys_title_len >= 255 || init_response_len >= 255)
	{
		return;
	}

	//主站ID
	packet->set_dst_port(client_id);
	packet->set_src_port(1);

	//用户数据域
	unsigned char buf_data_area[1024];
	int length = 0;
	int aare_pos = 0;

	asn_codec::PDU aare[5];

	//application-context-name
	unsigned char context_id = b_ciphering ? 3 : 1;
	unsigned char acn[] = {
		0xa1,//Context 1
		0x09,//长度
		0x06,//OBJECT IDENTIFIER 类型标识 06
		0x07,//长度
		0x60,0x85,0x74,0x05,0x08,0x01,context_id};//内容（2 16 756 5 8 1 context_id）
	memcpy(aare[aare_pos].context_area,acn,11);
	aare[aare_pos++].len_context_area = 11;


	//Association-result
	unsigned char ar[8] = {0xa2, 0x03, 0x02, 0x01};
	ar[4] = aassociation_result;
	memcpy(aare[aare_pos].context_area, ar, 5);
	aare[aare_pos++].len_context_area = 5;


	//result-source-diagnostic
	unsigned char rsd[16] = {0xa3, 0x05, 0xa1, 0x03, 0x02, 0x01, 0x00};
	memcpy(aare[aare_pos].context_area, rsd, 7);
	aare[aare_pos++].len_context_area = 7;


	//user-information
	unsigned char tag_component_len = init_response_len + 2;	//length of the tagged component's value field.
	unsigned char ui_len = tag_component_len + 2;	//length of user information.
	unsigned char ui[255] = {
		0xbe,//Context 30
		tag_component_len,//长度
		0x04,//OTCET String 类型标识 04
		init_response_len//长度
	};
	memcpy(ui + 4,init_response,init_response_len); //init_request
	memcpy(aare[aare_pos].context_area,ui,ui_len);
	aare[aare_pos++].len_context_area = ui_len;


	//responding-AP-title  if cipher
	if (b_ciphering) {
		unsigned char rat[64] = {0xa4,0x0a,0x04};

		rat[3] = sys_title_len;
		memcpy(rat+4,sys_title,sys_title_len);
		memcpy(aare[aare_pos].context_area,rat,4+sys_title_len);
		aare[aare_pos++].len_context_area = 4 + sys_title_len;
	}

	

	//AARE
	int len_aarq = asn_codec::format_ber_seqence(asn_codec::asn_tagging_class_application,1,aare,aare_pos,buf_data_area+length);
	length = length + len_aarq;

	packet->set_data_area_buffer(buf_data_area,length);

	packet->format_packet();
}

void ptcl_module_dlms47::format_packet_hls_aare_ln(ptcl_packet_dlms47 *packet,unsigned int client_id,
							   bool b_ciphering,unsigned char mech_id,unsigned char* sys_title,int sys_title_len, 
							   unsigned char* init_response,int init_response_len, unsigned char association_result,
							   char *authen_value)
{
	if(sys_title_len >= 255 || init_response_len >= 255)
	{
		return;
	}

	//主站ID
	packet->set_dst_port(client_id);
	packet->set_src_port(1);

	//用户数据域
	unsigned char buf_data_area[1024];
	int length = 0;
	int aare_pos = 0;

	asn_codec::PDU aare[8];

	//application-context-name
	unsigned char context_id = b_ciphering ? 3 : 1;
	unsigned char acn[] = {
		0xa1,//Context 1
		0x09,//长度
		0x06,//OBJECT IDENTIFIER 类型标识 06
		0x07,//长度
		0x60,0x85,0x74,0x05,0x08,0x01,context_id};//内容（2 16 756 5 8 1 context_id）
	memcpy(aare[aare_pos].context_area,acn,11);
	aare[aare_pos++].len_context_area = 11;


	//Association-result
	unsigned char ar[8] = {0xa2, 0x03, 0x02, 0x01, association_result};
	memcpy(aare[aare_pos].context_area, ar, 5);
	aare[aare_pos++].len_context_area = 5;


	//result-source-diagnostic
	unsigned char rsd[16] = {0xa3, 0x05, 0xa1, 0x03, 0x02, 0x01, 0x00};
	memcpy(aare[aare_pos].context_area, rsd, 7);
	aare[aare_pos++].len_context_area = 7;


	//Mechanism-name
	unsigned char mn[10] = {0x89, 0x07, 0x60, 0x85, 0x74, 0x05, 0x08, 0x02, mech_id};
	memcpy(aare[aare_pos].context_area, mn, 9);
	aare[aare_pos++].len_context_area = 9;

	//Authentication-value
	unsigned char av[1024];
	int len_sub_cav = asn_codec::format_ber_graphic_string(asn_codec::asn_tagging_class_context,0, authen_value, av);
	aare[aare_pos++].len_context_area = asn_codec::format_ber_choice(asn_codec::asn_tagging_class_context,10,av,len_sub_cav,aare[aare_pos].context_area);

	//user-information
	unsigned char tag_component_len = init_response_len + 2;	//length of the tagged component's value field.
	unsigned char ui_len = tag_component_len + 2;	//length of user information.
	unsigned char ui[255] = {
		0xbe,//Context 30
		tag_component_len,//长度
		0x04,//OTCET String 类型标识 04
		init_response_len//长度
	};
	memcpy(ui + 4,init_response,init_response_len); //init_request
	memcpy(aare[aare_pos].context_area,ui,ui_len);
	aare[aare_pos++].len_context_area = ui_len;

	//responding-AP-title  if cipher
	if (b_ciphering) {
		unsigned char rat[64] = {0xa4,0x0a,0x04};

		rat[3] = sys_title_len;
		memcpy(rat+4,sys_title,sys_title_len);
		memcpy(aare[aare_pos].context_area,rat,4+sys_title_len);
		aare[aare_pos++].len_context_area = 4 + sys_title_len;
	}

	//AARE
	int len_aarq = asn_codec::format_ber_seqence(asn_codec::asn_tagging_class_application,1,aare,aare_pos,buf_data_area+length);
	length = length + len_aarq;

	packet->set_data_area_buffer(buf_data_area,length);

	packet->format_packet();
}

void ptcl_module_dlms47::format_packet_action_response(ptcl_packet_dlms47 *packet,int error_code, unsigned int client_id, get_data_result *result, int invoke_id)
{
	//主站ID
	packet->set_host_id(1);
	packet->set_dst_port(client_id);

	//用户数据域
	unsigned char buf_data_area[1024];
	int length = 0;

	buf_data_area[length++] = 0xC7; 
	buf_data_area[length++] = 0x01; 
	buf_data_area[length++] = invoke_id; //InvokeIdAndPriority
	buf_data_area[length++] = error_code;

	if (error_code > 0 || result == NULL)
	{
		buf_data_area[length++] = 0;
	}
	else
	{
		buf_data_area[length++] = 0x01;
		buf_data_area[length++] = 0x00;
		buf_data_area[length++] = result->result_type;
		if ( result->result_type == 4 || result->result_type == 9 ||  result->result_type == 10 ||  result->result_type == 12)
			buf_data_area[length++] =  result->length;
		memcpy(buf_data_area+length, result->buffer, result->length);
		length += result->length;
	}

	packet->set_data_area_buffer(buf_data_area,length);

	packet->format_packet();
}

void ptcl_module_dlms47::format_packet_rlre(ptcl_packet_dlms47 *packet,unsigned int client_id,int use_flags,
													 unsigned char rsp_resson,unsigned char* init_response,int len)
{
	//设置src_wport
	packet->set_dst_port(client_id);

	//用户数据域
	unsigned char buf_data_area[512] = {0};

	//设置相关数据
	int pos = 0;
	buf_data_area[pos++] = 0x63;
	int len_pos = pos++;

	//RLRE's  reason field
	if(use_flags & RLRQ_COMPON_FLAGS::REASON_FIELD)
	{
		unsigned char reason_field[3] = {0x80,0x01,rsp_resson};
		memcpy(buf_data_area+pos,reason_field,3);
		pos = pos + 3; 
	}

	//RLRQ's user_information field
	if(use_flags & RLRQ_COMPON_FLAGS::USER_INFO_RLRE)
	{
		if(len > 255)
		{
			return;
		}

		int tpos = 0;
		unsigned char user_data[256] = {0};
		user_data[tpos++] = 0xBE;		//tag ([30])
		user_data[tpos++] = len + 2;	//length of the user information
		user_data[tpos++] = 0x04,		//choice for user-information (OCTET STRING) 
			user_data[tpos++] = len;		//length of the OCTET STRING
		memcpy(user_data + tpos,init_response,len);
		tpos = tpos + len;

		memcpy(buf_data_area + pos,user_data,tpos);
		pos = pos + tpos; 
	}

	buf_data_area[len_pos] = pos - 2; //减去2得到length of the RLRQ's contents field

	//将数据填进packet
	packet->set_data_area_buffer(buf_data_area,pos); //此处用pos，是整个rlre消息的长度

	packet->format_packet();
}

void ptcl_module_dlms47::format_packet_pass4(ptcl_packet_dlms47 *packet,unsigned int client_id, unsigned char error_code, int invoke_id, std::string str_data)
{
	//主站ID
	packet->set_host_id(1);
	packet->set_dst_port(client_id);

	//用户数据域
	unsigned char buf_data_area[1024];
	int length = 0;

	buf_data_area[length++] = 0xC7; 
	buf_data_area[length++] = 0x01; 
	buf_data_area[length++] = invoke_id; //InvokeIdAndPriority
	buf_data_area[length++] = error_code;

	if (error_code > 0)
	{
		buf_data_area[length++] = 0;
	}
	else
	{
		buf_data_area[length++] = 0x01;
		buf_data_area[length++] = 0x00;
		buf_data_area[length++] = 0x09;
		buf_data_area[length++] = str_data.size()/2;
		length += asn_codec::format_context_area_octet_string((char*)str_data.c_str(), buf_data_area+length);
	}

	packet->set_data_area_buffer(buf_data_area,length);

	packet->format_packet();
}