#include <ptcl_codec/ptcl_module_dlms.h>
#include <ptcl_codec/asn_codec.h>
#include <time.h>
#include <stdio.h>

ptcl_module_dlms *ptcl_module_dlms::instance_ = NULL;
ptcl_module_dlms::ptcl_module_dlms(void)
{

}

ptcl_module_dlms::~ptcl_module_dlms(void)
{

}

ptcl_module_dlms *ptcl_module_dlms::get_instance()
{
	if (instance_ == NULL)
	{
		instance_ = new ptcl_module_dlms;
	}
	return instance_;
}

void ptcl_module_dlms::format_packet_hdlc_connect(ptcl_packet_hdlc *packet,
								short int max_send_bytes,short int max_recv_bytes,
								int send_window,int recv_window)
{
	//主站ID
	packet->set_host_id(host_id_);

	//控制域功能码
	packet->set_ctrl_code_area(ctrl_code_snrm);//SNRM

	unsigned char buf_data_area[] = {
		0x81,//格式标识符
		0x80,//组标识符
		0x14,//后续长度
		0x05,0x02,0x04,0x00,//发送最大信息长度1024
		0x06,0x02,0x04,0x00,//接收最大信息长度1024
		0x07,0x04,0x00,0x00,0x00,0x01, //发送窗口大小1
		0x08,0x04,0x00,0x00,0x00,0x07}; //接收窗口大小1

	packet->set_data_area_buffer(buf_data_area, 23);
	packet->format_packet();

}

void ptcl_module_dlms::format_packet_hdlc_disconnect(ptcl_packet_hdlc *packet)
{
	//主站ID
	packet->set_host_id(host_id_);

	//控制域功能码
	packet->set_ctrl_code_area(ctrl_code_disc);//DISC

	packet->format_packet();
}


void ptcl_module_dlms::format_packet_cosem_connect(ptcl_packet_hdlc *packet,char* psw)
{
	//主站ID
	packet->set_host_id(host_id_);

	//控制域功能码
	reset_frame_seq(packet->get_device_addr());
	packet->set_cca_recv_seq(0);
	packet->set_cca_pf(1);
	packet->set_cca_send_seq(0);
	packet->set_cca_end(0);

	//用户数据域
	unsigned char buf_data_area[1024];
	int length = 0;

	asn_codec::PDU aarq[5];
	//LLC
	buf_data_area[length++] = 0xE6;
	buf_data_area[length++] = 0xE6;
	buf_data_area[length++] = 0x00;

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
	int oi[] = {2,16,756,5,8,2,1};
	aarq[2].len_context_area = asn_codec::format_ber_object_indentifier(asn_codec::asn_tagging_class_context,11, oi,7,aarq[2].context_area);

	//calling-authentication-value
	unsigned char sub_cav[1024];
	int len_sub_cav = asn_codec::format_ber_graphic_string(asn_codec::asn_tagging_class_context,0, psw, sub_cav);
	aarq[3].len_context_area = asn_codec::format_ber_choice(asn_codec::asn_tagging_class_context,12,sub_cav,len_sub_cav,aarq[3].context_area);


	//user-information
	unsigned char ui[]  = {
		0xbe,//Context 30
		0x10,//长度
		0x04,//OTCET String 类型标识 04
		0x0e,//长度
		0x01,0x00,0x00,0x00,0x06,
		0x5f,0x1f,0x04,0x00,0x00,
		0x7E,0x1F,0xFF,0xFF};
	memcpy(aarq[4].context_area,ui,18);
	aarq[4].len_context_area = 18;

	//AARQ
	int len_aarq = asn_codec::format_ber_seqence(asn_codec::asn_tagging_class_application,0,aarq,5,buf_data_area+length);
	length = length + len_aarq;

	packet->set_data_area_buffer(buf_data_area,length);

	packet->format_packet();
}

void ptcl_module_dlms::format_packet_cosem_disConnect(ptcl_packet_hdlc *packet)
{

}

void ptcl_module_dlms::format_packet_cosem(ptcl_packet_hdlc *packet, int invokeid, int recv_seq)
{
	//主站ID
	packet->set_host_id(host_id_);

	//控制域功能码
	packet->set_cca_recv_seq(recv_seq);
	packet->set_cca_pf(1);
	packet->set_cca_send_seq(get_frame_seq(packet->get_device_addr()));
	packet->set_cca_end(0);
	unsigned char *cosem_data = packet->get_data_area_buffer()+3;
	cosem_data[2] = (cosem_data[2] & 0x80) + (invokeid % 128);

	packet->format_packet();
}

void ptcl_module_dlms::fill_packet_cosem_get(ptcl_packet_hdlc *packet, short obis_class, char* obis, unsigned char attr_id)
{
	//用户数据域
	unsigned char buf_data_area[1024];
	int length = 0;

	//LLC
	buf_data_area[length++] = 0xE6;
	buf_data_area[length++] = 0xE6;
	buf_data_area[length++] = 0x00;

	//APDU
	buf_data_area[length++] = 0xC0;//A-XDR编码get-request [192] IMPLICIT
	buf_data_area[length++] = 0x01;//[1] IMPLICIT Get-Request-Normal,
	buf_data_area[length++] = 0x00;

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

void ptcl_module_dlms::fill_packet_cosem_get(ptcl_packet_hdlc *packet, char* obis, time_t start, time_t end)
{
	//用户数据域
	unsigned char buf_data_area[1024];
	int length = 0;

	//LLC
	buf_data_area[length++] = 0xE6;
	buf_data_area[length++] = 0xE6;
	buf_data_area[length++] = 0x00;

	//APDU
	buf_data_area[length++] = 0xC0;//A-XDR编码get-request [192] IMPLICIT
	buf_data_area[length++] = 0x01;//[1] IMPLICIT Get-Request-Normal,
	buf_data_area[length++] = 0;

	//class id
	int len = asn_codec::format_context_area_short(ptcl_module_dlms::ci_profile,buf_data_area+length);
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
	len = asn_codec::format_context_area_short(ptcl_module_dlms::ci_clock,buf_data_area+length);
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
	buf_data_area[length++] = 0x19;
	sprintf(time_s,"%04x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",t->tm_year+1900,t->tm_mon+1,t->tm_mday,t->tm_wday,t->tm_hour,t->tm_min,t->tm_sec,0,0x80,0,0);
	len = asn_codec::format_context_area_obis(time_s,buf_data_area+length);
	length = length + len;

	// instance_specific  date_time
	//char time_e[16] = {'\0'};
	t = localtime(&end);
	buf_data_area[length++] = 0x19;
	sprintf(time_s,"%04x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",t->tm_year+1900,t->tm_mon+1,t->tm_mday,t->tm_wday,t->tm_hour,t->tm_min,t->tm_sec,0,0x80,0,0);
	len = asn_codec::format_context_area_obis(time_s,buf_data_area+length);
	length = length + len;

	// array capture_object_definition
	buf_data_area[length++] = 0x01;
	buf_data_area[length++] = 0x00;

	packet->set_data_area_buffer(buf_data_area,length);

	packet->format_packet();

}

void ptcl_module_dlms::fill_packet_cosem_next(ptcl_packet_hdlc *packet, unsigned int block_no)
{
	//用户数据域
	unsigned char buf_data_area[1024];
	int length = 0;

	//LLC
	buf_data_area[length++] = 0xE6;
	buf_data_area[length++] = 0xE6;
	buf_data_area[length++] = 0x00;

	//APDU
	buf_data_area[length++] = 0xC0;//A-XDR编码get-request [192] IMPLICIT
	buf_data_area[length++] = 0x02;//[1] IMPLICIT Get-Request-Normal,
	buf_data_area[length++] = 0x00;

	//class id
	int len = asn_codec::format_context_area_int(block_no,buf_data_area+length);
	length = length + len;

	packet->set_data_area_buffer(buf_data_area,length);

	packet->format_packet();
}

void ptcl_module_dlms::format_packet_cosem_set(ptcl_packet_hdlc *packet,int last_recv_seq,short obis_class,char* obis)
{
}

/*
int ptcl_module_dlms::format_packet_SNRM(unsigned char *xml,unsigned int device_addr,unsigned char *psw)
{
	boost::property_tree::ptree pt;

	pt.put("COSEM_OPEN_REQ","");
	boost::property_tree::ptree result_node;
	boost::property_tree::ptree & root_node = pt.get_child("COSEM_OPEN_REQ");
	result_node.add("<xmlattr>.Value","80");
	root_node.add_child("ACSE_Protocol_Version",result_node);
	result_node.put("<xmlattr>.Value","1");
	root_node.add_child("Application_Context_Name",result_node);
	result_node.put("<xmlattr>.Value","80");
	root_node.add_child("Sender_Acse_Requirement",result_node);
	result_node.put("<xmlattr>.Value","1");
	root_node.add_child("Security_Mechanism_Name",result_node);
	result_node.put("<xmlattr>.Value",psw);
	root_node.add_child("Calling_Authentication_Value",result_node);
	result_node.put("<xmlattr>.Value","6");;
	root_node.add_child("DLMS_Version_Number",result_node);
	result_node.put("<xmlattr>.Value","007E1F");;
	root_node.add_child("DLMS_Conformance",result_node);
	result_node.put("<xmlattr>.Value","65535");;
	root_node.add_child("Client_Max_Receive_PDU_Size",result_node);
	result_node.put("<xmlattr>.Value","1");;
	root_node.add_child("Service_Class",result_node);

	 boost::property_tree::xml_writer_settings<char> settings('\t', 1,"gb2312");

	std::ostringstream os;
	write_xml(os,pt,settings);
	memcpy(xml,os.str().c_str(),strlen(os.str().c_str()));
	return 0;
}

//disc命令,断开连接
int ptcl_module_dlms::format_packet_DISC(unsigned char *xml,unsigned int device_addr)
{
	boost::property_tree::ptree pt;
	pt.put("COSEM_RELEASE_REQ","");
	std::ostringstream os;
	write_xml(os,pt);
	memcpy(xml,os.str().c_str(),strlen(os.str().c_str()));
	return 0;
}

//ua命令,表示回复
int ptcl_module_dlms::format_packet_UA(unsigned char *xml,unsigned int device_addr)
{
	return 0;
}

//dm命令,表示收到disc命令前已经断开连接
int ptcl_module_dlms::format_packet_DM(unsigned char *xml,unsigned int device_addr)
{
	return 0;
}

//应用层建立连接
int ptcl_module_dlms::format_packet_AARQ(unsigned char *xml,unsigned int device_addr)
{
	return 0;
}

//应用层连接建立回复
int ptcl_module_dlms::format_packet_AARE(unsigned char *xml,unsigned int device_addr, unsigned int host_id)
{
	return 0;
}

//采集历史数据
int ptcl_module_dlms::format_packet_history_data(unsigned char *xml,unsigned char *obis,unsigned int device_addr,
							   time_t start_time,time_t end_time)
{
	boost::property_tree::ptree pt_obis;
	boost::property_tree::ptree obis_result;
	pt_obis.put("GET_REQ","");
	boost::property_tree::ptree & obis_node = pt_obis.get_child("GET_REQ");

	obis_result.add("<xmlattr>.Value", "0");
	obis_node.add_child("Invoke_Id",obis_result);
	obis_result.put("<xmlattr>.Value", "1");
	obis_node.add_child("Service_Class",obis_result);
	obis_node.add_child("Request_Type",obis_result);
	obis_result.put("<xmlattr>.Value", "0");
	obis_node.add_child("Priority",obis_result);
	obis_node.put("COSEM_Attribute_Descriptor","");
	obis_result.put("<xmlattr>.Value", "7");
	obis_node.add_child("COSEM_Attribute_Descriptor.COSEM_Class_Id",obis_result);
	obis_result.put("<xmlattr>.Value", obis);
	obis_node.add_child("COSEM_Attribute_Descriptor.COSEM_Instance_Id",obis_result);
	obis_result.put("<xmlattr>.Value", "2");
	obis_node.add_child("COSEM_Attribute_Descriptor.COSEM_Attribute_Id",obis_result);

	obis_node.put("COSEM_Attribute_Descriptor.Selective_Access_Parameters","");
	obis_result.put("<xmlattr>.Value", "1");
	obis_node.add_child("COSEM_Attribute_Descriptor.Selective_Access_Parameters.access_selector",obis_result);

	obis_node.put("COSEM_Attribute_Descriptor.Selective_Access_Parameters.access_parameters","");

	obis_result.clear();
	obis_result.add("<xmlattr>.Type","structure");
	obis_node.add_child("COSEM_Attribute_Descriptor.Selective_Access_Parameters.access_parameters.range_descriptor",obis_result);

	obis_node.add_child("COSEM_Attribute_Descriptor.Selective_Access_Parameters.access_parameters.range_descriptor.restricting_object",obis_result);
	obis_result.put("<xmlattr>.Type", "long-unsigned");
	obis_result.add("<xmlattr>.Value","1");
	obis_node.add_child("COSEM_Attribute_Descriptor.Selective_Access_Parameters.access_parameters.range_descriptor.restricting_object.class_id",obis_result);
	obis_result.put("<xmlattr>.Type", "octet-string");
	obis_result.put("<xmlattr>.Value","0100980101ff");
	obis_node.add_child("COSEM_Attribute_Descriptor.Selective_Access_Parameters.access_parameters.range_descriptor.restricting_object.logical_name",obis_result);
	obis_result.put("<xmlattr>.Type", "integer");
	obis_result.put("<xmlattr>.Value","2");
	obis_node.add_child("COSEM_Attribute_Descriptor.Selective_Access_Parameters.access_parameters.range_descriptor.restricting_object.attribute_index",obis_result);
	obis_result.put("<xmlattr>.Type", "long-unsigned");
	obis_result.put("<xmlattr>.Value","0");
	obis_node.add_child("COSEM_Attribute_Descriptor.Selective_Access_Parameters.access_parameters.range_descriptor.restricting_object.data_index",obis_result);

	tm *t = localtime(&start_time);
	char start[16] = {'\0'};
	char end[16] = {'\0'};
	sprintf(start,"%04x%02x%02x%02x%02x%02x%02x%02x%04x%02x",t->tm_year+1900,t->tm_mon,t->tm_wday,t->tm_mday,t->tm_hour,t->tm_min,t->tm_sec,0,0x80,0);
	obis_result.put("<xmlattr>.Type", "date_time");
	obis_result.put("<xmlattr>.Value","07ec0a1506002a09008000");
	obis_node.add_child("COSEM_Attribute_Descriptor.Selective_Access_Parameters.access_parameters.range_descriptor.from_value",obis_result);
	obis_result.put("<xmlattr>.Value","07ec0a1506002b09008000");
	obis_node.add_child("COSEM_Attribute_Descriptor.Selective_Access_Parameters.access_parameters.range_descriptor.to_value",obis_result);
	obis_result.clear();
	obis_result.add("<xmlattr>.Type", "array");
	obis_node.add_child("COSEM_Attribute_Descriptor.Selective_Access_Parameters.access_parameters.range_descriptor.selected_values",obis_result);

	std::ostringstream os;
	write_xml(os,pt_obis);
	std::string s_xml_obis = os.str();
	memcpy(xml,s_xml_obis.c_str(),strlen(s_xml_obis.c_str()));
	return 0;
}
*/
