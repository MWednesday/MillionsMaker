#include "JSONBase.h"
#include <fstream>
#include <sstream>
#include "Logging.h"

bool JSONBase::Deserialize(const std::string& s)
{
	rapidjson::Document doc;
	if (!InitDocument(s, doc))
		return false;

	Deserialize(doc);

	return true;
}

bool JSONBase::DeserializeFromFile(const std::string& filePath)
{
	std::ifstream f(filePath);
	std::stringstream buffer;
	buffer << f.rdbuf();
	f.close();

	return Deserialize(buffer.str());
}

bool JSONBase::InitDocument(const std::string& s, rapidjson::Document& doc)
{
	if (s.empty())
		return false;

	std::string validJson(s);

	bool successful = !doc.Parse(validJson.c_str()).HasParseError();

    rapidjson::ParseErrorCode error = doc.GetParseError();
    if (error != rapidjson::ParseErrorCode::kParseErrorNone)
    {
	  	ReportError("Failed to parse json! Error code: %u", error);
    }
    
    return successful;
}