#ifndef FRONTEND_INTERFACE_FRONTEND_H
#define FRONTEND_INTERFACE_FRONTEND_H

#include "../Algebra/Algebra.h"
#include "../Schema/Schema.h"
#include "../define/constants.h"

class Frontend {
 public:
  // DDL
  static int create_table(char relname[ATTR_SIZE], int no_attrs, char attributes[][ATTR_SIZE], int type_attrs[]);

  static int drop_table(char relname[ATTR_SIZE]);

  static int open_table(char relname[ATTR_SIZE]);

  static int close_table(char relname[ATTR_SIZE]);

  static int create_index(char relname[ATTR_SIZE], char attrname[ATTR_SIZE]);

  static int drop_index(char relname[ATTR_SIZE], char attrname[ATTR_SIZE]);

  static int alter_table_rename(char relname_from[ATTR_SIZE], char relname_to[ATTR_SIZE]);

  static int alter_table_rename_column(char relname[ATTR_SIZE], char attrname_from[16], char attrname_to[16]);

  // DML
  static int insert_into_table_values(char relname[ATTR_SIZE], int attr_count, char attr_values[][ATTR_SIZE]);

  static int select_from_table(char relname_source[ATTR_SIZE], char relname_target[ATTR_SIZE]);

  static int select_attrlist_from_table(char relname_source[ATTR_SIZE], char relname_target[ATTR_SIZE],
                                        int attr_count, char attr_list[][ATTR_SIZE]);

  static int select_from_table_where(char relname_source[ATTR_SIZE], char relname_target[ATTR_SIZE],
                                     char attribute[ATTR_SIZE], int op, char value[ATTR_SIZE]);

  static int select_attrlist_from_table_where(char relname_source[ATTR_SIZE], char relname_target[ATTR_SIZE],
                                              int attr_count, char attr_list[][ATTR_SIZE],
                                              char attribute[ATTR_SIZE], int op, char value[ATTR_SIZE]);

  static int select_from_join_where(char relname_source_one[ATTR_SIZE], char relname_source_two[ATTR_SIZE],
                                    char relname_target[ATTR_SIZE],
                                    char join_attr_one[ATTR_SIZE], char join_attr_two[ATTR_SIZE]);

  static int select_attrlist_from_join_where(char relname_source_one[ATTR_SIZE], char relname_source_two[ATTR_SIZE],
                                             char relname_target[ATTR_SIZE],
                                             char join_attr_one[ATTR_SIZE], char join_attr_two[ATTR_SIZE],
                                             int attr_count, char attr_list[][ATTR_SIZE]);

  static int custom_function(int argc, char argv[][ATTR_SIZE]);
};

#endif  // FRONTEND_INTERFACE_FRONTEND_H
