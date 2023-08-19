// #include "BlockBuffer.h"

// #include <cstdlib>
// #include <cstring>
// #include <iostream>

// // the declarations for these functions can be found in "BlockBuffer.h"

// BlockBuffer::BlockBuffer(int blockNum)
// {
//   this->blockNum = blockNum;
//   // initialise this.blockNum with the argument
// }

// // calls the parent class constructor
// RecBuffer::RecBuffer(int blockNum) : BlockBuffer::BlockBuffer(blockNum) {}

// // load the block header into the argument pointer
// int BlockBuffer::getHeader(struct HeadInfo *head)
// {
//   unsigned char buffer[BLOCK_SIZE];
//   // read the block at this.blockNum into the buffer
//   Disk::readBlock(buffer, this->blockNum);
//   // populate the numEntries, numAttrs and numSlots fields in *head
//   memcpy(&head->numSlots, buffer + 24, 4);
//   memcpy(&head->numEntries, buffer+16, 4);
//   memcpy(&head->numAttrs, buffer+20, 4);
//   memcpy(&head->rblock, buffer+12, 4);
//   memcpy(&head->lblock, buffer+8, 4);
//   return SUCCESS;
// }

// // load git the record at slotNum into the argument pointer
// int RecBuffer::getRecord(union Attribute *rec, int slotNum)
// {
//   struct HeadInfo head;
//   // get the header using this.getHeader() function
//   this->getHeader(&head);

//   int attrCount = head.numAttrs;  
//   int slotCount = head.numSlots;
  

//   // q1

//   // read the block at this.blockNum into a buffer
//   unsigned char buffer[BLOCK_SIZE];
//   Disk::readBlock(buffer, this->blockNum);
  
//   /* record at slotNum will be at offset HEADER_SIZE + slotMapSize + (recordSize * slotNum)
//      - each record will have size attrCount * ATTR_SIZE
//      - slotMap will be of size slotCount
//   */
 
//   int recordSize = attrCount * ATTR_SIZE;
//   unsigned char *slotPointer = buffer+32+slotCount+slotNum*recordSize; /* calculate buffer + offset */

//   // load the record into the rec data structure
//   memcpy(rec, slotPointer, recordSize);

//   return SUCCESS;
// }

#include "BlockBuffer.h"

#include <cstdlib>
#include <cstring>

// the declarations for these functions can be found in "BlockBuffer.h"

BlockBuffer::BlockBuffer (int blockNum) {
    // initialise this.blockNum with the argument
    this->blockNum = blockNum;
}

// calls the parent class constructor
RecBuffer::RecBuffer(int blockNum) : BlockBuffer::BlockBuffer(blockNum) {}

// load the block header into the argument pointer
int BlockBuffer::getHeader (HeadInfo *head) {
    // read the block at this.blockNum into the buffer
    unsigned char buffer[BLOCK_SIZE];
    Disk::readBlock(buffer, this->blockNum);

    // populate the numEntries, numAttrs and numSlots fields in *head
    memcpy(&head->pblock, buffer + 4, 4);
    memcpy(&head->lblock, buffer + 8, 4);
    memcpy(&head->rblock, buffer + 12, 4);
    memcpy(&head->numEntries, buffer + 16, 4);
    memcpy(&head->numAttrs, buffer + 20, 4);
    memcpy(&head->numSlots, buffer + 24, 4);

    return SUCCESS;
}

// load the record at slotNum into the argument pointer
int RecBuffer::getRecord (union Attribute *record, int slotNum) {
    // get the header using this.getHeader() function
    HeadInfo head;
    BlockBuffer::getHeader(&head);

    int attrCount = head.numAttrs;
    int slotCount = head.numSlots;

    // read the block at this.blockNum into a buffer
    unsigned char buffer[BLOCK_SIZE];
    Disk::readBlock(buffer, this->blockNum);

    /* record at slotNum will be at offset HEADER_SIZE + slotMapSize + (recordSize * slotNum)
       - each record will have size attrCount * ATTR_SIZE
       - slotMap will be of size slotCount
    */
    int recordSize = attrCount * ATTR_SIZE;
    unsigned char *slotPointer = buffer + (32 + slotCount 
                                            + (recordSize*slotNum)
                                        ); // calculate buffer + offset 

    // load the record into the rec data structure
    memcpy(record, slotPointer, recordSize);

    return SUCCESS;
}

// load the record at slotNum into the argument pointer
int RecBuffer::setRecord (union Attribute *record, int slotNum) {
    // get the header using this.getHeader() function
    HeadInfo head;
    BlockBuffer::getHeader(&head);

    int attrCount = head.numAttrs;
    int slotCount = head.numSlots;

    // read the block at this.blockNum into a buffer
    unsigned char buffer[BLOCK_SIZE];
    Disk::readBlock(buffer, this->blockNum);

    /* record at slotNum will be at offset HEADER_SIZE + slotMapSize + (recordSize * slotNum)
       - each record will have size attrCount * ATTR_SIZE
       - slotMap will be of size slotCount
    */
    int recordSize = attrCount * ATTR_SIZE;
    unsigned char *slotPointer = buffer + (32 + slotCount 
                                            + (recordSize*slotNum)
                                        ); // calculate buffer + offset 

    // load the record into the rec data structure
    memcpy(slotPointer, record, recordSize);

    Disk::writeBlock(buffer, this->blockNum);

    return SUCCESS;
}


