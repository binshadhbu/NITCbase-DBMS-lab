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
      index=0;

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

    while (index < leafHead.numEntries ) {

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