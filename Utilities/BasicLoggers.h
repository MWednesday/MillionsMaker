#pragma once

#include "Logging.h"

namespace BasicLogging
{
  void RegisterBasicLoggers(); // Call this at the beginning of your program
  void LogOutputDebugString(const Message& message);
  void LogCout(const Message& message);
}