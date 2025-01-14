#include "util/no_destructor.h"

#include <cstdint>
#include <cstdlib>
#include <utility>

#include "gtest/gtest.h"

namespace kvstorage {

namespace {

struct DoNotDestruct {
public:
    DoNotDestruct(uint32_t a, uint64_t b) : a(a), b(b) {}
    ~DoNotDestruct() { std::abort(); }

    // Used to check constructor argument forwarding.
    uint32_t a;
    uint64_t b;
};

constexpr const uint32_t kGoldenA = 0xdeadbeef;
constexpr const uint64_t kGoldenB = 0xaabbccddeeffaabb;

}  // namespace

TEST(NoDestructorTest, StackInstance) {
    NoDestructor<DoNotDestruct> instance(kGoldenA, kGoldenB);
    ASSERT_EQ(kGoldenA, instance.get()->a);
    ASSERT_EQ(kGoldenB, instance.get()->b);
}

TEST(NoDestructorTest, StaticInstance) {
    static NoDestructor<DoNotDestruct> instance(kGoldenA, kGoldenB);
    ASSERT_EQ(kGoldenA, instance.get()->a);
    ASSERT_EQ(kGoldenB, instance.get()->b);
}

}  // namespace kvstorage
