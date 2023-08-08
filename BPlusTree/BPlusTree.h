#ifndef NITCBASE_BPLUSTREE_H
#define NITCBASE_BPLUSTREE_H

#include "../Buffer/BlockBuffer.h"
#include "../Buffer/StaticBuffer.h"
#include "../Cache/OpenRelTable.h"
#include "../define/constants.h"
#include "../define/id.h"

class BPlusTree {
 private:
  static int findLeafToInsert(int rootBlock, Attribute attrVal, int attrType);
  static int insertIntoLeaf(int relId, char attrName[ATTR_SIZE], int blockNum, Index entry);
  static int splitLeaf(int leafBlockNum, Index indices[]);
  static int insertIntoInternal(int relId, char attrName[ATTR_SIZE], int intBlockNum, InternalEntry entry);
  static int splitInternal(int intBlockNum, InternalEntry internalEntries[]);
  static int createNewRoot(int relId, char attrName[ATTR_SIZE], Attribute attrVal, int lChild, int rChild);

 public:
  static int bPlusCreate(int relId, char attrName[ATTR_SIZE]);
  static int bPlusInsert(int relId, char attrName[ATTR_SIZE], union Attribute attrVal, RecId recordId);
  static RecId bPlusSearch(int relId, char attrName[ATTR_SIZE], union Attribute attrVal, int op);
  static int bPlusDestroy(int rootBlockNum);
};

#endif  // NITCBASE_BPLUSTREE_H
