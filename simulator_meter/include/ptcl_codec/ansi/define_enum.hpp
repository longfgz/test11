#ifndef LCLWH8V2L8S6M7N9_PUBLIC_UTIL_DEFINE_ENUM
#define LCLWH8V2L8S6M7N9_PUBLIC_UTIL_DEFINE_ENUM

#include <type_traits>
#include <boost/preprocessor/control/iif.hpp>
#include <boost/preprocessor/comparison/equal.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/seq/enum.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/seq/transform.hpp>

// 内部使用的宏
#define DEFINE_ENUM_VALUE_(s, data, elem_)                      \
  BOOST_PP_SEQ_HEAD(elem_)                                      \
  BOOST_PP_IIF(BOOST_PP_EQUAL(BOOST_PP_SEQ_SIZE(elem_), 2),     \
               = BOOST_PP_SEQ_TAIL(elem_), )

// 内部使用的宏
#define ENUM_FORMAT_INNER_CASE_(r, data, elem_)                 \
  case data::BOOST_PP_SEQ_HEAD(elem_):                          \
    return BOOST_PP_STRINGIZE(BOOST_PP_SEQ_HEAD(elem_));

// 定义枚举类型，并同时定义将其转换为字符串的函数（to_string）
// 使用示例如下：
// DEFINE_ENUM( Error_ET,
//              ((ErrorA)(0))
//              ((ErrorB))
//              ((ErrorC)))
#define DEFINE_ENUM(name_, values_)                             \
  enum class name_                                              \
  {                                                             \
    BOOST_PP_SEQ_ENUM(                                          \
      BOOST_PP_SEQ_TRANSFORM(DEFINE_ENUM_VALUE_, , values_))    \
  };                                                            \
  inline const char* to_string(name_ val)                       \
  {                                                             \
    switch (val)                                                \
    {                                                           \
    BOOST_PP_SEQ_FOR_EACH(                                      \
      ENUM_FORMAT_INNER_CASE_, name_, values_)                  \
    default:                                                    \
      return "[Unknown " BOOST_PP_STRINGIZE(name_) "]";         \
    }                                                           \
  }

// 内部使用的宏
#define ENUM_CONVERTOR_TO_STRING_INNER_CASE_(r, data, elem_)    \
  case data::elem_:                                             \
    return BOOST_PP_STRINGIZE(elem_);

// 为已经在其它地方定义的枚举类型提供字符串转换函数（to_string）
#define DEFINE_TO_STRING_FUNCTION(enum_type_, enum_elems_)      \
  inline char const* to_string(enum_type_ val)                  \
  {                                                             \
    switch (val)                                                \
    {                                                           \
      BOOST_PP_SEQ_FOR_EACH(                                    \
        ENUM_CONVERTOR_TO_STRING_INNER_CASE_,                   \
        enum_type_, enum_elems_)                                \
      default:                                                  \
        return "[Unknown " BOOST_PP_STRINGIZE(enum_type_) "]";  \
    }                                                           \
  }

// 枚举类型转换为整型，如果是使用 enum class 定义的枚举类型的话
template <typename Enum_T> 
auto to_integer(Enum_T const &value) 
  -> typename std::underlying_type<Enum_T>::type
{
  static_assert(std::is_enum<Enum_T>::value, "parameter is not of type enum or enum class!");
  return static_cast<typename std::underlying_type<Enum_T>::type>(value);
}

#endif  // #ifndef LCLWH8V2L8S6M7N9_PUBLIC_UTIL_DEFINE_ENUM