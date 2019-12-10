#include "core.hpp"

namespace lk
{

Core::Core(const base::PropertyTree& config) : _config{config}, _blockchain{config}, _host{config}
{}

} // namespace lk
