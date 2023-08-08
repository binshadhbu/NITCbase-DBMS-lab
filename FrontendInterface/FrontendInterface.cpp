// clang-format off
#include <cstring>
#include <fstream>
#include <string>
#include <iostream>
#include <readline/history.h>
#include <readline/readline.h>
// clang-format on

#include "FrontendInterface.h"

#include "../Disk_Class/Disk.h"
#include "../Frontend/Frontend.h"
#include "../define/constants.h"

using namespace std;

int getOperator(string op_str);

void attrToTruncatedArray(string nameString, char *nameArray);

void printErrorMsg(int error);

void printHelp();

// extract tokens delimited by whitespace and comma
vector<string> RegexHandler::extractTokens(string input) {
  regex re("\\s*,\\s*|\\s+");
  sregex_token_iterator first(input.begin(), input.end(), re, -1), last;
  vector<string> tokens(first, last);
  return tokens;
}

// handler functions
int RegexHandler::helpHandler() {
  printHelp();
  return SUCCESS;
};

int RegexHandler::exitHandler() {
  return EXIT;
};

int RegexHandler::echoHandler() {
  string message = m[1];
  cout << message << endl;
  return SUCCESS;
}

int RegexHandler::runHandler() {
  string fileName = m[1];
  const string filePath = BATCH_FILES_PATH;
  fstream commandsFile;
  commandsFile.open(filePath + fileName, ios::in);
  string command;
  if (!commandsFile.is_open()) {
    cout << "The file " << fileName << " does not exist\n";
    return FAILURE;
  }

  int lineNumber = 1;
  while (getline(commandsFile, command)) {
    int ret = this->handle(command);
    if (ret == EXIT) {
      break;
    } else if (ret != SUCCESS) {
      cout << "Executed up till line " << lineNumber - 1 << ".\n";
      cout << "Error at line number " << lineNumber << ". Subsequent lines will be skipped.\n";
      break;
    }
    lineNumber++;
  }

  commandsFile.close();

  return SUCCESS;  // error messages if any will be printed in recursive call to handle
}

int RegexHandler::openHandler() {
  char relName[ATTR_SIZE];
  attrToTruncatedArray(m[1], relName);

  int ret = Frontend::open_table(relName);
  if (ret == SUCCESS) {
    cout << "Relation " << relName << " opened successfully\n";
  }
  return ret;
}

int RegexHandler::closeHandler() {
  char relName[ATTR_SIZE];
  attrToTruncatedArray(m[1], relName);

  int ret = Frontend::close_table(relName);
  if (ret == SUCCESS) {
    cout << "Relation " << relName << " closed successfully\n";
  }

  return ret;
}

int RegexHandler::createTableHandler() {
  char relName[ATTR_SIZE];
  attrToTruncatedArray(m[1], relName);

  vector<string> words = extractTokens(m[2]);

  int attrCount = words.size() / 2;

  if (attrCount > 125) {
    return E_MAXATTRS;
  }

  char attrNames[attrCount][ATTR_SIZE];
  int attrTypes[attrCount];

  for (int i = 0, k = 0; i < attrCount; i++, k += 2) {
    attrToTruncatedArray(words[k], attrNames[i]);
    if (words[k + 1] == "STR")
      attrTypes[i] = STRING;
    else if (words[k + 1] == "NUM")
      attrTypes[i] = NUMBER;
  }

  int ret = Frontend::create_table(relName, attrCount, attrNames, attrTypes);
  if (ret == SUCCESS) {
    cout << "Relation " << relName << " created successfully" << endl;
  }

  return ret;
}

int RegexHandler::dropTableHandler() {
  char relName[ATTR_SIZE];
  attrToTruncatedArray(m[1], relName);

  int ret = Frontend::drop_table(relName);
  if (ret == SUCCESS) {
    cout << "Relation " << relName << " deleted successfully" << endl;
  }
  return ret;
}

int RegexHandler::createIndexHandler() {
  char relName[ATTR_SIZE], attrName[ATTR_SIZE];

  attrToTruncatedArray(m[1], relName);
  attrToTruncatedArray(m[2], attrName);

  int ret = Frontend::create_index(relName, attrName);
  if (ret == SUCCESS) {
    cout << "Index created successfully\n";
  }

  return ret;
}

int RegexHandler::dropIndexHandler() {
  char relName[ATTR_SIZE], attrName[ATTR_SIZE];
  attrToTruncatedArray(m[1], relName);
  attrToTruncatedArray(m[2], attrName);

  int ret = Frontend::drop_index(relName, attrName);
  if (ret == SUCCESS) {
    cout << "Index deleted successfully\n";
  }

  return ret;
}

int RegexHandler::renameTableHandler() {
  char oldRelName[ATTR_SIZE];
  char newRelName[ATTR_SIZE];
  attrToTruncatedArray(m[1], oldRelName);
  attrToTruncatedArray(m[2], newRelName);

  int ret = Frontend::alter_table_rename(oldRelName, newRelName);
  if (ret == SUCCESS) {
    cout << "Renamed Relation Successfully" << endl;
  }

  return ret;
}

int RegexHandler::renameColumnHandler() {
  char relName[ATTR_SIZE];
  char oldColName[ATTR_SIZE];
  char newColName[ATTR_SIZE];
  attrToTruncatedArray(m[1], relName);
  attrToTruncatedArray(m[2], oldColName);
  attrToTruncatedArray(m[3], newColName);

  int ret = Frontend::alter_table_rename_column(relName, oldColName, newColName);
  if (ret == SUCCESS) {
    cout << "Renamed Attribute Successfully" << endl;
  }

  return ret;
}

int RegexHandler::insertSingleHandler() {
  char relName[ATTR_SIZE];
  attrToTruncatedArray(m[1], relName);

  vector<string> words = extractTokens(m[2]);

  int attrCount = words.size();
  char attrValues[attrCount][ATTR_SIZE];
  for (int i = 0; i < attrCount; ++i) {
    attrToTruncatedArray(words[i], attrValues[i]);
  }

  int ret = Frontend::insert_into_table_values(relName, attrCount, attrValues);
  if (ret == SUCCESS) {
    cout << "Inserted successfully" << endl;
  }

  return ret;
}

int RegexHandler::insertFromFileHandler() {
  char relName[ATTR_SIZE];
  attrToTruncatedArray(m[1], relName);

  string filePath = string(INPUT_FILES_PATH) + m[2].str();
  std::cout << "File path: " << filePath << endl;

  ifstream file(filePath);
  if (!file.is_open()) {
    cout << "Invalid file path or file does not exist" << endl;
    return FAILURE;
  }

  string errorMsg("");
  string fileLine;

  int retVal = SUCCESS;
  int columnCount = -1, lineNumber = 1;
  while (getline(file, fileLine)) {
    vector<string> row;

    stringstream lineStream(fileLine);

    string item;
    while (getline(lineStream, item, ',')) {
      if (item.size() == 0) {
        errorMsg += "Null values not allowed in attribute values\n";
        retVal = FAILURE;
        break;
      }
      row.push_back(item);
    }
    if (retVal == FAILURE) {
      break;
    }

    if (columnCount == -1) {
      columnCount = row.size();
    } else if (columnCount != row.size()) {
      errorMsg += "Mismatch in number of attributes\n";
      retVal = FAILURE;
      break;
    }

    char rowArray[columnCount][ATTR_SIZE];
    for (int i = 0; i < columnCount; ++i) {
      attrToTruncatedArray(row[i], rowArray[i]);
    }

    retVal = Frontend::insert_into_table_values(relName, columnCount, rowArray);

    if (retVal != SUCCESS) {
      break;
    }

    lineNumber++;
  }

  file.close();

  if (retVal == SUCCESS) {
    cout << lineNumber - 1 << " rows inserted successfully" << endl;
  } else {
    if (lineNumber > 1) {
      std::cout << "Rows till line " << lineNumber - 1 << " successfully inserted\n";
    }
    std::cout << "Insertion error at line " << lineNumber << " in file \n";
    std::cout << "Subsequent lines will be skipped\n";
    if (retVal == FAILURE) {
      std::cout << "Error:" << errorMsg;
    }
  }

  return retVal;
}

int RegexHandler::selectFromHandler() {
  char sourceRelName[ATTR_SIZE];
  char targetRelName[ATTR_SIZE];
  attrToTruncatedArray(m[1], sourceRelName);
  attrToTruncatedArray(m[2], targetRelName);

  int ret = Frontend::select_from_table(sourceRelName, targetRelName);
  if (ret == SUCCESS) {
    cout << "Selected successfully into " << targetRelName << endl;
  }

  return ret;
}

int RegexHandler::selectFromWhereHandler() {
  char sourceRelName[ATTR_SIZE];
  char targetRelName[ATTR_SIZE];
  char attribute[ATTR_SIZE];
  char valueStr[ATTR_SIZE];
  attrToTruncatedArray(m[1], sourceRelName);
  attrToTruncatedArray(m[2], targetRelName);
  attrToTruncatedArray(m[3], attribute);
  int op = getOperator(m[4]);
  attrToTruncatedArray(m[5], valueStr);

  int ret = Frontend::select_from_table_where(sourceRelName, targetRelName, attribute, op, valueStr);
  if (ret == SUCCESS) {
    cout << "Selected successfully into " << targetRelName << endl;
  }

  return ret;
}

int RegexHandler::selectAttrFromHandler() {
  char sourceRelName[ATTR_SIZE];
  char targetRelName[ATTR_SIZE];
  attrToTruncatedArray(m[2], sourceRelName);
  attrToTruncatedArray(m[3], targetRelName);

  vector<string> words = extractTokens(m[1]);

  int attrCount = words.size();
  char attrNames[attrCount][ATTR_SIZE];
  for (int i = 0; i < attrCount; i++) {
    attrToTruncatedArray(words[i], attrNames[i]);
  }

  int ret = Frontend::select_attrlist_from_table(sourceRelName, targetRelName, attrCount, attrNames);
  if (ret == SUCCESS) {
    cout << "Selected successfully into " << targetRelName << endl;
  }

  return ret;
}

int RegexHandler::selectAttrFromWhereHandler() {
  char sourceRelName[ATTR_SIZE];
  char targetRelName[ATTR_SIZE];
  char attribute[ATTR_SIZE];
  char value[ATTR_SIZE];

  attrToTruncatedArray(m[2], sourceRelName);
  attrToTruncatedArray(m[3], targetRelName);
  attrToTruncatedArray(m[4], attribute);
  int op = getOperator(m[5]);
  attrToTruncatedArray(m[6], value);

  vector<string> attrTokens = extractTokens(m[1]);

  int attrCount = attrTokens.size();
  char attrNames[attrCount][ATTR_SIZE];
  for (int i = 0; i < attrCount; i++) {
    attrToTruncatedArray(attrTokens[i], attrNames[i]);
  }

  int ret = Frontend::select_attrlist_from_table_where(sourceRelName, targetRelName, attrCount, attrNames,
                                                       attribute, op, value);
  if (ret == SUCCESS) {
    cout << "Selected successfully into " << targetRelName << endl;
  }

  return ret;
}

int RegexHandler::selectFromJoinHandler() {
  char sourceRelOneName[ATTR_SIZE];
  char sourceRelTwoName[ATTR_SIZE];
  char targetRelName[ATTR_SIZE];
  char joinAttributeOne[ATTR_SIZE];
  char joinAttributeTwo[ATTR_SIZE];

  attrToTruncatedArray(m[1], sourceRelOneName);
  attrToTruncatedArray(m[2], sourceRelTwoName);
  attrToTruncatedArray(m[3], targetRelName);

  if (m[1] == m[4] && m[2] == m[6]) {
    attrToTruncatedArray(m[5], joinAttributeOne);
    attrToTruncatedArray(m[7], joinAttributeTwo);
  } else if (m[1] == m[6] && m[2] == m[4]) {
    attrToTruncatedArray(m[7], joinAttributeOne);
    attrToTruncatedArray(m[5], joinAttributeTwo);

  } else {
    cout << "Syntax Error: Relation names do not match" << endl;
    return FAILURE;
  }

  int ret = Frontend::select_from_join_where(sourceRelOneName, sourceRelTwoName, targetRelName,
                                             joinAttributeOne, joinAttributeTwo);
  if (ret == SUCCESS) {
    cout << "Selected successfully into " << targetRelName << endl;
  }

  return ret;
}

int RegexHandler::selectAttrFromJoinHandler() {
  char sourceRelOneName[ATTR_SIZE];
  char sourceRelTwoName[ATTR_SIZE];
  char targetRelName[ATTR_SIZE];
  char joinAttributeOne[ATTR_SIZE];
  char joinAttributeTwo[ATTR_SIZE];

  attrToTruncatedArray(m[2], sourceRelOneName);
  attrToTruncatedArray(m[3], sourceRelTwoName);
  attrToTruncatedArray(m[4], targetRelName);

  if (m[2] == m[5] && m[3] == m[7]) {
    attrToTruncatedArray(m[6], joinAttributeOne);
    attrToTruncatedArray(m[8], joinAttributeTwo);
  } else if (m[2] == m[7] && m[3] == m[5]) {
    attrToTruncatedArray(m[8], joinAttributeOne);
    attrToTruncatedArray(m[6], joinAttributeTwo);
  } else {
    cout << "Syntax Error: Relation names do not match" << endl;
    return FAILURE;
  }

  attrToTruncatedArray(m[6], joinAttributeOne);
  attrToTruncatedArray(m[8], joinAttributeTwo);

  vector<string> attrTokens = extractTokens(m[1]);
  int attrCount = attrTokens.size();
  char attrNames[attrCount][ATTR_SIZE];
  for (int i = 0; i < attrCount; i++) {
    attrToTruncatedArray(attrTokens[i], attrNames[i]);
  }

  int ret = Frontend::select_attrlist_from_join_where(sourceRelOneName, sourceRelTwoName, targetRelName,
                                                      joinAttributeOne, joinAttributeTwo, attrCount,
                                                      attrNames);
  if (ret == SUCCESS) {
    cout << "Selected successfully into " << targetRelName;
  }

  return ret;
}

int RegexHandler::customFunctionHandler() {
  vector<string> tokens = extractTokens(m[1]);

  char tokensAsArray[tokens.size()][ATTR_SIZE];
  for (int i = 0; i < tokens.size(); ++i) {
    attrToTruncatedArray(tokens[i], tokensAsArray[i]);
  }

  int ret = Frontend::custom_function(tokens.size(), tokensAsArray);
  return ret;
}

int RegexHandler::handle(const string command) {
  for (auto iter = handlers.begin(); iter != handlers.end(); ++iter) {
    regex testCommand = iter->first;
    handlerFunction handler = iter->second;
    if (regex_match(command, testCommand)) {
      regex_search(command, m, testCommand);
      int status = (this->*handler)();
      if (status == SUCCESS || status == EXIT) {
        return status;
      }
      printErrorMsg(status);
      return FAILURE;
    }
  }
  cout << "Syntax Error" << endl;
  return FAILURE;
}

RegexHandler FrontendInterface::regexHandler;
int FrontendInterface::handleFrontend(int argc, char *argv[]) {
  // Taking Run Command as Command Line Argument(if provided)
  if (argc == 3 && strcmp(argv[1], "run") == 0) {
    string run_command("run ");
    run_command.append(argv[2]);
    int ret = regexHandler.handle(run_command);
    if (ret == EXIT) {
      return 0;
    }
  }
  char *buf;
  rl_bind_key('\t', rl_insert);
  while ((buf = readline("# ")) != nullptr) {
    if (strlen(buf) > 0) {
      add_history(buf);
    }
    int ret = regexHandler.handle(string(buf));
    free(buf);
    if (ret == EXIT) {
      return 0;
    }
  }
  return 0;
}

// get the operator constant corresponding to the string
int getOperator(string opStr) {
  int op = 0;
  if (opStr == "=")
    op = EQ;
  else if (opStr == "<")
    op = LT;
  else if (opStr == "<=")
    op = LE;
  else if (opStr == ">")
    op = GT;
  else if (opStr == ">=")
    op = GE;
  else if (opStr == "!=")
    op = NE;
  return op;
}

// truncates a given name string to ATTR_NAME sized char array
void attrToTruncatedArray(string nameString, char *nameArray) {
  string truncated = nameString.substr(0, ATTR_SIZE - 1);
  truncated.c_str();
  strcpy(nameArray, truncated.c_str());
  if (nameString.size() >= ATTR_SIZE) {
    printf("(warning: \'%s\' truncated to \'%s\')\n", nameString.c_str(), nameArray);
  }
}

void printErrorMsg(int error) {
  if (error == FAILURE)
    cout << "Error: Command Failed" << endl;
  else if (error == E_OUTOFBOUND)
    cout << "Error: Out of bound" << endl;
  else if (error == E_FREESLOT)
    cout << "Error: Free slot" << endl;
  else if (error == E_NOINDEX)
    cout << "Error: No index" << endl;
  else if (error == E_DISKFULL)
    cout << "Error: Insufficient space in disk" << endl;
  else if (error == E_INVALIDBLOCK)
    cout << "Error: Invalid block" << endl;
  else if (error == E_RELNOTEXIST)
    cout << "Error: Relation does not exist" << endl;
  else if (error == E_RELEXIST)
    cout << "Error: Relation already exists" << endl;
  else if (error == E_ATTRNOTEXIST)
    cout << "Error: Attribute does not exist" << endl;
  else if (error == E_ATTREXIST)
    cout << "Error: Attribute already exists" << endl;
  else if (error == E_CACHEFULL)
    cout << "Error: Cache is full" << endl;
  else if (error == E_RELNOTOPEN)
    cout << "Error: Relation is not open" << endl;
  else if (error == E_RELNOTOPEN)
    cout << "Error: Relation is not open" << endl;
  else if (error == E_NATTRMISMATCH)
    cout << "Error: Mismatch in number of attributes" << endl;
  else if (error == E_DUPLICATEATTR)
    cout << "Error: Duplicate attributes found" << endl;
  else if (error == E_RELOPEN)
    cout << "Error: Relation is open" << endl;
  else if (error == E_ATTRTYPEMISMATCH)
    cout << "Error: Mismatch in attribute type" << endl;
  else if (error == E_INVALID)
    cout << "Error: Invalid index or argument" << endl;
  else if (error == E_MAXRELATIONS)
    cout << "Error: Maximum number of relations already present" << endl;
  else if (error == E_MAXATTRS)
    cout << "Error: Maximum number of attributes allowed for a relation is 125" << endl;
  else if (error == E_NOTPERMITTED)
    cout << "Error: This operation is not permitted" << endl;
  else if (error == E_INDEX_BLOCKS_RELEASED)
    cout << "Warning: Operation succeeded, but some indexes had to be dropped" << endl;
}

void printHelp() {
  printf("CREATE TABLE tablename(attr1_name attr1_type ,attr2_name attr2_type....); \n\t -create a relation with given attribute names\n \n");
  printf("DROP TABLE tablename;\n\t-delete the relation\n  \n");
  printf("OPEN TABLE tablename;\n\t-open the relation \n\n");
  printf("CLOSE TABLE tablename;\n\t-close the relation \n \n");
  printf("CREATE INDEX ON tablename.attributename;\n\t-create an index on a given attribute. \n\n");
  printf("DROP INDEX ON tablename.attributename; \n\t-delete the index. \n\n");
  printf("ALTER TABLE RENAME tablename TO new_tablename;\n\t-rename an existing relation to a given new name. \n\n");
  printf("ALTER TABLE RENAME tablename COLUMN column_name TO new_column_name;\n\t-rename an attribute of an existing relation.\n\n");
  printf("INSERT INTO tablename VALUES ( value1,value2,value3,... );\n\t-insert a single record into the given relation. \n\n");
  printf("INSERT INTO tablename VALUES FROM filepath; \n\t-insert multiple records from a csv file \n\n");
  printf("SELECT * FROM source_relation INTO target_relation; \n\t-creates a relation with the same attributes and records as of source relation\n\n");
  printf("SELECT Attribute1,Attribute2,....FROM source_relation INTO target_relation; \n\t-creates a relation with attributes specified and all records\n\n");
  printf("SELECT * FROM source_relation INTO target_relation WHERE attrname OP value; \n\t-retrieve records based on a condition and insert them into a target relation\n\n");
  printf("SELECT Attribute1,Attribute2,....FROM source_relation INTO target_relation;\n\t-creates a relation with the attributes specified and inserts those records which satisfy the given condition.\n\n");
  printf("SELECT * FROM source_relation1 JOIN source_relation2 INTO target_relation WHERE source_relation1.attribute1 = source_relation2.attribute2; \n\t-creates a new relation with by equi-join of both the source relations\n\n");
  printf("SELECT Attribute1,Attribute2,.. FROM source_relation1 JOIN source_relation2 INTO target_relation WHERE source_relation1.attribute1 = source_relation2.attribute2; \n\t-creates a new relation by equi-join of both the source relations with the attributes specified \n\n");
  printf("echo <any message> \n\t  -echo back the given string. \n\n");
  printf("run <filename> \n\t  -run commands from an input file in sequence. \n\n");
  printf("exit \n\t-Exit the interface\n");
}
