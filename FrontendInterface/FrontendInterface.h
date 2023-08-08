#ifndef FRONTEND_INTERFACE_H
#define FRONTEND_INTERFACE_H

#include "RegexHandler.h"

class FrontendInterface {
 private:
  static RegexHandler regexHandler;

 public:
  static int handleFrontend(int argc, char *argv[]);
};

#endif
