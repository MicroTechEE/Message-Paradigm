/*
 * Message.c
 *
 *  Uses dedicated memory for data being sent and received. Messages are stored until no longer needed.
 *
 */


#include <msp430.h>
#include <Message.h>
#include "misc.h"
#include <stdlib.h>  //for NULL
#include <stdbool.h>
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~GLOBALS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//Create some unsigned char array buffers in memory
unsigned char rxBuffSmall[MESSAGE_BUFF_COUNT_SMALL][MESSAGE_BUFF_SIZE_SMALL];
unsigned char rxBuffMedium[MESSAGE_BUFF_COUNT_MEDIUM][MESSAGE_BUFF_SIZE_MEDIUM];
unsigned char rxBuffLarge[MESSAGE_BUFF_COUNT_LARGE][MESSAGE_BUFF_SIZE_LARGE];
unsigned char rxBuffXxl[MESSAGE_BUFF_COUNT_XXL][MESSAGE_BUFF_SIZE_XXL];

//Create MessageStructures for each (keep here so we can only pass pointers to them)
Message Messages[TOTAL_MESSAGE_BUFFERS];

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~FUNCTIONS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


void message_init(void)
{
	int i,j;
	int structStart, structEnd;

	structStart = 0;
	structEnd= MESSAGE_BUFF_COUNT_SMALL;


	//setup structures and give each a pointer to an array
	for (i=structStart;i<structEnd; i++)
	{
		Messages[i].buffNum = i;
		Messages[i].bufferType = small;
		Messages[i].inUseFlag = false;
		Messages[i].msgPtr = rxBuffSmall[i];
		Messages[i].source = NONE;

		//fill the buffer with zeros
	    for (j=0;j<small ; j++)  //bufferType should also be equal to the #defined size
	    {
	        *(Messages[i].msgPtr + j) = 0;
	    }
	}

	structStart +=MESSAGE_BUFF_COUNT_SMALL;
	structEnd +=MESSAGE_BUFF_COUNT_MEDIUM;

	for(i= structStart ; i<structEnd ; i++)
	{
		Messages[i].buffNum = i;
		Messages[i].bufferType = medium;
		Messages[i].inUseFlag = false;
		Messages[i].msgPtr = rxBuffMedium[i - structStart];
		Messages[i].source = NONE;

		//fill the buffer with zeros
	    for (j=0;j<medium; j++)  //bufferType should also be equal to the #defined size
	    {
	        *(Messages[i].msgPtr + j) = 0;
	    }
	}

	structStart +=MESSAGE_BUFF_COUNT_MEDIUM;
	structEnd +=MESSAGE_BUFF_COUNT_LARGE;

	for(i= structStart ; i<structEnd ; i++)
	{
		Messages[i].buffNum = i;
		Messages[i].bufferType = large;
		Messages[i].inUseFlag = false;
		Messages[i].msgPtr = rxBuffLarge[i - structStart];
		Messages[i].source = NONE;

		//fill the buffer with zeros
	    for (j=0;j<large; j++)  //bufferType should also be equal to the #defined size
	    {
	        *(Messages[i].msgPtr + j) = 0;
	    }
	}

	structStart += MESSAGE_BUFF_COUNT_LARGE;
	structEnd += MESSAGE_BUFF_COUNT_XXL;

	for(i= structStart ; i<structEnd ; i++)
	{
		Messages[i].buffNum = i;
		Messages[i].bufferType = xxl;
		Messages[i].inUseFlag = false;
		Messages[i].msgPtr = rxBuffXxl[i - structStart];
		Messages[i].source = NONE;

		//fill the buffer with zeros
	    for (j=0;j<xxl; j++)  //bufferType should also be equal to the #defined size
	    {
	        *(Messages[i].msgPtr + j) = 0;
	    }
	}
}

void message_clearData(Message * msg)
{
	int i;

	if(msg !=0)
	{
		for (i=0;i<msg->bufferType; i++) //bufferType should also be equal to the defined size
			*(msg->msgPtr + i) = 0;

		msg->dataLength = 0;
		msg->expectedDataLength = 0;
	}
}

Message * message_requestMsgBuff(int size)
{
	int i, bufferIndex = 0;
	Message * msgPtr = NULL;

	if(size > MESSAGE_BUFF_SIZE_LARGE)
		bufferIndex = MESSAGE_BUFF_COUNT_SMALL + MESSAGE_BUFF_COUNT_MEDIUM + MESSAGE_BUFF_COUNT_LARGE; //skip over structures with large buffers
	else if(size > MESSAGE_BUFF_SIZE_MEDIUM)
		bufferIndex = MESSAGE_BUFF_COUNT_SMALL + MESSAGE_BUFF_COUNT_MEDIUM; //skip over structures with medium buffers
	else if(size > MESSAGE_BUFF_SIZE_SMALL)
		bufferIndex = MESSAGE_BUFF_COUNT_SMALL; //skip over structures with small buffers

	//See if we can get a message struct
	for (i = bufferIndex;i<TOTAL_MESSAGE_BUFFERS; i++)
	{
		if(Messages[i].inUseFlag == false)
		{
			Messages[i].inUseFlag = true;
			msgPtr = &Messages[i];
			break;
		}
	}

	return msgPtr;
}

Message * message_getMsg(enum Sources source)
{
	int i;
	Message * msgPtr = NULL;

	//look throught the messages and see if there are any new ones from this source
	for (i=0;i<TOTAL_MESSAGE_BUFFERS; i++)
	{
		if(Messages[i].inUseFlag == true && Messages[i].source == source)
		{
			msgPtr = &Messages[i];
			break;
		}
	}

	return msgPtr;
}

Message * message_seekMsg(enum Sources source, unsigned char orderNumber)
{
    int i, count = 0;
    Message * msgPtr = NULL;

    for (i=0;i<TOTAL_MESSAGE_BUFFERS; i++)
    {
        if(Messages[i].inUseFlag == true && Messages[i].source == source)
        {
            count++;
            if(count == orderNumber){
                msgPtr = &Messages[i];
                break;  // only break when we have the Message of correct Source and Occurance ( Second, Third, etc)
            }
        }
    }
    return msgPtr;
}

bool message_append(Message * Msg, unsigned char* newData, unsigned char newDataLen)
{
	int i;
	// check and see if there is anything in the buffer to clear
	// returns true when successful, otherwise returns false
	if(Msg != 0)
	{
		if (Msg->dataLength + newDataLen < Msg->bufferType) //NEED TO BE CAREFUL HERE and dont excede buffer size when adding to it
		{
			for(i=0;i<newDataLen;i++)
			{
				*(Msg->msgPtr + Msg->dataLength)  = newData[i];
				Msg->dataLength++;
			}
			return true;
		}
	}
	return false;
}

void message_freeMsg(Message * msg)
{
	//check and see if there is anything in the buffer to clear
	if(msg == 0)
		return;

	//figure out which message struct this is and free it clear it out
	msg->inUseFlag = false;
	msg->dataLength = 0;
	msg->expectedDataLength = 0;
	msg->source = NONE;
}

void message_killMsg(Message ** msgStructPtr)
{
	if(*msgStructPtr !=0)
	{
		message_clearData(*msgStructPtr);
		message_freeMsg(*msgStructPtr);
		*msgStructPtr = 0;
	}
}

