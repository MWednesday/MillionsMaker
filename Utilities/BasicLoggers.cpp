#include "BasicLoggers.h"
#include <atlstr.h> // CA2CT
#include <iostream>

namespace BasicLogging
{
  void RegisterBasicLoggers(bool logDebugString, bool logCout)
  {
      if (logDebugString)
      {
          g_Loggers.emplace_back(BasicLogging::LogOutputDebugString);
      }
      if (logCout)
      {
          g_Loggers.emplace_back(BasicLogging::LogCout);
      }
  }

  void LogOutputDebugString(const Message& message)
  {
    OutputDebugString(CA2CT(message.m_Text));
  }

  void LogCout(const Message& message)
  {
    std::cout << message.m_Text;
  }
}