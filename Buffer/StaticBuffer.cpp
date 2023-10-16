// the declarations for this class can be found at "StaticBuffer.h"
#include "StaticBuffer.h"

unsigned char StaticBuffer::blocks[BUFFER_CAPACITY][BLOCK_SIZE];
struct BufferMetaInfo StaticBuffer::metainfo[BUFFER_CAPACITY];
// declare the blockAllocMap array
unsigned char StaticBuffer::blockAllocMap[DISK_BLOCKS];

/*
TODO:: write block allocation map from block 0-3 to block allocation map*/
StaticBuffer::StaticBuffer() {
  // initialise all blocks as free
  for (int i = 0, blockMapslot = 0; i < 4; i++) {
    unsigned char buffer[BLOCK_SIZE];
    Disk::readBlock(buffer, i);
    for (int slot = 0; slot < BLOCK_SIZE; slot++, blockMapslot++) {
      StaticBuffer::blockAllocMap[blockMapslot] = buffer[slot];
    }
  }

  for (int bufferIndex = 0; bufferIndex < BUFFER_CAPACITY; bufferIndex++) {
    metainfo[bufferIndex].free = true;
    metainfo[bufferIndex].dirty = false;
    metainfo[bufferIndex].blockNum = -1;
    metainfo[bufferIndex].timeStamp = -1;
  }
}

/*
At this stage, we are not writing back from the buffer to the disk since we are
not modifying the buffer. So, we will define an empty destructor for now. In
subsequent stages, we will implement the write-back functionality here.
*/

/* 
TODO::writing back to block allocation map*/
StaticBuffer::~StaticBuffer() {
  for (int i = 0, blockMapslot = 0; i < 4; i++) {
    unsigned char buffer[BLOCK_SIZE];
    for (int slot = 0; slot < BLOCK_SIZE; slot++, blockMapslot++) {
      buffer[slot] = blockAllocMap[blockMapslot];
    }
    Disk::writeBlock(buffer, i);
  }

  for (int bufferIndex = 0; bufferIndex < BUFFER_CAPACITY; bufferIndex++) {
    if (metainfo[bufferIndex].free == false and
        metainfo[bufferIndex].dirty == true) {
      Disk::writeBlock(blocks[bufferIndex], metainfo[bufferIndex].blockNum);
    }
  }
}
int StaticBuffer::getFreeBuffer(int blockNum) {
  // Assigns a buffer to the block and returns the buffer number. If no free
  // buffer block is found, the least recently used (LRU) buffer block is
  // replaced.

  if (blockNum < 0 || blockNum > DISK_BLOCKS) {
    return E_OUTOFBOUND;
  }
  int allocatedBuffer = -1;
  // iterate through all the blocks in the StaticBuffer
  // find the first free block in the buffer (check metainfo)
  // assign allocatedBuffer = index of the free block
  int timeStamp = 0, maxindex = 0;

  for (int bufferIndex = 0; bufferIndex < BUFFER_CAPACITY; bufferIndex++) {

    if (metainfo[bufferIndex].timeStamp > timeStamp) {
      timeStamp = metainfo[bufferIndex].timeStamp;
      maxindex = bufferIndex;
    }
    if (metainfo[bufferIndex].free) {
      allocatedBuffer = bufferIndex;
      break;
    }
  }

  if (allocatedBuffer == -1) {
    if (metainfo[maxindex].dirty == true) {
      Disk::writeBlock(blocks[maxindex], metainfo[maxindex].blockNum);
      allocatedBuffer = maxindex;
    }
  }

  metainfo[allocatedBuffer].free = false;
  metainfo[allocatedBuffer].blockNum = blockNum;
  metainfo[allocatedBuffer].dirty = false;
  metainfo[allocatedBuffer].timeStamp = 0;

  return allocatedBuffer;
}
/* Get the buffer index where a particular block is stored
   or E_BLOCKNOTINBUFFER otherwise
*/
int StaticBuffer::getBufferNum(int blockNum) {
  // Check if blockNum is valid (between zero and DISK_BLOCKS)
  // and return E_OUTOFBOUND if not valid.
  if (blockNum < 0 || blockNum >= DISK_BLOCKS) {
    return E_OUTOFBOUND;
  }

  // find and return the bufferIndex which corresponds to blockNum (check
  // metainfo)
  for (int bufferIndex = 0; bufferIndex < BUFFER_CAPACITY; bufferIndex++) {
    if (metainfo[bufferIndex].blockNum == blockNum and metainfo[bufferIndex].free==false) {
      return bufferIndex;
    }
  }
  // if block is not in the buffer
  return E_BLOCKNOTINBUFFER;
}

int StaticBuffer::setDirtyBit(int blockNum) {
  // find the buffer index corresponding to the block using getBufferNum().
  int bufferIndex = getBufferNum(blockNum);

  // if block is not present in the buffer (bufferNum = E_BLOCKNOTINBUFFER)
  //     return E_BLOCKNOTINBUFFER
  if (bufferIndex == E_BLOCKNOTINBUFFER) {
    return E_BLOCKNOTINBUFFER;
  }
  if (bufferIndex == E_OUTOFBOUND) {
    return E_OUTOFBOUND;
  } else {
    metainfo[bufferIndex].dirty = true;
  }
  return SUCCESS;
  // if blockNum is out of bound (bufferNum = E_OUTOFBOUND)
  //     return E_OUTOFBOUND

  // else
  //     (the bufferNum is valid)
  //     set the dirty bit of that buffer to true in metainfo

  // return SUCCESS
}