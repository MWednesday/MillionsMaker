#pragma once

#include "Logging.h"

namespace BasicLogging
{
  void RegisterBasicLoggers(bool logDebugString = true, bool logCout = true); // Call this at the beginning of your program
  void LogOutputDebugString(const Message& message);
  void LogCout(const Message& message);
}