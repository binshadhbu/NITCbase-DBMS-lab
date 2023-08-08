#ifndef NITCBASE_CONSTANTS_H
#define NITCBASE_CONSTANTS_H

#define DISK_PATH "../Disk/disk"                            // Path to disk
#define DISK_RUN_COPY_PATH "../Disk/disk_run_copy"          // Path to run copy of the disk
#define Files_Path "../Files/"                              // Path to Files directory
#define INPUT_FILES_PATH "../Files/Input_Files/"            // Path to Input_Files directory inside the Files directory
#define OUTPUT_FILES_PATH "../Files/Output_Files/"          // Path to Output_Files directory inside the Files directory
#define BATCH_FILES_PATH "../Files/Batch_Execution_Files/"  // Path to Batch_Execution_Files directory inside the Files directory

#define BLOCK_SIZE 2048             // Size of Block in bytes
#define ATTR_SIZE 16                // Size of an attribute in bytes
#define DISK_SIZE 16 * 1024 * 1024  // Size of Disk in bytes
#define HEADER_SIZE 32              // Size of Header of a block in bytes (not including slotmap)
#define LCHILD_SIZE 4               // Size of field Lchild in bytes
#define RCHILD_SIZE 4               // Size of field Rchild in bytes
#define PBLOCK_SIZE 4               // Size of field Pblock in bytes
#define BLOCKNUM_SIZE 4             // Size of field BlockNum in bytes
#define SLOTNUM_SIZE 4              // Size of field SlotNum in bytes
#define INDEX_BLOCK_UNUSED_BYTES 8  // Size of unused field in index block (in bytes)
#define INTERNAL_ENTRY_SIZE 24      // Size of an Internal Index Entry in the Internal Index Block (in bytes)
#define LEAF_ENTRY_SIZE 32          // Size of an Leaf Index Entry in the Leaf Index Block (in bytes)

#define DISK_BLOCKS 8192             // Number of block in disk
#define BUFFER_CAPACITY 32           // Total number of blocks available in the Buffer (Capacity of the Buffer in blocks)
#define MAX_OPEN 12                  // Maximum number of relations allowed to be open and cached in Cache Layer.
#define BLOCK_ALLOCATION_MAP_SIZE 4  // Number of blocks given for Block Allocation Map in the disk

#define RELCAT_NO_ATTRS 6   // Number of attributes present in one entry / record of the Relation Catalog
#define ATTRCAT_NO_ATTRS 6  // Number of attributes present in one entry / record of the Attribute Catalog

#define RELCAT_BLOCK 4   // Disk block number for the block of Relation Catalog
#define ATTRCAT_BLOCK 5  // Disk block number for the first block of Attribute Catalog

#define NO_OF_ATTRS_RELCAT_ATTRCAT 6    // Common variable to indicate the number of attributes present in one entry of Relation Catalog / Attribute Catalog
#define SLOTMAP_SIZE_RELCAT_ATTRCAT 20  // Size of slotmap in both Relation Catalog and Attribute Catalog

#define SLOT_OCCUPIED '1'    // Value to mark a slot in Slotmap as Occupied
#define SLOT_UNOCCUPIED '0'  // Value to mark a slot in Slotmap as Unoccupied

#define RELCAT_RELID 0   // Relid for Relation catalog
#define ATTRCAT_RELID 1  // Relid for Attribute catalog

#define RELCAT_SLOTNUM_FOR_RELCAT 0   // Slot number for relation catalog in relation catalog
#define RELCAT_SLOTNUM_FOR_ATTRCAT 1  // Slot number for attribute catalog in relation catalog

#define INVALID_BLOCKNUM -1  // Indicates the Block number as Invalid.

enum AttributeType {
  NUMBER = 0,  // for an integer or a floating point number
  STRING = 1,
};

enum ConditionalOperators {
  EQ,  // =
  LE,  // <=
  LT,  // <
  GE,  // >=
  GT,  // >
  NE   // !=
};

enum BlockType {
  REC,           // record block
  IND_INTERNAL,  // internal index block
  IND_LEAF,      // leaf index block
  UNUSED_BLK,    // unused block
  BMAP           // block allocation map
};

enum OpenRelationEntryStatus {
  OCCUPIED = 1,
  FREE = 0
};

// Indexes for Relation Catalog Attributes
enum RelCatFieldIndex {
  RELCAT_REL_NAME_INDEX = 0,           // Relation Name
  RELCAT_NO_ATTRIBUTES_INDEX = 1,      // #Attributes
  RELCAT_NO_RECORDS_INDEX = 2,         // #Records
  RELCAT_FIRST_BLOCK_INDEX = 3,        // First Block
  RELCAT_LAST_BLOCK_INDEX = 4,         // Last Block
  RELCAT_NO_SLOTS_PER_BLOCK_INDEX = 5  // #Slots
};

// Indexes for Attribute Catalog Attributes
enum AttrCatFieldIndex {
  ATTRCAT_REL_NAME_INDEX = 0,      // Relation Name
  ATTRCAT_ATTR_NAME_INDEX = 1,     // Attribute Name
  ATTRCAT_ATTR_TYPE_INDEX = 2,     // Attribute Type
  ATTRCAT_PRIMARY_FLAG_INDEX = 3,  // Primary Flag
  ATTRCAT_ROOT_BLOCK_INDEX = 4,    // Root Block
  ATTRCAT_OFFSET_INDEX = 5         // Offset
};

enum ReturnTypes {
  SUCCESS = 0,
  FAILURE = -1,
  EXIT = -100,
  E_OUTOFBOUND,             // Out of bound
  E_FREESLOT,               // Free slot
  E_NOINDEX,                // No index
  E_DISKFULL,               // Insufficient space in Disk
  E_INVALIDBLOCK,           // Invalid block
  E_RELNOTEXIST,            // Relation does not exist
  E_RELEXIST,               // Relation already exists
  E_ATTRNOTEXIST,           // Attribute does not exist
  E_ATTREXIST,              // Attribute already exists
  E_CACHEFULL,              // Cache is full
  E_RELNOTOPEN,             // Relation is not open
  E_NATTRMISMATCH,          // Mismatch in number of attributes
  E_DUPLICATEATTR,          // Duplicate attributes found
  E_RELOPEN,                // Relation is open
  E_ATTRTYPEMISMATCH,       // Mismatch in attribute type
  E_INVALID,                // Invalid index or argument
  E_MAXRELATIONS,           // Maximum number of relations already present
  E_MAXATTRS,               // Maximum number of attributes allowed for a relation is 125
  E_NOTPERMITTED,           // Operation not permitted
  E_NOTFOUND,               // Search for requested record unsuccessful
  E_BLOCKNOTINBUFFER,       // Block not found in buffer
  E_INDEX_BLOCKS_RELEASED,  // Due to insufficient disk space, index blocks have been released from the disk
};

#define TEMP ".temp"  // Used for internal purposes

// Global variables for B+ Tree Layer
#define MAX_KEYS_INTERNAL 100     // Maximum number of keys allowed in an Internal Node of a B+ tree
#define MIDDLE_INDEX_INTERNAL 50  // Index of the middle element in an Internal Node of a B+ tree
#define MAX_KEYS_LEAF 63          // Maximum number of keys allowed in a Leaf Node of a B+ tree
#define MIDDLE_INDEX_LEAF 31      // Index of the middle element in a Leaf Node of a B+ tree

// Name strings for Relation Catalog and Attribute Catalog (as it is stored in the Relation catalog)
#define RELCAT_RELNAME "RELATIONCAT"
#define ATTRCAT_RELNAME "ATTRIBUTECAT"

// Relation Catalog attribute name strings
#define RELCAT_ATTR_RELNAME "RelName"
#define RELCAT_ATTR_NO_ATTRIBUTES "#Attributes"
#define RELCAT_ATTR_NO_RECORDS "#Records"
#define RELCAT_ATTR_FIRST_BLOCK "FirstBlock"
#define RELCAT_ATTR_LAST_BLOCK "LastBlock"
#define RELCAT_ATTR_NO_SLOTS "#Slots"

// Attribte Catalog attribute name strings
#define ATTRCAT_ATTR_RELNAME "RelName"
#define ATTRCAT_ATTR_ATTRIBUTE_NAME "AttributeName"
#define ATTRCAT_ATTR_ATTRIBUTE_TYPE "AttributeType"
#define ATTRCAT_ATTR_PRIMARY_FLAG "PrimaryFlag"
#define ATTRCAT_ATTR_ROOT_BLOCK "RootBlock"
#define ATTRCAT_ATTR_OFFSET "Offset"

#endif  // NITCBASE_CONSTANTS_H