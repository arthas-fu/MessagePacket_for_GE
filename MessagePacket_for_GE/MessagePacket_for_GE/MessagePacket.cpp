// MessagePacket.cpp 
//

#include <stdio.h>
#include <iostream>
#include "MessagePacket.h"

using namespace std;

MessagePacket::MessagePacket()
{
	this->address =  0;
	this->command = 0;
	this->byte_count = 0;
	this->data = NULL;
	this->data_char = 0;
	this->data_float = 0.0;
	this->check_byte = 0;
	this->xml_doc = NULL;
}

MessagePacket::~MessagePacket()
{

}

int MessagePacket::parse_xml_command(const char* xml_data, const unsigned int xml_data_len)
{
	int ret = SUCCESS;	
	xmlNodePtr root_node = NULL, node = NULL;
	xmlAttrPtr node_attr = NULL;
	unsigned char* data_in_xml = NULL;

	if(NULL == xml_data || 0 == xml_data_len)
	{
		ret = FAILED;
		cout << "wrong param" << endl;
		goto END;
	}

	if (NULL != this->xml_doc)
	{
		xmlFreeDoc(xml_doc);
		xmlCleanupParser();
		xmlCleanupMemory();
	}

	this->xml_doc = xmlParseMemory(xml_data, xml_data_len);
	if(NULL == this->xml_doc)
	{
		ret = FAILED;
		cout << "xml format is wrong" << endl;
		goto END;
	}

	root_node = this->xml_doc->children;
	//check whether root node is "Command"
	if(NULL == root_node || strncmp((const char*)root_node->name, "Command", strlen("Command")) != 0)
	{
		ret = FAILED;
		cout << "command format is wrong at root node" << endl;
		goto END;
	}

	if (root_node != this->xml_doc->last)
	{
		cout << "more than one command node, only first one shall be packeted" << endl;
	}

	node_attr = root_node->properties;
	//check whether first property is "Name"
	if(NULL == node_attr || strncmp((const char*)node_attr->name, "Name", strlen("Name")) != 0)
	{
		ret = FAILED;
		cout << "command format is wrong at first property of root node" << endl;
		goto END;
	}

	node = node_attr->children;
	if (NULL == node)
	{
		ret = FAILED;
		cout << "command format is wrong at content of first property of root node" << endl;
		goto END;
	}

	//Command Name		command value
	//Write Number		1
	//Read Number		2
	//others			0
	if(strncmp((const char*)node->content, "Write Number", strlen("Write Number")) == 0)
	{
		this->command = 1;
	}	
	else if(strncmp((const char*)node->content, "Read Number", strlen("Read Number")) == 0)
	{
		this->command = 2;
	}
	else
	{
		this->command = 0;
	}

	node_attr = node_attr->next;
	//check whether second property is "Number"
	if(NULL == node_attr || strncmp((const char*)node_attr->name, "Number", strlen("Number")) != 0)
	{
		ret = FAILED;
		cout << "command format is wrong at second property of root node" << endl;
		goto END;
	}

	node = node_attr->children;
	if (NULL == node)
	{
		ret = FAILED;
		cout << "command format is wrong at content of second property of root node" << endl;
		goto END;
	}

	data_in_xml = node->content;

	node = root_node->children;
	if (NULL == node)
	{
		ret = FAILED;
		cout << "command format is wrong at sub node of root node" << endl;
		goto END;
	}

	//skip node "text"
	node = node->next;
	if (NULL == node || strncmp((const char*)node->name, "Param", strlen("Param")) != 0)
	{
		ret = FAILED;
		cout << "command format is wrong at node \"Param\"" << endl;
		goto END;
	}

	node = node->children;	
	if (NULL == node)
	{
		ret = FAILED;
		cout << "command format is wrong at sub node of node \"Param\"" << endl;
		goto END;
	}

	if (strncmp((const char*)node->content, "Tag", strlen("Tag")) == 0)
	{
		this->byte_count = 1;
		this->data_char = atoi((const char*)data_in_xml);
		this->data = &this->data_char;
	}
	else if (strncmp((const char*)node->content, "AssyNumber", strlen("AssyNumber")) == 0)
	{
		this->byte_count = 4;
		this->data_float = (float)atof((const char*)data_in_xml);
		//TODO this part need to be modified in case of endian difference, we think the message endian is the same with current platform here.
		this->data = (unsigned char*)&this->data_float;
	}
	else
	{
		ret = FAILED;
		cout << "unrecognized param type " << node->content << endl;
		goto END;
	}


END:
	return ret;
}

int MessagePacket::packet_message(unsigned char *buf, unsigned int *buf_len)
{
	int ret = SUCCESS;	
	unsigned char *p = buf;
	unsigned int i = 0;

	if(NULL == buf || NULL == buf_len || 0 == *buf_len)
	{
		ret = FAILED;
		cout << "wrong param" << endl;
		goto END;
	}

	//calculate how many bytes needed
	this->bytes_needed = sizeof(this->address) + sizeof(this->command) + sizeof(this->byte_count) + this->byte_count + sizeof(this->check_byte);

	//check whether buffer is enough
	if(*buf_len < this->bytes_needed)
	{
		ret = FAILED;
		cout << "buffer length is not enough" << endl;
		goto END;
	}

	//address 1 byte
	*p++ = this->address;

	//command 1 byte
	*p++ = this->command;

	//byte count 1 byte
	*p++ = this->byte_count;

	//check whether data is null
	if(NULL == this->data)
	{
		ret = FAILED;
		cout << "internal error: data is null" << endl;
		goto END;
	}
	//data <byte_count> bytes
	memcpy(p, this->data, this->byte_count);
	p += this->byte_count;

	//exclusive OR each byte
	for (i = 0; i < bytes_needed - 1; ++i)
	{
		this->check_byte ^= buf[i];
	}

	//check type 1 byte
	*p = this->check_byte;

	*buf_len = bytes_needed;

END:
	return ret;
}

int MessagePacket::convert_command_to_message(const char* xml_data, const unsigned int xml_data_len,const unsigned char target_address, unsigned char *buf, unsigned int *buf_len)
{
	int ret = SUCCESS;

	if(NULL == xml_data || 0 == xml_data_len || NULL == buf || NULL == buf_len || 0 == *buf_len)
	{
		ret = FAILED;
		cout << "wrong param" << endl;
		goto END;
	}

	//TODO check address is valid
	this->address = target_address;

	//parse xml command to what we need
	ret = this->parse_xml_command(xml_data, xml_data_len);
	if(SUCCESS != ret)
	{
		cout << "parse xml failed" << endl;
		goto END;
	}

	//packet all the elements to message
	ret = this->packet_message(buf, buf_len);
	if(FAILED == ret)
	{
		cout << "packet message failed" << endl;
		goto END;
	}

END:
	return ret;
}
