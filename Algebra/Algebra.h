#ifndef NITCBASE_ALGEBRA_H
#define NITCBASE_ALGEBRA_H

#include "../Cache/OpenRelTable.h"
#include "../Schema/Schema.h"
#include "../define/constants.h"

class Algebra {
 public:
  // Insert
  static int insert(char relName[ATTR_SIZE], int numberOfAttributes, char record[][ATTR_SIZE]);

  // Select
  static int select(char srcRel[ATTR_SIZE], char targetRel[ATTR_SIZE], char attr[ATTR_SIZE], int op, char strVal[ATTR_SIZE]);

  // Project all (Copy)
  static int project(char srcRel[ATTR_SIZE], char targetRel[ATTR_SIZE]);

  // Project
  static int project(char srcRel[ATTR_SIZE], char targetRel[ATTR_SIZE], int tar_nAttrs, char tar_Attrs[][ATTR_SIZE]);

  // Join
  static int join(char srcRelOne[ATTR_SIZE], char srcRelTwo[ATTR_SIZE], char targetRel[ATTR_SIZE],
                  char attrOne[ATTR_SIZE], char attrTwo[ATTR_SIZE]);
};

#endif  // NITCBASE_ALGEBRA_H
