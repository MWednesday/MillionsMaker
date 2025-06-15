#pragma once

#include <string>
#include "rapidjson\document.h"
#include "rapidjson\prettywriter.h"	// for stringify JSON

class JSONBase
{
public:
  bool DeserializeFromFile(const std::string& filePath);
  virtual bool Deserialize(const std::string& s);
  virtual bool Deserialize(const rapidjson::Value& obj) = 0;

  static bool InitDocument(const std::string& s, rapidjson::Document& doc);
};