#ifndef REGEX_HANDLER_H
#define REGEX_HANDLER_H

#include <regex>
#include <string>
#include <vector>

/* External File System Commands */
#define HELP_CMD "\\s*HELP\\s*;?"
#define EXIT_CMD "\\s*EXIT\\s*;?"
#define RUN_CMD "\\s*RUN\\s+([a-zA-Z0-9_/.-]+)\\s*;?"
#define ECHO_CMD "\\s*ECHO\\s*([a-zA-Z0-9 _,()'?:+*.-]*)\\s*;?"

/* DDL Commands*/
#define CREATE_TABLE_CMD "\\s*CREATE\\s+TABLE\\s+([A-Za-z0-9_-]+)\\s*\\(\\s*((?:[#A-Za-z0-9_-]+\\s+(?:STR|NUM)\\s*,\\s*)*(?:[#A-Za-z0-9_-]+\\s+(?:STR|NUM)))\\s*\\)\\s*;?"
#define DROP_TABLE_CMD "\\s*DROP\\s+TABLE\\s+([A-Za-z0-9_-]+)\\s*;?"
#define OPEN_TABLE_CMD "\\s*OPEN\\s+TABLE\\s+([A-Za-z0-9_-]+)\\s*;?"
#define CLOSE_TABLE_CMD "\\s*CLOSE\\s+TABLE\\s+([A-Za-z0-9_-]+)\\s*;?"
#define CREATE_INDEX_CMD "\\s*CREATE\\s+INDEX\\s+ON\\s+([A-Za-z0-9_-]+)\\s*\\.\\s*([#A-Za-z0-9_-]+)\\s*;?"
#define DROP_INDEX_CMD "\\s*DROP\\s+INDEX\\s+ON\\s+([A-Za-z0-9_-]+)\\s*\\.\\s*([#A-Za-z0-9_-]+)\\s*;?"
#define RENAME_TABLE_CMD "\\s*ALTER\\s+TABLE\\s+RENAME\\s+([a-zA-Z0-9_-]+)\\s+TO\\s+([a-zA-Z0-9_-]+)\\s*;?"
#define RENAME_COLUMN_CMD "\\s*ALTER\\s+TABLE\\s+RENAME\\s+([a-zA-Z0-9_-]+)\\s+COLUMN\\s+([#a-zA-Z0-9_-]+)\\s+TO\\s+([#a-zA-Z0-9_-]+)\\s*;?"

/* DML Commands */
#define SELECT_FROM_CMD "\\s*SELECT\\s+\\*\\s+FROM\\s+([A-Za-z0-9_-]+)\\s+INTO\\s+([A-Za-z0-9_-]+)\\s*;?"
#define SELECT_ATTR_FROM_CMD "\\s*SELECT\\s+((?:[#A-Za-z0-9_-]+\\s*,\\s*)*(?:[#A-Za-z0-9_-]+))\\s+FROM\\s+([A-Za-z0-9_-]+)\\s+INTO\\s+([A-Za-z0-9_-]+)\\s*;?"
#define SELECT_FROM_WHERE_CMD "\\s*SELECT\\s+\\*\\s+FROM\\s+([A-Za-z0-9_-]+)\\s+INTO\\s+([A-Za-z0-9_-]+)\\s+WHERE\\s+([#A-Za-z0-9_-]+)\\s*(<|<=|>|>=|=|!=)\\s*([A-Za-z0-9_-]+|([0-9]+(\\.)[0-9]+))\\s*;?"
#define SELECT_ATTR_FROM_WHERE_CMD "\\s*SELECT\\s+((?:[#A-Za-z0-9_-]+\\s*,\\s*)*(?:[#A-Za-z0-9_-]+))\\s+FROM\\s+([A-Za-z0-9_-]+)\\s+INTO\\s+([A-Za-z0-9_-]+)\\s+WHERE\\s+([#A-Za-z0-9_-]+)\\s*(<|<=|>|>=|=|!=)\\s*([A-Za-z0-9_-]+|([0-9]+(\\.)[0-9]+))\\s*;?"
#define SELECT_FROM_JOIN_CMD "\\s*SELECT\\s+\\*\\s+FROM\\s+([A-Za-z0-9_-]+)\\s+JOIN\\s+([A-Za-z0-9_-]+)\\s+INTO\\s+([A-Za-z0-9_-]+)\\s+WHERE\\s+([A-Za-z0-9_-]+)\\s*\\.([#A-Za-z0-9_-]+)\\s*\\=\\s*([A-Za-z0-9_-]+)\\s*\\.([#A-Za-z0-9_-]+)\\s*;?"
#define SELECT_ATTR_FROM_JOIN_CMD "\\s*SELECT\\s+((?:[#A-Za-z0-9_-]+\\s*,\\s*)*(?:[#A-Za-z0-9_-]+))\\s+FROM\\s+([A-Za-z0-9_-]+)\\s+JOIN\\s+([A-Za-z0-9_-]+)\\s+INTO\\s+([A-Za-z0-9_-]+)\\s+WHERE\\s+([A-Za-z0-9_-]+)\\s*\\.([#A-Za-z0-9_-]+)\\s*\\=\\s*([A-Za-z0-9_-]+)\\s*\\.([#A-Za-z0-9_-]+)\\s*;?"
#define INSERT_SINGLE_CMD "\\s*INSERT\\s+INTO\\s+([A-Za-z0-9_-]+)\\s+VALUES\\s*\\(\\s*((?:(?:[A-Za-z0-9_-]+|[0-9]+\\.[0-9]+)\\s*,\\s*)*(?:[A-Za-z0-9_-]+|[0-9]+\\.[0-9]+))\\s*\\)\\s*;?"
#define INSERT_MULTIPLE_CMD "\\s*INSERT\\s+INTO\\s+([A-Za-z0-9_-]+)\\s+VALUES\\s+FROM\\s+([a-zA-Z0-9_-]+\\.csv)\\s*;?"
#define CUSTOM_CMD "\\s*FUNCTION\\s+([A-Za-z,#0-9\\s()_-]+)\\s*;?"

#define REGEX(c) std::regex(c, std::regex_constants::icase)

class RegexHandler {
  typedef int (RegexHandler::*handlerFunction)(void);  // function pointer type

 private:
  // command to handler mappings
  const std::vector<std::pair<std::regex, handlerFunction>> handlers = {
      {REGEX(HELP_CMD), &RegexHandler::helpHandler},
      {REGEX(EXIT_CMD), &RegexHandler::exitHandler},
      {REGEX(ECHO_CMD), &RegexHandler::echoHandler},
      {REGEX(RUN_CMD), &RegexHandler::runHandler},
      {REGEX(OPEN_TABLE_CMD), &RegexHandler::openHandler},
      {REGEX(CLOSE_TABLE_CMD), &RegexHandler::closeHandler},
      {REGEX(CREATE_TABLE_CMD), &RegexHandler::createTableHandler},
      {REGEX(DROP_TABLE_CMD), &RegexHandler::dropTableHandler},
      {REGEX(CREATE_INDEX_CMD), &RegexHandler::createIndexHandler},
      {REGEX(DROP_INDEX_CMD), &RegexHandler::dropIndexHandler},
      {REGEX(RENAME_TABLE_CMD), &RegexHandler::renameTableHandler},
      {REGEX(RENAME_COLUMN_CMD), &RegexHandler::renameColumnHandler},
      {REGEX(INSERT_SINGLE_CMD), &RegexHandler::insertSingleHandler},
      {REGEX(INSERT_MULTIPLE_CMD), &RegexHandler::insertFromFileHandler},
      {REGEX(SELECT_FROM_CMD), &RegexHandler::selectFromHandler},
      {REGEX(SELECT_FROM_WHERE_CMD), &RegexHandler::selectFromWhereHandler},
      {REGEX(SELECT_ATTR_FROM_CMD), &RegexHandler::selectAttrFromHandler},
      {REGEX(SELECT_ATTR_FROM_WHERE_CMD), &RegexHandler::selectAttrFromWhereHandler},
      {REGEX(SELECT_FROM_JOIN_CMD), &RegexHandler::selectFromJoinHandler},
      {REGEX(SELECT_ATTR_FROM_JOIN_CMD), &RegexHandler::selectAttrFromJoinHandler},
      {REGEX(CUSTOM_CMD), &RegexHandler::customFunctionHandler},
  };

  // extract tokens delimited by whitespace and comma
  std::vector<std::string> extractTokens(std::string input);

  // handler functions
  std::smatch m;  // to store matches while parsing the regex
  int helpHandler();
  int exitHandler();
  int echoHandler();
  int runHandler();
  int openHandler();
  int closeHandler();
  int createTableHandler();
  int dropTableHandler();
  int createIndexHandler();
  int dropIndexHandler();
  int renameTableHandler();
  int renameColumnHandler();
  int insertSingleHandler();
  int insertFromFileHandler();
  int selectFromHandler();
  int selectFromWhereHandler();
  int selectAttrFromHandler();
  int selectAttrFromWhereHandler();
  int selectFromJoinHandler();
  int selectAttrFromJoinHandler();
  int customFunctionHandler();

 public:
  int handle(const std::string command);
};

#endif  // REGEX_HANDLER_H
