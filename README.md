# Message-Paradigm

Message paradigm uses the concept of Messages for sending and receiving data in embedded devices. 
Memory is allocated for messages using Message.h

Messages are structs that contain a message's Data, dataLength, expectedDataLength ( if desired ), timestamp, and source of message.

! ! Message Structs must be initialized with message_init() ! !

        message_init();

To use the paradigm follow these steps. Sometimes you may want temporary message pointers while other times you may want one global message pointer.
This paradigm is most useful for RX:

1) Code your RX function/ISR to use a global, dedicated Message:


        Message* spiRxMsg;

        mySpiRxISR{
        
        // If incoming message contains expected length, you can populated your message struct with that
        
        
        spiRxMsg->msgPtr+spiRxMsg->dataLength = SPI_RX_REG;  // receive the register data into msgPtr
        spiRxMsg->dataLength++;  ( you can also increment in the above line, saving a line of code )
        
        // Update the source of the message so you can find it later
        spiRxMsg->source = TEMP_SENSOR_SPI  ( these sources are completely arbitrary, make up whatever you want )
        
        // If using my Event Handling paradigm, create an event at the end of the ISR
        // event_add(TEMP_SENSOR_SPI_RX)
        // These events are handled in a way that mimics RTOS
        }
 

2) After you receive your data, seek out the message whenever your CPU is ready:
    If using a global Message*, you don't need to seek, you already know where it is:
    
         spiRxMsg
              
    If using a temporary Message*, you'll need to seek via SOURCE:
    
         Message* tempMsg;
         tempMsg = message_getMsg(TEMP_SENSOR_SPI);  // this will find a message from passed source
              

3) If you are behind, and get more than one message from the same source, you can seek a second, third, fourth, etc one by using:

         tempMsg = message_seekMsg(TEMP_SENSOR_SPI, orderNumber); // orderNumber would be 2 for second, 3 for third, etc 
 
 
4) After you're done with the message contents, Kill/Free it so it can be used by something else:

         message_freeMessage(tempMsg);
         message_killMessage(tempMsg);
      
    If you are using a global, dedicated Message, just clear the msgPtr and clear dataLength:
    
         message_clearData(spiRxMsg);
      

For TX:
1) Request a Message that will hold the length ( in bytes ) of your data:

        Message* txMsgPtr;
        txMsgPtr = message_requestMsgBuff(50);
    
    
2) Fill the Message with your data. Append() will automatically add data length to txMsgPtr->dataLength:

        message_append(txMsgBuff, lengthOfTxData);
    
    
3) Use txMsgBuffer->msgPtr and txMsgBuffer->dataLength in your TX function and TX ISR.

        RAM is used for these Messages. Configure the size and number of these Messages according to your RAM in Message.h :
        #define MESSAGE_BUFF_SIZE_SMALL         50
        #define MESSAGE_BUFF_SIZE_MEDIUM        200
        #define MESSAGE_BUFF_SIZE_LARGE         400	  // you can change the size of each buffer type here
        #define MESSAGE_BUFF_SIZE_XXL           1000

        #define MESSAGE_BUFF_COUNT_SMALL        10
        #define MESSAGE_BUFF_COUNT_MEDIUM       4	  // if you don't need larger buffers, zero them out here to save on RAM
        #define MESSAGE_BUFF_COUNT_LARGE        2
        #define MESSAGE_BUFF_COUNT_XXL          0

        #define TOTAL_MESSAGE_BUFFERS	MESSAGE_BUFF_COUNT_SMALL + MESSAGE_BUFF_COUNT_MEDIUM + MESSAGE_BUFF_COUNT_LARGE + MESSAGE_BUFF_COUNT_XXL

