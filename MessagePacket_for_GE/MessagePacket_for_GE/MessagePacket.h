#ifndef MESSAGE_PACKET_H
#define MESSAGE_PACKET_H

extern "C"
{
#include "libxml/tree.h"
#include "libxml/parser.h"
}

#define SUCCESS		(0)
#define FAILED		(-1)

class MessagePacket{
private:
	unsigned char address;
	unsigned char command;
	unsigned char byte_count;
	unsigned char* data;
	unsigned char data_char;
	float data_float;
	unsigned char check_byte;
	unsigned int bytes_needed;
	xmlDocPtr xml_doc;
	//parse xml data to internal information
	int parse_xml_command(const char* xml_data, const unsigned int xml_data_len);
	//packet internal information to message buffer
	int packet_message(unsigned char *buf, unsigned int *buf_len);
public:
	MessagePacket();
	~MessagePacket();
	//convert command data to message data
	int convert_command_to_message(const char* xml_data, const unsigned int xml_data_len,const unsigned char target_address, unsigned char *buf, unsigned int *buf_len);
};
#endif