#include "BPlusTree.h"
#include <cstring>
#include <iostream>

RecId BPlusTree::bPlusSearch(int relId, char attrName[ATTR_SIZE],
                             Attribute attrVal, int op) {
  // declare searchIndex which will be used to store search index for attrName.
  IndexId searchIndex;
  int ret = AttrCacheTable::getSearchIndex(relId, attrName, &searchIndex);
  if (ret != SUCCESS) {
    printf("search index not available");
  }

  AttrCatEntry attrCatEntry;
  ret = AttrCacheTable::getAttrCatEntry(relId, attrName, &attrCatEntry);
  if (ret != SUCCESS) {
    printf("cant get the attrCatEntry");
  }

  int block = -1, index = -1;

  if (searchIndex.block == -1 and searchIndex.index == -1) {
    // (search is done for the first time)

    // start the search from the first entry of root.
    block = attrCatEntry.rootBlock;
    index = 0;

    if (block == -1) {
      return RecId{-1, -1};
    }

  } else {
    /*a valid searchIndex points to an entry in the leaf index of the
    attribute's B+ Tree which had previously satisfied the op for the given
    attrVal.*/

    block = searchIndex.block;
    index = searchIndex.index + 1; // search is resumed from the next index.

    // load block into leaf using IndLeaf::IndLeaf().
    IndLeaf leaf(block);

    // declare leafHead which will be used to hold the header of leaf.
    HeadInfo leafHead;

    leaf.getHeader(&leafHead);

    // load header into leafHead using BlockBuffer::getHeader().

    if (index >= leafHead.numEntries) {
      /* (all the entries in the block has been searched; search from the
      beginning of the next leaf index block. */

      // update block to rblock of current block and index to 0.
      block = leafHead.rblock;
      index = 0;

      if (block == -1) {
        // (end of linked list reached - the search is done.)
        return RecId{-1, -1};
      }
    }
  }

  /******  Traverse through all the internal nodes according to value
           of attrVal and the operator op                             ******/

  /* (This section is only needed when
      - search restarts from the root block (when searchIndex is reset by
     caller)
      - root is not a leaf
      If there was a valid search index, then we are already at a leaf block
      and the test condition in the following loop will fail)
  */

  while (StaticBuffer::getStaticBlockType(block) == IND_INTERNAL) { // use
    // StaticBuffer::getStaticBlockType()

    // load the block into internalBlk using IndInternal::IndInternal().
    IndInternal internalBlk(block);

    HeadInfo intHead;
    internalBlk.getHeader(&intHead);

    // load the header of internalBlk into intHead using
    // BlockBuffer::getHeader()

    // declare intEntry which will be used to store an entry of internalBlk.
    InternalEntry intEntry;

    if (op == NE or op == LT or op == LE) {
      /*
      - NE: need to search the entire linked list of leaf indices of the B+
      Tree, starting from the leftmost leaf index. Thus, always move to the
      left.

      - LT and LE: the attribute values are arranged in ascending order in the
      leaf indices of the B+ Tree. Values that satisfy these conditions, if
      any exist, will always be found in the left-most leaf index. Thus,
      always move to the left.
      */

      // load entry in the first slot of the block into intEntry
      // using IndInternal::getEntry().
      internalBlk.getEntry(&intEntry, 0);
      block = intEntry.lChild;

    } else {
      /*
      - EQ, GT and GE: move to the left child of the first entry that is
      greater than (or equal to) attrVal
      (we are trying to find the first entry that satisfies the condition.
      since the values are in ascending order we move to the left child which
      might contain more entries that satisfy the condition)
      */

      /*
       traverse through all entries of internalBlk and find an entry that
       satisfies the condition.
       if op == EQ or GE, then intEntry.attrVal >= attrVal
       if op == GT, then intEntry.attrVal > attrVal
       Hint: the helper function compareAttrs() can be used for comparing
      */
      index = 0;
      bool found = false;
      while (index < intHead.numEntries) {
        ret = internalBlk.getEntry(&intEntry, index);
        int cmpVal = compareAttrs(intEntry.attrVal, attrVal, NUMBER);
        if ((op == EQ and cmpVal == 0) or (op == GE and cmpVal >= 0) or
            (op == GT and cmpVal > 0)) {
          found = true;
          break;
        }
        index++;
      }

      if (found) {
        // move to the left child of that entry
        block = intEntry.lChild;

      } else {
        // move to the right child of the last entry of the block
        // i.e numEntries - 1 th entry of the block

        block = intEntry.rChild; // right child of last entry
      }
    }
  }

  // todo: `block` now has the block number of a leaf index block.

  /******  Identify the first leaf index entry from the current position
              that satisfies our condition (moving right)             ******/

  while (block != -1) {
    // load the block into leafBlk using IndLeaf::IndLeaf().
    IndLeaf leafBlk(block);
    HeadInfo leafHead;
    leafBlk.getHeader(&leafHead);
    // load the header to leafHead using BlockBuffer::getHeader().

    // declare leafEntry which will be used to store an entry from leafBlk
    Index leafEntry;

    while (index < leafHead.numEntries) {

      // load entry corresponding to block and index into leafEntry
      // using IndLeaf::getEntry().
      leafBlk.getEntry(&leafEntry, index);

      int cmpVal = compareAttrs(leafEntry.attrVal, attrVal, NUMBER);
      /* comparison between leafEntry's attribute value
                     and input attrVal using compareAttrs()*/

      if ((op == EQ && cmpVal == 0) || (op == LE && cmpVal <= 0) ||
          (op == LT && cmpVal < 0) || (op == GT && cmpVal > 0) ||
          (op == GE && cmpVal >= 0) || (op == NE && cmpVal != 0)) {
        // (entry satisfying the condition found)
        searchIndex.block = block;
        searchIndex.index = index;
        AttrCacheTable::setSearchIndex(relId, attrName, &searchIndex);

        // set search index to {block, index}
        return RecId{leafEntry.block, leafEntry.slot};

        // return the recId {leafEntry.block, leafEntry.slot}.
      } else if ((op == EQ || op == LE || op == LT) && cmpVal > 0) {
        //*future entries will not satisfy EQ, LE, LT since the values
        //* are arranged in ascending order in the leaves */
        return RecId{-1, -1};

        // return RecId {-1, -1};
      }

      // search next index.
      ++index;
    }

    /*only for NE operation do we have to check the entire linked list;
    for all the other op it is guaranteed that the block being searched
    will have an entry, if it exists, satisying that op. */
    if (op != NE) {
      break;
    }
    block = leafHead.rblock;
    index = 0;

    // block = next block in the linked list, i.e., the rblock in leafHead.
    // update index to 0.
  }
  return RecId{-1, -1};

  // no entry satisying the op was found; return the recId {-1,-1}
}

int BPlusTree::bPlusCreate(int relId, char attrName[ATTR_SIZE]) {

  // if relId is either RELCAT_RELID or ATTRCAT_RELID:
  //     return E_NOTPERMITTED;
  if (RELCAT_RELID == relId or relId == ATTRCAT_RELID) {
    return E_NOTPERMITTED;
  }

  // get the attribute catalog entry of attribute `attrName`
  // using AttrCacheTable::getAttrCatEntry()
  AttrCatEntry *attrCatBuffer;
  int ret = AttrCacheTable::getAttrCatEntry(relId, attrName, attrCatBuffer);

  // if getAttrCatEntry fails
  //     return the error code from getAttrCatEntry
  if (ret != SUCCESS) {
    return ret;
  }

  if (attrCatBuffer->rootBlock != -1) {
    return SUCCESS;
  }

  /******Creating a new B+ Tree ******/

  // get a free leaf block using constructor 1 to allocate a new block
  IndLeaf rootBlockBuf;

  // (if the block could not be allocated, the appropriate error code
  //  will be stored in the blockNum member field of the object)

  // declare rootBlock to store the blockNumber of the new leaf block
  int rootBlock = rootBlockBuf.getBlockNum();

  // if there is no more disk space for creating an index
  if (rootBlock == E_DISKFULL) {
    return E_DISKFULL;
  }

  RelCatEntry relCatEntry;
  RelCacheTable::getRelCatEntry(relId, &relCatEntry);

  // load the relation catalog entry into relCatEntry
  // using RelCacheTable::getRelCatEntry().

  int block = relCatEntry.firstBlk;
  /* first record block of the relation */;

  /***** Traverse all the blocks in the relation and insert them one
         by one into the B+ Tree *****/
  while (block != -1) {

    // declare a RecBuffer object for `block` (using appropriate constructor)
    RecBuffer recBuffer(block);

    unsigned char slotMap[relCatEntry.numSlotsPerBlk];
    recBuffer.getSlotMap(slotMap);

    // load the slot map into slotMap using RecBuffer::getSlotMap().
    for (int slot = 0; slot < relCatEntry.numSlotsPerBlk; slot++) {
      if (slotMap[slot] == SLOT_OCCUPIED) {

        Attribute record[relCatEntry.numAttrs];
        recBuffer.getRecord(record, slot);
        // load the record corresponding to the slot into `record`
        // using RecBuffer::getRecord().
        Attribute attrVal;

        // declare recId and store the rec-id of this record in it
        // RecId recId{block, slot};
        RecId recId{block, slot};
        ret = BPlusTree::bPlusInsert(relId, attrName,
                                     record[attrCatBuffer->offset], recId);

        // insert the attribute value corresponding to attrName from the record
        // into the B+ tree using bPlusInsert.
        // (note that bPlusInsert will destroy any existing bplus tree if
        // insert fails i.e when disk is full)
        // retVal = bPlusInsert(relId, attrName, attribute value, recId);
        if (ret == E_DISKFULL) {
          return E_DISKFULL;
        }
      }
    }
    HeadInfo header;
    recBuffer.getHeader(&header);
    header.rblock = block;
    // get the header of the block using BlockBuffer::getHeader()

    // set block = rblock of current block (from the header)
  }

  return SUCCESS;
}



int BPlusTree::bPlusDestroy(int rootBlockNum) {
  if (rootBlockNum < 0 or rootBlockNum >= DISK_BLOCKS) {
    return E_OUTOFBOUND;
  }

  int type = StaticBuffer::getStaticBlockType(rootBlockNum);
  /* type of block (using StaticBuffer::getStaticBlockType())*/;

  if (type == IND_LEAF) {
    // declare an instance of IndLeaf for rootBlockNum using appropriate
    // constructor
    IndLeaf leafBlock(rootBlockNum);
    leafBlock.releaseBlock();

    // release the block using BlockBuffer::releaseBlock().

    return SUCCESS;

  } else if (type == IND_INTERNAL) {
    // declare an instance of IndInternal for rootBlockNum using appropriate
    // constructor
    IndInternal internalBlock(rootBlockNum);
    HeadInfo header;
    internalBlock.getHeader(&header);

    // load the header of the block using BlockBuffer::getHeader().
    InternalEntry blockEntry;
    internalBlock.getEntry(&blockEntry, 0);
    BPlusTree::bPlusDestroy(blockEntry.lChild);
    for (int i = 0; i < header.numEntries; i++) {
      internalBlock.getEntry(&blockEntry, i);
      BPlusTree::bPlusDestroy(blockEntry.rChild);
    }

    internalBlock.releaseBlock();

    /*iterate through all the entries of the internalBlk and destroy the lChild
    of the first entry and rChild of all entries using
    BPlusTree::bPlusDestroy(). (the rchild of an entry is the same as the lchild
    of the next entry. take care not to delete overlapping children more than
    once ) */

    // release the block using BlockBuffer::releaseBlock().

    return SUCCESS;

  } else {
    // (block is not an index block.)
    return E_INVALIDBLOCK;
  }
}

int BPlusTree::bPlusInsert(int relId, char attrName[ATTR_SIZE],Attribute attrVal, RecId recId) {
  // get the attribute cache entry corresponding to attrName
  // using AttrCacheTable::getAttrCatEntry().
  AttrCatEntry attrCatEntry;
  int ret = AttrCacheTable::getAttrCatEntry(relId, attrName, &attrCatEntry);
  if (ret != SUCCESS) {
    return ret;
  }
  // if getAttrCatEntry() failed
  //     return the error code

  int blockNum = attrCatEntry.rootBlock;
  /* rootBlock of B+ Tree (from attrCatEntry) */;

  if (blockNum == -1) {
    return E_NOINDEX;
  }

  // find the leaf block to which insertion is to be done using the
  // findLeafToInsert() function

  int leafBlkNum =findLeafToInsert(blockNum,attrVal,attrCatEntry.attrType);
   /* findLeafToInsert(root block num, attrVal, attribute type)
                    */
      // insert the attrVal and recId to the leaf block at blockNum using the
      // insertIntoLeaf() function.
      // declare a struct Index with attrVal = attrVal, block = recId.block and
      // slot = recId.slot to pass as argument to the function.
      Index entry;
      entry.attrVal = attrVal;
      entry.block = recId.block;
      entry.slot = recId.slot;
       ret=insertIntoLeaf(relId,attrName,leafBlkNum,entry);
      // insertIntoLeaf(relId, attrName, leafBlkNum, Index entry)
      // NOTE: the insertIntoLeaf() function will propagate the insertion to the
      //       required internal nodes by calling the required helper functions
      //       like insertIntoInternal() or createNewRoot()

      if (ret==E_DISKFULL) {
    // destroy the existing B+ tree by passing the rootBlock to bPlusDestroy().
    bPlusDestroy(blockNum);
    attrCatEntry.rootBlock=-1;
    AttrCacheTable::setAttrCatEntry(relId,attrName,&attrCatEntry);
    // update the rootBlock of attribute catalog cache entry to -1 using
    // AttrCacheTable::setAttrCatEntry().
    return E_DISKFULL;
  }

  return SUCCESS;
}

//! Used to find the leaf index block to which an attribute would be inserted to
//! in the B+ insertion process.
//! If this leaf turns out to be full, the caller will need to handle the
//! splitting of this block to insert the entry.

int BPlusTree::findLeafToInsert(int rootBlock, Attribute attrVal,
                                int attrType) {
  int blockNum = rootBlock;

  while (StaticBuffer::getStaticBlockType(blockNum) !=
         IND_LEAF) { // use StaticBuffer::getStaticBlockType()

    // declare an IndInternal object for block using appropriate constructor
    IndInternal internalBlock(blockNum);
    HeadInfo header;

    internalBlock.getHeader(&header);

    // get header of the block using BlockBuffer::getHeader()
    InternalEntry entry;
    int index = 0;
    while (index < header.numEntries) {
      internalBlock.getEntry(&entry, index);
      if (compareAttrs(attrVal, entry.attrVal, attrType) <= 0) {
        break;
      }
      index++;
    }

    /* iterate through all the entries, to find the first entry whose
         attribute value >= value to be inserted.
         NOTE: the helper function compareAttrs() declared in BlockBuffer.h
               can be used to compare two Attribute values. */
    if (index == header.numEntries) {
      internalBlock.getEntry(&entry, header.numEntries - 1);
      blockNum = entry.rChild;
    } else {
      internalBlock.getEntry(&entry, index);
      blockNum = entry.lChild;
    }

    // if (/*no such entry is found*/) {
    //     // set blockNum = rChild of (nEntries-1)'th entry of the block
    //     // (i.e. rightmost child of the block)

    // } else {
    //     // set blockNum = lChild of the entry that was found
    // }
  }

  return blockNum;
}

//! insert an index entry into a leaf index block of an existing B+ tree
//! If the leaf is full and requires splitting, this function will call other B+
//! Tree Layer functions to handle any updation required to the parent internal
//! index blocks of the B+ tree.

int BPlusTree::insertIntoLeaf(int relId, char attrName[ATTR_SIZE], int blockNum,
                              Index indexEntry) {
  // get the attribute cache entry corresponding to attrName
  // using AttrCacheTable::getAttrCatEntry().
  AttrCatEntry attrCatEntry;
  AttrCacheTable::getAttrCatEntry(relId, attrName, &attrCatEntry);

  // declare an IndLeaf instance for the block using appropriate constructor
  IndLeaf leafBlock(blockNum);
  HeadInfo blockHeader;
  leafBlock.getHeader(&blockHeader);
  // store the header of the leaf index block into blockHeader
  // using BlockBuffer::getHeader()

  // the following variable will be used to store a list of index entries with
  // existing indices + the new index to insert
  Index indices[blockHeader.numEntries + 1];
  bool insert = false;
  for (int index = 0; index < blockHeader.numEntries; index++) {
    Index entry;
    leafBlock.getEntry(&entry, index);
    if (compareAttrs(entry.attrVal, indexEntry.attrVal,
                     attrCatEntry.attrType) <= 0) {
      indices[index] = entry;
    } else {
      indices[index] = indexEntry;
      insert = true;
      for (index++; index <= blockHeader.numEntries; index++) {
        leafBlock.getEntry(&entry, index - 1);
        indices[index] = entry;
      }
      break;
    }
  }
  /*
  Iterate through all the entries in the block and copy them to the array
  indices. Also insert `indexEntry` at appropriate position in the indices array
  maintaining the ascending order.
  - use IndLeaf::getEntry() to get the entry
  - use compareAttrs() declared in BlockBuffer.h to compare two Attribute
  structs
  */
  if (insert == false) {
    indices[blockHeader.numEntries] = indexEntry;
  }

  if (blockHeader.numEntries < MAX_KEYS_LEAF) {
    // (leaf block has not reached max limit)

    // increment blockHeader.numEntries and update the header of block
    // using BlockBuffer::setHeader().
    blockHeader.numEntries++;
    leafBlock.setHeader(&blockHeader);
    for (int index = 0; index < blockHeader.numEntries; index) {
      leafBlock.setEntry(&indices[index], index);
    }

    // iterate through all the entries of the array `indices` and populate the
    // entries of block with them using IndLeaf::setEntry().

    return SUCCESS;
  }

  // If we reached here, the `indices` array has more than entries than can fit
  // in a single leaf index block. Therefore, we will need to split the entries
  // in `indices` between two leaf blocks. We do this using the splitLeaf()
  // function. This function will return the blockNum of the newly allocated
  // block or E_DISKFULL if there are no more blocks to be allocated.

  int newRightBlk = splitLeaf(blockNum, indices);

  // if splitLeaf() returned E_DISKFULL
  //     return E_DISKFULL
  if (newRightBlk == E_DISKFULL)
    return newRightBlk;

  if (blockHeader.pblock !=
      -1 /* the current leaf block was not the root */) { // check pblock in
                                                          // header
    // insert the middle value from `indices` into the parent block using the
    // insertIntoInternal() function. (i.e the last value of the left block)

    // the middle value will be at index 31 (given by constant
    // MIDDLE_INDEX_LEAF)
    InternalEntry middleEntry;
    middleEntry.attrVal = indices[MIDDLE_INDEX_LEAF].attrVal;
    middleEntry.lChild = blockNum;
    middleEntry.rChild = newRightBlk;
    return insertIntoInternal(relId, attrName, blockHeader.pblock, middleEntry);

    // create a struct InternalEntry with attrVal =
    // indices[MIDDLE_INDEX_LEAF].attrVal, lChild = currentBlock, rChild =
    // newRightBlk and pass it as argument to the insertIntoInternalFunction as
    // follows

    // insertIntoInternal(relId, attrName, parent of current block, new internal
    // entry)

  } else {
    // the current block was the root block and is now split. a new internal
    // index block needs to be allocated and made the root of the tree. To do
    // this, call the createNewRoot() function with the following arguments
    int ret = createNewRoot(relId, attrName, indices[MIDDLE_INDEX_LEAF].attrVal,
                            blockNum, newRightBlk);
    if (ret != SUCCESS) {
      printf("cant add attribute when block is root and to be splitted");
      return ret;
    }

    // createNewRoot(relId, attrName, indices[MIDDLE_INDEX_LEAF].attrVal,
    //               current block, new right block)
  }
  return SUCCESS;
  // if either of the above calls returned an error (E_DISKFULL), then return
  // that else return SUCCESS
}

//! Distributes an array of index entries between an existing leaf index block
//! and a newly allocated leaf index block.
//*leafBlockNum corresponds to a fully filled leaf index block in a B+ tree
int BPlusTree::splitLeaf(int leafBlockNum, Index indices[]) {
  // declare rightBlk, an instance of IndLeaf using constructor 1 to obtain new
  // leaf index block that will be used as the right block in the splitting
  IndLeaf rightBlock;
  IndLeaf leftBlock(leafBlockNum);

  // declare leftBlk, an instance of IndLeaf using constructor 2 to read from
  // the existing leaf block

  int rightBlkNum = rightBlock.getBlockNum();
  /* block num of right blk */;
  int leftBlkNum = leftBlock.getBlockNum(); /* block num of left blk */
  ;

  if (rightBlkNum == E_DISKFULL) {
    //(failed to obtain a new leaf index block because the disk is full)
    return E_DISKFULL;
  }

  HeadInfo leftBlkHeader, rightBlkHeader;
  rightBlock.getHeader(&rightBlkHeader);
  leftBlock.getHeader(&leftBlkHeader);
  // get the headers of left block and right block using
  // BlockBuffer::getHeader()

  // set rightBlkHeader with the following values
  // - number of entries = (MAX_KEYS_LEAF+1)/2 = 32,
  // - pblock = pblock of leftBlk
  // - lblock = leftBlkNum
  // - rblock = rblock of leftBlk
  // and update the header of rightBlk using BlockBuffer::setHeader()
  rightBlkHeader.numEntries = (MAX_KEYS_LEAF + 1) / 2;
  rightBlkHeader.pblock = leftBlkHeader.pblock;
  rightBlkHeader.rblock = leftBlkHeader.rblock;
  rightBlkHeader.lblock = leftBlkNum;
  rightBlock.setHeader(&rightBlkHeader);

  leftBlkHeader.numEntries = (MAX_KEYS_LEAF + 1) / 2;
  leftBlkHeader.rblock = rightBlkNum;
  leftBlock.setHeader(&leftBlkHeader);
  // set leftBlkHeader with the following values
  // - number of entries = (MAX_KEYS_LEAF+1)/2 = 32
  // - rblock = rightBlkNum
  // and update the header of leftBlk using BlockBuffer::setHeader() */

  // set the first 32 entries of leftBlk = the first 32 entries of indices array
  // and set the first 32 entries of newRightBlk = the next 32 entries of
  for (int index = 0; index < 32; index++) {
    leftBlock.setEntry(&indices[index], index);
    rightBlock.setEntry(&indices[index + MIDDLE_INDEX_LEAF + 1], index);
  }

  // indices array using IndLeaf::setEntry().

  return rightBlkNum;
}

//! Used to insert an index entry into an internal index block of an existing B+
//! tree. This function will call itself to handle any updation required to it's
//! parent internal index blocks.

int BPlusTree::insertIntoInternal(int relId, char attrName[ATTR_SIZE],
                                  int intBlockNum, InternalEntry intEntry) {
  // get the attribute cache entry corresponding to attrName
  // using AttrCacheTable::getAttrCatEntry().
  AttrCatEntry attrCatEntry;
  AttrCacheTable::getAttrCatEntry(relId, attrName, &attrCatEntry);

  // declare intBlk, an instance of IndInternal using constructor 2 for the
  // block corresponding to intBlockNum
  IndInternal intBlk(intBlockNum);
  HeadInfo blockHeader;
  intBlk.getHeader(&blockHeader);
  // load blockHeader with header of intBlk using BlockBuffer::getHeader().

  // declare internalEntries to store all existing entries + the new entry
  InternalEntry internalEntries[blockHeader.numEntries + 1];
  int insertedIndex = -1;
  for (int index = 0; index < blockHeader.numEntries; index++) {
    InternalEntry entry;
    intBlk.getEntry(&entry, index);
    if (compareAttrs(entry.attrVal, intEntry.attrVal, attrCatEntry.attrType) <=
        0) {
      internalEntries[index] = entry;
    } else {

      internalEntries[index] = intEntry;
      insertedIndex = index;
      for (index++; index <= blockHeader.numEntries; index++) {
        intBlk.getEntry(&entry, index - 1);
        internalEntries[index] = entry;
      }

      break;
    }
  }

  if (insertedIndex == -1) {
    internalEntries[blockHeader.numEntries] = intEntry;
    insertedIndex = blockHeader.numEntries;
  }
  if (insertedIndex > 0) {
    internalEntries[insertedIndex - 1].rChild = intEntry.lChild;
  }

  if (insertedIndex < blockHeader.numEntries) {
    internalEntries[insertedIndex].lChild = intEntry.rChild;
  }

  /*
  Iterate through all the entries in the block and copy them to the array
  `internalEntries`. Insert `indexEntry` at appropriate position in the
  array maintaining the ascending order.
      - use IndInternal::getEntry() to get the entry
      - use compareAttrs() to compare two structs of type Attribute

  Update the lChild of the internalEntry immediately following the newly added
  entry to the rChild of the newly added entry.
  */

  if (blockHeader.numEntries < MAX_KEYS_INTERNAL) {
    // (internal index block has not reached max limit)
    blockHeader.numEntries++;
    intBlk.setHeader(&blockHeader);

    // increment blockheader.numEntries and update the header of intBlk
    // using BlockBuffer::setHeader().

    // iterate through all entries in internalEntries array and populate the
    // entries of intBlk with them using IndInternal::setEntry().
    for (int index = 0; index < blockHeader.numEntries; index++) {
      intBlk.setEntry(&internalEntries[index], index);
    }

    return SUCCESS;
  }

  // If we reached here, the `internalEntries` array has more than entries than
  // can fit in a single internal index block. Therefore, we will need to split
  // the entries in `internalEntries` between two internal index blocks. We do
  //! this using the splitInternal() function.
  //! This function will return the blockNum of the newly allocated block or
  //! E_DISKFULL if there are no more blocks to be allocated.

  int newRightBlk = splitInternal(intBlockNum, internalEntries);

  if (newRightBlk == E_DISKFULL) {
    bPlusDestroy(intEntry.rChild);

    // Using bPlusDestroy(), destroy the right subtree, rooted at
    // intEntry.rChild. This corresponds to the tree built up till now that has
    // not yet been connected to the existing B+ Tree

    return E_DISKFULL;
  }

  if (blockHeader.pblock != -1) { // (check pblock in header)
    // insert the middle value from `internalEntries` into the parent block
    // using the insertIntoInternal() function (recursively).

    // the middle value will be at index 50 (given by constant
    // MIDDLE_INDEX_INTERNAL)
    InternalEntry middleEntry;
    middleEntry.attrVal = internalEntries[MIDDLE_INDEX_INTERNAL].attrVal;
    middleEntry.rChild = newRightBlk;
    middleEntry.lChild = intBlockNum;

    // create a struct InternalEntry with lChild = current block, rChild =
    // newRightBlk and attrVal = internalEntries[MIDDLE_INDEX_INTERNAL].attrVal
    // and pass it as argument to the insertIntoInternalFunction as follows
    return insertIntoInternal(relId, attrName, blockHeader.pblock, middleEntry);

    // insertIntoInternal(relId, attrName, parent of current block, new internal
    // entry)

  } else {
    // the current block was the root block and is now split. a new internal
    // index block needs to be allocated and made the root of the tree. To do
    // this, call the createNewRoot() function with the following arguments
    return createNewRoot(relId, attrName,
                         internalEntries[MIDDLE_INDEX_INTERNAL].attrVal,
                         intBlockNum, newRightBlk);

    // createNewRoot(relId, attrName,
    //               internalEntries[MIDDLE_INDEX_INTERNAL].attrVal,
    //               current block, new right block)
  }

  // if either of the above calls returned an error (E_DISKFULL), then return
  return SUCCESS;
  // that else return SUCCESS
}

//! Distributes an array of index entries between an existing internal index
//! block and a newly allocated internal index block.

int BPlusTree::splitInternal(int intBlockNum, InternalEntry internalEntries[]) {
  // declare rightBlk, an instance of IndInternal using constructor 1 to obtain
  // new internal index block that will be used as the right block in the
  // splitting
  IndInternal rightBlk;
  IndInternal leftBlk(intBlockNum);

  // declare leftBlk, an instance of IndInternal using constructor 2 to read
  // from the existing internal index block

  int rightBlkNum = rightBlk.getBlockNum();
  int leftBlkNum = leftBlk.getBlockNum();

  if (rightBlkNum == E_DISKFULL) {
    //(failed to obtain a new internal index block because the disk is full)
    return E_DISKFULL;
  }

  HeadInfo leftBlkHeader, rightBlkHeader;
  // get the headers of left block and right block using
  // BlockBuffer::getHeader()
  leftBlk.getHeader(&leftBlkHeader);
  rightBlk.getHeader(&rightBlkHeader);

  // set rightBlkHeader with the following values
  // - number of entries = (MAX_KEYS_INTERNAL)/2 = 50
  // - pblock = pblock of leftBlk
  // and update the header of rightBlk using BlockBuffer::setHeader()
  rightBlkHeader.numEntries = MAX_KEYS_INTERNAL / 2;
  rightBlkHeader.pblock = leftBlkHeader.pblock;
  rightBlk.setHeader(&rightBlkHeader);

  // set leftBlkHeader with the following values
  // - number of entries = (MAX_KEYS_INTERNAL)/2 = 50
  // - rblock = rightBlkNum
  // and update the header using BlockBuffer::setHeader()
  leftBlkHeader.numEntries = MAX_KEYS_INTERNAL / 2;
  leftBlkHeader.rblock = rightBlkNum;
  leftBlk.setHeader(&leftBlkHeader);

  /*
  - set the first 50 entries of leftBlk = index 0 to 49 of internalEntries
    array
  - set the first 50 entries of newRightBlk = entries from index 51 to 100
    of internalEntries array using IndInternal::setEntry().
    (index 50 will be moving to the parent internal index block)
  */
  for (int index = 0; index < 50; index++) {
    leftBlk.setEntry(&internalEntries[index].attrVal, index);
    rightBlk.setEntry(&internalEntries[index + MIDDLE_INDEX_INTERNAL].attrVal,
                      index);
  }

  int type = StaticBuffer::getStaticBlockType(internalEntries[0].lChild);
  /* block type of a child of any entry of the internalEntries array */;
  //            (use StaticBuffer::getStaticBlockType())
  BlockBuffer blockBuffer(internalEntries[MIDDLE_INDEX_INTERNAL + 1].lChild);
  HeadInfo blockHeader;
  blockBuffer.getHeader(&blockHeader);
  blockHeader.pblock = rightBlkNum;
  blockBuffer.setHeader(&blockHeader);

  for (int index = 0; index < 50; index++) {
    // declare an instance of BlockBuffer to access the child block using
    // constructor 2
    BlockBuffer blockBuffer(internalEntries[index + MIDDLE_INDEX_INTERNAL + 1].rChild);
    blockBuffer.getHeader(&blockHeader);
    blockHeader.pblock = rightBlkNum;
    blockBuffer.setHeader(&blockHeader);

    // update pblock of the block to rightBlkNum using BlockBuffer::getHeader()
    // and BlockBuffer::setHeader().
  }

  return rightBlkNum;
}



//! This function will allocate a new root block and update the attribute cache
//!  entry of the attribute in the specified relation to point to the new root block.

int BPlusTree::createNewRoot(int relId, char attrName[ATTR_SIZE], Attribute attrVal, int lChild, int rChild) {
    // get the attribute cache entry corresponding to attrName
    // using AttrCacheTable::getAttrCatEntry().
    AttrCatEntry attrCatEntry;
    AttrCacheTable::getAttrCatEntry(relId,attrName,&attrCatEntry);

    // declare newRootBlk, an instance of IndInternal using appropriate constructor
    // to allocate a new internal index block on the disk
    IndInternal newRootBlk;

    int newRootBlkNum =newRootBlk.getBlockNum(); /* block number of newRootBlk */;

    if (newRootBlkNum == E_DISKFULL) {
        // (failed to obtain an empty internal index block because the disk is full)
        BPlusTree::bPlusDestroy(rChild);

        // Using bPlusDestroy(), destroy the right subtree, rooted at rChild.
        //! This corresponds to the tree built up till now that has not yet been
        //! connected to the existing B+ Tree

        return E_DISKFULL;
    }

    // update the header of the new block with numEntries = 1 using
    // BlockBuffer::getHeader() and BlockBuffer::setHeader()
    HeadInfo newHeader;
    newRootBlk.getHeader(&newHeader);
    newHeader.numEntries=1;
    newRootBlk.setHeader(&newHeader);

    // create a struct InternalEntry with lChild, attrVal and rChild from the
    // arguments and set it as the first entry in newRootBlk using IndInternal::setEntry()
    InternalEntry entry;
    entry.attrVal=attrVal;
    entry.rChild=rChild;
    entry.lChild=lChild;
    newRootBlk.setEntry(&entry,0);

    // declare BlockBuffer instances for the `lChild` and `rChild` blocks using
    // appropriate constructor and update the pblock of those blocks to `newRootBlkNum`
    // using BlockBuffer::getHeader() and BlockBuffer::setHeader()

    BlockBuffer leftChild(lChild);
    BlockBuffer rightChild(rChild);
    HeadInfo lHead,rHead;
    leftChild.getHeader(&lHead);
    rightChild.getHeader(&rHead);
    lHead.pblock=newRootBlkNum;
    rHead.pblock=newRootBlkNum;
    leftChild.setHeader(&lHead);
    rightChild.setHeader(&rHead);


    // update rootBlock = newRootBlkNum for the entry corresponding to `attrName`
    // in the attribute cache using AttrCacheTable::setAttrCatEntry().

    attrCatEntry.rootBlock=newRootBlkNum;
    AttrCacheTable::setAttrCatEntry(relId,attrName,&attrCatEntry);

    return SUCCESS;
}