#pragma once

#include <cstddef>
#include <deque>
#include <mutex>
#include <string>

// Trace of what the backends created (USE_LOGGER). Kept in memory for a
// debugger or a test to read with getAll().
//
// Bounded: only the most recent kCapacity messages are kept, so a long-running
// application does not grow the trace forever. Safe to call from any thread.
class Logger
{
public:
	static constexpr std::size_t kCapacity = 1000;

	static Logger& instance();

	void log(const std::string& message);
	void stopLogging();
	std::string getAll() const;

private:
	Logger() = default;
	Logger(const Logger&) = delete;
	Logger& operator=(const Logger&) = delete;

	mutable std::mutex m_mutex;
	std::deque<std::string> m_messages;
	bool m_stopped = false;
};
