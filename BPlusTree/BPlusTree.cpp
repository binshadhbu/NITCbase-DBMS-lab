#include "BPlusTree.h"

#include <cstring>

inline bool operator == (RecId lhs, RecId rhs) {
	return (lhs.block == rhs.block && lhs.slot == rhs.slot);
}

inline bool operator != (RecId lhs, RecId rhs) {
	return (lhs.block != rhs.block || lhs.slot != rhs.slot);
}

inline bool operator == (IndexId lhs, IndexId rhs) {
	return (lhs.block == rhs.block && lhs.index == rhs.index);
}

inline bool operator != (IndexId lhs, IndexId rhs) {
	return (lhs.block != rhs.block || lhs.index != rhs.index);
}

RecId BPlusTree::bPlusSearch(int relId, char attrName[ATTR_SIZE], 
                                Attribute attrVal, int op) 
{
    // declare searchIndex which will be used to store search index for attrName.
    IndexId searchIndex;

    // get the search index corresponding to attribute with name attrName
    // using AttrCacheTable::getSearchIndex().
    int ret = AttrCacheTable::getSearchIndex(relId, attrName, &searchIndex);

    AttrCatEntry attrCatEntry;
    // load the attribute cache entry into attrCatEntry using
    // AttrCacheTable::getAttrCatEntry().
    ret = AttrCacheTable::getAttrCatEntry(relId, attrName, &attrCatEntry);

    // declare variables block and index which will be used during search
    int block = -1, index = -1;

    if (searchIndex == IndexId{-1, -1}) // (search is done for the first time)
    {
        // start the search from the first entry of root.
        block = attrCatEntry.rootBlock;
        index = 0;

        if (block == -1) // attrName doesn't have a B+ tree (block == -1)
            return RecId{-1, -1};

    } 
    else 
    {
        // a valid searchIndex points to an entry in the leaf index of the attribute's
        // B+ Tree which had previously satisfied the op for the given attrVal.

        block = searchIndex.block, index = searchIndex.index + 1;  // search is resumed from the next index.

        // load block into leaf using IndLeaf::IndLeaf().
        IndLeaf leaf(block);

        // declare leafHead which will be used to hold the header of leaf.
        HeadInfo leafHead;

        // load header into leafHead using BlockBuffer::getHeader().
        leaf.getHeader(&leafHead);

        if (index >= leafHead.numEntries) {
            // (all the entries in the block has been searched; search from the
            // beginning of the next leaf index block.

            // update block to rblock of current block and index to 0.
            block = leafHead.rblock, index = 0;

            if (block == -1) {
                // (end of linked list reached - the search is done.)
                return RecId{-1, -1};
            }
        }
    }

    /******  Traverse through all the internal nodes according to value of attrVal and the operator op ******/

    /* 
    * This section is only needed when :
        * search starts from the root block and the root is not a leaf
    * If there was a valid search index, then we are already at a leaf block
    * and the test condition in the following loop will fail
    */

    while(StaticBuffer::getStaticBlockType(block) == IND_INTERNAL) /* block is of type IND_INTERNAL */ // use StaticBuffer::getStaticBlockType()
    {  
        // load the block into internalBlk using IndInternal::IndInternal().
        IndInternal internalBlk(block);

        // load the header of internalBlk into intHead using BlockBuffer::getHeader()
        HeadInfo intHead;
        internalBlk.getHeader(&intHead);

        // declare intEntry which will be used to store an entry of internalBlk.
        InternalEntry intEntry;

        // if (/* op is one of NE, LT, LE */)
        if (op == NE || op == LT || op == LE) 
        {
            /*
            - NE: need to search the entire linked list of leaf indices of the B+ Tree,
            starting from the leftmost leaf index. Thus, always move to the left.

            - LT and LE: the attribute values are arranged in ascending order in the
            leaf indices of the B+ Tree. Values that satisfy these conditions, if
            any exist, will always be found in the left-most leaf index. Thus,
            always move to the left.
            */

            // load entry in the first slot of the block into intEntry
            // using IndInternal::getEntry().
            internalBlk.getEntry(&intEntry, 0);
            block = intEntry.lChild;

        } 
        
        else 
        {
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

            int entryindex = 0;
            while (entryindex < intHead.numEntries)
            {
                ret = internalBlk.getEntry(&intEntry, entryindex);
                
                int cmpVal = compareAttrs(intEntry.attrVal, attrVal, attrCatEntry.attrType);
                if (
                    (op == EQ && cmpVal >= 0) ||
                    (op == GE && cmpVal >= 0) ||
                    (op == GT && cmpVal > 0)
                )
                    break;

                entryindex++;
            }

            // if (/* such an entry is found*/) 
            if (entryindex < intHead.numEntries)
            {
                // move to the left child of that entry
                block = intEntry.lChild; // left child of the entry
            }
            else 
            {
                // move to the right child of the last entry of the block
                // i.e numEntries - 1 th entry of the block
                block = intEntry.rChild; // right child of last entry
            }

            // index = 0;
        }
    }

    // NOTE: `block` now has the block number of a leaf index block.

    /******  Identify the first leaf index entry from the current position
                that satisfies our condition (moving right)             ******/

    while (block != -1) {
        // load the block into leafBlk using IndLeaf::IndLeaf().
        IndLeaf leafBlk(block);

        // load the header to leafHead using BlockBuffer::getHeader().
        HeadInfo leafHead;
        leafBlk.getHeader(&leafHead);

        // declare leafEntry which will be used to store an entry from leafBlk
        Index leafEntry;

        // while (/*index < numEntries in leafBlk*/) 
        while (index < leafHead.numEntries)
        {
            // load entry corresponding to block and index into leafEntry
            // using IndLeaf::getEntry().
            leafBlk.getEntry(&leafEntry, index);

            // comparison between leafEntry's attribute value and input attrVal using compareAttrs()
            int cmpVal = compareAttrs(leafEntry.attrVal, attrVal, attrCatEntry.attrType); 

            if (
                (op == EQ && cmpVal == 0) ||
                (op == LE && cmpVal <= 0) ||
                (op == GE && cmpVal >= 0) ||
                (op == LT && cmpVal < 0) ||
                (op == GT && cmpVal > 0) ||
                (op == NE && cmpVal != 0)
            ) {
                // (entry satisfying the condition found)

                // set search index to {block, index}
                searchIndex = IndexId{block, index};
                AttrCacheTable::setSearchIndex(relId, attrName, &searchIndex);

                // return the recId {leafEntry.block, leafEntry.slot}.
                return RecId{leafEntry.block, leafEntry.slot};
            } 
            else if ((op == EQ || op == LE || op == LT) && cmpVal > 0) 
            {
                // future entries will not satisfy EQ, LE, LT since the values
                // are arranged in ascending order in the leaves 

                return RecId {-1, -1};
            }

            // search next index.
            ++index;
        }

        // only for NE operation do we have to check the entire linked list;
        // for all the other op it is guaranteed that the block being searched
        // will have an entry, if it exists, satisying that op. 
        if (op != NE) {
            break;
        }

        // block = next block in the linked list, i.e., the rblock in leafHead.
        // update index to 0.
        block = leafHead.rblock, index = 0;
    }

    // no entry satisying the op was found; return the recId {-1,-1}
    return RecId{-1, -1};
}

int BPlusTree::bPlusCreate(int relId, char attrName[ATTR_SIZE]) 
{
    // if relId is either RELCAT_RELID or ATTRCAT_RELID:
    //     return E_NOTPERMITTED;
    if (relId == RELCAT_RELID || relId == ATTRCAT_RELID)
        return E_NOTPERMITTED;

    // get the attribute catalog entry of attribute `attrName`
    // using AttrCacheTable::getAttrCatEntry()
    AttrCatEntry attrCatEntryBuffer;
    int ret = AttrCacheTable::getAttrCatEntry(relId, attrName, &attrCatEntryBuffer);

    // if getAttrCatEntry fails
    //     return the error code from getAttrCatEntry
    if (ret != SUCCESS) return ret;

    // if (/* an index already exists for the attribute (check rootBlock field) */) 
    if (attrCatEntryBuffer.rootBlock != -1)
        return SUCCESS;

    /******Creating a new B+ Tree ******/

    // get a free leaf block using constructor 1 to allocate a new block
    IndLeaf rootBlockBuf;

    // (if the block could not be allocated, the appropriate error code
    //  will be stored in the blockNum member field of the object)

    // declare rootBlock to store the blockNumber of the new leaf block
    int rootBlock = rootBlockBuf.getBlockNum();

    // if there is no more disk space for creating an index
    if (rootBlock == E_DISKFULL) return E_DISKFULL;

    attrCatEntryBuffer.rootBlock = rootBlock;
    AttrCacheTable::setAttrCatEntry(relId, attrName, &attrCatEntryBuffer);

    RelCatEntry relCatEntryBuffer;

    // load the relation catalog entry into relCatEntry
    // using RelCacheTable::getRelCatEntry().
    RelCacheTable::getRelCatEntry(relId, &relCatEntryBuffer);
    int block = relCatEntryBuffer.firstBlk; // first record block of the relation 

    /***** Traverse all the blocks in the relation and insert them one
           by one into the B+ Tree *****/
    while (block != -1) {
        // declare a RecBuffer object for `block` (using appropriate constructor)
        RecBuffer blockBuffer (block);

        // load the slot map into slotMap using RecBuffer::getSlotMap().
        unsigned char slotmap[relCatEntryBuffer.numSlotsPerBlk];
        blockBuffer.getSlotMap(slotmap);

        for (int slot = 0; slot < relCatEntryBuffer.numSlotsPerBlk; slot++)
        {
            if (slotmap[slot] == SLOT_OCCUPIED)
            {
                Attribute record[relCatEntryBuffer.numAttrs];
                // load the record corresponding to the slot into `record`
                // using RecBuffer::getRecord().
                blockBuffer.getRecord(record, slot);

                // declare recId and store the rec-id of this record in it
                // RecId recId{block, slot};
                RecId recId = RecId{block, slot};

                // insert the attribute value corresponding to attrName from the record
                // into the B+ tree using bPlusInsert.
                // (note that bPlusInsert will destroy any existing bplus tree if
                // insert fails i.e when disk is full)

                ret = bPlusInsert(relId, attrName, 
                                    record[attrCatEntryBuffer.offset], recId);

                if (ret == E_DISKFULL) {
                    // (unable to get enough blocks to build the B+ Tree.)
                    return E_DISKFULL;
                }
            }
        }
        
        // get the header of the block using BlockBuffer::getHeader()
        HeadInfo blockHeader;
        blockBuffer.getHeader(&blockHeader);

        // set block = rblock of current block (from the header)
        block = blockHeader.rblock;
    }

    return SUCCESS;
}

int BPlusTree::bPlusInsert(int relId, char attrName[ATTR_SIZE], 
                            Attribute attrVal, RecId recId) {
    // get the attribute cache entry corresponding to attrName
    // using AttrCacheTable::getAttrCatEntry().
    AttrCatEntry attrCatEntryBuffer;
    int ret = AttrCacheTable::getAttrCatEntry(relId, attrName, &attrCatEntryBuffer);

    // if getAttrCatEntry() failed
    //     return the error code
    if (ret != SUCCESS) return ret;

    int rootBlock = attrCatEntryBuffer.rootBlock; // rootBlock of B+ Tree 

    // if (/* there is no index on attribute (rootBlock is -1) */) 
    if (rootBlock == -1) return E_NOINDEX;

    // find the leaf block to which insertion is to be done using the
    // findLeafToInsert() function

    int leafBlkNum = findLeafToInsert(rootBlock, attrVal, attrCatEntryBuffer.attrType); // findLeafToInsert(root block num, attrVal, attribute type) 

    // insert the attrVal and recId to the leaf block at blockNum using the
    // insertIntoLeaf() function.

    // declare a struct Index with attrVal = attrVal, block = recId.block and
    // slot = recId.slot to pass as argument to the function.
    // insertIntoLeaf(relId, attrName, leafBlkNum, Index entry)
    // NOTE: the insertIntoLeaf() function will propagate the insertion to the
    //       required internal nodes by calling the required helper functions
    //       like insertIntoInternal() or createNewRoot()

    Index indexEntry; 
    indexEntry.attrVal = attrVal, indexEntry.block = recId.block, indexEntry.slot = recId.slot;
    
    // if (/*insertIntoLeaf() returns E_DISKFULL */) 
    if (insertIntoLeaf(relId, attrName, leafBlkNum, indexEntry) == E_DISKFULL)
    {
        // destroy the existing B+ tree by passing the rootBlock to bPlusDestroy().
        BPlusTree::bPlusDestroy(rootBlock);

        // update the rootBlock of attribute catalog cache entry to -1 using
        // AttrCacheTable::setAttrCatEntry().
        attrCatEntryBuffer.rootBlock = -1;
        AttrCacheTable::setAttrCatEntry(relId, attrName, &attrCatEntryBuffer);

        return E_DISKFULL;
    }

    return SUCCESS;
}

int BPlusTree::findLeafToInsert(int rootBlock, Attribute attrVal, int attrType) {
    int blockNum = rootBlock;

    // while (/*block is not of type IND_LEAF */) // use StaticBuffer::getStaticBlockType()
    while (StaticBuffer::getStaticBlockType(blockNum) != IND_LEAF) 
    {  
        // declare an IndInternal object for block using appropriate constructor
        IndInternal internalBlock (blockNum);

        // get header of the block using BlockBuffer::getHeader()
        HeadInfo blockHeader;
        internalBlock.getHeader(&blockHeader);

        /* iterate through all the entries, to find the first entry whose
             attribute value >= value to be inserted.
             NOTE: the helper function compareAttrs() declared in BlockBuffer.h
                   can be used to compare two Attribute values. */
        int index = 0;
        while (index < blockHeader.numEntries)
        {
            InternalEntry entry;
            internalBlock.getEntry(&entry, index);

            if (compareAttrs(attrVal, entry.attrVal, attrType) <= 0)
                break;

            index++;
        }


        if (index == blockHeader.numEntries) 
        {
            // set blockNum = rChild of (nEntries-1)'th entry of the block
            // (i.e. rightmost child of the block)
            InternalEntry entry;
            internalBlock.getEntry(&entry, blockHeader.numEntries-1);

            blockNum = entry.rChild;
        } 
        else 
        {
            // set blockNum = lChild of the entry that was found
            InternalEntry entry;
            internalBlock.getEntry(&entry, index);

            blockNum = entry.lChild;
        }
    }

    return blockNum;
}

int BPlusTree::insertIntoLeaf(int relId, char attrName[ATTR_SIZE], int leafBlockNum, Index indexEntry) 
{
    // get the attribute cache entry corresponding to attrName
    // using AttrCacheTable::getAttrCatEntry().
    AttrCatEntry attrCatEntryBuffer;
    AttrCacheTable::getAttrCatEntry(relId, attrName, &attrCatEntryBuffer);

    // declare an IndLeaf instance for the block using appropriate constructor
    IndLeaf leafBlock (leafBlockNum);

    HeadInfo blockHeader;
    // store the header of the leaf index block into blockHeader
    // using BlockBuffer::getHeader()
    leafBlock.getHeader(&blockHeader);

    // the following variable will be used to store a list of index entries with
    // existing indices + the new index to insert
    Index indices[blockHeader.numEntries + 1];

    /*
    Iterate through all the entries in the block and copy them to the array indices.
    Also insert `indexEntry` at appropriate position in the indices array maintaining
    the ascending order.
    - use IndLeaf::getEntry() to get the entry
    - use compareAttrs() declared in BlockBuffer.h to compare two Attribute structs
    */

    bool inserted = false;
    for (int entryindex = 0; entryindex < blockHeader.numEntries; entryindex++)
    {
        Index entry;
        leafBlock.getEntry(&entry, entryindex);

        if (compareAttrs(entry.attrVal, indexEntry.attrVal, attrCatEntryBuffer.attrType) <= 0)
        {
            // the current entry element is lesser than the one to be inserted
            indices[entryindex] = entry;
        }
        else
        {
            // the indexEntry is to be placed here
            indices[entryindex] = indexEntry;
            // insertmarker++;
            inserted = true;

            for (entryindex++; entryindex <= blockHeader.numEntries; entryindex++)
            {
                leafBlock.getEntry(&entry, entryindex-1);
                indices[entryindex] = entry;
            }
            break;
        }
    }

    // adding the last element in indices
    if (!inserted) indices[blockHeader.numEntries] = indexEntry;

    if (blockHeader.numEntries < MAX_KEYS_LEAF) {
        // (leaf block has not reached max limit)

        // increment blockHeader.numEntries and update the header of block
        // using BlockBuffer::setHeader().
        blockHeader.numEntries++;
        leafBlock.setHeader(&blockHeader);

        // iterate through all the entries of the array `indices` and populate the
        // entries of block with them using IndLeaf::setEntry().

        for (int indicesIt = 0; indicesIt < blockHeader.numEntries; indicesIt++)
            leafBlock.setEntry(&indices[indicesIt], indicesIt);

        return SUCCESS;
    }

    // If we reached here, the `indices` array has more than entries than can fit
    // in a single leaf index block. Therefore, we will need to split the entries
    // in `indices` between two leaf blocks. We do this using the splitLeaf() function.
    // This function will return the blockNum of the newly allocated block or
    // E_DISKFULL if there are no more blocks to be allocated.

    int newRightBlk = splitLeaf(leafBlockNum, indices);

    // if splitLeaf() returned E_DISKFULL
    //     return E_DISKFULL
    if (newRightBlk == E_DISKFULL) return E_DISKFULL;

    // if (/* the current leaf block was not the root */) // check pblock in header
    if (blockHeader.pblock != -1)
    {
        //TODO: insert the middle value from `indices` into the parent block using the
        //TODO: insertIntoInternal() function. (i.e the last value of the left block)

        //* the middle value will be at index 31 (given by constant MIDDLE_INDEX_LEAF)

        // create a struct InternalEntry with attrVal = indices[MIDDLE_INDEX_LEAF].attrVal,
        // lChild = currentBlock, rChild = newRightBlk and pass it as argument to
        // the insertIntoInternalFunction as follows

        InternalEntry middleEntry;
        middleEntry.attrVal = indices[MIDDLE_INDEX_LEAF].attrVal, 
        middleEntry.lChild = leafBlockNum, middleEntry.rChild = newRightBlk;

        // insertIntoInternal(relId, attrName, parent of current block, new internal entry)
        return insertIntoInternal(relId, attrName, blockHeader.pblock, middleEntry);
        // // if (ret == E_DISKFULL) return E_DISKFULL;
    } 
    else 
    {
        // the current block was the root block and is now split. a new internal index
        // block needs to be allocated and made the root of the tree.
        // To do this, call the createNewRoot() function with the following arguments

        // createNewRoot(relId, attrName, indices[MIDDLE_INDEX_LEAF].attrVal,
        //               current block, new right block)

        if(
            createNewRoot(relId, attrName, indices[MIDDLE_INDEX_LEAF].attrVal, 
                            leafBlockNum, newRightBlk) == E_DISKFULL
        )
            return E_DISKFULL;
    }

    // if either of the above calls returned an error (E_DISKFULL), then return that
    // else return SUCCESS
    return SUCCESS;
}

int BPlusTree::splitLeaf(int leafBlockNum, Index indices[]) {
    // declare rightBlk, an instance of IndLeaf using constructor 1 to obtain new
    // leaf index block that will be used as the right block in the splitting
    IndLeaf rightBlock;

    // declare leftBlk, an instance of IndLeaf using constructor 2 to read from
    // the existing leaf block
    IndLeaf leftBlock(leafBlockNum);

    int rightBlockNum = rightBlock.getBlockNum();
    int leftBlockNum = leftBlock.getBlockNum();

    if (rightBlockNum == E_DISKFULL) 
    {
        //(failed to obtain a new leaf index block because the disk is full)
        return E_DISKFULL;
    }

    // get the headers of left block and right block using BlockBuffer::getHeader()
    HeadInfo leftBlockHeader, rightBlockHeader;
    leftBlock.getHeader(&leftBlockHeader);
    rightBlock.getHeader(&rightBlockHeader);

    // set rightBlkHeader with the following values
    // - number of entries = (MAX_KEYS_LEAF+1)/2 = 32,
    // - pblock = pblock of leftBlk
    // - lblock = leftBlkNum
    // - rblock = rblock of leftBlk
    // and update the header of rightBlk using BlockBuffer::setHeader()
    
    rightBlockHeader.blockType = leftBlockHeader.blockType, 
    rightBlockHeader.numEntries = (MAX_KEYS_LEAF+1)/2;
    
    rightBlockHeader.pblock = leftBlockHeader.pblock, 
    rightBlockHeader.lblock = leftBlockNum, 
    rightBlockHeader.rblock = leftBlockHeader.rblock;

    rightBlock.setHeader(&rightBlockHeader);

    // set leftBlkHeader with the following values
    // - number of entries = (MAX_KEYS_LEAF+1)/2 = 32
    // - rblock = rightBlkNum
    // and update the header of leftBlk using BlockBuffer::setHeader() */

    leftBlockHeader.numEntries = (MAX_KEYS_LEAF+1)/2;
    leftBlockHeader.rblock = rightBlockNum;

    leftBlock.setHeader(&leftBlockHeader);

    // set the first 32 entries of leftBlk = the first 32 entries of indices array
    // and set the first 32 entries of newRightBlk = the next 32 entries of
    // indices array using IndLeaf::setEntry().

    for (int entryindex = 0; entryindex <= MIDDLE_INDEX_LEAF; entryindex++)
    {
        leftBlock.setEntry(&indices[entryindex], entryindex);
        rightBlock.setEntry(&indices[entryindex + MIDDLE_INDEX_LEAF+1], entryindex);
    }

    return rightBlockNum;
}

int BPlusTree::insertIntoInternal(int relId, char attrName[ATTR_SIZE], 
                                    int intBlockNum, InternalEntry intEntry) {
    // get the attribute cache entry corresponding to attrName
    // using AttrCacheTable::getAttrCatEntry().
    AttrCatEntry attrCatEntryBuffer;
    AttrCacheTable::getAttrCatEntry(relId, attrName, &attrCatEntryBuffer);

    // declare intBlk, an instance of IndInternal using constructor 2 for the block
    // corresponding to intBlockNum
    IndInternal internalBlock (intBlockNum);

    HeadInfo blockHeader;
    // load blockHeader with header of intBlk using BlockBuffer::getHeader().
    internalBlock.getHeader(&blockHeader);

    // declare internalEntries to store all existing entries + the new entry
    InternalEntry internalEntries[blockHeader.numEntries + 1];

    /*
    Iterate through all the entries in the block and copy them to the array
    `internalEntries`. Insert `indexEntry` at appropriate position in the
    array maintaining the ascending order.
        - use IndInternal::getEntry() to get the entry
        - use compareAttrs() to compare two structs of type Attribute

    Update the lChild of the internalEntry immediately following the newly added
    entry to the rChild of the newly added entry.
    */

    // bool inserted = false;
    int insertedIndex = -1;
    for (int entryindex = 0; entryindex < blockHeader.numEntries; entryindex++)
    {
        InternalEntry internalBlockEntry;
        internalBlock.getEntry(&internalBlockEntry, entryindex);

        if (compareAttrs(internalBlockEntry.attrVal, intEntry.attrVal, attrCatEntryBuffer.attrType) <= 0)
        {
            // the new value is lesser (or equal to), hence does not go here
            internalEntries[entryindex] = internalBlockEntry;
        }
        else 
        {
            // // setting the previous entry's rChild to the lChild of this entry
            // if (entryindex > 0)
            // {
            //     internalBlock.getEntry(&internalBlockEntry, entryindex-1);
            //     internalBlockEntry.rChild = intEntry.lChild;
            // }

            // setting the correct entry in `internalEntries`
            internalEntries[entryindex] = intEntry;
            insertedIndex = entryindex;

            // // setting the following entry's lChild to the rChild of this entry
            // internalBlock.getEntry(&internalBlockEntry, entryindex+1);
            // internalBlockEntry.lChild = intEntry.rChild;

            for (entryindex++; entryindex <= blockHeader.numEntries; entryindex++)
            {
                internalBlock.getEntry(&internalBlockEntry, entryindex-1);
                internalEntries[entryindex] = internalBlockEntry;
            }

            break;
        }
    }

    // inserting the last remaining entry, if any
    //// internalEntries[blockHeader.numEntries] = inserted ? lastEntry : intEntry;
    if (insertedIndex == -1) {
        internalEntries[blockHeader.numEntries] = intEntry;
        insertedIndex = blockHeader.numEntries;
    }

    // setting the previous entry's rChild to lChild of `intEntry`
    if (insertedIndex > 0)
    {
        // InternalEntry entry;
        // internalBlock.getEntry(&entry, insertedIndex-1);
        internalEntries[insertedIndex-1].rChild = intEntry.lChild;
    }

    // setting the following entry's lChild to rChild of `intEntry`
    if (insertedIndex < blockHeader.numEntries)
    {
        // InternalEntry entry;
        // internalBlock.getEntry(&entry, insertedIndex+1);
        internalEntries[insertedIndex+1].lChild = intEntry.rChild;
    }


    if (blockHeader.numEntries < MAX_KEYS_INTERNAL) {
        // (internal index block has not reached max limit)

        // increment blockheader.numEntries and update the header of intBlk
        // using BlockBuffer::setHeader().
        blockHeader.numEntries++;
        internalBlock.setHeader(&blockHeader);

        // iterate through all entries in internalEntries array and populate the
        // entries of intBlk with them using IndInternal::setEntry().
        for (int entryindex = 0; entryindex < blockHeader.numEntries; entryindex++)
            internalBlock.setEntry(&internalEntries[entryindex], entryindex);

        return SUCCESS;
    }

    // If we reached here, the `internalEntries` array has more than entries than
    // can fit in a single internal index block. Therefore, we will need to split
    // the entries in `internalEntries` between two internal index blocks. We do
    // this using the splitInternal() function.
    // This function will return the blockNum of the newly allocated block or
    // E_DISKFULL if there are no more blocks to be allocated.

    int newRightBlk = splitInternal(intBlockNum, internalEntries);

    // if (/* splitInternal() returned E_DISKFULL */) 
    if (newRightBlk == E_DISKFULL)
    {
        // Using bPlusDestroy(), destroy the right subtree, rooted at intEntry.rChild.
        // This corresponds to the tree built up till now that has not yet been
        // connected to the existing B+ Tree

        BPlusTree::bPlusDestroy(intEntry.rChild);

        return E_DISKFULL;
    }

    // if the current block was not the root  
    if (blockHeader.pblock != -1) // (check pblock in header)
    {  
        // insert the middle value from `internalEntries` into the parent block
        // using the insertIntoInternal() function (recursively).

        // create a struct InternalEntry with lChild = current block, rChild = newRightBlk
        // and attrVal = internalEntries[MIDDLE_INDEX_INTERNAL].attrVal
        // and pass it as argument to the insertIntoInternalFunction as follows

        InternalEntry middleEntry;
        middleEntry.lChild = intBlockNum, middleEntry.rChild = newRightBlk;

        // the middle value will be at index 50 (given by constant MIDDLE_INDEX_INTERNAL)
        middleEntry.attrVal = internalEntries[MIDDLE_INDEX_INTERNAL].attrVal;

        // insertIntoInternal(relId, attrName, parent of current block, new internal entry)
        return insertIntoInternal(relId, attrName, blockHeader.pblock, middleEntry);
        // // if (ret == E_DISKFULL) return E_DISKFULL;
    } 
    else 
    {
        // the current block was the root block and is now split. a new internal index
        // block needs to be allocated and made the root of the tree.
        // To do this, call the createNewRoot() function with the following arguments

        return createNewRoot(relId, attrName, internalEntries[MIDDLE_INDEX_INTERNAL].attrVal, 
                                intBlockNum, newRightBlk);


        // createNewRoot(relId, attrName,
        //               internalEntries[MIDDLE_INDEX_INTERNAL].attrVal,
        //               current block, new right block)

        //// if (ret == E_DISKFULL) return E_DISKFULL;
    }

    // if either of the above calls returned an error (E_DISKFULL), then return that
    // else return SUCCESS
    return SUCCESS;
}

int BPlusTree::splitInternal(int intBlockNum, InternalEntry internalEntries[]) 
{
    // declare rightBlk, an instance of IndInternal using constructor 1 to obtain new
    // internal index block that will be used as the right block in the splitting
    IndInternal rightBlock;

    // declare leftBlk, an instance of IndInternal using constructor 2 to read from
    // the existing internal index block
    IndInternal leftBlock (intBlockNum);

    int leftBlockNum = leftBlock.getBlockNum();
    int rightBlockNum = rightBlock.getBlockNum();

    // if newly allocated block has blockNum E_DISKFULL 
    if (rightBlockNum == E_DISKFULL)
    {
        //(failed to obtain a new internal index block because the disk is full)
        return E_DISKFULL;
    }

    HeadInfo leftBlockHeader, rightBlockHeader;

    // get the headers of left block and right block using BlockBuffer::getHeader()
    leftBlock.getHeader(&leftBlockHeader);
    rightBlock.getHeader(&rightBlockHeader);

    // set rightBlkHeader with the following values
    // - number of entries = (MAX_KEYS_INTERNAL)/2 = 50
    // - pblock = pblock of leftBlk
    // and update the header of rightBlk using BlockBuffer::setHeader()
    
    rightBlockHeader.numEntries = MAX_KEYS_INTERNAL / 2;
    rightBlockHeader.pblock = leftBlockHeader.pblock;

    rightBlock.setHeader(&rightBlockHeader);

    // set leftBlkHeader with the following values
    // - number of entries = (MAX_KEYS_INTERNAL)/2 = 50
    // - rblock = rightBlkNum
    // and update the header using BlockBuffer::setHeader()

    leftBlockHeader.numEntries = MAX_KEYS_INTERNAL / 2;
    leftBlockHeader.rblock = rightBlockNum;

    leftBlock.setHeader(&leftBlockHeader);

    /*
    - set the first 50 entries of leftBlk = index 0 to 49 of internalEntries
      array
    - set the first 50 entries of newRightBlk = entries from index 51 to 100
      of internalEntries array using IndInternal::setEntry().
      (index 50 will be moving to the parent internal index block)
    */

    for (int entryindex = 0; entryindex < MIDDLE_INDEX_INTERNAL; entryindex++)
    {
        leftBlock.setEntry(&internalEntries[entryindex], entryindex);
        rightBlock.setEntry(&internalEntries[entryindex + MIDDLE_INDEX_INTERNAL + 1], entryindex);
    }

    /* block type of a child of any entry of the internalEntries array */;
    //            (use StaticBuffer::getStaticBlockType())
    int type = StaticBuffer::getStaticBlockType(internalEntries[0].lChild);

    // for (/* each child block of the new right block */) 
    BlockBuffer blockbuffer (internalEntries[MIDDLE_INDEX_INTERNAL+1].lChild);

    HeadInfo blockHeader;
    blockbuffer.getHeader(&blockHeader);

    blockHeader.pblock = rightBlockNum;
    blockbuffer.setHeader(&blockHeader);

    for (int entryindex = 0; entryindex < MIDDLE_INDEX_INTERNAL; entryindex++)
    {
        // declare an instance of BlockBuffer to access the child block using
        // constructor 2
        BlockBuffer blockbuffer (internalEntries[entryindex + MIDDLE_INDEX_INTERNAL+1].rChild);

        // update pblock of the block to rightBlkNum using BlockBuffer::getHeader()
        // and BlockBuffer::setHeader().
        // HeadInfo blockHeader;
        blockbuffer.getHeader(&blockHeader);

        blockHeader.pblock = rightBlockNum;
        blockbuffer.setHeader(&blockHeader);
    }

    return rightBlockNum; 
}

int BPlusTree::createNewRoot(int relId, char attrName[ATTR_SIZE], Attribute attrVal, int lChild, int rChild) {
    // get the attribute cache entry corresponding to attrName
    // using AttrCacheTable::getAttrCatEntry().
    AttrCatEntry attrCatEntryBuffer;
    AttrCacheTable::getAttrCatEntry(relId, attrName, &attrCatEntryBuffer);

    // declare newRootBlk, an instance of IndInternal using appropriate constructor
    // to allocate a new internal index block on the disk
    IndInternal newRootBlock;

    int newRootBlkNum = newRootBlock.getBlockNum();

    if (newRootBlkNum == E_DISKFULL) 
    {
        // (failed to obtain an empty internal index block because the disk is full)

        // Using bPlusDestroy(), destroy the right subtree, rooted at rChild.
        // This corresponds to the tree built up till now that has not yet been
        // connected to the existing B+ Tree

        BPlusTree::bPlusDestroy(rChild);

        return E_DISKFULL;
    }

    // update the header of the new block with numEntries = 1 using
    // BlockBuffer::getHeader() and BlockBuffer::setHeader()
    HeadInfo blockHeader;
    newRootBlock.getHeader(&blockHeader);

    blockHeader.numEntries = 1;
    newRootBlock.setHeader(&blockHeader);

    // create a struct InternalEntry with lChild, attrVal and rChild from the
    // arguments and set it as the first entry in newRootBlk using IndInternal::setEntry()
    InternalEntry internalentry;
    internalentry.lChild = lChild, internalentry.rChild = rChild, internalentry.attrVal = attrVal;

    newRootBlock.setEntry(&internalentry, 0);

    // declare BlockBuffer instances for the `lChild` and `rChild` blocks using
    // appropriate constructor and update the pblock of those blocks to `newRootBlkNum`
    // using BlockBuffer::getHeader() and BlockBuffer::setHeader()
    BlockBuffer leftChildBlock (lChild);
    BlockBuffer rightChildBlock (rChild);

    HeadInfo leftChildHeader, rightChildHeader;
    leftChildBlock.getHeader(&leftChildHeader);
    rightChildBlock.getHeader(&rightChildHeader);

    leftChildHeader.pblock = newRootBlkNum;
    rightChildHeader.pblock = newRootBlkNum;

    leftChildBlock.setHeader(&leftChildHeader);
    rightChildBlock.setHeader(&rightChildHeader);

    // update rootBlock = newRootBlkNum for the entry corresponding to `attrName`
    // in the attribute cache using AttrCacheTable::setAttrCatEntry().
    attrCatEntryBuffer.rootBlock = newRootBlkNum;
    AttrCacheTable::setAttrCatEntry(relId, attrName,  &attrCatEntryBuffer);

    return SUCCESS;
}

int BPlusTree::bPlusDestroy(int rootBlockNum) {
    // if (/*rootBlockNum lies outside the valid range [0,DISK_BLOCKS-1]*/) 
    if (rootBlockNum < 0 || rootBlockNum >= DISK_BLOCKS)
        return E_OUTOFBOUND;

    int type = StaticBuffer::getStaticBlockType(rootBlockNum); // type of block using StaticBuffer::getStaticBlockType() 

    if (type == IND_LEAF) 
    {
        // declare an instance of IndLeaf for rootBlockNum using appropriate constructor
        IndLeaf leafBlock (rootBlockNum);

        // release the block using BlockBuffer::releaseBlock().
        leafBlock.releaseBlock();

        return SUCCESS;

    } 
    else if (type == IND_INTERNAL) 
    {
        // declare an instance of IndInternal for rootBlockNum using appropriate constructor
        IndInternal internalBlock (rootBlockNum);

        // load the header of the block using BlockBuffer::getHeader().
        HeadInfo blockHeader;
        internalBlock.getHeader(&blockHeader);

        /*iterate through all the entries of the internalBlk and destroy the lChild
        of the first entry and rChild of all entries using BPlusTree::bPlusDestroy().
        (the rchild of an entry is the same as the lchild of the next entry.
         take care not to delete overlapping children more than once ) */
        
        InternalEntry blockEntry;
        internalBlock.getEntry (&blockEntry, 0);

        BPlusTree::bPlusDestroy(blockEntry.lChild);

        for (int entry = 0; entry < blockHeader.numEntries; entry++) {
            internalBlock.getEntry (&blockEntry, entry);
            BPlusTree::bPlusDestroy(blockEntry.rChild);
        }

        // release the block using BlockBuffer::releaseBlock().
        internalBlock.releaseBlock();

        return SUCCESS;

    } 
    else // (block is not an index block.)
        return E_INVALIDBLOCK;
}
























// #include "BPlusTree.h"
// #include <iostream>
// #include <cstring>

// RecId BPlusTree::bPlusSearch(int relId, char attrName[ATTR_SIZE], Attribute attrVal, int op) {
//     // declare searchIndex which will be used to store search index for attrName.
//     IndexId searchIndex;

//     /* get the search index corresponding to attribute with name attrName
//        using AttrCacheTable::getSearchIndex(). */
//     int response = AttrCacheTable::getSearchIndex(relId, attrName, &searchIndex);

//     if (response != SUCCESS) {
//         printf("failed to get search index for %s\n", attrName);
//         exit(1);
//     }

//     AttrCatEntry attrCatEntry;

//     /* load the attribute cache entry into attrCatEntry using
//      AttrCacheTable::getAttrCatEntry(). */
//     response = AttrCacheTable::getAttrCatEntry(relId, attrName, &attrCatEntry);

//     if (response != SUCCESS) {
//         printf("failed to get attrCatEntry for %s\n", attrName);
//         exit(1);
//     }
    


//     // declare variables block and index which will be used during search
//     int block, index;
//     /* searchIndex == {-1, -1}*/
//     if (searchIndex.block == -1 || searchIndex.index == -1){
//         // (search is done for the first time)

//         // start the search from the first entry of root.
//         block = attrCatEntry.rootBlock;
//         index = 0;
//         /* attrName doesn't have a B+ tree (block == -1)*/
//         if (block==-1) {
//             return RecId{-1, -1};
//         }

//     } else {
//         /*a valid searchIndex points to an entry in the leaf index of the attribute's
//         B+ Tree which had previously satisfied the op for the given attrVal.*/

//         block = searchIndex.block;
//         index = searchIndex.index + 1;  // search is resumed from the next index.

//         // load block into leaf using IndLeaf::IndLeaf().
//         IndLeaf leaf(block);

//         // declare leafHead which will be used to hold the header of leaf.
//         HeadInfo leafHead;

//         // load header into leafHead using BlockBuffer::getHeader().
//         response = leaf.getHeader(&leafHead);

//         if (response != SUCCESS) {
//             printf("failed to get header for block %d\n", block);
//             exit(1);
//         }

//         if (index >= leafHead.numEntries) {
//             /* (all the entries in the block has been searched; search from the
//             beginning of the next leaf index block. */

//             // block = rblock of leafHead.
//             block = leafHead.rblock;
//             index = 0;
//             // update block to rblock of current block and index to 0.

//             if (block == -1) {
//                 // (end of linked list reached - the search is done.)
//                 return RecId{-1, -1};
//             }
//         }
//     }

//     /******  Traverse through all the internal nodes according to value
//              of attrVal and the operator op                             ******/

//     /* (This section is only needed when
//         - search restarts from the root block (when searchIndex is reset by caller)
//         - root is not a leaf
//         If there was a valid search index, then we are already at a leaf block
//         and the test condition in the following loop will fail)
//     */
//     /* block is of type IND_INTERNAL */
//     while (StaticBuffer::getStaticBlockType(block) == IND_INTERNAL){
//         //use StaticBuffer::getStaticBlockType()

//         // load the block into internalBlk using IndInternal::IndInternal().
//         IndInternal internalBlk(block);

//         HeadInfo intHead;

//         // load the header of internalBlk into intHead using BlockBuffer::getHeader()
//         response = internalBlk.getHeader(&intHead);

//         // declare intEntry which will be used to store an entry of internalBlk.
//         InternalEntry intEntry;
//         /* op is one of NE, LT, LE */
//         if (op == NE || op == LT || op == LE) {
//             /*
//             - NE: need to search the entire linked list of leaf indices of the B+ Tree,
//             starting from the leftmost leaf index. Thus, always move to the left.

//             - LT and LE: the attribute values are arranged in ascending order in the
//             leaf indices of the B+ Tree. Values that satisfy these conditions, if
//             any exist, will always be found in the left-most leaf index. Thus,
//             always move to the left.
//             */


//             // load entry in the first slot of the block into intEntry
//             // using IndInternal::getEntry().
//             response = internalBlk.getEntry (&intEntry, 0);

//             block = intEntry.lChild;

//         } else {
//             /*
//             - EQ, GT and GE: move to the left child of the first entry that is
//             greater than (or equal to) attrVal
//             (we are trying to find the first entry that satisfies the condition.
//             since the values are in ascending order we move to the left child which
//             might contain more entries that satisfy the condition)
//             */


//             /*
//              traverse through all entries of internalBlk and find an entry that
//              satisfies the condition.
//              if op == EQ or GE, then intEntry.attrVal >= attrVal
//              if op == GT, then intEntry.attrVal > attrVal
//              Hint: the helper function compareAttrs() can be used for comparing
//             */
//            int index = 0;
//            while(index<intHead.numEntries){
//                 response = internalBlk.getEntry (&intEntry, index);
//                 int cmpVal = compareAttrs(intEntry.attrVal, attrVal, attrCatEntry.attrType);
//                 if (((op==EQ || op==GE) && cmpVal >= 0)|| (op==GT && cmpVal > 0)){
//                      break;
//                 }
//                 index++;
//             }
    
            

//             /* such an entry is found*/
//             if (index < intHead.numEntries) {
//                 // move to the left child of that entry
//                 // left child of the entry
//                 block = intEntry.lChild;  

//             } else {
//                 // move to the right child of the last entry of the block
//                 // i.e numEntries - 1 th entry of the block
//                 response = internalBlk.getEntry(&intEntry, intHead.numEntries - 1);
//                 if (response != SUCCESS) {
//                     printf("failed to get entry %d of block %d\n", intHead.numEntries - 1, block);
//                     exit(1);
//                 }

//                  // right child of last entry
//                 block =  intEntry.rChild;
//             }
//         }
//     }

//     // NOTE: `block` now has the block number of a leaf index block.

//     /******  Identify the first leaf index entry from the current position
//                 that satisfies our condition (moving right)             ******/

//     while(block != -1) {
//         // load the block into leafBlk using IndLeaf::IndLeaf().
//         IndLeaf leafBlk(block);
//         HeadInfo leafHead;

//         // load the header to leafHead using BlockBuffer::getHeader().
//         response = leafBlk.getHeader(&leafHead);

//         // declare leafEntry which will be used to store an entry from leafBlk
//         Index leafEntry;
//         /*index < numEntries in leafBlk*/
//         while (index < leafHead.numEntries) {

//             // load entry corresponding to block and index into leafEntry
//             // using IndLeaf::getEntry().
//             response = leafBlk.getEntry(&leafEntry, index);

//             /* comparison between leafEntry's attribute value
//                             and input attrVal using compareAttrs()*/
//             int cmpVal = compareAttrs(leafEntry.attrVal, attrVal, attrCatEntry.attrType);
//             if (
//                 (op == EQ && cmpVal == 0) ||
//                 (op == LE && cmpVal <= 0) ||
//                 (op == LT && cmpVal < 0) ||
//                 (op == GT && cmpVal > 0) ||
//                 (op == GE && cmpVal >= 0) ||
//                 (op == NE && cmpVal != 0)
//             ) {
//                 // (entry satisfying the condition found)

//                 // set search index to {block, index}
//                 searchIndex.block = block;
//                 searchIndex.index = index;
//                 response = AttrCacheTable::setSearchIndex(relId, attrName, &searchIndex);
//                 if(response!=SUCCESS){
//                     printf("failed to set search index for %s\n", attrName);
//                     exit(1);
//                 }



//                 // return the recId {leafEntry.block, leafEntry.slot}.
//                 return RecId{leafEntry.block, leafEntry.slot};

//             } else if ((op == EQ || op == LE || op == LT) && cmpVal > 0) {
//                 /*future entries will not satisfy EQ, LE, LT since the values
//                     are arranged in ascending order in the leaves */

//                 // return RecId {-1, -1};
//                 return RecId{-1, -1};
//             }

//             // search next index.
//             index++;
//         }

//         /*only for NE operation do we have to check the entire linked list;
//         for all the other op it is guaranteed that the block being searched
//         will have an entry, if it exists, satisying that op. */
//         if (op != NE) {
//             break;
//         }

//         // block = next block in the linked list, i.e., the rblock in leafHead.
//         block = leafHead.rblock;
//         // update index to 0.
//         index = 0;
        
//     }

//     // no entry satisying the op was found; return the recId {-1,-1}
//     return RecId{-1, -1};
// }

// int BPlusTree::bPlusCreate(int relId, char attrName[ATTR_SIZE]) {

//     // if relId is either RELCAT_RELID or ATTRCAT_RELID:
//     //     return E_NOTPERMITTED;
//     if(relId == RELCAT_RELID || relId == ATTRCAT_RELID){
//         return E_NOTPERMITTED;
//     }

//     // get the attribute catalog entry of attribute `attrName`
//     // using AttrCacheTable::getAttrCatEntry()
//     AttrCatEntry attrCatEntry;
//     int response = AttrCacheTable::getAttrCatEntry(relId, attrName, &attrCatEntry);

//     // if getAttrCatEntry fails
//     //     return the error code from getAttrCatEntry
//     if(response!=SUCCESS){
//         return response;
//     }
    
//     // an index already exists for the attribute (check rootBlock field) 
//     if (attrCatEntry.rootBlock != -1) {
//         return SUCCESS;
//     }

//     /******Creating a new B+ Tree ******/

//     // get a free leaf block using constructor 1 to allocate a new block
//     IndLeaf rootBlockBuf;

//     // declare rootBlock to store the blockNumber of the new leaf block
//     int rootBlock = rootBlockBuf.getBlockNum();

//     // (if the block could not be allocated, the appropriate error code
//     //  will be stored in the blockNum member field of the object)
//     if (rootBlock<0) {
//         return rootBlock;
//     }


//     // if there is no more disk space for creating an index
//     if (rootBlock == E_DISKFULL) {
//         return E_DISKFULL;
//     }

//     // update attrCatEntry.rootBlock to rootBlock
//     attrCatEntry.rootBlock = rootBlock;

//     // set the attrCatEntry using AttrCacheTable::setAttrCatEntry()
//     response = AttrCacheTable::setAttrCatEntry(relId, attrName, &attrCatEntry);

//     RelCatEntry relCatEntry;

//     // load the relation catalog entry into relCatEntry
//     // using RelCacheTable::getRelCatEntry().
//     response = RelCacheTable::getRelCatEntry(relId, &relCatEntry);
    
//     if(response!=SUCCESS){
//         printf("failed to get relCatEntry for relId %d\n", relId);
//     }

//     /* first record block of the relation */;
//     int block = relCatEntry.firstBlk;
//     /***** Traverse all the blocks in the relation and insert them one by one into the B+ Tree *****/
   
//     while (block != -1) {

//         // declare a RecBuffer object for `block` (using appropriate constructor)
//         RecBuffer recBuffer(block);

//         unsigned char slotMap[relCatEntry.numSlotsPerBlk];

//         // load the slot map into slotMap using RecBuffer::getSlotMap().
//         response = recBuffer.getSlotMap(slotMap);
        
//         if(response!=SUCCESS){
//             printf("failed to get slot map for block %d\n", block);
//             exit(1);
//         }

//         // for every occupied slot of the block
//         for(int slot =0; slot<relCatEntry.numSlotsPerBlk; slot++)
//         {
//             if(slotMap[slot]==SLOT_OCCUPIED){

//                 Attribute record[relCatEntry.numAttrs];
//                 // load the record corresponding to the slot into `record`
//                 // using RecBuffer::getRecord().
//                 response = recBuffer.getRecord(record, slot);
                
//                 if(response!=SUCCESS){
//                     printf("failed to get record for block %d, slot %d\n", block, slot);
//                     exit(1);
//                 }

//                 // RecId recId{block, slot};
//                 // declare recId and store the rec-id of this record in it
//                 RecId recId ={block,slot};
                
//                 // The caller is expected to ensure that
//                 // the RecId passed belongs to a valid record in the same relation


//                 // a duplicate index entry for this record does not already exist in the B+ tree. 
//                 // recId actually points to the specific record that the argument attribute value belongs to
                


//                 // insert the attribute value corresponding to attrName from the record
//                 // into the B+ tree using bPlusInsert.
//                 response = bPlusInsert(relId, attrName, record[attrCatEntry.offset], recId);
//                 // (note that bPlusInsert will destroy any existing bplus tree if
//                 // insert fails i.e when disk is full)
//                 // retVal = bPlusInsert(relId, attrName, attribute value, recId);

//                 if (response == E_DISKFULL) {
//                     // (unable to get enough blocks to build the B+ Tree.)
//                     return E_DISKFULL;
//                 }
                
//                 if(response!=SUCCESS){
//                     printf("failed to insert record for block %d, slot %d\n", block, slot);
//                     exit(1);
//                 }
//             }
//         }

//         // get the header of the block using BlockBuffer::getHeader()
//         HeadInfo headInfo;
//         response = recBuffer.getHeader(&headInfo);
//         if(response!=SUCCESS){
//             printf("failed to get header for block %d\n", block);
//             exit(1);
//         }

//         // set block = rblock of current block (from the header)
//         block = headInfo.rblock;
//     }

//     return SUCCESS;
// }


// int BPlusTree::bPlusDestroy(int rootBlockNum) {
//     /*rootBlockNum lies outside the valid range [0,DISK_BLOCKS-1]*/
//     if (rootBlockNum < 0 || rootBlockNum >= DISK_BLOCKS) {
//         return E_OUTOFBOUND;
//     }

//     /* type of block (using StaticBuffer::getStaticBlockType())*/;
//     int type = StaticBuffer::getStaticBlockType(rootBlockNum);

//     if (type == IND_LEAF) {
//         // declare an instance of IndLeaf for rootBlockNum using appropriate
//         // constructor
//         IndLeaf leaf(rootBlockNum);

//         // release the block using BlockBuffer::releaseBlock().
//         leaf.releaseBlock();

//         return SUCCESS;

//     } else if (type == IND_INTERNAL) {
//         // declare an instance of IndInternal for rootBlockNum using appropriate
//         // constructor
//         IndInternal internalBlk(rootBlockNum);

//         // load the header of the block using BlockBuffer::getHeader().
//         HeadInfo headInfo;
//         int response = internalBlk.getHeader(&headInfo);

//         if (response != SUCCESS) {
//             printf("failed to get header for block %d\n", rootBlockNum);
//             exit(1);
//         }

//         /*iterate through all the entries of the internalBlk and destroy the lChild
//         of the first entry and rChild of all entries using BPlusTree::bPlusDestroy().
//         (the rchild of an entry is the same as the lchild of the next entry.
//          take care not to delete overlapping children more than once ) */
//         InternalEntry internalEntry;
//         response = internalBlk.getEntry(&internalEntry, 0);
//         // (destroy the lChild of the first entry)
//         response = bPlusDestroy(internalEntry.lChild);


//         for (int i = 0; i < headInfo.numEntries; i++) {
//             // declare an instance of InternalEntry to store the entry
//             // using IndInternal::getEntry().
//             InternalEntry internalEntry;
//             response = internalBlk.getEntry(&internalEntry, i);

//             if (response != SUCCESS) {
//                 printf("failed to get entry %d of block %d\n", i, rootBlockNum);
//                 exit(1);
//             }

//             // (destroy the rChild of the entry)
//             response = bPlusDestroy(internalEntry.rChild);
//         }


//         // release the block using BlockBuffer::releaseBlock().
//         internalBlk.releaseBlock();

//         return SUCCESS;

//     } else {
//         // (block is not an index block.)
//         return E_INVALIDBLOCK;
//     }
// }

// /*
// The caller is expected to ensure that

// the RecId passed belongs to a valid record in the same relation
// a duplicate index entry for this record does not already exist in the B+ tree.
// recId actually points to the specific record that the argument attribute value belongs to
// This function will add the pair (attrVal, recId) to the B+ tree without any validation on these arguments.
// */

// int BPlusTree::bPlusInsert(int relId, char attrName[ATTR_SIZE], Attribute attrVal, RecId recId) {
//     // get the attribute cache entry corresponding to attrName
//     // using AttrCacheTable::getAttrCatEntry().
//     AttrCatEntry attrCatEntry;
//     int response = AttrCacheTable::getAttrCatEntry(relId, attrName, &attrCatEntry);

//     // if getAttrCatEntry() failed
//     //     return the error code
//     if (response != SUCCESS) {
//         return response;
//     }

//     /* rootBlock of B+ Tree (from attrCatEntry) */;
//     int blockNum = attrCatEntry.rootBlock;

//     /* there is no index on attribute (rootBlock is -1) */
//     if (blockNum == -1) {
//         return E_NOINDEX;
//     }

//     // find the leaf block to which insertion is to be done using the
//     // findLeafToInsert() function

//     // The caller must ensure that
//         // the argument passed is the block number of root block of a B+ tree on the disk
//         // the attribute type that is present in the index matches the attribute type that is passed

//     /* findLeafToInsert(root block num, attrVal, attribute type) */;
//     int leafBlkNum = BPlusTree::findLeafToInsert(blockNum, attrVal, attrCatEntry.attrType);

//     // insert the attrVal and recId to the leaf block at blockNum using the
//     // insertIntoLeaf() function.
//     // declare a struct Index with attrVal = attrVal, block = recId.block and
//     // slot = recId.slot to pass as argument to the function.
//     // insertIntoLeaf(relId, attrName, leafBlkNum, Index entry)
//     Index entry;
//     entry.attrVal = attrVal;
//     entry.block = recId.block;
//     entry.slot = recId.slot;

//     response = BPlusTree::insertIntoLeaf(relId, attrName, leafBlkNum, entry);

//     // NOTE: the insertIntoLeaf() function will propagate the insertion to the
//     //       required internal nodes by calling the required helper functions
//     //       like insertIntoInternal() or createNewRoot()

//     /*insertIntoLeaf() returns E_DISKFULL */
//     if (response == E_DISKFULL) {
//         // destroy the existing B+ tree by passing the rootBlock to bPlusDestroy().
//         response = BPlusTree::bPlusDestroy(blockNum);

//         if(response!=SUCCESS){
//             printf("failed to destroy tree after disk full error");
//             exit(1);
//         }

//         // update the rootBlock of attribute catalog cache entry to -1 using
//         attrCatEntry.rootBlock=-1;
//         // AttrCacheTable::setAttrCatEntry().
//         response = AttrCacheTable::setAttrCatEntry(relId, attrName, &attrCatEntry);

//         if(response!=SUCCESS){
//             printf("failed to set attrCatEntry for relId %d, attrName %s\n", relId, attrName);
//             exit(1);
//         }

//         return E_DISKFULL;
//     }

//     return SUCCESS;
// }

// int BPlusTree::findLeafToInsert(int rootBlock, Attribute attrVal, int attrType) {
//     int blockNum = rootBlock;
//     /* block is not of type IND_LEAF */
//     // use StaticBuffer::getStaticBlockType()
//     int blockType = StaticBuffer::getStaticBlockType(blockNum);
//     while (blockType != IND_LEAF) {

//         // declare an IndInternal object for block using appropriate constructor
//         IndInternal internalBlk(blockNum);

//         // get header of the block using BlockBuffer::getHeader()
//         HeadInfo headInfo;
//         int response = internalBlk.getHeader(&headInfo);

//         if (response != SUCCESS) {
//             printf("failed to get header for block %d\n", blockNum);
//             exit(1);
//         }


//         /* iterate through all the entries, to find the first entry whose
//              attribute value >= value to be inserted.
//              NOTE: the helper function compareAttrs() declared in BlockBuffer.h
//                    can be used to compare two Attribute values. */

//         InternalEntry internalEntry;
//         int index = 0;
//         while(index<headInfo.numEntries){
//             response = internalBlk.getEntry(&internalEntry, index);

//             if (response != SUCCESS) {
//                 printf("failed to get entry %d of block %d\n", index, blockNum);
//                 exit(1);
//             }

//             if (compareAttrs(internalEntry.attrVal, attrVal, attrType) > 0){
//                 break;
//             }
//             index++;
//         }
        

//         /*no such entry is found*/

//         if (index == headInfo.numEntries) {
//             // set blockNum = rChild of (nEntries-1)'th entry of the block     
//             // (i.e. rightmost child of the block)
//             blockNum = internalEntry.rChild;

//         } else {
//             // set blockNum = lChild of the entry that was found
//             blockNum = internalEntry.lChild;
//         }
//         blockType = StaticBuffer::getStaticBlockType(blockNum);

//     }

//     return blockNum;
// }

// int BPlusTree::insertIntoLeaf(int relId, char attrName[ATTR_SIZE], int blockNum, Index indexEntry) {
//     // get the attribute cache entry corresponding to attrName
//     // using AttrCacheTable::getAttrCatEntry().
//     AttrCatEntry attrCatEntry;
//     int response = AttrCacheTable::getAttrCatEntry(relId, attrName, &attrCatEntry);

//     if(response != SUCCESS){
//         printf("failed to get attrCatEntry for relId %d, attrName %s\n", relId, attrName);
//         exit(1);
//     }
//     // check if the block is a leaf block
//     // using StaticBuffer::getStaticBlockType()
//     int blockType = StaticBuffer::getStaticBlockType(blockNum);
//     if(blockType != IND_LEAF){
//         printf("block %d is not a leaf block\n", blockNum);
//         exit(1);
//     }

//     // declare an IndLeaf instance for the block using appropriate constructor
//     IndLeaf leaf(blockNum);

//     HeadInfo blockHeader;
//     // store the header of the leaf index block into blockHeader
//     // using BlockBuffer::getHeader()
//     response = leaf.getHeader(&blockHeader);

//     // the following variable will be used to store a list of index entries with
//     // existing indices + the new index to insert
//     Index indices[blockHeader.numEntries + 1];

//     /*
//     Iterate through all the entries in the block and copy them to the array indices.
//     Also insert `indexEntry` at appropriate position in the indices array maintaining
//     the ascending order.
//     - use IndLeaf::getEntry() to get the entry
//     - use compareAttrs() declared in BlockBuffer.h to compare two Attribute structs
//     */
//    int indicesArrayIndex = 0, recordEntry = 0;
//    bool inserted= false;
//    while(recordEntry<blockHeader.numEntries){
//        response = leaf.getEntry(&indices[indicesArrayIndex], recordEntry);
//        if(response!=SUCCESS){
//            printf("failed to get entry %d of block %d\n", recordEntry, blockNum);
//            exit(1);
//        }
//        if(!inserted && compareAttrs(indices[indicesArrayIndex].attrVal, indexEntry.attrVal, attrCatEntry.attrType) >= 0){
//            indices[indicesArrayIndex+1] = indices[indicesArrayIndex];
//            indices[indicesArrayIndex] = indexEntry;
//            indicesArrayIndex++;
//            inserted = true;
//        }
//        indicesArrayIndex++;
//        recordEntry++;
//     }
//     // insert `indexEntry` at appropriate position in the indices array maintaining the ascending order
//     // use compareAttrs() declared in BlockBuffer.h to compare two Attribute structs

 
//     if(!inserted){
//         indices[indicesArrayIndex] = indexEntry;
//     }
    

//     if (blockHeader.numEntries != MAX_KEYS_LEAF) {
//         // (leaf block has not reached max limit)


//         // increment blockHeader.numEntries and update the header of block
//         // using BlockBuffer::setHeader().
//         blockHeader.numEntries++;
//         response = leaf.setHeader(&blockHeader);

//         if (response != SUCCESS) {
//             printf("failed to set header for block %d\n", blockNum);
//             exit(1);
//         }

//         // iterate through all the entries of the array `indices` and populate the
//         // entries of block with them using IndLeaf::setEntry().
//         for (int i = 0; i < blockHeader.numEntries; i++) {
//             response = leaf.setEntry(&indices[i], i);
//             if (response != SUCCESS) {
//                 printf("failed to set entry %d of block %d\n", i, blockNum);
//                 exit(1);
//             }
//         }

//         return SUCCESS;
//     }

//     // If we reached here, the `indices` array has more than entries than can fit
//     // in a single leaf index block. Therefore, we will need to split the entries
//     // in `indices` between two leaf blocks. We do this using the splitLeaf() function.
//     // This function will return the blockNum of the newly allocated block or
//     // E_DISKFULL if there are no more blocks to be allocated.

//     int newRightBlk = splitLeaf(blockNum, indices);

//     // if splitLeaf() returned E_DISKFULL
//     //     return E_DISKFULL
//     if (newRightBlk == E_DISKFULL) {
//         return E_DISKFULL;
//     }
//     /* the current leaf block was not the root */
//     if (blockHeader.pblock != -1) {
//         // check pblock in header
//         // insert the middle value from `indices` into the parent block using the
//         // insertIntoInternal() function. (i.e the last value of the left block)

//         // the middle value will be at index 31 (given by constant MIDDLE_INDEX_LEAF)

//         // create a struct InternalEntry with attrVal = indices[MIDDLE_INDEX_LEAF].attrVal,
//         // lChild = currentBlock, rChild = newRightBlk and pass it as argument to
//         InternalEntry internalEntry;
//         internalEntry.attrVal = indices[MIDDLE_INDEX_LEAF].attrVal;
//         internalEntry.lChild = blockNum;
//         internalEntry.rChild = newRightBlk;


//         // the insertIntoInternalFunction as follows
//         // insertIntoInternal(relId, attrName, parent of current block, new internal entry)
//         response = BPlusTree::insertIntoInternal(relId, attrName, blockHeader.pblock, internalEntry);

//         if(response!=SUCCESS){
//             if(response == E_DISKFULL){
//                 return response;
//             }
//             printf("failed to insert into internal for relId %d, attrName %s, parent block %d\n", relId, attrName, blockHeader.pblock);
//             exit(1);
//         }

//     } else {
//         // the current block was the root block and is now split. a new internal index
//         // block needs to be allocated and made the root of the tree.
//         // To do this, call the createNewRoot() function with the following arguments

//         // createNewRoot(relId, attrName, indices[MIDDLE_INDEX_LEAF].attrVal,
//         //               current block, new right block)
//         response = BPlusTree::createNewRoot(relId, attrName, indices[MIDDLE_INDEX_LEAF].attrVal, blockNum, newRightBlk);

//         if(response!=SUCCESS){
//             if(response == E_DISKFULL){
//                 return response;
//             }
//             printf("failed to create new root for relId %d, attrName %s\n", relId, attrName);
//             exit(1);
//         }
//     }

//     // if either of the above calls returned an error (E_DISKFULL), then return that

//     // else return SUCCESS
//     return SUCCESS;
// }

// int BPlusTree::splitLeaf(int leafBlockNum, Index indices[]) {
//   // obtain new leaf index block to be used as the right block in the splitting
//   IndLeaf rightBlk;
//   // assign the existing block as the left block in the splitting.
//   IndLeaf leftBlk(leafBlockNum);

//   int rightBlkNum = rightBlk.getBlockNum();
//   int leftBlkNum = leftBlk.getBlockNum();

//   if (rightBlkNum == E_DISKFULL) {
//     return E_DISKFULL;
//   }

//   HeadInfo leftBlkHeader, rightBlkHeader;
//   leftBlk.getHeader(&leftBlkHeader);
//   rightBlk.getHeader(&rightBlkHeader);

//   rightBlkHeader.numEntries = (MAX_KEYS_LEAF + 1) / 2;
//   rightBlkHeader.rblock = leftBlkHeader.rblock;
//   rightBlkHeader.pblock = leftBlkHeader.pblock;
//   rightBlkHeader.lblock = leftBlkNum;
//   rightBlk.setHeader(&rightBlkHeader);

//   leftBlkHeader.numEntries = (MAX_KEYS_LEAF + 1) / 2;
//   leftBlkHeader.rblock = rightBlkNum;
//   leftBlk.setHeader(&leftBlkHeader);

//   for (int i = 0; i < leftBlkHeader.numEntries; ++i) {
//     leftBlk.setEntry(indices + i, i);
//     rightBlk.setEntry(indices + MIDDLE_INDEX_LEAF + i + 1, i);
//   }

//   return rightBlkNum;
// }

// int BPlusTree::insertIntoInternal(int relId, char attrName[ATTR_SIZE], int intBlockNum, InternalEntry intEntry) {
//     // get the attribute cache entry corresponding to attrName
//     // using AttrCacheTable::getAttrCatEntry().
//     AttrCatEntry attrCatEntry;
//     int response = AttrCacheTable::getAttrCatEntry(relId, attrName, &attrCatEntry);

//     if(response!=SUCCESS){
//         printf("failed to get attrCatEntry for relId %d, attrName %s\n", relId, attrName);
//         exit(1);
//     }

//     // declare intBlk, an instance of IndInternal using constructor 2 for the block
//     // corresponding to intBlockNum
//     IndInternal intBlk(intBlockNum);

//     HeadInfo blockHeader;
//     // load blockHeader with header of intBlk using BlockBuffer::getHeader().
//     response = intBlk.getHeader(&blockHeader);

//     if (response != SUCCESS) {
//         printf("failed to get header for block %d\n", intBlockNum);
//         exit(1);
//     }

//     // declare internalEntries to store all existing entries + the new entry
//     InternalEntry internalEntries[blockHeader.numEntries + 1];

//     /*
//     Iterate through all the entries in the block and copy them to the array
//     `internalEntries`. Insert `indexEntry` at appropriate position in the
//     array maintaining the ascending order.
//         - use IndInternal::getEntry() to get the entry
//         - use compareAttrs() to compare two structs of type Attribute

//     Update the lChild of the internalEntry immediately following the newly added
//     entry to the rChild of the newly added entry.
//     */
//     int indicesArrayIndex = 0, recordEntry = 0;
//     bool inserted = false;
//     while (recordEntry < blockHeader.numEntries) {
//         response = intBlk.getEntry(&internalEntries[indicesArrayIndex], recordEntry);
//         if (response != SUCCESS) {
//             printf("failed to get entry %d of block %d\n", recordEntry, intBlockNum);
//             exit(1);
//         }
//         if (!inserted && compareAttrs(internalEntries[indicesArrayIndex].attrVal, intEntry.attrVal, attrCatEntry.attrType) >= 0) {
//             internalEntries[indicesArrayIndex + 1] = internalEntries[indicesArrayIndex];
//             internalEntries[indicesArrayIndex] = intEntry;
//             internalEntries[indicesArrayIndex].lChild = internalEntries[indicesArrayIndex + 1].rChild;
//             indicesArrayIndex++;
//             inserted = true;
//         }
//         indicesArrayIndex++;
//         recordEntry++;
//     }
//     if(!inserted){
//         internalEntries[indicesArrayIndex] = intEntry;
//     }


//     if (blockHeader.numEntries != MAX_KEYS_INTERNAL) {
//         // (internal index block has not reached max limit)

//         // increment blockheader.numEntries and update the header of intBlk
//         // using BlockBuffer::setHeader().
//         blockHeader.numEntries++;
//         response = intBlk.setHeader(&blockHeader);

//         // iterate through all entries in internalEntries array and populate the
//         // entries of intBlk with them using IndInternal::setEntry().
//         for (int i = 0; i < blockHeader.numEntries; i++) {
//             response = intBlk.setEntry(&internalEntries[i], i);
//             if (response != SUCCESS) {
//                 printf("failed to set entry %d of block %d\n", i, intBlockNum);
//                 exit(1);
//             }
//         }

//         return SUCCESS;
//     }

//     // If we reached here, the `internalEntries` array has more than entries than
//     // can fit in a single internal index block. Therefore, we will need to split
//     // the entries in `internalEntries` between two internal index blocks. We do
//     // this using the splitInternal() function.
//     // This function will return the blockNum of the newly allocated block or
//     // E_DISKFULL if there are no more blocks to be allocated.

//     int newRightBlk = splitInternal(intBlockNum, internalEntries);
    
//     /* splitInternal() returned E_DISKFULL */
//     if (newRightBlk == E_DISKFULL) {

//         // Using bPlusDestroy(), destroy the right subtree, rooted at intEntry.rChild.
//         // This corresponds to the tree built up till now that has not yet been
//         // connected to the existing B+ Tree
//         response = bPlusDestroy(intEntry.rChild);
//         if (response != SUCCESS) {
//             printf("failed to destroy tree after disk full error");
//             exit(1);
//         }

//         return E_DISKFULL;
//     }
//     /* the current block was not the root */
//     if (blockHeader.pblock != -1) {   // (check pblock in header)
//         // insert the middle value from `internalEntries` into the parent block
//         // using the insertIntoInternal() function (recursively).



//         // the middle value will be at index 50 (given by constant MIDDLE_INDEX_INTERNAL)


//         // create a struct InternalEntry with lChild = current block, rChild = newRightBlk
//         // and attrVal = internalEntries[MIDDLE_INDEX_INTERNAL].attrVal
//         InternalEntry internalEntry;
//         internalEntry.lChild = intBlockNum;
//         internalEntry.rChild = newRightBlk;
//         internalEntry.attrVal = internalEntries[MIDDLE_INDEX_INTERNAL].attrVal;

//         // and pass it as argument to the insertIntoInternalFunction as follows

//         // insertIntoInternal(relId, attrName, parent of current block, new internal entry)
//         response = BPlusTree::insertIntoInternal(relId, attrName, blockHeader.pblock, internalEntry);

//         if(response!=SUCCESS){
//             if(response == E_DISKFULL){
//                 return response;
//             }
//             printf("failed to insert into internal for relId %d, attrName %s, parent block %d\n", relId, attrName, blockHeader.pblock);
//             exit(1);
//         }

//     } else {
//         // the current block was the root block and is now split. a new internal index
//         // block needs to be allocated and made the root of the tree.
//         // To do this, call the createNewRoot() function with the following arguments

//         // createNewRoot(relId, attrName,
//         //               internalEntries[MIDDLE_INDEX_INTERNAL].attrVal,
//         //               current block, new right block)
//         response = BPlusTree::createNewRoot(relId, attrName, internalEntries[MIDDLE_INDEX_INTERNAL].attrVal, intBlockNum, newRightBlk);

//         if(response!=SUCCESS){
//             if(response == E_DISKFULL){
//                 return response;
//             }
//             printf("failed to create new root for relId %d, attrName %s\n", relId, attrName);
//             exit(1);
//         }
//     }

//     // if either of the above calls returned an error (E_DISKFULL), then return that
//     // else return SUCCESS
//     return SUCCESS;
// }


// int BPlusTree::splitInternal(int intBlockNum, InternalEntry internalEntries[]) {
//     // declare rightBlk, an instance of IndInternal using constructor 1 to obtain new
//     // internal index block that will be used as the right block in the splitting
//     IndInternal rightBlk;

//     // declare leftBlk, an instance of IndInternal using constructor 2 to read from
//     // the existing internal index block
//     IndInternal leftBlk(intBlockNum);

//     /* block num of right blk */;
//     int rightBlkNum = rightBlk.getBlockNum();
//     /* block num of left blk */;
//     int leftBlkNum = leftBlk.getBlockNum();

//     /* newly allocated block has blockNum E_DISKFULL */
//     if (rightBlkNum == E_DISKFULL) {
//         //(failed to obtain a new internal index block because the disk is full)
//         return E_DISKFULL;
//     }

//     HeadInfo leftBlkHeader, rightBlkHeader;
//     // get the headers of left block and right block using BlockBuffer::getHeader()

//     int response = leftBlk.getHeader(&leftBlkHeader);

//     if (response != SUCCESS) {
//         printf("failed to get header for block %d\n", leftBlkNum);
//         exit(1);
//     }

//     response = rightBlk.getHeader(&rightBlkHeader);

//     if (response != SUCCESS) {
//         printf("failed to get header for block %d\n", rightBlkNum);
//         exit(1);
//     }

//     // set rightBlkHeader with the following values
//     // - number of entries = (MAX_KEYS_INTERNAL)/2 = 50
//     // - pblock = pblock of leftBlk
//     // and update the header of rightBlk using BlockBuffer::setHeader()

//     rightBlkHeader.numEntries = (MAX_KEYS_INTERNAL)/2;
//     rightBlkHeader.pblock = leftBlkHeader.pblock;

//     response = rightBlk.setHeader(&rightBlkHeader);

//     if (response != SUCCESS) {
//         printf("failed to set header for block %d\n", rightBlkNum);
//         exit(1);
//     }

//     // set leftBlkHeader with the following values
//     // - number of entries = (MAX_KEYS_INTERNAL)/2 = 50
//     // - rblock = rightBlkNum
//     // and update the header using BlockBuffer::setHeader()

//     leftBlkHeader.numEntries = (MAX_KEYS_INTERNAL)/2;
//     leftBlkHeader.rblock = rightBlkNum;

//     response = leftBlk.setHeader(&leftBlkHeader);

//     if (response != SUCCESS) {
//         printf("failed to set header for block %d\n", leftBlkNum);
//         exit(1);
//     }


//     /*
//     - set the first 50 entries of leftBlk = index 0 to 49 of internalEntries
//       array
//     - set the first 50 entries of newRightBlk = entries from index 51 to 100
//       of internalEntries array using IndInternal::setEntry().
//       (index 50 will be moving to the parent internal index block)
//     */

//     for (int i = 0; i < (MAX_KEYS_INTERNAL)/2; i++) {
//         response = leftBlk.setEntry(&internalEntries[i], i);
//         if (response != SUCCESS) {
//             printf("failed to set entry %d of block %d\n", i, leftBlkNum);
//             exit(1);
//         }

//         response = rightBlk.setEntry(&internalEntries[i + (MAX_KEYS_INTERNAL)/2+1], i);
//         if (response != SUCCESS) {
//             printf("failed to set entry %d of block %d\n", i, rightBlkNum);
//             exit(1);
//         }
//     }


//     /* block type of a child of any entry of the internalEntries array */;
//     //            (use StaticBuffer::getStaticBlockType())
//     int type = StaticBuffer::getStaticBlockType(internalEntries[0].rChild);
//     /* each child block of the new right block */
//     for (int i = 0; i < (MAX_KEYS_INTERNAL)/2 + 1; i++) {
//         // declare an instance of BlockBuffer to access the child block using
//         // constructor 2
//         BlockBuffer childBlk(internalEntries[i + (MAX_KEYS_INTERNAL)/2].lChild);

//         // update pblock of the block to rightBlkNum using BlockBuffer::getHeader()
//         HeadInfo childHeadInfo;
//         response = childBlk.getHeader(&childHeadInfo);

//         if (response != SUCCESS) {
//             printf("failed to get header for block %d\n", internalEntries[i + (MAX_KEYS_INTERNAL)/2].lChild);
//             exit(1);
//         }
//         // and BlockBuffer::setHeader().
//         childHeadInfo.pblock = rightBlkNum;
//     }

//     return rightBlkNum;
// }


// int BPlusTree::createNewRoot(int relId, char attrName[ATTR_SIZE], Attribute attrVal, int lChild, int rChild) {
//     // get the attribute cache entry corresponding to attrName
//     // using AttrCacheTable::getAttrCatEntry().
//     AttrCatEntry attrCatEntry;
//     int response = AttrCacheTable::getAttrCatEntry(relId, attrName, &attrCatEntry);

//     if(response!=SUCCESS){
//         printf("failed to get attrCatEntry for relId %d, attrName %s\n", relId, attrName);
//         exit(1);
//     }

//     // declare newRootBlk, an instance of IndInternal using appropriate constructor
//     // to allocate a new internal index block on the disk
//     IndInternal newRootBlk;


//     /* block number of newRootBlk */;
//     int newRootBlkNum = newRootBlk.getBlockNum();

//     if (newRootBlkNum == E_DISKFULL) {
//         // (failed to obtain an empty internal index block because the disk is full)

//         // Using bPlusDestroy(), destroy the right subtree, rooted at rChild.
//         response = BPlusTree::bPlusDestroy(rChild);
//         // This corresponds to the tree built up till now that has not yet been
//         // connected to the existing B+ Tree

//         return E_DISKFULL;
//     }

//     // update the header of the new block with numEntries = 1 using
//     // BlockBuffer::getHeader() and BlockBuffer::setHeader()
//     HeadInfo headInfo;
//     response = newRootBlk.getHeader(&headInfo);

//     if (response != SUCCESS) {
//         printf("failed to get header for block %d\n", newRootBlkNum);
//         exit(1);
//     }

//     headInfo.numEntries = 1;

//     response = newRootBlk.setHeader(&headInfo);

//     // create a struct InternalEntry with lChild, attrVal and rChild from the
//     // arguments and set it as the first entry in newRootBlk using IndInternal::setEntry()
//     InternalEntry internalEntry;
//     internalEntry.lChild = lChild;
//     internalEntry.attrVal = attrVal;
//     internalEntry.rChild = rChild;

//     response = newRootBlk.setEntry(&internalEntry, 0);

//     // declare BlockBuffer instances for the `lChild` and `rChild` blocks using
//     // appropriate constructor and update the pblock of those blocks to `newRootBlkNum`
//     // using BlockBuffer::getHeader() and BlockBuffer::setHeader()
//     BlockBuffer lChildBlk(lChild);
//     BlockBuffer rChildBlk(rChild);

//     HeadInfo lChildHeadInfo, rChildHeadInfo;
//     response = lChildBlk.getHeader(&lChildHeadInfo);

//     if (response != SUCCESS) {
//         printf("failed to get header for block %d\n", lChild);
//         exit(1);
//     }

//     lChildHeadInfo.pblock = newRootBlkNum;

//     response = lChildBlk.setHeader(&lChildHeadInfo);

//     if (response != SUCCESS) {
//         printf("failed to set header for block %d\n", lChild);
//         exit(1);
//     }

//     response = rChildBlk.getHeader(&rChildHeadInfo);

//     if (response != SUCCESS) {
//         printf("failed to get header for block %d\n", rChild);
//         exit(1);
//     }

//     rChildHeadInfo.pblock = newRootBlkNum;

//     response = rChildBlk.setHeader(&rChildHeadInfo);

//     if (response != SUCCESS) {
//         printf("failed to set header for block %d\n", rChild);
//         exit(1);
//     }

//     // update rootBlock = newRootBlkNum for the entry corresponding to `attrName`
//     // in the attribute cache using AttrCacheTable::setAttrCatEntry().

//     attrCatEntry.rootBlock = newRootBlkNum;

//     response = AttrCacheTable::setAttrCatEntry(relId, attrName, &attrCatEntry);

//     if(response!=SUCCESS){
//         printf("failed to set attrCatEntry for relId %d, attrName %s\n", relId, attrName);
//         exit(1);
//     }

//     return SUCCESS;
// }
