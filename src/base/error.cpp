#include "error.hpp"

namespace base
{

Error::Error(const char* file_name, std::size_t line_number, const char* function_signature, std::string message)
  : _file_name{ file_name }
  , _line_number{ line_number }
  , _function_signature{ function_signature }
  , _message{ std::move(message) }
{
    //    _full_message = std::string(_file_name) + ":" + std::to_string(_line_number)
    //                    + " " + _function_signature + " " + _message;
    _full_message = _message;
}


const std::string& Error::getMessage() const noexcept
{
    return _message;
}


const char* Error::what() const noexcept
{
    return _full_message.c_str();
}

} // namespace base