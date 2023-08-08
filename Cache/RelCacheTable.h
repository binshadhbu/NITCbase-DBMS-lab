#ifndef NITCBASE_RELCACHETABLE_H
#define NITCBASE_RELCACHETABLE_H

#include "../Buffer/BlockBuffer.h"
#include "../define/constants.h"
#include "../define/id.h"

typedef struct RelCatEntry {
  char relName[ATTR_SIZE];
  int numAttrs;
  int numRecs;
  int firstBlk;
  int lastBlk;
  int numSlotsPerBlk;

} RelCatEntry;

typedef struct RelCacheEntry {
  RelCatEntry relCatEntry;
  bool dirty;
  RecId recId;
  RecId searchIndex;

} RelCacheEntry;

class RelCacheTable {
  friend class OpenRelTable;

 public:
  // methods
  static int getRelCatEntry(int relId, RelCatEntry *relCatBuf);
  static int setRelCatEntry(int relId, RelCatEntry *relCatBuf);
  static int getSearchIndex(int relId, RecId *searchIndex);
  static int setSearchIndex(int relId, RecId *searchIndex);
  static int resetSearchIndex(int relId);

 private:
  // field
  static RelCacheEntry *relCache[MAX_OPEN];

  // methods
  static void recordToRelCatEntry(union Attribute record[RELCAT_NO_ATTRS], RelCatEntry *relCatEntry);
  static void relCatEntryToRecord(RelCatEntry *relCatEntry, union Attribute record[RELCAT_NO_ATTRS]);
};
#endif  // NITCBASE_RELCACHETABLE_H
