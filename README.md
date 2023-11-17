
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

