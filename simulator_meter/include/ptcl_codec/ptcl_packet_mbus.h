#include <ptcl_codec/ptcl_packet.h>


class PTCL_CODEC_EXPORT ptcl_packet_mbus : public ptcl_packet
{
	public:
	ptcl_packet_mbus(int packet_type = 1);
	virtual ~ptcl_packet_mbus(void);

	virtual void reset();

	//组装报文，必须先set_packet_buffer
	virtual void format_packet();

	//解析报文 必须先set_packet_buffer
	//返回值 >0:解析报文成功 0:报文长度不够 <0:报文格式不对
	virtual int parse_packet();
	
	//返回值 >0:解析报文成功 0:报文长度不够 <0:报文格式不对
	virtual int parse_header();

	//返回值 >0:解析报文成功 0:报文长度不够 <0:报文格式不对
	virtual int parse_header(unsigned char *buffer,int len_buffer);

	//返回终端逻辑地址 按照报文顺序
	virtual unsigned int get_device_addr();

	//返回报文序列号
	virtual int get_frame_seq();

	//返回主站ID
	virtual unsigned char get_host_id();

	unsigned int get_serial_number();

	unsigned short int get_macfacture();

	unsigned char get_version();

	unsigned char get_device_type();

	unsigned char get_ci();

	unsigned char get_access_no();

	unsigned char get_status();

	unsigned short int get_signature();

	unsigned int get_ci_s_id();

	unsigned char get_ci_version();

	unsigned char get_ci_device_type();

	void set_status(unsigned char status);


protected:
	//报文头结构体
#pragma pack(push, 1)
	struct frame_header_struct_7a
	{
		unsigned char len;

		unsigned char c;		//0x44

		unsigned short int man_id;  //manufacture id

		unsigned int s_number;

		unsigned char version;
		unsigned char device_type;

		unsigned char ci;

		unsigned char access_no;
		unsigned char status;
		unsigned short int signature;
	};

	struct frame_header_struct_72
	{
		unsigned char len;

		unsigned char c;		//0x44

		unsigned short int man_id;  //manufacture id

		unsigned int s_number;

		unsigned char version;
		unsigned char device_type;

		unsigned char ci;

		unsigned int ci_s_id;
		unsigned short int ci_man_id;
		unsigned char ci_version;
		unsigned char ci_device_type;
		unsigned char access_no;
		unsigned char status;
		unsigned short int signature;
	};

#pragma pack(pop)

	struct frame_header_struct_7a *frame_header_7a_;

	struct frame_header_struct_72 *frame_header_72_;

	int packet_type_;

};