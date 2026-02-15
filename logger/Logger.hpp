#pragma once

#include <string>
#include <vector>

class Logger
{
public:
	static Logger& instance();

	void log(const std::string& message);
	void stopLogging();
	std::string getAll() const;

private:
	Logger() = default;
	Logger(const Logger&) = delete;
	Logger& operator=(const Logger&) = delete;

	std::vector<std::string> m_messages;
	bool m_stopped = false;
};
