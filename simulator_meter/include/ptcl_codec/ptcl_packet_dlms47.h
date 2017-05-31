#pragma once

#include <ptcl_codec/ptcl_packet.h>

class PTCL_CODEC_EXPORT ptcl_packet_dlms47 : public ptcl_packet
{
public:
	ptcl_packet_dlms47(int packet_type = 1);
	ptcl_packet_dlms47(ptcl_packet_dlms47& packet);
	virtual ~ptcl_packet_dlms47(void);
	//unsigned int pack_to_gateway_data(std::string phy_add, unsigned char* output_data);
	unsigned int pack_to_gateway_data(unsigned char* phy_add,unsigned char len, unsigned char* output_data);

	virtual void reset();

	//��װ���ģ�������set_packet_buffer
	virtual void format_packet();

	//�������� ������set_packet_buffer
	//����ֵ >0:�������ĳɹ� 0:���ĳ��Ȳ��� <0:���ĸ�ʽ����
	virtual int parse_packet();
	
	//����ֵ >0:�������ĳɹ� 0:���ĳ��Ȳ��� <0:���ĸ�ʽ����
	virtual int parse_header();

	//����ֵ >0:�������ĳɹ� 0:���ĳ��Ȳ��� <0:���ĸ�ʽ����
	virtual int parse_header(unsigned char *buffer,int len_buffer);

	//�����ն��߼���ַ ���ձ���˳��
	virtual unsigned int get_device_addr();

	//���ر������к�
	virtual int get_frame_seq();

	//������վID
	virtual unsigned char get_host_id();

	//�߼���ַ
	void set_device_addr(unsigned short dev_addr);

	void set_src_port(unsigned short port);
	void set_dst_port(unsigned short port);

	unsigned int get_src_port();

	//��վ��ַ
	void set_host_id(unsigned char id);

	//֡���
	void set_frame_seq(unsigned char seq);

	int is_send_packet(){return is_send_packet_;}

	//����ͷ�ṹ��
#pragma pack(push, 1)
	struct frame_header_struct
	{
		unsigned short version;
		unsigned short src_port;
		unsigned short dest_port;
		unsigned short length;
	};
#pragma pack(pop)

protected:
	frame_header_struct *frame_header_;
	int is_send_packet_;
	unsigned short reverse_value(unsigned short value);
};