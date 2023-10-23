#include "AttrCacheTable.h"

#include <cstring>
#include <stdio.h>

AttrCacheEntry *AttrCacheTable::attrCache[MAX_OPEN];

/* returns the attrOffset-th attribute for the relation corresponding to relId
NOTE: this function expects the caller to allocate memory for `*attrCatBuf`
*/
int AttrCacheTable::getAttrCatEntry(int relId, int attrOffset,
                                    AttrCatEntry *attrCatBuf) {
  // check if 0 <= relId < MAX_OPEN and return E_OUTOFBOUND otherwise
  if (relId < 0 || relId >= MAX_OPEN) {
    return E_OUTOFBOUND;
  }

  // check if attrCache[relId] == nullptr and return E_RELNOTOPEN if true
  if (attrCache[relId] == nullptr) {
    return E_RELNOTOPEN;
  }
  // traverse the linked list of attribute cache entries

  for (AttrCacheEntry *entry = attrCache[relId]; entry != nullptr;
       entry = entry->next) {
    if (entry->attrCatEntry.offset == attrOffset) {
      // copy entry->attrCatEntry to *attrCatBuf and return SUCCESS;
      *attrCatBuf = entry->attrCatEntry;
    }
  }

  // there is no attribute at this offset
  return E_ATTRNOTEXIST;
}

/* Converts a attribute catalog record to AttrCatEntry struct
    We get the record as Attribute[] from the BlockBuffer.getRecord() function.
    This function will convert that to a struct AttrCatEntry type.
*/

/* returns the attribute with name `attrName` for the relation corresponding to
relId NOTE: this function expects the caller to allocate memory for
`*attrCatBuf`
*/
int AttrCacheTable::getAttrCatEntry(int relId, char attrName[ATTR_SIZE],
                                    AttrCatEntry *attrCatBuf) {
  // check that relId is valid and corresponds to an open relation
  if (relId < 0 || relId >= MAX_OPEN) {
    return E_OUTOFBOUND;
  }

  // check if attrCache[relId] == nullptr and return E_RELNOTOPEN if true
  if (attrCache[relId] == nullptr) {
    return E_RELNOTOPEN;
  }
  // for (AttrCacheEntry *entry = attrCache[relId]; entry != nullptr;entry =
  // entry->next) {
  //   if (strcmp(entry->attrCatEntry.attrName, attrName) == 0) {
  //     strcpy(attrCatBuf->relName, entry->attrCatEntry.relName);
  //     strcpy(attrCatBuf->attrName, entry->attrCatEntry.attrName);
  //     attrCatBuf->attrType = entry->attrCatEntry.attrType;
  //     attrCatBuf->offset = entry->attrCatEntry.offset;
  //     attrCatBuf->primaryFlag = entry->attrCatEntry.primaryFlag;
  //     attrCatBuf->rootBlock = entry->attrCatEntry.rootBlock;
  //     return SUCCESS;
  //   }
  // }

  for (AttrCacheEntry *entry = attrCache[relId]; entry != nullptr;
       entry = entry->next) {

    if (strcmp(entry->attrCatEntry.attrName, attrName) == 0) {
      strcpy(attrCatBuf->relName, entry->attrCatEntry.relName);
      strcpy(attrCatBuf->attrName, entry->attrCatEntry.attrName);
      attrCatBuf->attrType = entry->attrCatEntry.attrType;
      attrCatBuf->offset = entry->attrCatEntry.offset;
      attrCatBuf->primaryFlag = entry->attrCatEntry.primaryFlag;
      attrCatBuf->rootBlock = entry->attrCatEntry.rootBlock;
      return SUCCESS;
    }
  }
  // iterate over the entries in the attribute cache and set attrCatBuf to the
  // entry that
  //    matches attrName

  // no attribute with name attrName for the relation
  return E_ATTRNOTEXIST;
}

void AttrCacheTable::recordToAttrCatEntry(
    union Attribute record[ATTRCAT_NO_ATTRS], AttrCatEntry *attrCatEntry) {
  strcpy(attrCatEntry->relName, record[ATTRCAT_REL_NAME_INDEX].sVal);
  strcpy(attrCatEntry->attrName, record[ATTRCAT_ATTR_NAME_INDEX].sVal);
  attrCatEntry->offset = (int)record[ATTRCAT_OFFSET_INDEX].nVal;
  attrCatEntry->attrType = (int)record[ATTRCAT_ATTR_TYPE_INDEX].nVal;
  attrCatEntry->rootBlock = (int)record[ATTRCAT_ROOT_BLOCK_INDEX].nVal;
  attrCatEntry->primaryFlag = (bool)record[ATTRCAT_PRIMARY_FLAG_INDEX].nVal;

  // copy the rest of the fields in the record to the attrCacheEntry struct
}

void AttrCacheTable::attrCatEntryToRecord(AttrCatEntry *attrCatEntry,
                                          Attribute record[ATTRCAT_NO_ATTRS]) {
  strcpy(record[ATTRCAT_REL_NAME_INDEX].sVal, attrCatEntry->relName);
  strcpy(record[ATTRCAT_ATTR_NAME_INDEX].sVal, attrCatEntry->attrName);

  record[ATTRCAT_ATTR_TYPE_INDEX].nVal = attrCatEntry->attrType;
  record[ATTRCAT_PRIMARY_FLAG_INDEX].nVal = attrCatEntry->primaryFlag;
  record[ATTRCAT_ROOT_BLOCK_INDEX].nVal = attrCatEntry->rootBlock;
  record[ATTRCAT_OFFSET_INDEX].nVal = attrCatEntry->offset;

  // copy the rest of the fields in the record to the attrCacheEntry struct
}

int AttrCacheTable:: resetSearchIndex(int relId, char attrName[ATTR_SIZE]) {
  IndexId index;
  index.block = -1;
  index.index = -1;
  AttrCacheTable::setSearchIndex(relId,attrName, &index);
}

int  AttrCacheTable:: setSearchIndex(int relId, char attrName[ATTR_SIZE], IndexId *searchIndex){
  if(relId<0 or relId>=MAX_OPEN)return E_OUTOFBOUND;
  
  if(AttrCacheTable::attrCache[relId]==nullptr){
    return E_RELNOTOPEN;
  }

  AttrCacheEntry *curr=AttrCacheTable::attrCache[relId];
  while(  curr){
    if(strcmp(curr->attrCatEntry.attrName,attrName)==0){
      curr->searchIndex=*searchIndex;
      return SUCCESS;
    }
    curr=curr->next;
  }
  return E_ATTRNOTEXIST;
}