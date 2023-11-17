#include "AttrCacheTable.h"

#include <cstring>

AttrCacheEntry *AttrCacheTable::attrCache[MAX_OPEN];

//* returns the attrOffset-th attribute for the relation corresponding to relId
//* NOTE: this function expects the caller to allocate memory for `*attrCatBuf`
int AttrCacheTable::getAttrCatEntry(int relId, int attrOffset, AttrCatEntry *attrCatBuf)
{
    //! check if 0 <= relId < MAX_OPEN and return E_OUTOFBOUND otherwise
    if (relId < 0 || relId >= MAX_OPEN)
        return E_OUTOFBOUND;

    //! check if attrCache[relId] == nullptr and return E_RELNOTOPEN if true
    if (attrCache[relId] == nullptr)
        return E_RELNOTOPEN;

    // traverse the linked list of attribute cache entries
    for (AttrCacheEntry *entry = attrCache[relId]; entry != nullptr; entry = entry->next)
    {
        if (entry->attrCatEntry.offset == attrOffset)
        {
            // TODO: copy entry->attrCatEntry to *attrCatBuf and return SUCCESS;
            strcpy(attrCatBuf->relName, entry->attrCatEntry.relName);
            strcpy(attrCatBuf->attrName, entry->attrCatEntry.attrName);

            attrCatBuf->attrType = entry->attrCatEntry.attrType;
            attrCatBuf->primaryFlag = entry->attrCatEntry.primaryFlag;
            attrCatBuf->rootBlock = entry->attrCatEntry.rootBlock;
            attrCatBuf->offset = entry->attrCatEntry.offset;

            return SUCCESS;
        }
    }

    //! there is no attribute at this offset
    return E_ATTRNOTEXIST;
}

//* returns the attribute with name `attrName` for the relation corresponding to relId
//* NOTE: this function expects the caller to allocate memory for `*attrCatBuf`
int AttrCacheTable::getAttrCatEntry(int relId, char attrName[ATTR_SIZE], AttrCatEntry *attrCatBuf)
{

    // TODO: check that relId is valid and corresponds to an open relation
    //! check if 0 <= relId < MAX_OPEN and return E_OUTOFBOUND otherwise
    if (relId < 0 || relId >= MAX_OPEN)
        return E_OUTOFBOUND;

    //! check if attrCache[relId] == nullptr and return E_RELNOTOPEN if true
    if (attrCache[relId] == nullptr)
        return E_RELNOTOPEN;

    // TODO: iterate over the entries in the attribute cache and set attrCatBuf to the entry that matches attrName
    // TODO: traverse the linked list of attribute cache entries
    for (AttrCacheEntry *entry = attrCache[relId]; entry != nullptr; entry = entry->next)
    {
        if (strcmp(entry->attrCatEntry.attrName, attrName) == 0) // matching the name
        {
            // TODO: copy entry->attrCatEntry to *attrCatBuf and return SUCCESS;
            strcpy(attrCatBuf->relName, entry->attrCatEntry.relName);
            strcpy(attrCatBuf->attrName, entry->attrCatEntry.attrName);

            attrCatBuf->attrType = entry->attrCatEntry.attrType;
            attrCatBuf->primaryFlag = entry->attrCatEntry.primaryFlag;
            attrCatBuf->rootBlock = entry->attrCatEntry.rootBlock;
            attrCatBuf->offset = entry->attrCatEntry.offset;

            return SUCCESS;
        }
    }


    //! no attribute with name attrName for the relation
    return E_ATTRNOTEXIST;
}

/*  
    * Converts a attribute catalog record to AttrCatEntry struct
    * We get the record as Attribute[] from the BlockBuffer.getRecord() function.
    * This function will convert that to a struct AttrCatEntry type.
*/
void AttrCacheTable::recordToAttrCatEntry(union Attribute record[ATTRCAT_NO_ATTRS],
                                          AttrCatEntry *attrCatEntry)
{
    strcpy(attrCatEntry->relName, record[ATTRCAT_REL_NAME_INDEX].sVal);
    strcpy(attrCatEntry->attrName, record[ATTRCAT_ATTR_NAME_INDEX].sVal);

    attrCatEntry->attrType = record[ATTRCAT_ATTR_TYPE_INDEX].nVal;
    attrCatEntry->primaryFlag = record[ATTRCAT_PRIMARY_FLAG_INDEX].nVal;
    attrCatEntry->rootBlock = record[ATTRCAT_ROOT_BLOCK_INDEX].nVal;
    attrCatEntry->offset = record[ATTRCAT_OFFSET_INDEX].nVal;

    // copy the rest of the fields in the record to the attrCacheEntry struct
}

void AttrCacheTable::attrCatEntryToRecord(AttrCatEntry *attrCatEntry, Attribute record[ATTRCAT_NO_ATTRS])
{
    strcpy(record[ATTRCAT_REL_NAME_INDEX].sVal, attrCatEntry->relName);
    strcpy(record[ATTRCAT_ATTR_NAME_INDEX].sVal, attrCatEntry->attrName);

    record[ATTRCAT_ATTR_TYPE_INDEX].nVal = attrCatEntry->attrType;
    record[ATTRCAT_PRIMARY_FLAG_INDEX].nVal = attrCatEntry->primaryFlag;
    record[ATTRCAT_ROOT_BLOCK_INDEX].nVal = attrCatEntry->rootBlock;
    record[ATTRCAT_OFFSET_INDEX].nVal = attrCatEntry->offset;

    // copy the rest of the fields in the record to the attrCacheEntry struct
}

int AttrCacheTable::setSearchIndex(int relId, char attrName[ATTR_SIZE], IndexId *searchIndex)
{
    if (relId < 0 || relId >= MAX_OPEN) return E_OUTOFBOUND;

    AttrCacheEntry *curr = AttrCacheTable::attrCache[relId];
    while (curr) {
        if (strcmp(curr->attrCatEntry.attrName, attrName) == 0)
        {
            curr->searchIndex = *searchIndex;
            return SUCCESS;
        }
        curr = curr->next;
    }

    return E_ATTRNOTEXIST;
}

int AttrCacheTable::setSearchIndex(int relId, int attrOffset, IndexId *searchIndex)
{
    if (relId < 0 || relId >= MAX_OPEN) return E_OUTOFBOUND;

    AttrCacheEntry *curr = AttrCacheTable::attrCache[relId];
    int index = 0;

    while (curr) {
        if (index == attrOffset)
        {
            curr->searchIndex = *searchIndex;
            return SUCCESS;
        }
        curr = curr->next;
        index++;
    }

    return E_ATTRNOTEXIST;
}

int AttrCacheTable::resetSearchIndex(int relId, char attrName[ATTR_SIZE])
{
    // curr->searchIndex = RecId{-1, -1};
    IndexId indexId = {-1, -1};
    return AttrCacheTable::setSearchIndex(relId, attrName, &indexId);
}

int AttrCacheTable::resetSearchIndex(int relId, int attrOffset)
{
    // curr->searchIndex = RecId{-1, -1};
    IndexId indexId = {-1, -1};
    return AttrCacheTable::setSearchIndex(relId, attrOffset, &indexId);
}

int AttrCacheTable::getAttributeOffset(int relId, char attrName[ATTR_SIZE]) {
    if (relId < 0 || relId >= MAX_OPEN) return E_RELNOTOPEN;

    AttrCacheEntry *current = AttrCacheTable::attrCache[relId];
    int attrOffset = 0;

    while (current) {
        if (strcmp (attrName, current->attrCatEntry.attrName) == 0)
        {
            return attrOffset;
        }
        current = current->next;
        attrOffset++;
    }

    return E_ATTRNOTEXIST;
}

int AttrCacheTable::getSearchIndex(int relId, char attrName[ATTR_SIZE], IndexId *searchIndex)
{
    if (relId < 0 || relId >= MAX_OPEN) return E_OUTOFBOUND;

    AttrCacheEntry *curr = AttrCacheTable::attrCache[relId];
    while (curr) {
        if (strcmp(curr->attrCatEntry.attrName, attrName) == 0)
        {
            *searchIndex = curr->searchIndex;
            return SUCCESS;
        }
        curr = curr->next;
    }

    return E_ATTRNOTEXIST;
}

int AttrCacheTable::getSearchIndex(int relId, int attrOffset, IndexId *searchIndex)
{
    if (relId < 0 || relId >= MAX_OPEN) return E_OUTOFBOUND;

    AttrCacheEntry *curr = AttrCacheTable::attrCache[relId];
    int index = 0;

    while (curr) {
        if (index == attrOffset)
        {
            *searchIndex = curr->searchIndex;
            return SUCCESS;
        }
        curr = curr->next;
        index++;
    }

    return E_ATTRNOTEXIST;
}

//* returns the attrOffset-th attribute for the relation corresponding to relId
//* NOTE: this function expects the caller to allocate memory for `*attrCatBuf`
int AttrCacheTable::setAttrCatEntry(int relId, int attrOffset, AttrCatEntry *attrCatBuf)
{
    //! check if 0 <= relId < MAX_OPEN and return E_OUTOFBOUND otherwise
    if (relId < 0 || relId >= MAX_OPEN)
        return E_OUTOFBOUND;

    //! check if attrCache[relId] == nullptr and return E_RELNOTOPEN if true
    if (attrCache[relId] == nullptr)
        return E_RELNOTOPEN;

    // traverse the linked list of attribute cache entries
    for (AttrCacheEntry *entry = attrCache[relId]; entry != nullptr; entry = entry->next)
    {
        if (entry->attrCatEntry.offset == attrOffset)
        {
            // TODO: copy entry->attrCatEntry to *attrCatBuf and return SUCCESS;
            strcpy(entry->attrCatEntry.relName, attrCatBuf->relName);
            strcpy(entry->attrCatEntry.attrName, attrCatBuf->attrName);

            entry->attrCatEntry.attrType = attrCatBuf->attrType;
            entry->attrCatEntry.primaryFlag = attrCatBuf->primaryFlag;
            entry->attrCatEntry.rootBlock = attrCatBuf->rootBlock;
            entry->attrCatEntry.offset = attrCatBuf->offset;

            entry->dirty = true;

            return SUCCESS;
        }
    }

    //! there is no attribute at this offset
    return E_ATTRNOTEXIST;
}

//* returns the attribute with name `attrName` for the relation corresponding to relId
//* NOTE: this function expects the caller to allocate memory for `*attrCatBuf`
int AttrCacheTable::setAttrCatEntry(int relId, char attrName[ATTR_SIZE], AttrCatEntry *attrCatBuf)
{

    // TODO: check that relId is valid and corresponds to an open relation
    //! check if 0 <= relId < MAX_OPEN and return E_OUTOFBOUND otherwise
    if (relId < 0 || relId >= MAX_OPEN)
        return E_OUTOFBOUND;

    //! check if attrCache[relId] == nullptr and return E_RELNOTOPEN if true
    if (attrCache[relId] == nullptr)
        return E_RELNOTOPEN;

    // TODO: iterate over the entries in the attribute cache and set attrCatBuf to the entry that matches attrName
    // TODO: traverse the linked list of attribute cache entries
    for (AttrCacheEntry *entry = attrCache[relId]; entry != nullptr; entry = entry->next)
    {
        if (strcmp(entry->attrCatEntry.attrName, attrName) == 0) // matching the name
        {
            // TODO: copy entry->attrCatEntry to *attrCatBuf and return SUCCESS;
            strcpy(entry->attrCatEntry.relName, attrCatBuf->relName);
            strcpy(entry->attrCatEntry.attrName, attrCatBuf->attrName);

            entry->attrCatEntry.attrType = attrCatBuf->attrType;
            entry->attrCatEntry.primaryFlag = attrCatBuf->primaryFlag;
            entry->attrCatEntry.rootBlock = attrCatBuf->rootBlock;
            entry->attrCatEntry.offset = attrCatBuf->offset;

            entry->dirty = true;

            return SUCCESS;
        }
    }


    //! no attribute with name attrName for the relation
    return E_ATTRNOTEXIST;
}