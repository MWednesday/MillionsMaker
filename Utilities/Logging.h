#pragma once

#include <vector>
#include <functional>
#include <string>

enum class MessageType
{
  Debug,
  Info,
  Warning,
  Error
};

struct Message
{
  char* m_Text = nullptr;
  MessageType m_Type;
};

extern std::vector<std::function<void(const Message& message)>> g_Loggers;

#ifdef _DEBUG
#define ReportDebug(debug, ...) FormatAndPrintMessage(MessageType::Debug, debug, __VA_ARGS__)
#else
#define ReportDebug(debug, ...)
#endif

#define ReportInfo(info, ...) FormatAndPrintMessage(MessageType::Info, info, __VA_ARGS__)
#define ReportWarning(warning, ...) FormatAndPrintMessage(MessageType::Warning, warning, __VA_ARGS__)
#define ReportError(error, ...) FormatAndPrintMessage(MessageType::Error, error, __VA_ARGS__)

void FormatAndPrintMessage(MessageType type, std::string message, ...);