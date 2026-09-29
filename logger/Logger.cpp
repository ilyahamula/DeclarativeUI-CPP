#include "Logger.hpp"

Logger& Logger::instance()
{
	static Logger logger;
	return logger;
}

void Logger::log(const std::string& message)
{
	const std::lock_guard<std::mutex> lock(m_mutex);
	if (m_stopped)
		return;
	if (m_messages.size() == kCapacity)
		m_messages.pop_front();
	m_messages.push_back(message);
}

void Logger::stopLogging()
{
	const std::lock_guard<std::mutex> lock(m_mutex);
	m_stopped = true;
}

std::string Logger::getAll() const
{
	const std::lock_guard<std::mutex> lock(m_mutex);
	std::string result;
	for (const auto& msg : m_messages)
		result += msg;
	return result;
}
