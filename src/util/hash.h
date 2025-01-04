#ifndef KVSTORAGE_UTIL_HASH_H_
#define KVSTORAGE_UTIL_HASH_H_

#include <cstddef>
#include <cstdint>

namespace kvstorage {

uint32_t Hash(const char* data, size_t n, uint32_t seed);

}  // namespace kvstorage

#endif  // KVSTORAGE_UTIL_HASH_H_