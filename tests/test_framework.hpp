#pragma once

// Minimal test framework for the backend-free engine tests (see
// docs/specs/custom_layout_system/tasks.md, T0.1). One process runs every
// TEST(); ctest treats a non-zero exit code as failure.

#include <cstdio>
#include <functional>
#include <sstream>
#include <string>
#include <vector>

namespace testfw
{

struct TestCase
{
	std::string name;
	std::function<void()> fn;
};

inline std::vector<TestCase>& registry()
{
	static std::vector<TestCase> r;
	return r;
}

inline int& failureCount()
{
	static int count = 0;
	return count;
}

struct Registrar
{
	Registrar(std::string name, std::function<void()> fn)
	{
		registry().push_back({ std::move(name), std::move(fn) });
	}
};

inline int runAll()
{
	int failedTests = 0;
	for (const auto& test : registry())
	{
		const int failuresBefore = failureCount();
		test.fn();
		const bool failed = failureCount() != failuresBefore;
		std::printf("[%s] %s\n", failed ? "FAIL" : " OK ", test.name.c_str());
		if (failed)
			++failedTests;
	}
	std::printf("%zu test(s), %d failed\n", registry().size(), failedTests);
	return failedTests == 0 ? 0 : 1;
}

} // namespace testfw

#define TEST(name) \
	static void test_##name(); \
	static const testfw::Registrar registrar_##name(#name, &test_##name); \
	static void test_##name()

#define CHECK(cond) \
	do \
	{ \
		if (!(cond)) \
		{ \
			++testfw::failureCount(); \
			std::printf("    CHECK failed: %s (%s:%d)\n", #cond, __FILE__, __LINE__); \
		} \
	} while (0)

// Operands must be equality-comparable and streamable.
#define CHECK_EQ(a, b) \
	do \
	{ \
		const auto& va = (a); \
		const auto& vb = (b); \
		if (!(va == vb)) \
		{ \
			++testfw::failureCount(); \
			std::ostringstream os; \
			os << "    CHECK_EQ failed: " << #a << " == " << #b \
			   << " (" << va << " vs " << vb << ") (" << __FILE__ << ":" << __LINE__ << ")"; \
			std::puts(os.str().c_str()); \
		} \
	} while (0)
