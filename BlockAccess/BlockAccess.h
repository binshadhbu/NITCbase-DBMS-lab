#ifndef NITCBASE_BLOCKACCESS_H
#define NITCBASE_BLOCKACCESS_H

#include "../BPlusTree/BPlusTree.h"
#include "../Buffer/BlockBuffer.h"
#include "../Cache/AttrCacheTable.h"
#include "../Cache/RelCacheTable.h"
#include "../define/constants.h"
#include "../define/id.h"

class BlockAccess {
 public:
  static int search(int relId, Attribute *record, char *attrName, Attribute attrVal, int op);

  static int insert(int relId, union Attribute *record);

  static int renameRelation(char *oldName, char *newName);

  static int renameAttribute(char *relName, char *oldName, char *newName);

  static int deleteRelation(char *relName);

  static RecId linearSearch(int relId, char *attrName, Attribute attrVal, int op);

  static int project(int relId, Attribute *record);
};

#endif  // NITCBASE_BLOCKACCESS_H