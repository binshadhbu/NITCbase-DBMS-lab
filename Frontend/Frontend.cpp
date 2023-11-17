#include "Frontend.h"

#include <cstring>
#include <iostream>

int Frontend::create_table(char relname[ATTR_SIZE], int no_attrs, char attributes[][ATTR_SIZE], int type_attrs[])
{
	return Schema::createRel(relname, no_attrs, attributes, type_attrs);
}

int Frontend::drop_table(char relname[ATTR_SIZE])
{
	return Schema::deleteRel(relname);
}

int Frontend::open_table(char relname[ATTR_SIZE])
{
	return Schema::openRel(relname);
}

int Frontend::close_table(char relname[ATTR_SIZE])
{
	return Schema::closeRel(relname);
}

int Frontend::alter_table_rename(char relname_from[ATTR_SIZE], char relname_to[ATTR_SIZE])
{
	return Schema::renameRel(relname_from, relname_to);
}

int Frontend::alter_table_rename_column(char relname[ATTR_SIZE], char attrname_from[ATTR_SIZE],
										char attrname_to[ATTR_SIZE])
{
	return Schema::renameAttr(relname, attrname_from, attrname_to);
}

int Frontend::create_index(char relname[ATTR_SIZE], char attrname[ATTR_SIZE])
{
	Schema::createIndex(relname, attrname);
	return SUCCESS;
}

int Frontend::drop_index(char relname[ATTR_SIZE], char attrname[ATTR_SIZE])
{
	Schema::dropIndex(relname, attrname);
	return SUCCESS;
}

int Frontend::insert_into_table_values(char relname[ATTR_SIZE], int attr_count, char attr_values[][ATTR_SIZE])
{
	return Algebra::insert(relname, attr_count, attr_values);
}

int Frontend::select_from_table(char relname_source[ATTR_SIZE], char relname_target[ATTR_SIZE])
{
	return Algebra::project(relname_source, relname_target);
	// return SUCCESS;
}

int Frontend::select_attrlist_from_table(char relname_source[ATTR_SIZE], char relname_target[ATTR_SIZE],
										 int attr_count, char attr_list[][ATTR_SIZE])
{
	return Algebra::project(relname_source, relname_target, attr_count, attr_list);
	// return SUCCESS;
}

int Frontend::select_from_table_where(char relname_source[ATTR_SIZE], char relname_target[ATTR_SIZE],
									  char attribute[ATTR_SIZE], int op, char value[ATTR_SIZE])
{
	return Algebra::select(relname_source, relname_target, attribute, op, value);
}

int Frontend::select_attrlist_from_table_where(char relname_source[ATTR_SIZE], char relname_target[ATTR_SIZE],
											   int attr_count, char attr_list[][ATTR_SIZE],
											   char attribute[ATTR_SIZE], int op, char value[ATTR_SIZE])
{
	int ret = Algebra::select(relname_source, "temp", attribute, op, value);
	if (ret != SUCCESS)
		return ret;

	ret = OpenRelTable::openRel("temp");
	if (ret < 0 || ret >= MAX_OPEN)
		return ret;

	ret = Algebra::project("temp", relname_target, attr_count, attr_list);
	if (ret != SUCCESS)
		return ret;

	Schema::closeRel("temp");
	Schema::deleteRel("temp");

	return SUCCESS;
}

// int Frontend::select_from_join_where(char relname_source_one[ATTR_SIZE], char relname_source_two[ATTR_SIZE],
// 									 char relname_target[ATTR_SIZE],
// 									 char join_attr_one[ATTR_SIZE], char join_attr_two[ATTR_SIZE])
// {
// 	// Algebra::join
// 	return SUCCESS;
// }

int Frontend::select_from_join_where(char relname_source_one[ATTR_SIZE], 
										char relname_source_two[ATTR_SIZE], 
										char relname_target[ATTR_SIZE], 
										char join_attr_one[ATTR_SIZE], 
										char join_attr_two[ATTR_SIZE]) 
{
    // Call join() method of the Algebra Layer with correct arguments
	return Algebra::join(relname_source_one, relname_source_two, 
							relname_target, join_attr_one, join_attr_two);

    // Return Success or Error values appropriately
	// return ret;
}


int Frontend::select_attrlist_from_join_where(char relname_source_one[ATTR_SIZE], 
												char relname_source_two[ATTR_SIZE],
											  	char relname_target[ATTR_SIZE],
											  	char join_attr_one[ATTR_SIZE], 
												char join_attr_two[ATTR_SIZE],
											  	int attr_count, char attr_list[][ATTR_SIZE])
{
	// Algebra::join + project
	int ret = Algebra::join(relname_source_one, relname_source_two, 
							"temp_relation", join_attr_one, join_attr_two);
	if (ret != SUCCESS) return ret;

	ret = Algebra::project("temp_relation", relname_target, attr_count, attr_list);
	OpenRelTable::closeRel(OpenRelTable::getRelId("temp_relation"));
	Schema::deleteRel("temp_relation");

	return ret;
}

int Frontend::custom_function(int argc, char argv[][ATTR_SIZE])
{
	// argc gives the size of the argv array
	// argv stores every token delimited by space and comma

	// implement whatever you desire
}