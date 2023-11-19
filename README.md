
# Algebra-Layer

The Front End parses SQL-Like queries and converts them into a sequence of algebra layer and schema layer method calls. The algebra layer functions process the basic **insert** and **retrieval** requests **to** and **from** the database. _Retrieval functions will create a **target relation** into which the retrieved data will be stored._
#### Return values
for all functions output will be similar to this in algebra layer

|**Value**|**Description**|
|---|---|
|[`SUCCESS`](https://nitcbase.github.io/docs/constants)|On successful insert of the given record into the relation|
|[`E_RELNOTOPEN`](https://nitcbase.github.io/docs/constants)|If the relation is not open.|
|[`E_NATTRMISMATCH`](https://nitcbase.github.io/docs/constants)|If the actual number of attributes in the relation is different from the provided number of attributes|
|[`E_ATTRTYPEMISMATCH`](https://nitcbase.github.io/docs/constants)|If the actual type of the attribute in the relation is different from the type of provided attribute in the record.|
|[`E_DISKFULL`](https://nitcbase.github.io/docs/constants)|If disk space is not sufficient for inserting the record / index|
|[`E_NOTPERMITTED`](https://nitcbase.github.io/docs/constants)|If relName is either `RELATIONCAT` or `ATTRIBUTECAT`. i.e, when the user tries to insert a record into any of the catalogs|

## 1. insert  
```cpp
int insert(char relName[ATTR_SIZE], int numberOfAttributes, char record[][ATTR_SIZE]);
```
This method **inserts the given record** into the specified Relation. Insertion is only done if the Relation is open and attribute number and types match.
## 2. Project Specified Attributes 

```c
int project(char srcRel[ATTR_SIZE], char targetRel[ATTR_SIZE], int tar_nAttrs, char tar_Attrs[][ATTR_SIZE]);
```

This function creates a new target relation with list of attributes specified in the arguments. For each record of the source relation, it inserts a new record into the target relation **with the attribute values corresponding to the attributes specified in the attribute list.**

## 3.Project All Attributes (Copy Relation) 
```c
int project(char srcRel[ATTR_SIZE], char targetRel[ATTR_SIZE]);
```

This function creates a copy of the source relation in the target relation. **Every record** of the source relation is inserted into the target relation.

## 4. Select 
```c
int select(char srcRel[ATTR_SIZE], char targetRel[ATTR_SIZE], char attr[ATTR_SIZE], int op, char strVal[ATTR_SIZE]);
```

This function creates a new target relation with attributes as that of source relation. It inserts the records of source relation which **satisfies the given condition** into the target Relation.

## 5. Join

```c
int join(char srcRelOne[ATTR_SIZE], char srcRelTwo[ATTR_SIZE], char targetRel[ATTR_SIZE],char attrOne[ATTR_SIZE], char attrTwo[ATTR_SIZE]);
```

This function creates a new target relation with _attributes constituting from both the source relations (excluding the specified join-attribute from the second source relation)_. It inserts the records obtained by **_Equi-join_** of both the source relations into the target relation. An attribute from each relation specified in arguments is used for _equi-join called the join-attributes._ Note that both the relations are expected to have distinct attribute names for all attributes aside from the join attribute.



# Block Access Layer

In any database management system, in order to retrieve data from the database or to alter the schema of the relations in the database, the system has to work with the disk blocks. _Block Access layer provides an abstraction that hides the disk structures to the above layers ([Algebra layer](https://nitcbase.github.io/docs/Design/Algebra Layer) and [Schema layer](https://nitcbase.github.io/docs/Design/Schema Layer))_.

The Block Access layer processes the requests for update/retrieval from the algebra and schema layers and works with disk blocks that are buffered by the [_Buffer layer_](https://nitcbase.github.io/docs/Design/Buffer Layer/intro).

## BlockAccess ::  linearSearch
```c
RecId linearSearch(int relId, char *attrName, Attribute attrVal, int op);
```

This method searches the relation specified linearly to find the next record that satisfies the specified condition. The condition value is given by the argument `attrVal`. This function returns the recId of the next record satisfying the condition. The condition that is checked for is the following.

##   BlockAccess :: search()

```c
int search(int relId, Attribute *record, char *attrName, Attribute attrVal, int op);
```
This method searches the relation specified to find the next record that satisfies the specified condition on attribute attrVal and updates the corresponding search index in the cache entry of the relation. It uses the B+ tree if target attribute is indexed, otherwise, it does linear search.


## BlockAccess :: insert()

```c
int insert(int relId, union Attribute *record);
```

This method inserts the record into relation as specified in arguments.

## BlockAccess :: renameRelation()

```c
int renameRelation(char *oldName, char *newName);
```

This method changes the relation name of specified relation to the new name specified in arguments.

## BlockAccess :: renameAttribute()

```c
int renameAttribute(char *relName, char *oldName, char *newName);
```

This method changes the name of an attribute/column present in a specified relation, to the new name specified in arguments.

## BlockAccess :: deleteRelation()

```c
int deleteRelation(char *relName);
```

This method deletes the relation with the name specified as argument. This involves freeing the record blocks and index blocks allocated to this relation, as well as deleting the records corresponding to the relation in the relation catalog and attribute catalog.

## BlockAccess :: project()
```c
int project(int relId, Attribute *record);
```
#### Description

This function is used to fetch **one** record of the relation. Each subsequent call would return the next record until there are no more records to be returned. It also updates `searchIndex` in the cache.

# B+ Tree Layer
## public functions
### BPlusTree::bPlusCreate()

```c
int bPlusCreate(int relId, char attrName[ATTR_SIZE]);
```

This method creates a B+ Tree (Indexing) for the input attribute of the specified relation. It inserts the attribute value corresponding to attrName of all entries in the relation into the B+Tree using bPlusinsert()

|**Name**|**Type**|**Description**|
|---|---|---|
|relId|`int`|Relation Id of the relation whose attribute a B+ tree is to be created for.|
|attrName|`char[ATTR_SIZE]`|Attribute/column name for which B+ tree (index) is to be created.|


### BPlusTree::bPlusSearch
```cpp
RecId bPlusSearch(int relId, char attrName[ATTR_SIZE], union Attribute attrVal, int op);
```

This method searches the relation specified using a B+ tree to find the next record that satisfies the specified condition. The condition value is given by the argument `attrVal`. This function returns the recId of the next record satisfying the condition. The condition that is checked for is the following.

|**Name**|**Type**|**Description**|
|---|---|---|
|relId|`int`|Relation Id of the relation containing the attribute with index.|
|attrName|`char[ATTR_SIZE]`|Attribute/column name (which has an index) to which condition need to be checked with.|
|attrVal|`union Attribute`|value of attribute that has to be checked against the operater.|
|op|`int`|Conditional Operator (can be one among `EQ` , `LE` , `LT` , `GE` , `GT` , `NE` corresponding to equal, less or than equal, less than ,greater than or equal, greater than, not equal operators respectively).|


### BPlusTree::bPlusDestroy

```c
int bPlusDestroy(int rootBlockNum);
```


Used to delete a B+ Tree rooted at a particular block passed as input to the method. The method recursively deletes the constituent index blocks, both internal and leaf index blocks, until the full B+ Tree is deleted.

This function is called when

- the user issues the `DROP INDEX` command
- in a situation where no further disk blocks can be allotted during the creation of/insertion to a B+ Tree
- while deleting an entire relation in NITCbase.

### BPlusTree::bPlusInsert

```c
int bPlusInsert(int relId, char attrName[ATTR_SIZE], union Attribute attrVal, RecId recordId);
```

Inserts an attribute value and the rec-id of the corresponding record into a B+ tree index on the disk

## private functions
### BPlusTree::findLeafToInsert

```c
int findLeafToInsert(int rootBlock, Attribute attrVal, int attrType);
```

Used to find the leaf index block to which an attribute would be inserted to in the B+ insertion process. If this leaf turns out to be full, the caller will need to handle the splitting of this block to insert the entry.

According to the NITCbase specification, this function will only be called from 
#### Arguments

|**Name**|**Type**|**Description**|
|---|---|---|
|rootBlock|`int`|The root block of a B+ tree on the disk|
|attrVal|`struct Attribute`|The attrVal for which the appropriate leaf node is to be found|
|attrType|`int`|The type of the attribute `attrVal`, that is, [`NUMBER`/`STRING`](https://nitcbase.github.io/docs/constants)|

#### Return values

|**Value**|**Description**|
|---|---|
|leafBlkNum|The block number of the leaf block to which insertion can be done|



### BPlusTree::insertIntoLeaf

```c
int insertIntoLeaf(int relId, char attrName[ATTR_SIZE], int blockNum, Index entry);
```

Used to insert an index entry into a leaf index block of an existing B+ tree. If the leaf is full and requires splitting, this function will call other B+ Tree Layer functions to handle any updation required to the parent internal index blocks of the B+ tree.

According to the NITCbase specification, this function will only be called from

|**Name**|**Type**|**Description**|
|---|---|---|
|rootBlock|`int`|The root block of a B+ tree on the disk|
|attrVal|`struct Attribute`|The attrVal for which the appropriate leaf node is to be found|
|attrType|`int`|The type of the attribute `attrVal`, that is, [`NUMBER`/`STRING`](https://nitcbase.github.io/docs/constants)|

#### Return values

|**Value**|**Description**|
|---|---|
|leafBlkNum|The block number of the leaf block to which insertion can be done|



### BPlusTree::splitLeaf

```c
int splitLeaf(int leafBlockNum, Index indices[]);
```

Distributes an array of index entries between an existing leaf index block and a newly allocated leaf index block.
#### Arguments

|**Name**|**Type**|**Description**|
|---|---|---|
|leafBlockNum|`int`|The block number of the existing leaf index block that needs to be split|
|indices|`struct Index[]`|Array of index entries that needs to be split among two leaf index blocks|

#### Return values

|**Value**|**Description**|
|---|---|
|`rightBlkNum`|The block number of the right block in the splitting, that is, the newly allocated block.|
|[`E_DISKFULL`](https://nitcbase.github.io/docs/constants)|If disk space is not sufficient for splitting the leaf index block|

### BPlusTree::insertIntoInternal
```c
int insertIntoLeaf(int relId, char attrName[ATTR_SIZE], int blockNum, Index entry);
```

Used to insert an index entry into an internal index block of an existing B+ tree. This function will call itself to handle any updation required to it's parent internal index blocks.

|**Name**|**Type**|**Description**|
|---|---|---|
|relId|`int`|Relation Id of the relation containing the attribute|
|attrName|`char[ATTR_SIZE]`|Attribute/column name of the relation with given rel-id to whose B+ tree (index) an entry is to be added|
|intBlockNum|`int`|The block number of the internal index block to which insertion is to be done|
|intEntry|`struct InternalEntry`|The index entry that is to be inserted into the internal index block|

#### Return values

|**Value**|**Description**|
|---|---|
|[`SUCCESS`](https://nitcbase.github.io/docs/constants)|On successful insertion into the internal index block|
|[`E_DISKFULL`](https://nitcbase.github.io/docs/constants)|If disk space is not sufficient for insertion into the B+ tree|


### BPlusTree::splitInternal

```c
int splitInternal(int intBlockNum, InternalEntry internalEntries[]);
```

Distributes an array of index entries between an existing internal index block and a newly allocated internal index block.

|**Name**|**Type**|**Description**|
|---|---|---|
|intBlockNum|`int`|The block number of the existing internal index block that needs to be split|
|internalEntries|`struct InternalEntry[]`|Array of index entries that needs to be split among two internal index blocks|

#### Return values

|**Value**|**Description**|
|---|---|
|`rightBlkNum`|The block number of the right block in the splitting, that is, the newly allocated block.|
|[`E_DISKFULL`](https://nitcbase.github.io/docs/constants)|If disk space is not sufficient for splitting the internal index block|


### BPlusTree::createNewRoot

```c
int createNewRoot(int relId, char attrName[ATTR_SIZE], Attribute attrVal, int lChild, int rChild);
```

Used to update the root of an existing B+ tree when the previous root block was split. This function will allocate a new root block and update the attribute cache entry of the attribute in the specified relation to point to the new root block.


# class StaticBuffer

```c
struct BufferMetaInfo{
bool free;
bool dirty;
int blockNum;
int timeStamp;
};
```

### StaticBuffer :: StaticBuffer()

- `Constructor` of the `class StaticBuffer`
- Copies `Block Allocation Map` from disk to buffer memory and updates the meta information of each buffer to initial empty conditions.
- Should be called at the beginning of the session after the `Disk constructor`.

### StaticBuffer :: ~StaticBuffer()

#### Description

- `Destructor` of the `class StaticBuffer`
- Copies the `Block Allocation Map` and the dirty blocks from the buffer memory to disk.
- Should be called at the end of the session before the `Disk destructor`.

### StaticBuffer :: getStaticBlockType()

```c
int getStaticBlockType(int blockNum);
```

Returns the block type of the block corresponding to the input block number. This function is used to find the block type without the creation of a block object.

### StaticBuffer :: setDirtyBit()

```c
int setDirtyBit(int blockNum);
```

Sets the `dirty bit` of the buffer corresponding to the block.

### StaticBuffer :: getBufferNum() (private)
```c
int getBufferNum(int blockNum);
```

Returns the buffer number of the buffer to which the block with the given block number is loaded.

### StaticBuffer :: getFreeBuffer()

```c
int getBufferNum(int blockNum);
```

Assigns a buffer to the block and returns the buffer number. If no free buffer block is found, the least recently used (`LRU`) buffer block is replaced.

# class BlockBuffer

```c
struct HeadInfo
{
int32_t blockType;
int32_t pblock;
int32_t lblock;
int32_t rblock;
int32_t numEntries;
int32_t numAttrs;
int32_t numSlots;
unsigned char reserved[4];
};

typedef union Attribute
{
double nVal;
char sVal[ATTR_SIZE];
} 

struct InternalEntry
{
int32_t lChild;
union Attribute attrVal;
int32_t rChild;
};

struct Index{
union Attribute attrVal;
int32_t block;
int32_t slot;
unsigned char unused[8];
};
```

### BlockBuffer :: BlockBuffer() (Constructor1)

- One of the`Constructors` of the `class BlockBuffer`
- Called if a new block of the input type is to be allocated in the disk.

|**Name**|**Type**|**Description**|
|---|---|---|
|blockType|`char`|Type of the new block to be allotted. It can be one of the following: `'R'`,`'I'` or `'L'` where,  <br>`R`-`REC`  <br>`I`-`IND_INTERNAL`  <br>`L`-`IND_LEAF`|

### BlockBuffer :: BlockBuffer() (Constructor2)

#### Description

- One of the`Constructors` of the `class BlockBuffer`
- Called when the block has already been initialised as a record or index block on the disk.
#### Arguments

|**Name**|**Type**|**Description**|
|---|---|---|
|blockNum|`int`|Block number of the block whose object is to be created.|

### BlockBuffer :: getBlockNum()

Returns the block number of the block. Defined to access the private member field `blockNum` of the class.

### BlockBuffer :: getHeader()

```c
int BlockBuffer::getHeader(struct HeadInfo *head)
```

### BlockBuffer :: setHeader()

```c
int BlockBuffer::setHeader(struct HeadInfo *head)
```

### BlockBuffer :: releaseBlock()

The block number to which this instance of `BlockBuffer` is associated (given by the `blockNum` member field) is freed from the buffer and the disk. The `blockNum` field of the object is invalidated (set to `INVALID_BLOCK` (-1)).

```c
void BlockBuffer::releaseBlock()
```

### BlockBuffer :: loadBlockAndGetBufferPtr()

Returns a pointer to the first byte of the buffer storing the block. This function will load the block to the buffer if it is not already present.

```c
int BlockBuffer::loadBlockAndGetBufferPtr(unsigned char ** buffPtr) 
```

|**Value**|**Description**|
|---|---|
|bufferPtr|Pointer to the buffer containing the block.|
|[E_OUTOFBOUND]|If `blockNum` is not a valid disk block number.|

### BlockBuffer :: getFreeBlock()

Returns the block number of a free block. It sets up the header of the block with the input block type and updates the block allocation map with the same. A buffer is also allocated to the block. If a free block is not available, [E_DISKFULL](https://nitcbase.github.io/docs/constants) is returned.

```c
int BlockBuffer::getFreeBlock(int blockType)
```


### BlockBuffer :: setBlockType()

Sets the type of the block with the input block type. This method sets the type in both the header of the block and also in the block allocation map.

```c
int BlockBuffer::setBlockType(int blockType)
```


|**Name**|**Type**|**Description**|
|---|---|---|
|blockType|`int`|Type of the block(`REC`/`IND_INTERNAL`/`IND_LEAF`)|



# class RelCacheTable

```c++
typedef struct OpenRelTableMetaInfo{
bool free;
char relName[ATTR_SIZE];
}
```

### RelCacheTable :: getRelCatEntry

Gives the _Relation Catalog_ entry corresponding to the specified relation from _Relation Cache_ Table.

|**ame**|**Type**|**Description**|
|---|---|---|
|relId|`int`|The relation id of the relation in the _Relation Cache_ Table|
|relCatBuf|`RelCatEntry*`|Pointer to struct RelCatEntry to which the _Relation Catalog_ entry corresponding to input relId is to be copied|

```c
int RelCacheTable::getRelCatEntry(int relId, RelCatEntry *relCatBuf)
```

### RelCacheTable :: setRelCatEntry

```c
int RelCacheTable::setRelCatEntry(int relId, RelCatEntry *relCatBuf)
```

|**Name**|**Type**|**Description**|
|---|---|---|
|relId|`int`|The relation id of the relation in the _Relation Cache_ Table|
|relCatBuf|`RelCatEntry*`|Pointer to struct RelCatEntry using which the _Relation Catalog_ entry corresponding to input relId is to be updated|

### RelCacheTable :: getSearchIndex

Gives the value of `searchIndex` field of the given relation from _Relation Cache_ Table. This is used by the linear search algorithm to find the **location of the previous hit** so that the search can be resumed from the next record.

```c
int relCacheTable::getSearchIndex(int relid, recId *recidbuff_ptr)
```

|**Name**|**Type**|**Description**|
|---|---|---|
|relId|`int`|The relation id of the relation in the _Relation Cache_ Table|
|searchIndex|`RecId*`|Pointer to struct RecId to which the searchIndex field of the _Relation Cache_ entry corresponding to input relId is to be copied|

### RelCacheTable :: setSearchIndex

Sets the value of `searchIndex` field of the given relation in _Relation Cache_ Table. This is used by the linear search algorithm to set the location of the previous hit so that the search can be resumed from the next record.

|**Name**|**Type**|**Description**|
|---|---|---|
|relId|`int`|The relation id of the relation in the _Relation Cache_ Table|
|searchIndex|`RecId*`|Pointer to struct RecId using which the searchIndex field of the _Relation Cache_ entry corresponding to input relId is to be updated|

```c
int RelCacheTable::setSearchIndex(int relId, recId *searchIndex) 
```


### RelCacheTable :: resetSearchIndex

Resets the value of `searchIndex` field of the given relation in _Relation Cache_ Table to {-1, -1}. This is used so that the linear search can be restarted from the first record.

```c
int RelCacheTable::resetSearchIndex(int relId)
```

### RelCacheTable :: recordToRelCatEntry
A utility function that converts a record, implemented as an array of `union Attribute`, to `RelCatEntry` structure. This function can be used to convert a record in a _Relation Catalog_ block to the corresponding _Relation Cache_ entry when caching a relation in _Relation Cache_ Table. The details of the implementation are left to you.


# class AttrCacheTable
```c

static int getAttrCatEntry(int relId, char attrName[ATTR_SIZE], AttrCatEntry *attrCatBuf);

static int getAttrCatEntry(int relId, int attrOffset, AttrCatEntry *attrCatBuf);

static int setAttrCatEntry(int relId, char attrName[ATTR_SIZE], AttrCatEntry *attrCatBuf);

static int setAttrCatEntry(int relId, int attrOffset, AttrCatEntry *attrCatBuf);

static int getSearchIndex(int relId, char attrName[ATTR_SIZE], IndexId *searchIndex);

static int getSearchIndex(int relId, int attrOffset, IndexId *searchIndex);

static int setSearchIndex(int relId, char attrName[ATTR_SIZE], IndexId *searchIndex);

static int setSearchIndex(int relId, int attrOffset, IndexId *searchIndex);

static int resetSearchIndex(int relId, char attrName[ATTR_SIZE]);

static int resetSearchIndex(int relId, int attrOffset);

static int getAttributeOffset (int relId, char attrName [ATTR_SIZE]);
```


# class OpenRelTable

```c
struct OpenRelTableMetaInfo{
bool free;
char relName[ATTR_SIZE];
}
```

# Frontend


```c
static int create_table(char relname[ATTR_SIZE], int no_attrs, char attributes[][ATTR_SIZE], int type_attrs[]);

static int drop_table(char relname[ATTR_SIZE]);

static int open_table(char relname[ATTR_SIZE]);

static int close_table(char relname[ATTR_SIZE]);

static int create_index(char relname[ATTR_SIZE], char attrname[ATTR_SIZE]);

static int drop_index(char relname[ATTR_SIZE], char attrname[ATTR_SIZE]);

static int alter_table_rename(char relname_from[ATTR_SIZE], char relname_to[ATTR_SIZE]);

static int alter_table_rename_column(char relname[ATTR_SIZE], char attrname_from[16], char attrname_to[16]);

static int insert_into_table_values(char relname[ATTR_SIZE], int attr_count, char attr_values[][ATTR_SIZE]);

static int select_from_table(char relname_source[ATTR_SIZE], char relname_target[ATTR_SIZE]);

static int select_attrlist_from_table(char relname_source[ATTR_SIZE], char relname_target[ATTR_SIZE],int attr_count, char attr_list[][ATTR_SIZE]);

static int select_from_table_where(char relname_source[ATTR_SIZE], char relname_target[ATTR_SIZE],char attribute[ATTR_SIZE], int op, char value[ATTR_SIZE]);

static int select_attrlist_from_table_where(char relname_source[ATTR_SIZE], char relname_target[ATTR_SIZE],int attr_count, char attr_list[][ATTR_SIZE],char attribute[ATTR_SIZE], int op, char value[ATTR_SIZE]);

static int select_from_join_where(char relname_source_one[ATTR_SIZE], char relname_source_two[ATTR_SIZE],char relname_target[ATTR_SIZE],char join_attr_one[ATTR_SIZE], char join_attr_two[ATTR_SIZE]);

static int select_attrlist_from_join_where(char relname_source_one[ATTR_SIZE],char relname_source_two[ATTR_SIZE],
char relname_target[ATTR_SIZE],char join_attr_one[ATTR_SIZE], char join_attr_two[ATTR_SIZE],int attr_count, char attr_list[][ATTR_SIZE]);

static int custom_function(int argc, char argv[][ATTR_SIZE]);
```

# Schema Layer

### Schema :: createRel()
This method creates a new relation with the name, attribute/column list as specified in arguments. Verifying the maximum number of attributes in a relation is to be checked by the caller of this function (Frontend Interface) and is not handled by this function.

```c
int createRel(char relName[], int numOfAttributes, char attrNames[][ATTR_SIZE], int attrType[]);
```

### Schema :: deleteRel()

This method deletes the relation with name as specified in arguments.
```c
int deleteRel(char relName[ATTR_SIZE]);
```

### Schema :: createIndex()
This method creates a bplus indexing on an attribute attrName in a relation relName as specified in arguments.

```c
int createIndex(char relName[ATTR_SIZE], char attrName[ATTR_SIZE]);
```
### Schema :: dropIndex()

```c
int dropIndex(char relName[ATTR_SIZE], char attrName[ATTR_SIZE]);
```
This method drops the bplus indexing on an attribute attrName in a relation relName as specified in arguments.

### Schema :: renameRel()

```c
int renameRel(char oldRelName[ATTR_SIZE], char newRelName[ATTR_SIZE]);
```

### Schema :: renameAttr()
This method changes the name of an attribute/column present in a specified relation, to new name as specified in arguments.

```c
int renameAttr(char relName[ATTR_SIZE], char oldAttrName[ATTR_SIZE], char newAttrName[ATTR_SIZE]);
```
### Schema :: openRel()

```c
int openRel(char relName[ATTR_SIZE]);
```

### Schema :: closeRel()

```c
int closeRel(char relName[ATTR_SIZE]);
```