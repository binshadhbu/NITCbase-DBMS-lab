#ifndef NITCBASE_SCHEMA_H
#define NITCBASE_SCHEMA_H

#include "../BlockAccess/BlockAccess.h"
#include "../Cache/OpenRelTable.h"
#include "../define/constants.h"

class Schema {
 public:
  static int createRel(char relName[], int numOfAttributes, char attrNames[][ATTR_SIZE], int attrType[]);
  static int deleteRel(char relName[ATTR_SIZE]);
  static int createIndex(char relName[ATTR_SIZE], char attrName[ATTR_SIZE]);
  static int dropIndex(char relName[ATTR_SIZE], char attrName[ATTR_SIZE]);
  static int renameRel(char oldRelName[ATTR_SIZE], char newRelName[ATTR_SIZE]);
  static int renameAttr(char relName[ATTR_SIZE], char oldAttrName[ATTR_SIZE], char newAttrName[ATTR_SIZE]);
  static int openRel(char relName[ATTR_SIZE]);
  static int closeRel(char relName[ATTR_SIZE]);
};

#endif  // NITCBASE_SCHEMA_H
