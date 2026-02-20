#include "Logger.hpp"

Logger& Logger::instance()
{
	static Logger logger;
	return logger;
}

void Logger::log(const std::string& message)
{
	if (m_stopped)
		return;
	m_messages.push_back(message);
}

void Logger::stopLogging()
{
	m_stopped = true;
}

std::string Logger::getAll() const
{
	std::string result;
	for (const auto& msg : m_messages)
		result += msg;
	return result;
}
