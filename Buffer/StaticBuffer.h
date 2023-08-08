#ifndef NITCBASE_STATICBUFFER_H
#define NITCBASE_STATICBUFFER_H

#include "../Disk_Class/Disk.h"
#include "../define/constants.h"

struct BufferMetaInfo {
  bool free;
  bool dirty;
  int blockNum;
  int timeStamp;
};

class StaticBuffer {
  friend class BlockBuffer;

 private:
  // fields
  static unsigned char blocks[BUFFER_CAPACITY][BLOCK_SIZE];
  static struct BufferMetaInfo metainfo[BUFFER_CAPACITY];
  static unsigned char blockAllocMap[DISK_BLOCKS];

  // methods
  static int getFreeBuffer(int blockNum);
  static int getBufferNum(int blockNum);

 public:
  // methods
  static int getStaticBlockType(int blockNum);
  static int setDirtyBit(int blockNum);
  StaticBuffer();
  ~StaticBuffer();
};

#endif  // NITCBASE_STATICBUFFER_H
