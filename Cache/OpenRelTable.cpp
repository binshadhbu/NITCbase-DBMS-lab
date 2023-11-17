#include "OpenRelTable.h"

#include <cstring>
#include <stdlib.h>
#include <stdio.h>

OpenRelTableMetaInfo OpenRelTable::tableMetaInfo[MAX_OPEN];

inline bool operator == (const RecId& lhs, const RecId& rhs)
{
    return (lhs.block == rhs.block && lhs.slot == rhs.slot);
}

AttrCacheEntry *createAttrCacheEntryList(int size)
{
	AttrCacheEntry *head = nullptr, *curr = nullptr;
	head = curr = (AttrCacheEntry *)malloc(sizeof(AttrCacheEntry));
	size--;
	while (size--)
	{
		curr->next = (AttrCacheEntry *)malloc(sizeof(AttrCacheEntry));
		curr = curr->next;
	}
	curr->next = nullptr;

	return head;
}

/* This function will open a relation having name `relName`.
Since we are currently only working with the relation and attribute catalog, we
will just hardcode it. In subsequent stages, we will loop through all the relations
and open the appropriate one.
*/

OpenRelTable::OpenRelTable()
{
	// initialise all values in relCache and attrCache to be 
	// nullptr and all entries in tableMetaInfo to be free
	for (int i = 0; i < MAX_OPEN; ++i)
	{
		RelCacheTable::relCache[i] = nullptr;
		AttrCacheTable::attrCache[i] = nullptr;
		tableMetaInfo[i].free = true;
	}

	// load the relation and attribute catalog into the relation cache (we did this already)
	
	// setting up the variables
	RecBuffer relCatBlock(RELCAT_BLOCK);
	Attribute relCatRecord[RELCAT_NO_ATTRS];
	RelCacheEntry *relCacheEntry = nullptr;

	for (int relId = RELCAT_RELID; relId <= ATTRCAT_RELID; relId++)
	{
		relCatBlock.getRecord(relCatRecord, relId);

		relCacheEntry = (RelCacheEntry *)malloc(sizeof(RelCacheEntry));
		RelCacheTable::recordToRelCatEntry(relCatRecord, &(relCacheEntry->relCatEntry));
		relCacheEntry->recId.block = RELCAT_BLOCK;
		relCacheEntry->recId.slot = relId;

		relCacheEntry->searchIndex = {-1, -1};

		RelCacheTable::relCache[relId] = relCacheEntry;
	}

	// load the relation and attribute catalog into the attribute cache (we did this already)
	
	// setting up the variables
	RecBuffer attrCatBlock(ATTRCAT_BLOCK);
	Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
	AttrCacheEntry *attrCacheEntry = nullptr, *head = nullptr;

	for (int relId = RELCAT_RELID, recordId = 0; relId <= ATTRCAT_RELID; relId++)
	{
		int numberOfAttributes = RelCacheTable::relCache[relId]->relCatEntry.numAttrs;
		head = createAttrCacheEntryList(numberOfAttributes);
		attrCacheEntry = head;

		while (numberOfAttributes--)
		{
			attrCatBlock.getRecord(attrCatRecord, recordId);

			AttrCacheTable::recordToAttrCatEntry(
				attrCatRecord,
				&(attrCacheEntry->attrCatEntry));
			attrCacheEntry->recId.slot = recordId++;
			attrCacheEntry->recId.block = ATTRCAT_BLOCK;

			attrCacheEntry = attrCacheEntry->next;
		}

		AttrCacheTable::attrCache[relId] = head;
	}

	/************ Setting up tableMetaInfo entries ************/

	tableMetaInfo[RELCAT_RELID].free = false; 
	strcpy(tableMetaInfo[RELCAT_RELID].relName, RELCAT_RELNAME);

	tableMetaInfo[ATTRCAT_RELID].free = false; 
	strcpy(tableMetaInfo[ATTRCAT_RELID].relName, ATTRCAT_RELNAME);
}

// // OpenRelTable::~OpenRelTable()
// // {
// // 	// free all the memory that you allocated in the constructor
// 
// // 	//? close all open relations (from rel-id = 2 onwards. Why?)
// // 	for (int i = 2; i < MAX_OPEN; ++i)
// // 		if (!tableMetaInfo[i].free)
// // 			OpenRelTable::closeRel(i); // we will implement this function later
// 
// // 	// free the memory allocated for rel-id 0 and 1 in the caches
// // }

OpenRelTable::~OpenRelTable() {

    // for i from 2 to MAX_OPEN-1:
	for (int relId = 2; relId < MAX_OPEN; relId++)
    {
        // if ith relation is still open:
		if (OpenRelTable::tableMetaInfo[relId].free == false)
            // close the relation using openRelTable::closeRel().
			OpenRelTable::closeRel(relId);
    }

    // * Closing the catalog relations in the relation cache
    // TODO: release the relation cache entry of the attribute catalog
    // if RelCatEntry of the ATTRCAT_RELID-th RelCacheEntry has been modified 
	if (RelCacheTable::relCache[ATTRCAT_RELID]->dirty)
	{
        /* Get the Relation Catalog entry from RelCacheTable::relCache
        Then convert it to a record using RelCacheTable::relCatEntryToRecord(). */
		RelCatEntry relCatBuffer;
		RelCacheTable::getRelCatEntry(ATTRCAT_RELID, &relCatBuffer);

		Attribute relCatRecord [RELCAT_NO_ATTRS];
		RelCacheTable::relCatEntryToRecord(&relCatBuffer, relCatRecord);

		RecId recId = RelCacheTable::relCache[ATTRCAT_RELID]->recId;

        // declaring an object of RecBuffer class to write back to the buffer
        RecBuffer relCatBlock(recId.block);

        // Write back to the buffer using relCatBlock.setRecord() with recId.slot
		relCatBlock.setRecord(relCatRecord, recId.slot);
    }
    // free the memory dynamically allocated to this RelCacheEntry
	free(RelCacheTable::relCache[ATTRCAT_RELID]);

    // TODO: release the relation cache entry of the relation catalog
    // if RelCatEntry of the RELCAT_RELID-th RelCacheEntry has been modified
	if (RelCacheTable::relCache[RELCAT_RELID]->dirty)
	{
        /* Get the Relation Catalog entry from RelCacheTable::relCache
        Then convert it to a record using RelCacheTable::relCatEntryToRecord(). */
		RelCatEntry relCatBuffer;
		RelCacheTable::getRelCatEntry(RELCAT_RELID, &relCatBuffer);

		Attribute relCatRecord [RELCAT_NO_ATTRS];
		RelCacheTable::relCatEntryToRecord(&relCatBuffer, relCatRecord);

		RecId recId = RelCacheTable::relCache[RELCAT_RELID]->recId;

        // declaring an object of RecBuffer class to write back to the buffer
        RecBuffer relCatBlock(recId.block);

        // Write back to the buffer using relCatBlock.setRecord() with recId.slot
		relCatBlock.setRecord(relCatRecord, recId.slot);
    }
    // free the memory dynamically allocated for this RelCacheEntry
	free(RelCacheTable::relCache[RELCAT_RELID]);

    // free the memory allocated for the attribute cache entries of the
    // relation catalog and the attribute catalog
	for (int relId = ATTRCAT_RELID; relId >= RELCAT_RELID; relId--)
	{
		AttrCacheEntry *curr = AttrCacheTable::attrCache[relId], *next = nullptr;
		for (int attrIndex = 0; attrIndex < 6; attrIndex++)
		{
			next = curr->next;

			// check if the AttrCatEntry was written back
			if (curr->dirty)
			{
				AttrCatEntry attrCatBuffer;
				AttrCacheTable::getAttrCatEntry(relId, attrIndex, &attrCatBuffer);

				Attribute attrCatRecord [ATTRCAT_NO_ATTRS];
				AttrCacheTable::attrCatEntryToRecord(&attrCatBuffer, attrCatRecord);

				RecId recId = curr->recId;

				// declaring an object if RecBuffer class to write back to the buffer
				RecBuffer attrCatBlock (recId.block);

				// write back to the buffer using RecBufer.setRecord()
				attrCatBlock.setRecord(attrCatRecord, recId.slot);
			}

			free (curr);
			curr = next;
		}
	}
}

int OpenRelTable::getFreeOpenRelTableEntry()
{
	// traverse through the tableMetaInfo array, and
	// find a free entry in the Open Relation Table.

	for (int relId = 0; relId < MAX_OPEN; relId++)
		if (tableMetaInfo[relId].free)
			return relId;

	//! if found return the relation id, else return E_CACHEFULL.
	return E_CACHEFULL;
}

int OpenRelTable::getRelId(char relName[ATTR_SIZE]) 
{
  	// traverse through the tableMetaInfo array,
	// find the entry in the Open Relation Table corresponding to relName.*/
	for (int relId = 0; relId < MAX_OPEN; relId++) 
		if (strcmp(tableMetaInfo[relId].relName, relName) == 0
			&& tableMetaInfo[relId].free == false)
			return relId;

  	// if found return the relation id, else indicate that the relation 
	// do not have an entry in the Open Relation Table.
	return E_RELNOTOPEN;
}

int OpenRelTable::openRel(char relName[ATTR_SIZE]) 
{
	// (checked using OpenRelTable::getRelId())
	int relId = getRelId(relName);
  	if(relId >= 0){
    	// return that relation id;
		return relId;
  	}

  	// TODO: find a free slot in the Open Relation Table
    // TODO: using OpenRelTable::getFreeOpenRelTableEntry().

  	// let relId be used to store the free slot.
	relId = OpenRelTable::getFreeOpenRelTableEntry();
  	if (relId == E_CACHEFULL) return E_CACHEFULL;

  	/****** Setting up Relation Cache entry for the relation ******/

  	// TODO: search for the entry with relation name, relName, 
	// TODO: in the Relation Catalog using BlockAccess::linearSearch().
    //* Care should be taken to reset the searchIndex of the relation RELCAT_RELID
    //* before calling linearSearch().

  	// relcatRecId stores the rec-id of the relation `relName` in the Relation Catalog.
	Attribute attrVal; strcpy(attrVal.sVal, relName);
	RelCacheTable::resetSearchIndex(RELCAT_RELID);

  	RecId relcatRecId = BlockAccess::linearSearch(RELCAT_RELID, RELCAT_ATTR_RELNAME, attrVal, EQ);

	if (relcatRecId == RecId{-1, -1}) {
		//! the relation is not found in the Relation Catalog
		return E_RELNOTEXIST;
	}

	// TODO: read the record entry corresponding to relcatRecId and create a relCacheEntry
	// TODO: on it using RecBuffer::getRecord() and RelCacheTable::recordToRelCatEntry().
	RecBuffer relationBuffer (relcatRecId.block);
	Attribute relationRecord [RELCAT_NO_ATTRS];
	RelCacheEntry *relCacheBuffer = nullptr;

	relationBuffer.getRecord(relationRecord, relcatRecId.slot);

	//* NOTE: make sure to allocate memory for the RelCacheEntry using malloc()
	relCacheBuffer = (RelCacheEntry*) malloc (sizeof(RelCacheEntry));
	RelCacheTable::recordToRelCatEntry(relationRecord, &(relCacheBuffer->relCatEntry));

	// update the recId field of this Relation Cache entry to relcatRecId.
	relCacheBuffer->recId.block = relcatRecId.block;
	relCacheBuffer->recId.slot = relcatRecId.slot;
	
	// use the Relation Cache entry to set the relId-th entry of the RelCacheTable.
	RelCacheTable::relCache[relId] = relCacheBuffer;	


  	/****** Setting up Attribute Cache entry for the relation ******/

	// {
	// 	RecId attrcatRecId;

	// 	read the record entry corresponding to attrcatRecId and create an
	// 	Attribute Cache entry on it using RecBuffer::getRecord() and
	// 	AttrCacheTable::recordToAttrCatEntry().
	// 	update the recId field of this Attribute Cache entry to attrcatRecId.
	// 	add the Attribute Cache entry to the linked list of listHead.
	// NOTE: make sure to allocate memory for the AttrCacheEntry using malloc()
	// }

	Attribute attrCatRecord[ATTRCAT_NO_ATTRS];

	// let listHead be used to hold the head of the linked list of attrCache entries.
	AttrCacheEntry *attrCacheEntry = nullptr, *head = nullptr;

	int numberOfAttributes = RelCacheTable::relCache[relId]->relCatEntry.numAttrs;
	head = createAttrCacheEntryList(numberOfAttributes);
	attrCacheEntry = head;

	// TODO: iterate over all the entries in the Attribute Catalog corresponding to each
	// TODO: attribute of the relation relName by multiple calls of BlockAccess::linearSearch()
	// * care should be taken to reset the searchIndex of the relation, ATTRCAT_RELID,
	// * corresponding to Attribute Catalog before the first call to linearSearch().

	RelCacheTable::resetSearchIndex(ATTRCAT_RELID);
	while (numberOfAttributes--)
	{
		// // AttrCacheTable::resetSearchIndex(relId, attr);
		// let attrcatRecId store a valid record id an entry of the relation, relName, in the Attribute Catalog.
		RecId attrcatRecId = BlockAccess::linearSearch(ATTRCAT_RELID, RELCAT_ATTR_RELNAME, attrVal, EQ);

		RecBuffer attrCatBlock(attrcatRecId.block);
		attrCatBlock.getRecord(attrCatRecord, attrcatRecId.slot);

		AttrCacheTable::recordToAttrCatEntry(
			attrCatRecord,
			&(attrCacheEntry->attrCatEntry)
		);

		attrCacheEntry->recId.block = attrcatRecId.block;
		attrCacheEntry->recId.slot = attrcatRecId.slot;

		attrCacheEntry = attrCacheEntry->next;
	}

	// set the relIdth entry of the AttrCacheTable to listHead.
	AttrCacheTable::attrCache[relId] = head;

  	/****** Setting up metadata in the Open Relation Table for the relation******/

	// update the relIdth entry of the tableMetaInfo with free as false and
	// relName as the input.
	tableMetaInfo[relId].free = false;
	strcpy(tableMetaInfo[relId].relName, relName);

  	return relId;
}


int OpenRelTable::closeRel(int relId) {
  	if (relId == RELCAT_RELID || relId == ATTRCAT_RELID) return E_NOTPERMITTED;

  	if (0 > relId || relId >= MAX_OPEN) return E_OUTOFBOUND;

  	if (tableMetaInfo[relId].free) return E_RELNOTOPEN;

	if (RelCacheTable::relCache[relId]->dirty == true) {
		/* Get the Relation Catalog entry from RelCacheTable::relCache
		Then convert it to a record using RelCacheTable::relCatEntryToRecord(). */
		Attribute relCatBuffer [RELCAT_NO_ATTRS];
		RelCacheTable::relCatEntryToRecord(&(RelCacheTable::relCache[relId]->relCatEntry), relCatBuffer);

		// declaring an object of RecBuffer class to write back to the buffer
		RecId recId = RelCacheTable::relCache[relId]->recId;
		RecBuffer relCatBlock(recId.block);

		// Write back to the buffer using relCatBlock.setRecord() with recId.slot
		relCatBlock.setRecord(relCatBuffer, RelCacheTable::relCache[relId]->recId.slot);
	}

	// free the memory allocated in the relation and attribute caches which was
	// allocated in the OpenRelTable::openRel() function
	free (RelCacheTable::relCache[relId]);
	
	// // RelCacheEntry *relCacheBuffer = RelCacheTable::relCache[relId];

	//* because we are not modifying the attribute cache at this stage,
	//* write-back is not required. We will do it in subsequent
  	//* stages when it becomes needed)

	AttrCacheEntry *head = AttrCacheTable::attrCache[relId];
	AttrCacheEntry *next = head->next;

	while (true) {
		if (head->dirty)
		{
			Attribute attrCatRecord [ATTRCAT_NO_ATTRS];
			AttrCacheTable::attrCatEntryToRecord(&(head->attrCatEntry), attrCatRecord);

			RecBuffer attrCatBlockBuffer (head->recId.block);
			attrCatBlockBuffer.setRecord(attrCatRecord, head->recId.slot);
		}


		free (head);
		head = next;

		if (head == NULL) break;
		next = next->next;
	}

	// update `tableMetaInfo` to set `relId` as a free slot
	// update `relCache` and `attrCache` to set the entry at `relId` to nullptr
	tableMetaInfo[relId].free = true;
	RelCacheTable::relCache[relId] = nullptr;
	AttrCacheTable::attrCache[relId] = nullptr;

  return SUCCESS;
}
