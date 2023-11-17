
# #Algebra-Layer
1. #select 
2. #project 
3. #project 
4. #select 

## 1. insert  #insert
```cpp
static int insert(char relName[ATTR_SIZE], int numberOfAttributes, char record[][ATTR_SIZE]);
```
This method **inserts the given record** into the specified Relation. Insertion is only done if the Relation is open and attribute number and types match.


## 2. Project Specified Attributes #project

```c
static int project(char srcRel[ATTR_SIZE], char targetRel[ATTR_SIZE], int tar_nAttrs, char tar_Attrs[][ATTR_SIZE]);
```

This function creates a new target relation with list of attributes specified in the arguments. For each record of the source relation, it inserts a new record into the target relation **with the attribute values corresponding to the attributes specified in the attribute list.**

## 3.Project All Attributes (Copy Relation) #project
```c
static int project(char srcRel[ATTR_SIZE], char targetRel[ATTR_SIZE]);
```

This function creates a copy of the source relation in the target relation. **Every record** of the source relation is inserted into the target relation.

## 4. Select #select
```c
static int select(char srcRel[ATTR_SIZE], char targetRel[ATTR_SIZE], char attr[ATTR_SIZE], int op, char strVal[ATTR_SIZE]);
```

This function creates a new target relation with attributes as that of source relation. It inserts the records of source relation which **satisfies the given condition** into the target Relation.
