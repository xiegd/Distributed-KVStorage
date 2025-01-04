#include "status.h"

#include <utility>

#include "gtest/gtest.h"
#include "slice.h"

namespace kvstorage {

TEST(Status, MoveConstructor) {
  {
    Status ok = Status::success();
    Status ok2 = std::move(ok);

    ASSERT_TRUE(ok2.ok());
  }

  {
    Status status = Status::notFound("custom NotFound status message");
    Status status2 = std::move(status);

    ASSERT_TRUE(status2.isNotFound());
    ASSERT_EQ("NotFound: custom NotFound status message", status2.toString());
  }

  {
    Status self_moved = Status::ioError("custom IOError status message");

    // Needed to bypass compiler warning about explicit move-assignment.
    Status& self_moved_reference = self_moved;
    self_moved_reference = std::move(self_moved);
  }
}

}  // namespace kvstorage