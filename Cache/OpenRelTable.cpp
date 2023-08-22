#include "OpenRelTable.h"
#include <iostream>

#include <cstring>

OpenRelTable::OpenRelTable() {

  // initialize relCache and attrCache with nullptr
  for (int i = 0; i < MAX_OPEN; ++i) {
    RelCacheTable::relCache[i] = nullptr;
    AttrCacheTable::attrCache[i] = nullptr;
  }

  /************ Setting up Relation Cache entries ************/
  // (we need to populate relation cache with entries for the relation catalog
  //  and attribute catalog.)

  /**** setting up Relation Catalog relation in the Relation Cache Table****/
  RecBuffer relCatBlock(RELCAT_BLOCK);

  Attribute relCatRecord[RELCAT_NO_ATTRS];
  relCatBlock.getRecord(relCatRecord, RELCAT_SLOTNUM_FOR_RELCAT);
  struct RelCacheEntry relCacheEntry;
  RelCacheTable::recordToRelCatEntry(relCatRecord, &relCacheEntry.relCatEntry);
  relCacheEntry.recId.block = RELCAT_BLOCK;
  relCacheEntry.recId.slot = RELCAT_SLOTNUM_FOR_RELCAT;

  // allocate this on the heap because we want it to persist outside this
  // function
  RelCacheTable::relCache[RELCAT_RELID] =
      (struct RelCacheEntry *)malloc(sizeof(RelCacheEntry));
  *(RelCacheTable::relCache[RELCAT_RELID]) = relCacheEntry;

  /**** setting up Attribute Catalog relation in the Relation Cache Table ****/

  // set up the relation cache entry for the attribute catalog similarly
  // from the record at RELCAT_SLOTNUM_FOR_ATTRCAT

  // set the value at RelCacheTable::relCache[ATTRCAT_RELID]

  /************ Setting up Attribute cache entries ************/
  // (we need to populate attribute cache with entries for the relation catalog
  //  and attribute catalog.)

  /**** setting up Relation Catalog relation in the Attribute Cache Table ****/
  RecBuffer attrCatBlock(ATTRCAT_BLOCK);
  Attribute attrCatRecord[ATTRCAT_NO_ATTRS];

  attrCatBlock.getRecord(attrCatRecord, RELCAT_SLOTNUM_FOR_ATTRCAT);

  struct AttrCacheEntry attrCacheEntry;
  AttrCacheTable::recordToAttrCatEntry(attrCatRecord,
                                       &attrCacheEntry.attrCatEntry);
  attrCacheEntry.recId.block = ATTRCAT_BLOCK;
  attrCacheEntry.recId.slot = RELCAT_SLOTNUM_FOR_ATTRCAT;

  RelCacheTable::relCache[ATTRCAT_RELID] =
      (struct RelCacheEntry *)malloc(sizeof(RelCacheEntry));
  *(RelCacheTable::relCache[ATTRCAT_RELID]) = relCacheEntry;

  // iterate through all the attributes of the relation catalog and create a
  // linked list of AttrCacheEntry (slots 0 to 5) for each of the entries, set
  //    attrCacheEntry.recId.block = ATTRCAT_BLOCK;
  //    attrCacheEntry.recId.slot = i   (0 to 5)
  //    and attrCacheEntry.next appropriately
  // NOTE: allocate each entry dynamically using malloc
  int numberOfAttributes =RelCacheTable::relCache[RELCAT_RELID]->relCatEntry.numAttrs;
  AttrCacheEntry *current = nullptr, *head = nullptr;
  head = current = (AttrCacheEntry *)malloc(sizeof(AttrCacheEntry));
  for (int i = 0; i < numberOfAttributes; i++) {
    attrCatBlock.getRecord(attrCatRecord, i);
    AttrCacheTable::recordToAttrCatEntry(attrCatRecord, &current->attrCatEntry);
    current->recId.block = ATTRCAT_BLOCK;
    current->recId.slot = i;
    current->next = (AttrCacheEntry *)malloc(sizeof(AttrCacheEntry));
    current = current->next;
  }
  current->next = nullptr;
  AttrCacheTable::attrCache[RELCAT_RELID] = head;

  // set the next field in the last entry to nullptr

  /**** setting up Attribute Catalog relation in the Attribute Cache Table ****/

  // set up the attributes of the attribute cache similarly.
  // read slots 6-11 from attrCatBlock and initialise recId appropriately

  // set the value at AttrCacheTable::attrCache[ATTRCAT_RELID]
  numberOfAttributes=RelCacheTable::relCache[ATTRCAT_RELID]->relCatEntry.numAttrs;
    current = nullptr, head = nullptr;
    head = current = (AttrCacheEntry *)malloc(sizeof(AttrCacheEntry));
    for (int i = 6; i <11; i++) {
      attrCatBlock.getRecord(attrCatRecord, i);
      AttrCacheTable::recordToAttrCatEntry(attrCatRecord, &current->attrCatEntry);
      current->recId.block = ATTRCAT_BLOCK;
      current->recId.slot = i;
      current->next = (AttrCacheEntry *)malloc(sizeof(AttrCacheEntry));
      current = current->next;
    }
    current->next = nullptr;
    AttrCacheTable::attrCache[ATTRCAT_RELID] = head;

}

OpenRelTable::~OpenRelTable() {
  // free all the memory that you allocated in the constructor
}