

#include "OpenRelTable.h"

#include <cstring>
#include <stdio.h>
#include <stdlib.h>

AttrCacheEntry *createAttrCacheEntryList(int size) {
  AttrCacheEntry *head = nullptr, *curr = nullptr;
  head = curr = (AttrCacheEntry *)malloc(sizeof(AttrCacheEntry));
  size--;
  while (size--) {
    curr->next = (AttrCacheEntry *)malloc(sizeof(AttrCacheEntry));
    curr = curr->next;
  }
  curr->next = nullptr;

  return head;
}

OpenRelTableMetaInfo OpenRelTable::tableMetaInfo[MAX_OPEN];

OpenRelTable::OpenRelTable() {

  // initialize relCache and attrCache with nullptr
  for (int i = 0; i < MAX_OPEN; ++i) {
    RelCacheTable::relCache[i] = nullptr;
    AttrCacheTable::attrCache[i] = nullptr;
  }

  /************ Setting up Relation Cache entries ************/
  // (we need to populate relation cache with entries for the relation catalog
  //  and attribute catalog.)

  // setting up the variables
  RecBuffer relCatBlock(RELCAT_BLOCK);
  Attribute relCatRecord[RELCAT_NO_ATTRS];
  RelCacheEntry *relCacheEntry = nullptr;

  for (int relId = RELCAT_RELID; relId <= ATTRCAT_RELID; relId++) {
    relCatBlock.getRecord(relCatRecord, relId);

    relCacheEntry = (RelCacheEntry *)malloc(sizeof(RelCacheEntry));
    RelCacheTable::recordToRelCatEntry(relCatRecord,
                                       &(relCacheEntry->relCatEntry));
    relCacheEntry->recId.block = RELCAT_BLOCK;
    relCacheEntry->recId.slot = relId;

    RelCacheTable::relCache[relId] = relCacheEntry;
  }

  /************ Setting up Attribute cache entries ************/
  // (we need to populate attribute cache with entries for the relation catalog
  //  and attribute catalog.)

  // setting up the variables
  RecBuffer attrCatBlock(ATTRCAT_BLOCK);
  Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
  AttrCacheEntry *attrCacheEntry = nullptr, *head = nullptr;

  for (int relId = RELCAT_RELID, recordId = 0; relId <= ATTRCAT_RELID;
       relId++) {
    int numberOfAttributes =
        RelCacheTable::relCache[relId]->relCatEntry.numAttrs;
    head = createAttrCacheEntryList(numberOfAttributes);
    attrCacheEntry = head;

    while (numberOfAttributes--) {
      attrCatBlock.getRecord(attrCatRecord, recordId);

      AttrCacheTable::recordToAttrCatEntry(attrCatRecord,
                                           &(attrCacheEntry->attrCatEntry));
      attrCacheEntry->recId.slot = recordId++;
      attrCacheEntry->recId.block = ATTRCAT_BLOCK;

      attrCacheEntry = attrCacheEntry->next;
    }

    AttrCacheTable::attrCache[relId] = head;
  }

  tableMetaInfo[RELCAT_RELID].free = false;
  tableMetaInfo[ATTRCAT_RELID].free = false;
  strcpy(tableMetaInfo[RELCAT_RELID].relName, "RELATIONCAT");
  strcpy(tableMetaInfo[ATTRCAT_RELID].relName, "ATTRIBUTECAT");
}

OpenRelTable::~OpenRelTable() {
  for (int i = 2; i < MAX_OPEN; ++i) {
    if (!tableMetaInfo[i].free) {
      OpenRelTable::closeRel(i); // we will implement this function later
    }
  }
  // free all the memory that you allocated in the constructor
}

/* This function will open a relation having name `relName`.
Since we are currently only working with the relation and attribute catalog, we
will just hardcode it. In subsequent stages, we will loop through all the
relations and open the appropriate one.
*/
int OpenRelTable::getRelId(char relName[ATTR_SIZE]) {
  for (int i = 0; i < MAX_OPEN; i++) {
    if (strcmp(relName, tableMetaInfo[i].relName) == 0) {
      return i;
    }
  }
  return E_RELNOTOPEN;
}

int OpenRelTable::getFreeOpenRelTableEntry() {

  for (int i = 2; i < MAX_OPEN; i++) {
    if (tableMetaInfo[i].free) {
      return i;
    }
  }
  return E_CACHEFULL;
  /* traverse through the tableMetaInfo array,
    find a free entry in the Open Relation Table.*/

  // if found return the relation id, else return E_CACHEFULL.
}

int OpenRelTable::closeRel(int relId) {
  if (relId == 0 or relId == 1) {
    return E_NOTPERMITTED;
  }

  if (relId < 0 || relId >= MAX_OPEN) {
    return E_OUTOFBOUND;
  }

  if (AttrCacheTable::attrCache[relId] == nullptr) {
    return E_RELNOTOPEN;
  }

  // free the memory allocated in the relation and attribute caches which was
  // allocated in the OpenRelTable::openRel() function
  tableMetaInfo[relId].free = true;
  AttrCacheTable::attrCache[relId] = nullptr;
  RelCacheTable::relCache[relId] = nullptr;
  // update `tableMetaInfo` to set `relId` as a free slot
  // update `relCache` and `attrCache` to set the entry at `relId` to nullptr

  return SUCCESS;
}
