#ifndef NITCBASE_OPENRELTABLE_H
#define NITCBASE_OPENRELTABLE_H

#include "../BlockAccess/BlockAccess.h"
#include "../Buffer/BlockBuffer.h"
#include "../define/constants.h"
#include "AttrCacheTable.h"
#include "RelCacheTable.h"

typedef struct OpenRelTableMetaInfo {
  bool free;
  char relName[ATTR_SIZE];

} OpenRelTableMetaInfo;

class OpenRelTable {
 public:
  // methods
  OpenRelTable();
  ~OpenRelTable();
  static int getRelId(char relName[ATTR_SIZE]);
  static int openRel(char relName[ATTR_SIZE]);
  static int closeRel(int relId);

 private:
  // field
  static OpenRelTableMetaInfo tableMetaInfo[MAX_OPEN];

  // method
  static int getFreeOpenRelTableEntry();
};

#endif  // NITCBASE_OPENRELTABLE_H
