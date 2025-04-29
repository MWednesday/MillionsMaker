#include "gecko.h"

bool gecko::api::Initialize(const std::string configFile)
{
	// Load the config file
	std::ifstream configFileStream(configFile);
	if (!configFileStream.is_open())
	{
		std::cerr << "Error: Could not open config file: " << configFile <<
			"\nMake sure it is present and filled with your API values." << std::endl;
		return false;
	}

	API_Config = new API_Configuration{};

	// Read the config file line by line
	std::string line;
	while (std::getline(configFileStream, line))
	{
		if (line.find("API_ENDPOINT") != std::string::npos)
		{
			API_Config->API_ENDPOINT = line.substr(line.find('=') + 1);
		}
		else if (line.find("API_HEADER") != std::string::npos)
		{
			API_Config->API_HEADER = line.substr(line.find('=') + 1);
		}
		else if (line.find("API_KEY") != std::string::npos)
		{
			API_Config->API_KEY = line.substr(line.find('=') + 1);
		}
	}
	configFileStream.close();

    if (API_Config->API_ENDPOINT.empty() || API_Config->API_HEADER.empty() || API_Config->API_KEY.empty())
    {
        std::cerr << "Error: Missing required configuration values in the config file." << std::endl;
        delete API_Config;
        API_Config = nullptr;
        return false;
    }

	return true;
}

void gecko::api::Shutdown()
{
	if (API_Config)
	{
		delete API_Config;
		API_Config = nullptr;
	}
}

bool gecko::api::ping()
{
	cpr::Response r = this->request("ping", NULL);
	if (r.status_code == 200) {
		return true;
	}
	return false; // implicit else - won't reach unless != 200
}