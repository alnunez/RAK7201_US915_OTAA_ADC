/*
 * command.c
 *
 *  Created on: 2019��7��22��
 *      Author: RAK
 */

#include "command.h"
#include "string.h"
#include "stdio.h"
#include "stddef.h"
#include "lora_config.h"
#include "stm32l0xx.h"


static void commmon_read_config(int argc, char *argv[]);
static void commmon_set_config(int argc, char *argv[]);
static int parse_args(char* str, char* argv[]);
static void reset(int argc, char *argv[]);
void boot(int argc, char *argv[]);
struct cli_cmds {
    /** The name of the CLI command */
    const char *name;
    /** The help text associated with the command */
    //const char *help;
    /** The function that should be invoked for this command. */
    void (*function) (int argc, char *argv[]);
};

struct cli_cmds First_cmds[] =
{
		{"join",				lora_join},
		{"get_config",		commmon_read_config},
		{"set_config",		commmon_set_config},
		{"reset",				reset	},
		{"boot",				boot	}

//		__set_FAULTMASK(1);
//		HAL_NVIC_SystemReset();


};

void CMD_Process( unsigned char* rxChar)
{
	int i;
	int argc;
	char* argv[MAX_ARGV]={0};

	    if ((strncmp((char*)rxChar, "at+", 3) != 0) || rxChar[3] == '\0')   //����atͷ
	    {
	    	//printf("AT Command ERROR\r\n");
	    	_write(0,"AT Command ERROR\r\n",18);
	        return;
	    }
	    //printf("[Echo cmd:] %s\r\n", rxChar);
	    rxChar += 3;
	    argc = parse_args((char*)rxChar, argv);
	    //printf("(1) argc:%d	argv[0]:%s	argv[1]:%s\r\n",argc,argv[0],argv[1]);


	    if (argc > 0)
	    {
	        for (i = 0; i < sizeof(First_cmds)/sizeof(struct cli_cmds); i++)
	        {
	            if (strcmp(argv[0], First_cmds[i].name) == 0)
	            {
	            	//printf("(1) argc:%d			argv[0]:%s	argv[1]:%s\r\n",argc,argv[0],argv[1]);   //argc����һ���������ֵ��ֻ��Ϊ�˺�����ʽͳһ
	            	First_cmds[i].function(argc, argv);

	                break;
	            }

	        }
	        if (i == sizeof(First_cmds)/sizeof(struct cli_cmds)) {
	        	 //printf("AT Command ERROR\r\n");
	        	 _write(0,"AT Command ERROR\r\n",18);
	        }
	    }
	    else
	    {
	       //printf("AT Command ERROR\r\n");
	       _write(0,"AT Command ERROR\r\n",18);
	    }

}


static int parse_args(char* str, char* argv[])
{
	        int i = 0;
	        char* ch = str;

	        while(*ch != '\0')
	        {
	            i++;
	            /*Check if length exceeds*/
	            if (i > MAX_ARGV)
	            {
	            	//printf("Error:AT Out ofMAX_ARGV\r\n");
	            	_write(0,"Error:AT Out ofMAX_ARGV\r\n",25);
	                return 0;
	            }

	            argv[i-1] = ch;    //ȡ����


	            while(*ch != ',' && *ch != '\0' && *ch != '\r')
	            {
	                if (*ch == ':')
	                {
	                    return i;
	                }

	                if(*ch == '=' && i == 1)
	                {
	                    break;
	                }
	                else
	                    ch++;
	            }
	            if (*ch == '\r')
	                break;
	            if (*ch != '\0')   //Ϊ�ַ�����ĩβ0�ַ�
	            {
	                *ch = '\0';
	                ch++;
//	                while(*ch == ',')
//	                {
//	                    ch++;
//	                }
	            }
	        }
	        return i;
}




static void commmon_read_config(int argc, char *argv[])
{
    int ret;
    if (argc != 2)
    {
        //printf("Parameter format error.\r\n");
        _write(0,"Parameter format error.\r\n",25);
        return;
    }

    ret = read_config(argc,argv);    //�����ַ�������
    if (ret < 0)
    {
        return;
    }
}

static void commmon_set_config(int argc, char *argv[])
{
    int ret;
    if (argc != 2)
    {
        //printf("Parameter format error.\r\n");
        _write(0,"Parameter format error.\r\n",25);
        return;
    }

    ret = set_config(argc,argv);    //�����ַ�������
    if (ret < 0)
    {
        return;
    }
}


char  AsciiToHex(unsigned char * pAscii, unsigned char * pHex, int nLen)
{
	int nHexLen = nLen / 2;
	unsigned char Nibble[2] = {0};
	int i = 0;
	int j = 0;

	if (nLen%2)
	{
		return 1;
	}

	for (i = 0; i < nHexLen; i ++)
	{
		Nibble[0] = *pAscii ++;
		Nibble[1] = *pAscii ++;
		for (j = 0; j < 2; j ++)
		{
			if (Nibble[j] <= 'F' && Nibble[j] >= 'A')
				Nibble[j] = Nibble[j] - 'A' + 10;
			else if (Nibble[j] <= 'f' && Nibble[j] >= 'a')
				Nibble[j] = Nibble[j] - 'a' + 10;
			else if (Nibble[j] >= '0' && Nibble[j] <= '9')
				Nibble [j] = Nibble[j] - '0';
			else
				return 1;//Nibble[j] = Nibble[j] - 'a' + 10;

		}	// for (int j = ...)
		pHex[i] = Nibble[0] << 4;	// Set the high nibble
		pHex[i] |= Nibble[1];	//Set the low nibble
	}	// for (int i = ...)
	return 0;
}

char  i16toa(unsigned char * pAscii, int number)
{
	// crude specific function to convert integer (les than 4 chars) to string
	int printzero=0;
	int tmpint;
	int tmpint2;

	tmpint = number;

	if(tmpint < 0 )
	{
		*pAscii = 0x2D;
		pAscii++;
		tmpint = tmpint * -1;
	}

	tmpint2=tmpint/100;
	if(tmpint2 > 0)
		printzero=1;

	if(printzero)
	{
		*pAscii=0x30+tmpint2;
		pAscii++;
	}

	tmpint=tmpint-(tmpint2*100);
	tmpint2=tmpint/10;
	if(tmpint2>0)
		 	 printzero=1;

	if(printzero)
	{
		*pAscii=0x30+tmpint2;
		pAscii++;
	}
	tmpint=tmpint-(tmpint2*10);

	*pAscii=0x30+tmpint;
	pAscii++;

	pAscii="0x00";

	return 0;
}

int strnlen2(unsigned char * pAscii, int number)
{
	// strlen() nor strnlen() doesn't work as expected so strnlen2 implemented!!
	int ctr=0;
	while( ctr < number)
	{
		if(* pAscii == 0x00)
			return (ctr);
		ctr ++;
		pAscii++;
	}
	return (ctr);


}

static void reset(int argc, char *argv[])
{
//	uint32_t  Boot_addr = 0x08080700;
//	uint32_t  *buffer;
//	*buffer = *(__IO uint32_t  *)Boot_addr;
//	printf("%08X\r\n",buffer[0]);
	HAL_NVIC_SystemReset();
}

void boot(int argc, char *argv[])
{
	 uint32_t  Boot_addr = 0x08080700;
	 //uint32_t  *buffer;
	 HAL_FLASH_Unlock();
	 HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, Boot_addr , 0x00008888);
	 HAL_FLASH_Lock();
//	 *buffer = *(__IO uint32_t  *)Boot_addr;
//	 printf("%08X\r\n",buffer[0]);
	 //printf("Enter bootloader ......\r\n");
	 _write(0,"Enter bootloader ......\r\n",25);
	 HAL_NVIC_SystemReset();

}






