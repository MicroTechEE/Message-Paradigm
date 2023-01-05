/*
 * Message.h
 *
 *  Uses dedicated memory for data being sent and received. Messages are stored until no longer needed.
 *
 */

#ifndef MESSAGE_H_
#define MESSAGE_H_

#include "stdbool.h"
#include "misc.h"

/*  NOTES
 *
 * These buffers will occupy RAM. Make sure you have enough available before creating these.
 * RAM usage can be observed by viewing your .map file when compiling/linking your project.
 * If you run out of RAM, sometimes you can modify your linker file. This is only true if you split your RAM
 * for things like LEA (MSP430), or some other purpose.
 * Modifying RAM usage is very ADVANCED and can Break Everything. Use fewer Messages if possible.
 *
 *
 * To Use Messages:  REFER TO THE README FOR MORE DETAILS !!
 * -> Create a global Message* pointer variable at the top of your .c file, this will point to an existing Message Buffer in memory --> Message* myMsgVar;
 *
 * RX:
 * -> In your Serial RX Interrupt SubRoutine / Callback, REQUEST a buffer of a particular size--> myMsgVar = message_requestMsgBuff(small);
 * -> Handle cases where requestMessage fails --> if(!myMsgVar) then exit ISR somehow
 * -> Stuff your incoming data into your buffer's msgPtr -->     myMsgVar->msgPtr + myMsgVar->dataLen++ = incomingByte; ( refer to README for why this works best )
 * -> When your ISR is finished, you will have a message buffer thats InUse, has data in msgPtr, and a known length of dataLen, of Source ...
 * -> Somewhere else in your code, you can request the buffer of SOURCE (...) and handle it from there
 *
 * TX:
 * -> In your TX function (preferrably before your ISR) request a Message of size (...) --> myMsgVar = message_requestMsgBuff(small);
 * -> Format whatever you are sending as an array -->     myMsgVar->msgPtr = myOutgoingArray;
 * -> Put the length into expectedDataLen  -->  myMsgVar->expectedDataLen = ( however you chose to measure your data: sizeof(), strlen(), indexNumber, etc )
 * -> Stuff the first byte of the msgPtr into your TX_BUFF_REGISTER and Enable Interrupts ( TX will interrupt will fire immediately after )
 * -> Your TX ISR will finish when myMsgVar->dataLen == myMsgVar->expectedDataLen    ( SEE README )
 *
 *
 * */

#define MESSAGE_BUFF_SIZE_SMALL		    50
#define MESSAGE_BUFF_SIZE_MEDIUM		200
#define MESSAGE_BUFF_SIZE_LARGE		    400	   // you can change the size of each buffer type here
#define MESSAGE_BUFF_SIZE_XXL		    1000

#define MESSAGE_BUFF_COUNT_SMALL		10
#define MESSAGE_BUFF_COUNT_MEDIUM 	    4	   // if you don't need larger buffers, zero them out here to save on RAM
#define MESSAGE_BUFF_COUNT_LARGE		2
#define MESSAGE_BUFF_COUNT_XXL   		0

#define TOTAL_MESSAGE_BUFFERS	MESSAGE_BUFF_COUNT_SMALL + MESSAGE_BUFF_COUNT_MEDIUM + MESSAGE_BUFF_COUNT_LARGE+ MESSAGE_BUFF_COUNT_XXL



/*~~~~~~~~~~~~~~~~~~~~~~WAY TO MAKE stuff more READABLE~~~~~~~~~~~~~~~~~~~~*/

enum BufferSize{small = MESSAGE_BUFF_SIZE_SMALL, medium = MESSAGE_BUFF_SIZE_MEDIUM, large = MESSAGE_BUFF_SIZE_LARGE, xxl = MESSAGE_BUFF_SIZE_XXL};
enum Sources{NONE, UART, SATin, SATout, XBEE}; // Add sources for your application i.e.: ACCEL, SPI, BLE, COMMS, RADIO, SATELLITE, I2C, or whatever you are interfacing with

/*~~~~~~~~~~~~~~~~~GENERIC STRUCTURE FOR HOLDING MESSAGES~~~~~~~~~~~~~~~~~~*/
typedef struct {

	enum BufferSize bufferType;         // Buffer Size Enum
	unsigned char * msgPtr;             // The actual data in the buffer
	unsigned char buffNum;		        // The buffer number
	int dataLength;                     // Length of data in buffer ( in bytes )
	int expectedDataLength;             // Maybe you want this . . .
	bool inUseFlag;                     // This flag drives the _getMessage() and _seekMessage() functions
	EpochTime timestamp;                // Might not need, tack on Epoch Time if desired
	enum Sources source;                // Describes where this message came from depending on application

} Message;

/**********************************************************************
* Function:     void message_init(void)
* Requirement:  None
* Input:		None
* Return 		None
* Overview:		Call this early in order to use messages
***********************************************************************/
void message_init(void);

/**********************************************************************
* Function:     void message_ClearData(struct Message * msg)
* Requirement:  MAKE SURE YOU OWN THIS MESSAGE DATA
* Input:		msg --pointer to message to clear
* Return 		None
* Overview:		sets all bytes(unsigned char) in buff array to zero
***********************************************************************/
void message_clearData(Message * msg);

/**********************************************************************
* Function:     struct Message * message_requestMsgBuff(enum BufferSize size)
* Requirement:  None
* Input:		size --requested buffer size
* Return 		pointer to Message structure with buffer of requested size (OR 0/NULL if none)
* Overview:		gets a message stucture if one is available
***********************************************************************/
Message * message_requestMsgBuff(int size);

/**********************************************************************
* Function:     struct Message * message_getMsg(enum Sources source)
* Requirement:  None
* Input:		source -- where the msg came from
* Return 		pointer to Message structure (OR 0/NULL if none)
* Overview:		gets a message stucture with data for this source if one is available
***********************************************************************/
Message * message_getMsg(enum Sources source);

/**********************************************************************
* Function:     Message * message_seekMsg(enum Sources source, unsigned char orderNumber)
* Requirement:  None
* Input:        source -- where the msg came from, orderNumber -- which msg of same source wanted
* Return        pointer to Message structure (OR 0/NULL if none)
* Overview:     gets a message stucture with data for this source when multiple exist
***********************************************************************/
Message * message_seekMsg(enum Sources source, unsigned char orderNumber);

/**********************************************************************
* Function:     int message_append(struct Message * Msg, unsigned char* newData, unsigned char newDataLen)
* Requirement:  None
* Input:		msg - structure to append data to
* Return 		0 if could not because of buffer size
* 				1 if could because there was room
* Overview:		Adds bytes to the messages buffer, does check first to see if it fits
***********************************************************************/
bool message_append(Message * Msg, unsigned char* newData, unsigned char newDataLen);

/**********************************************************************
* Function:     void message_freeMsg(struct Message * msg)
* Requirement:  None
* Input:		msg -- ptr to message that is no longer needed
* Return 		None
* Overview:		Frees the message resource for use later
***********************************************************************/
void message_freeMsg(Message * msg);

/**********************************************************************
* Function:     void message_killMsg(struct Message * msg)
* Requirement:  None
* Input:		msg -- ptr to message that is no longer needed
* Return 		None
* Overview:		Frees the message resource for use later, AND also clears the data as well as zeros the users pointer
***********************************************************************/
void message_killMsg(Message ** msgStructPtr);

#endif /* SUPPORT_MESSAGE_H_ */
