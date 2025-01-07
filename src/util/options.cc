#include "options.h"

#include "comparator.h"
#include "env.h"

namespace kvstorage {

Options::Options() : comparator(BytewiseComparator()), env(Env::defaultEnv()) {}

}