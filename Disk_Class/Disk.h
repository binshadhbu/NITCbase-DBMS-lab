#ifndef NITCBASE_H
#define NITCBASE_H
class Disk {
 public:
  Disk();
  ~Disk();
  static int readBlock(unsigned char *block, int blockNum);
  static int writeBlock(unsigned char *block, int blockNum);
};
#endif  // NITCBASE_H