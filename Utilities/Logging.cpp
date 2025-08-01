#include "Logging.h"
#include <cstdarg> // va_start

std::vector<std::function<void(const Message& message)>> g_Loggers;

void FormatAndPrintMessage(MessageType type, std::string message, ...)
{
  char formattedMessage[512];

  // If at the beginning of the string user added \n, make sure we add empty line.
  const size_t supportedNewLines = 5;
  std::size_t nCount = 0;
  while (nCount < supportedNewLines &&
         nCount < message.size() &&
         message[nCount] == '\n')
  {
      ++nCount;
  }
  std::string newLines(nCount, '\n');
  if (nCount > 0)
  {
      message.erase(0, nCount);
  }

  std::string messageTypeString;
  switch (type)
  {
  case MessageType::Debug:
    messageTypeString = "DEBUG";
    break;
  case MessageType::Info:
    messageTypeString = "INFO";
    break;
  case MessageType::Warning:
    messageTypeString = "WARNING";
    break;
  case MessageType::Error:
    messageTypeString = "ERROR";
    break;
  default:
    break;
  }

  va_list arg;
  va_start(arg, message);
  _vsnprintf_s(formattedMessage, sizeof(formattedMessage), _TRUNCATE, message.c_str(), arg); // %s %d %f meaning - https://stackoverflow.com/questions/37188727/what-does-the-output-of-this-code-mean-printfd-x
  va_end(arg);

  message = newLines + messageTypeString + ": " + formattedMessage + "\n";
  strcpy_s(formattedMessage, sizeof(formattedMessage), message.c_str());

  Message msg{ formattedMessage, type };

  // Printing
  for (const auto& callback : g_Loggers)
  {
    callback(msg);
  }
}