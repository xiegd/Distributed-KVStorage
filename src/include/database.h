#ifndef D_KVSTORAGE_DATABASE_H
#define D_KVSTORAGE_DATABASE_H

// #include "export.h"
#include "slice.h"
#include "iterator.h"
#include "options.h"

namespace kvstorage {

static const int s_major_version = 1;
static const int s_minor_version = 23;

struct Options;
struct ReadOptions;
struct WriteOptions;
class WriteBatch;

class Snapshot {
public:
    Snapshot() = default;
    Snapshot(const Snapshot&) = delete;
    Snapshot& operator=(const Snapshot&) = delete;

protected:
    virtual ~Snapshot();
};

struct Range {
    Range() = default;
    Range(const Slice& s, const Slice& l) : start(s), limit(l) {}

    Slice start;
    Slice limit;
}

}

#endif