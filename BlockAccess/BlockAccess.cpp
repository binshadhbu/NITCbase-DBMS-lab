#include "BlockAccess.h"
#include <cstring>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

RecId BlockAccess::linearSearch(int relId, char attrName[ATTR_SIZE],union Attribute attrVal, int op) {

  // get the previous search index of the relation relId from the relation cache
  // (use RelCacheTable::getSearchIndex() function)
  RecId prevRecId;
  RelCacheTable::getSearchIndex(relId, &prevRecId);
  // let block and slot denote the record id of the record being currently
  // checked
  int block = -1, slot = -1;
  // if the current search index record is invalid(i.e. both block and slot =
  // -1)
  if (prevRecId.block == -1 && prevRecId.slot == -1) {
    // (no hits from previous search; search should start from the
    // first record itself)
    // get the first record block of the relation from the relation cache
    // (use RelCacheTable::getRelCatEntry() function of Cache Layer)
    RelCatEntry RelCatBuf;
    RelCacheTable::getRelCatEntry(relId, &RelCatBuf);
    // block = first record block of the relation
    block = RelCatBuf.firstBlk;
    slot = 0;
    // slot = 0
  } else {
    // (there is a hit from previous search; search should start from
    // the record next to the search index record)

    // block = search index's block
    // slot = search index's slot + 1
    block = prevRecId.block;
    slot = prevRecId.slot + 1;
  }

  /* The following code searches for the next record in the relation
     that satisfies the given condition
     We start from the record id (block, slot) and iterate over the remaining
     records of the relation
  */
  RelCatEntry relCatBuffer;
  RelCacheTable::getRelCatEntry(relId, &relCatBuffer);
  while (block != -1) {
    /* create a RecBuffer object for block (use RecBuffer Constructor for
       existing block) */
    RecBuffer Buffer(block);

    HeadInfo header;
    Attribute CatRecord[RELCAT_NO_ATTRS];

    // get the record with id (block, slot) using RecBuffer::getRecord()
    Buffer.getRecord(CatRecord, slot);
    // get header of the block using RecBuffer::getHeader() function
    Buffer.getHeader(&header);
    // get slot map of the block using RecBuffer::getSlotMap() function
    unsigned char *slotMap =
        (unsigned char *)malloc(sizeof(unsigned char) * header.numSlots);
    Buffer.getSlotMap(slotMap);
    // If slot >= the number of slots per block(i.e. no more slots in this
    // block)
    // if (slot >= header.numSlots) {
    //   // update block = right block of block
    //   block = header.rblock;
    //   slot = 0;
    //   // update slot = 0
    //   continue; // continue to the beginning of this while loop
    // }

    if (slot >= relCatBuffer.numSlotsPerBlk) {
      block = header.rblock, slot = 0;
      continue; // continue to the beginning of this while loop
    }

    // if slot is free skip the loop
    // (i.e. check if slot'th entry in slot map of block contains
    // SLOT_UNOCCUPIED)
    if (slotMap[slot] == SLOT_UNOCCUPIED) {
      slot++;
      continue;
      // increment slot and continue to the next record slot
    }

    // compare record's attribute value to the the given attrVal as below:
    /*
        firstly get the attribute offset for the attrName attribute
        from the attribute cache entry of the relation using
        AttrCacheTable::getAttrCatEntry()
    */
    AttrCatEntry attrCatBuf;
    AttrCacheTable::getAttrCatEntry(relId, attrName, &attrCatBuf);

    /* use the attribute offset to get the value of the attribute from
       current record */
    Attribute *record =
        (Attribute *)malloc(sizeof(Attribute) * header.numAttrs);
    Buffer.getRecord(record, slot);
    int attrOffset = attrCatBuf.offset;

    int cmpVal = compareAttrs(
        record[attrOffset], attrVal,
        attrCatBuf
            .attrType); // will store the difference between the attributes
    // set cmpVal using compareAttrs()
    /* Next task is to check whether this record satisfies the given condition.
       It is determined based on the output of previous comparison and
       the op value received.
       The following code sets the cond variable if the condition is satisfied.
    */
    if ((op == NE && cmpVal != 0) || // if op is "not equal to"
        (op == LT && cmpVal < 0) ||  // if op is "less than"
        (op == LE && cmpVal <= 0) || // if op is "less than or equal to"
        (op == EQ && cmpVal == 0) || // if op is "equal to"
        (op == GT && cmpVal > 0) ||  // if op is "greater than"
        (op == GE && cmpVal >= 0)    // if op is "greater than or equal to"
    ) {
      /*
      set the search index in the relation cache as
      the record id of the record that satisfies the given condition
      (use RelCacheTable::setSearchIndex function)
      */
      RecId newIndex;
      newIndex.block = block;
      newIndex.slot = slot;
      RelCacheTable::setSearchIndex(relId, &newIndex);
      return RecId{block, slot};
    }
    slot++;
  }

  // no record in the relation with Id relid satisfies the given condition
  return RecId{-1, -1};
}


int BlockAccess::renameRelation(char oldName[ATTR_SIZE],char newName[ATTR_SIZE]) {
  /* reset the searchIndex of the relation catalog using
     RelCacheTable::resetSearchIndex() */
  RelCacheTable::resetSearchIndex(RELCAT_RELID);

  Attribute newRelationName; // set newRelationName with newName
  strcpy(newRelationName.sVal, newName);
  // search the relation catalog for an entry with "RelName" = newRelationName
  RecId relcatRecId = BlockAccess::linearSearch(
      RELCAT_RELID, RELCAT_ATTR_RELNAME, newRelationName, EQ);

  // If relation with name newName already exists (result of linearSearch
  //                                               is not {-1, -1})
  //    return E_RELEXIST;

  if (relcatRecId.block != -1 and relcatRecId.slot != -1) {

    return E_RELEXIST;
  }

  /* reset the searchIndex of the relation catalog using
     RelCacheTable::resetSearchIndex() */
  RelCacheTable::resetSearchIndex(RELCAT_RELID);

  Attribute oldRelationName; // set oldRelationName with oldName
  strcpy(oldRelationName.sVal, oldName);

  relcatRecId = BlockAccess::linearSearch(RELCAT_RELID, RELCAT_ATTR_RELNAME,
                                          oldRelationName, EQ);

  if (relcatRecId.block == -1 and relcatRecId.slot == -1) {

    return E_RELNOTEXIST;
  }

  // search the relation catalog for an entry with "RelName" = oldRelationName

  // If relation with name oldName does not exist (result of linearSearch is
  // {-1, -1})
  //    return E_RELNOTEXIST;

  /* get the relation catalog record of the relation to rename using a RecBuffer
     on the relation catalog [RELCAT_BLOCK] and RecBuffer.getRecord function
  */
  RecBuffer Buffer(relcatRecId.block);
  Attribute CatRecord[RELCAT_NO_ATTRS];
  Buffer.getRecord(CatRecord, relcatRecId.slot);
  strcpy(CatRecord[RELCAT_REL_NAME_INDEX].sVal, newName);
  /* update the relation name attribute in the record with newName.
     (use RELCAT_REL_NAME_INDEX) */
  // set back the record value using RecBuffer.setRecord
  Buffer.setRecord(CatRecord, relcatRecId.slot);

  /*TODO::update all the attribute catalog entries in the attribute catalog
  corresponding to the relation with relation name oldName to the relation name
  newName
  */

  RelCacheTable::resetSearchIndex(ATTRCAT_RELID);
  /* reset the searchIndex of the attribute catalog using
     RelCacheTable::resetSearchIndex() */
  for (int i = 0; i < CatRecord[RELCAT_NO_ATTRIBUTES_INDEX].nVal; i++) {
    relcatRecId = BlockAccess::linearSearch(ATTRCAT_RELID, ATTRCAT_ATTR_RELNAME,
                                            oldRelationName, EQ);
    RecBuffer attrCatBlock(relcatRecId.block);
    Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
    attrCatBlock.getRecord(attrCatRecord, relcatRecId.slot);
    strcpy(attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal, newName);
    attrCatBlock.setRecord(attrCatRecord, relcatRecId.slot);
  }
  // for i = 0 to numberOfAttributes :
  //    linearSearch on the attribute catalog for relName = oldRelationName
  //    get the record using RecBuffer.getRecord
  //
  //    update the relName field in the record to newName
  //    set back the record using RecBuffer.setRecord

  return SUCCESS;
}


int BlockAccess::renameAttribute(char relName[ATTR_SIZE],
                                 char oldName[ATTR_SIZE],
                                 char newName[ATTR_SIZE]) {

  /* reset the searchIndex of the relation catalog using
     RelCacheTable::resetSearchIndex() */
  RelCacheTable::resetSearchIndex(RELCAT_RELID);

  Attribute relNameAttr; // set relNameAttr to relName
  strcpy(relNameAttr.sVal, relName);
  // Search for the relation with name relName in relation catalog using
  // linearSearch()
  RecId relcatRecId = BlockAccess::linearSearch(
      RELCAT_RELID, RELCAT_ATTR_RELNAME, relNameAttr, EQ);
  // If relation with name relName does not exist (search returns {-1,-1})
  //    return E_RELNOTEXIST;
  if (relcatRecId.block == -1 and relcatRecId.slot == -1)
    return E_RELNOTEXIST;

  /* reset the searchIndex of the attribute catalog using
     RelCacheTable::resetSearchIndex() */
  RelCacheTable::resetSearchIndex(ATTRCAT_RELID);

  /* declare variable attrToRenameRecId used to store the attr-cat recId
  of the attribute to rename */
  RecId attrToRenameRecId{-1, -1};
  Attribute attrCatEntryRecord[ATTRCAT_NO_ATTRS];

  /* iterate over all Attribute Catalog Entry record corresponding to the
     relation to find the required attribute */
  while (true) {
    RecId searchIndex = BlockAccess::linearSearch(
        ATTRCAT_RELID, ATTRCAT_ATTR_RELNAME, relNameAttr, EQ);
    // linear search on the attribute catalog for RelName = relNameAttr
    if (searchIndex.block == -1 and searchIndex.slot == -1)
      break;
    // if there are no more attributes left to check (linearSearch returned
    // {-1,-1})
    //     break;
    RecBuffer attrCatBlock(searchIndex.block);
    attrCatBlock.getRecord(attrCatEntryRecord, searchIndex.slot);

    /* Get the record from the attribute catalog using RecBuffer.getRecord
      into attrCatEntryRecord */
      //todo::be careful
    if (strcmp(attrCatEntryRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, oldName) == 0) {
      attrToRenameRecId = searchIndex;
      break;
    }

    if (strcmp(attrCatEntryRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, newName) == 0){
			return E_ATTREXIST;
		}
    // if attrCatEntryRecord.attrName = oldName
    //     attrToRenameRecId = block and slot of this record

    // if attrCatEntryRecord.attrName = newName
    //     return E_ATTREXIST;
  }

  // if attrToRenameRecId == {-1, -1}
  //     return E_ATTRNOTEXIST;
  if(attrToRenameRecId.slot==-1 and attrToRenameRecId.block==-1){
    return E_ATTRNOTEXIST;
  }
  RecBuffer attrCatBlock(attrToRenameRecId.block);
  Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
  attrCatBlock.getRecord(attrCatRecord,attrToRenameRecId.slot);
  strcpy(attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal,newName);
  attrCatBlock.setRecord(attrCatRecord,attrToRenameRecId.slot);
  // Update the entry corresponding to the attribute in the Attribute Catalog
  // Relation.
  /*   declare a RecBuffer for attrToRenameRecId.block and get the record at
       attrToRenameRecId.slot */
  //   update the AttrName of the record with newName
  //   set back the record with RecBuffer.setRecord

  return SUCCESS;
}