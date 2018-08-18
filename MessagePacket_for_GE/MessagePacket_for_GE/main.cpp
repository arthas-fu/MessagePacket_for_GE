// main.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "stdio.h"
#include "MessagePacket.h"
#include <iostream>  
#include <sstream>  
using namespace  std; 

unsigned char message_buf[20] = {0};
unsigned int message_buf_len = sizeof(message_buf);

int _tmain(int argc, _TCHAR* argv[])
{
	char *command_buf = NULL;
	unsigned int command_buf_len = 0, i = 0;
	int ret = 0;
	FILE *fp = NULL;
	string s;

	MessagePacket *mp = new MessagePacket();

	cout << "Hints: " << endl;
	cout << "1. Write Number command convert to \"1\", Read Number command convert to \"2\", others convert to \"0\"." << endl;
	cout << "2. Process first Command and first Param only, others are abandoned." << endl;
	cout << "3. Endian issue is ignored. Considered it is the same as the running platform." << endl;
	cout << "4. Type in \"command.txt\" for a demo" << endl << endl;

	cout << "Input command file: ";

	while(getline(cin,s))
	{

		fp = fopen(s.c_str(), "r");
		if(NULL == fp)
		{
			cout << "Can not open command file: " << s << endl;
			goto NEXT_LOOP;
		}

		fseek(fp, 0, SEEK_END);

		command_buf_len = ftell(fp);
		if(0 == command_buf_len)
		{
			cout << "No data in file: " << s << endl;
			goto NEXT_LOOP;
		}

		command_buf = (char*)malloc(command_buf_len + 1);
		if(NULL == command_buf)
		{
			cout << "Memory not enough for " << s << "bytes" << endl;
			goto NEXT_LOOP;
		}
		memset(command_buf, 0, command_buf_len + 1);

		fseek(fp, 0, SEEK_SET);

		fread(command_buf, command_buf_len, 1, fp);

		ret = mp->convert_command_to_message(command_buf, command_buf_len, 0x00, message_buf, &message_buf_len);
		if(SUCCESS != ret)
		{
			cout << "message packet failed" << endl;
			goto NEXT_LOOP;
		}


		cout << "packeted massage data is:" << endl;
		for (i = 0; i < message_buf_len; ++i)
		{
			cout << hex << "0x" << (unsigned int)message_buf[i] << " ";
		}
		cout << endl;


NEXT_LOOP:
		if (NULL != fp)
		{
			fclose(fp);
			fp = NULL;
		}

		if (NULL != command_buf)
		{
			free(command_buf);
			command_buf = NULL;
		}

		memset(message_buf, 0, sizeof(message_buf));

		command_buf_len = 0;
		message_buf_len = sizeof(message_buf);

		cout << endl << "Input command file: ";
	}


	return 0;
}