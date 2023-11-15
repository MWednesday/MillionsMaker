#pragma once

#include <string>
#include "rapidjson\document.h"
#include "rapidjson\rapidjson.h"
#include "rapidjson\stringbuffer.h"	// wrapper of C stream for prettywriter as output
#include "rapidjson\prettywriter.h"	// for stringify JSON


class JSONBase
{
public:
  bool DeserializeFromFile(const std::string& filePath);
  //bool SerializeToFile(const std::string& filePath);

  //virtual std::string Serialize() const;
  virtual bool Deserialize(const std::string& s);

  virtual bool Deserialize(const rapidjson::Value& obj) = 0;
  //virtual bool Serialize(rapidjson::Writer<rapidjson::StringBuffer>* writer) const = 0;

  static bool InitDocument(const std::string & s, rapidjson::Document &doc);
};