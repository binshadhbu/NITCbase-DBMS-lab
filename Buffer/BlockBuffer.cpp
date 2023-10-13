#include "BlockBuffer.h"
#include <cstdlib>
#include <cstring>
#include <iostream>

BlockBuffer::BlockBuffer(int blockNum) { this->blockNum = blockNum; }

BlockBuffer::BlockBuffer(char blocktype) {

  int blockType = blocktype == 'R' ? REC : UNUSED_BLK;
  int blockNum = getFreeBlock(blockType);
  if (blockNum == E_DISKFULL) {
    printf("Didk full\n");
  }
  this->blockNum = blockNum;
  if (blockNum < 0 or blockNum >= DISK_BLOCKS) {
    printf("invalis disk number\n");
  }
  // allocate a block on the disk and a buffer in memory to hold the new block
  // of given type using getFreeBlock function and get the return error codes if
  // any.

  // set the blockNum field of the object to that of the allocated block
  // number if the method returned a valid block number,
  // otherwise set the error code returned as the block number.

  // (The caller must check if the constructor allocatted block successfully
  // by checking the value of block number field.)
}

// calls the parent class constructor
RecBuffer::RecBuffer(int blockNum) : BlockBuffer::BlockBuffer(blockNum) {}
RecBuffer::RecBuffer() : BlockBuffer('R') {}
/*
Used to get the header of the block into the location pointed to by `head`
NOTE: this function expects the caller to allocate memory for `head`
*/
// load the block header into the argument pointer
int BlockBuffer::getHeader(HeadInfo *head) {
  unsigned char *buffer;
  int ret = loadBlockAndGetBufferPtr(&buffer);
  if (ret != SUCCESS)
    return ret;

  // TODO: populate the numEntries, numAttrs and numSlots fields in *head
  memcpy(&head->pblock, buffer + 4, 4);
  memcpy(&head->lblock, buffer + 8, 4);
  memcpy(&head->rblock, buffer + 12, 4);
  memcpy(&head->numEntries, buffer + 16, 4);
  memcpy(&head->numAttrs, buffer + 20, 4);
  memcpy(&head->numSlots, buffer + 24, 4);

  return SUCCESS;
}

/*
Used to get the record at slot `slotNum` into the array `rec`
NOTE: this function expects the caller to allocate memory for `rec`
*/
// load the record at slotNum into the argument pointer
int RecBuffer::getRecord(union Attribute *record, int slotNum) {
  HeadInfo head;
  BlockBuffer::getHeader(&head);

  int attrCount = head.numAttrs;
  int slotCount = head.numSlots;

  // read the block at this.blockNum into a buffer
  unsigned char *buffer;
  // // Disk::readBlock(buffer, this->blockNum);
  int ret = loadBlockAndGetBufferPtr(&buffer);
  if (ret != SUCCESS)
    return ret;

  //* record at slotNum will be at offset HEADER_SIZE + slotMapSize +
  //(recordSize * slotNum)
  //     each record will have size attrCount * ATTR_SIZE
  //     slotMap will be of size slotCount
  int recordSize = attrCount * ATTR_SIZE;
  unsigned char *slotPointer =
      buffer +
      (32 + slotCount + (recordSize * slotNum)); // calculate buffer + offset

  // load the record into the rec data structure
  memcpy(record, slotPointer, recordSize);

  return SUCCESS;
}

/*
Used to load a block to the buffer and get a pointer to it.
NOTE: this function expects the caller to allocate memory for the argument
*/

int BlockBuffer::loadBlockAndGetBufferPtr(unsigned char **buffPtr) {
  /* check whether the block is already present in the buffer
    using StaticBuffer.getBufferNum() */
  int bufferNum = StaticBuffer::getBufferNum(this->blockNum);

  if(bufferNum==E_OUTOFBOUND)return E_OUTOFBOUND;

  if (bufferNum != E_BLOCKNOTINBUFFER) {
    for (int bufferIndex = 0; bufferIndex < BUFFER_CAPACITY; bufferIndex++) {
      StaticBuffer::metainfo[bufferIndex].timeStamp++;
    }
    StaticBuffer::metainfo[bufferNum].timeStamp = 0;
  } else {

    bufferNum = StaticBuffer::getFreeBuffer(this->blockNum);

    if (bufferNum == E_OUTOFBOUND) {
      return E_OUTOFBOUND; // the blockNum is invalid
    }

    Disk::readBlock(StaticBuffer::blocks[bufferNum], this->blockNum);
  }
  *buffPtr = StaticBuffer::blocks[bufferNum];
  return SUCCESS;
}

// load the record at slotNum into the argument pointer

int RecBuffer::setRecord(union Attribute *rec, int slotNum) {
  unsigned char *bufferPtr;
  /* get the starting address of the buffer containing the block
     using loadBlockAndGetBufferPtr(&bufferPtr). */
  int bufferNum = BlockBuffer::loadBlockAndGetBufferPtr(&bufferPtr);
  if (bufferNum != SUCCESS) {
    return bufferNum;
  }

  // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
  // return the value returned by the call.

  /* get the header of the block using the getHeader() function */
  HeadInfo head;
  BlockBuffer::getHeader(&head);

  // get number of attributes in the block.
  int attrCount = head.numAttrs;
  int slotCount = head.numSlots;
  // get the number of slots in the block.
  if (slotNum > slotCount or slotNum < 0) {
    return E_OUTOFBOUND;
  } // if input slotNum is not in the permitted range return E_OUTOFBOUND.

  /* offset bufferPtr to point to the beginning of the record at required
     slot. the block contains the header, the slotmap, followed by all
     the records. so, for example,
     record at slot x will be at bufferPtr + HEADER_SIZE + (x*recordSize)
     copy the record from `rec` to buffer using memcpy
     (hint: a record will be of size ATTR_SIZE * numAttrs)
  */
  //  unsigned char buffer[BLOCK_SIZE];
  // Disk::readBlock(buffer, this->blockNum);
  int recordSize = attrCount * ATTR_SIZE;
  unsigned char *slotPointer =
      bufferPtr + (32 + slotCount + (recordSize * slotNum));

  memcpy(slotPointer, rec, recordSize);
  // update dirty bit using setDirtyBit()
  int ret = StaticBuffer::setDirtyBit(this->blockNum);
  if (ret != SUCCESS) {
    std::cout << "something srong with the setDirty function";
  }
  /* (the above function call should not fail since the block is already
     in buffer and the blockNum is valid. If the call does fail, there
     exists some other issue in the code) */

  // return SUCCESS
  return SUCCESS;
}

/* used to get the slotmap from a record block
NOTE: this function expects the caller to allocate memory for `*slotMap`
*/
int RecBuffer::getSlotMap(unsigned char *slotMap) {
  unsigned char *bufferPtr;
  // get the starting address of the buffer containing the block using
  // loadBlockAndGetBufferPtr().
  int ret = loadBlockAndGetBufferPtr(&bufferPtr);
  if (ret != SUCCESS) {
    return ret;
  }
  struct HeadInfo head;
  RecBuffer recordBlock (this->blockNum);
  recordBlock.getHeader(&head);
  // get the header of the b
  int slotCount = head.numSlots; /* number of slots in block from header */
  // get a pointer to the beginning of the slotmap in memory by offsetting
  // HEADER_SIZE
  unsigned char *slotMapInBuffer = bufferPtr + HEADER_SIZE;
  // copy the values from `slotMapInBuffer` to `slotMap` (size is `slotCount`)
  memcpy(slotMap, slotMapInBuffer, slotCount);

  return SUCCESS;
}

int compareAttrs(union Attribute attr1, union Attribute attr2, int attrType) {
  return attrType == NUMBER
             ? (attr1.nVal < attr2.nVal ? -1
                                        : (attr1.nVal > attr2.nVal ? 1 : 0))
             : strcmp(attr1.sVal, attr2.sVal);
}

int BlockBuffer::setHeader(struct HeadInfo *head) {

  unsigned char *bufferPtr;
  int bufferreturn = loadBlockAndGetBufferPtr(&bufferPtr);
  if (bufferreturn != SUCCESS) {
    return bufferreturn;
  }
  // get the starting address of the buffer containing the block using
  // loadBlockAndGetBufferPtr(&bufferPtr).

  // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
  // return the value returned by the call.

  // cast bufferPtr to type HeadInfo*
  struct HeadInfo *bufferHeader = (struct HeadInfo *)bufferPtr;

  // copy the fields of the HeadInfo pointed to by head (except reserved) to
  // the header of the block (pointed to by bufferHeader)
  //(hint: bufferHeader->numSlots = head->numSlots )
  bufferHeader->numSlots = head->numSlots;
  bufferHeader->lblock = head->lblock;
  bufferHeader->numEntries = head->numEntries;
  bufferHeader->pblock = head->pblock;
  bufferHeader->rblock = head->rblock;
  bufferHeader->blockType = head->blockType;

  // update dirty bit by calling StaticBuffer::setDirtyBit()
  // if setDirtyBit() failed, return the error code
  int setDirty = StaticBuffer::setDirtyBit(this->blockNum);
  return setDirty;

  // return SUCCESS;
}
/*
TODO::Sets the type of the block with the input block type. This method sets the
type in both the header of the block and also in the block allocation map.*/

int BlockBuffer::setBlockType(int blockType) {

  unsigned char *bufferPtr;
  int ret = loadBlockAndGetBufferPtr(&bufferPtr);
  if (ret != SUCCESS) {
    return SUCCESS;
  }
  /* get the starting address of the buffer containing the block
     using loadBlockAndGetBufferPtr(&bufferPtr). */

  // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
  // return the value returned by the call.

  // store the input block type in the first 4 bytes of the buffer.
  // (hint: cast bufferPtr to int32_t* and then assign it)
  *((int32_t *)bufferPtr) = blockType;

  // update the StaticBuffer::blockAllocMap entry corresponding to the
  // object's block number to `blockType`.
  StaticBuffer::blockAllocMap[this->blockNum] = blockType;
  // update dirty bit by calling StaticBuffer::setDirtyBit()
  return StaticBuffer::setDirtyBit(this->blockNum);
  // if setDirtyBit() failed
  // return the returned value from the call

  // return SUCCESS
}

int BlockBuffer::getFreeBlock(int blockType) {
  int blockNum;
  for (blockNum = 0; blockNum < DISK_BLOCKS; blockNum++) {
    if (StaticBuffer::blockAllocMap[blockNum] == UNUSED_BLK) {
      break;
    }
  }
  // iterate through the StaticBuffer::blockAllocMap and find the block number
  // of a free block in the disk.
  if (blockNum == DISK_BLOCKS)
    return E_DISKFULL;

  // if no block is free, return E_DISKFULL.

  // set the object's blockNum to the block number of the free block.
  this->blockNum = blockNum;

  // find a free buffer using StaticBuffer::getFreeBuffer() .
  int bufferNum = StaticBuffer::getFreeBuffer(blockNum);
  if (bufferNum < 0 or bufferNum >= BUFFER_CAPACITY) {
    printf("Error:buffer is full\n");
    return bufferNum;
  }
  // initialize the header of the block passing a struct HeadInfo with values
  // pblock: -1, lblock: -1, rblock: -1, numEntries: 0, numAttrs: 0, numSlots: 0
  // to the setHeader() function.
  struct HeadInfo header;
  header.lblock = header.pblock = header.rblock = -1;
  header.numAttrs = header.numEntries = header.numSlots = 0;
  setHeader(&header);

  // update the block type of the block to the input block type using
  // setBlockType().
  setBlockType(blockType);
  return blockNum;

  // return block number of the free block.
}

int RecBuffer::setSlotMap(unsigned char *slotMap) {
  unsigned char *bufferPtr;
  /* get the starting address of the buffer containing the block using
     loadBlockAndGetBufferPtr(&bufferPtr). */
  int ret = loadBlockAndGetBufferPtr(&bufferPtr);
  if (ret != SUCCESS) {
    return ret;
  }
  // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
  // return the value returned by the call.

  // get the header of the block using the getHeader() function
  HeadInfo header;
  getHeader(&header);
  int numSlots = header.numSlots; /* the number of slots in the block */
  ;
  memcpy(bufferPtr + HEADER_SIZE, slotMap, numSlots);
  // the slotmap starts at bufferPtr + HEADER_SIZE. Copy the contents of the
  // argument `slotMap` to the buffinter replacing the existing slotmap.
  // Note that size of slotmap is `numSlots`
  ret = StaticBuffer::setDirtyBit(this->blockNum);
  // update dirty bit using StaticBuffer::setDirtyBit
  // if setDirtyBit failed, return the value returned by the call
  return SUCCESS;

  // return SUCCESS
}

int BlockBuffer::getBlockNum() {
  return this->blockNum;

  // return corresponding block number.
}