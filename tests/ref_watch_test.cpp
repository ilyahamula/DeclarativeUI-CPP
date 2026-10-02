#include "test_framework.hpp"

#include "frameworks_core/CoreTypes/BoundValue.hpp"
#include "frameworks_core/CoreTypes/Observable.hpp"
#include "frameworks_core/RefWatch.hpp"

#include <string>
#include <vector>

// Backend-free: RefWatch and Observable are what every retained poll asks
// first (review finding 7). A plain variable is compared with a kept copy; an
// Observable's value is answered from its change counter.

using Rows = std::vector<std::vector<std::string>>;

TEST(ref_watch_reports_a_plain_change_once)
{
	Rows rows { { "a", "1" }, { "b", "2" } };
	auto watch = watchRefs(&rows);
	CHECK(!watch.changed());
	rows[1][1] = "3";
	CHECK(watch.changed());
	CHECK(!watch.changed()); // adopted
	rows.push_back({ "c", "4" });
	CHECK(watch.changed());
}

TEST(ref_watch_null_sources_never_change)
{
	const std::string* unbound = nullptr;
	int value = 1;
	auto watch = watchRefs(unbound, &value);
	CHECK(!watch.changed());
	value = 2;
	CHECK(watch.changed());
}

TEST(ref_watch_asks_every_source)
{
	int a = 0;
	int b = 0;
	auto watch = watchRefs(&a, &b);
	a = 1;
	b = 1;
	CHECK(watch.changed());
	CHECK(!watch.changed()); // both adopted in the one call, not one per call
}

TEST(observable_counts_edit_and_set_and_framework_writes)
{
	Observable<Rows> rows { Rows { { "a", "1" } } };
	Rows& bound = rows; // what a widget binds
	auto watch = watchRefs(static_cast<const Rows*>(&bound));
	CHECK(observedVersion(&bound) != nullptr);
	CHECK(!watch.changed());

	rows.edit().push_back({ "b", "2" });
	CHECK(watch.changed());
	CHECK(!watch.changed());

	rows.set(Rows {});
	CHECK(watch.changed());

	// a write through the binding reference is NOT counted -- the documented
	// contract -- but the framework's own writes are
	bound.push_back({ "c", "3" });
	CHECK(!watch.changed());
	markChanged(&bound);
	CHECK(watch.changed());
}

TEST(observable_bound_value_set_is_counted)
{
	Observable<std::string> text { std::string("x") };
	BoundValue<std::string> value(static_cast<std::string&>(text));
	auto watch = watchRefs(value.boundValue());
	value.set("y");
	CHECK(watch.changed());
	CHECK_EQ(text.get(), std::string("y"));
}

TEST(observable_registry_follows_the_object)
{
	const void* address = nullptr;
	{
		Observable<int> counter { 1 };
		int& bound = counter;
		address = &bound;
		CHECK(observedVersion(address) != nullptr);

		Observable<int> copy = counter;
		int& copyBound = copy;
		CHECK(observedVersion(&copyBound) != nullptr);
		CHECK(observedVersion(&copyBound) != observedVersion(address));
	}
	CHECK(observedVersion(address) == nullptr);

	// markChanged on a plain variable is a no-op
	int plain = 0;
	markChanged(&plain);
	CHECK(observedVersion(&plain) == nullptr);
}
