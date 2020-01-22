#pragma once

#include <boost/preprocessor.hpp>

#include <functional>

namespace base
{

#define X_DEFINE_ENUM_CLASS_WITH_STRING_CONVERSIONS_TOSTRING_CASE(r, data, elem) \
    case data::elem: \
        return BOOST_PP_STRINGIZE(elem);

#define DEFINE_ENUM_CLASS_WITH_STRING_CONVERSIONS(name, base_type, enumerators) \
    enum class name : base_type \
    { \
        BOOST_PP_SEQ_ENUM(enumerators) \
    }; \
\
    inline const char* enumToString(name v) \
    { \
        switch(v) { \
            BOOST_PP_SEQ_FOR_EACH(X_DEFINE_ENUM_CLASS_WITH_STRING_CONVERSIONS_TOSTRING_CASE, name, enumerators) \
            default: \
                return nullptr; \
        } \
    }


template<typename... Types>
struct TypeList
{};


template<typename... Args>
class Observable
{
  public:
    using CallbackType = std::function<void(Args...)>;

    void subscribe(CallbackType callback);
    void notify(Args... args);

  private:
    std::vector<CallbackType> _observers;
};

} // namespace base

#include "utility.tpp"