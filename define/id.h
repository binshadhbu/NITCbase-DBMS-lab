#ifndef NITCBASE_ID_H
#define NITCBASE_ID_H

// for any opened relation
typedef int relId;

/* A record is identified by its block number and slot number */
struct RecId {
  int block;
  int slot;
};

/* An index is identified by its block number and index number */
struct IndexId {
  int block;
  int index;
};

#endif  // NITCBASE_ID_H