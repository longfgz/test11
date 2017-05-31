#ifndef LCLWH8V2L8S6M7N9_PUBLIC_UTIL_MACROS
#define LCLWH8V2L8S6M7N9_PUBLIC_UTIL_MACROS

#include <boost/preprocessor/cat.hpp> 
#include <boost/preprocessor/comparison/equal.hpp> 
#include <boost/preprocessor/control/if.hpp>

// 内部使用的宏
#define NORMAL_PARAMETER(type_)   type_ data
#define REF_PARAMETER(type_)      type_ const &data

// 内部使用的宏
#define FUNC_PARAMETER_(type_, use_ref_)                              \
  BOOST_PP_IF( BOOST_PP_EQUAL(use_ref_, 0)                            \
    , NORMAL_PARAMETER(type_)                                         \
    , REF_PARAMETER(type_) )

// 因为怕影响类的内存布局，所以此宏未整合类的成员变量的定义
#define DEFINE_MEMBER_ACCESSOR(type_, mem_, func_, use_ref_)          \
	public: inline void set_##func_(FUNC_PARAMETER_(type_, use_ref_))   \
          { mem_ = data; };                                           \
	public: inline type_ get_##func_() const { return mem_; };

// 为单元测试开放类的访问权限
// Mock能方便模拟非虚成员函数
#ifdef UNIT_TEST
#   define TEST_ACCESS public:
    // ATTENTION !!!!!!!!!!!!
    // virtual 关键字可能会改变对象的内存布局，请注意到此点后使用
#   define TEST_VIRTUAL virtual
    // 如果不存在虚析构类，则使用下面的宏定义一个，以防止测试时资源泄露
#   define TEST_VIRTUAL_DESTRUCTOR(class_name_) \
      public: virtual ~class_name_() {}
#else
#   define TEST_ACCESS
#   define TEST_VIRTUAL
#   define TEST_VIRTUAL_DESTRUCTOR(class_name_)
#endif  // #ifdef UNIT_TEST

#endif  // #ifndef LCLWH8V2L8S6M7N9_PUBLIC_UTIL_MACROS