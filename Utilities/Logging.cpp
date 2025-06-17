#include "Logging.h"
#include <cstdarg> // va_start

std::vector<std::function<void(const Message& message)>> g_Loggers;

void FormatAndPrintMessage(MessageType type, std::string message, ...)
{
  char formattedMessage[512];

  // If at the beginning of the string user added \n, make sure we add empty line.
  const size_t supportedNewLines = 5;
  std::string newLines;
  for (int i = 0; i < supportedNewLines; i++)
  {
    if (*(message.begin() + i) == '\n')
    {
      newLines = newLines + '\n';
    }
    else
    {
        break;
    }
  }
  message.erase(message.begin(), message.begin() + newLines.size());

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
  message = newLines + messageTypeString + ": " + message + "\n";

  va_list arg;
  va_start(arg, message);
  _vsnprintf_s(formattedMessage, sizeof(formattedMessage), message.c_str(), arg); // %s %d %f meaning - https://stackoverflow.com/questions/37188727/what-does-the-output-of-this-code-mean-printfd-x
  va_end(arg);

  Message msg{ formattedMessage, type };

  // Printing
  for (const auto& callback : g_Loggers)
  {
    callback(msg);
  }
}