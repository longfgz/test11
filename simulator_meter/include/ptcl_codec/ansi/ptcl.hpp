#ifndef LCLWH8V2L8S6M7N9_PUBLIC_PTCL_ANSI_C12
#define LCLWH8V2L8S6M7N9_PUBLIC_PTCL_ANSI_C12

#include "define_enum.hpp"
#include "macros.hpp"
#include <cassert>
#include <ctime>
#include <type_traits>
#include <boost/array.hpp>
#include <boost/concept_check.hpp>
#include <boost/crc.hpp>

typedef uint8_t byte_t;

namespace ptcl_utility {
template <typename ScalarType_T> 
ScalarType_T change_endian(ScalarType_T val)
{
  static_assert(std::is_scalar<ScalarType_T>::value, "Type is not of scalar!");
  int n = sizeof(val);
  int pos_b = 0;
  int pos_e = n - 1;
  byte_t *p = reinterpret_cast<byte_t*>(&val);
  byte_t tmp = 0;
  while (pos_b < pos_e)
  {
    tmp = p[pos_b];
    p[pos_b] = p[pos_e];
    p[pos_e] = tmp;
    ++pos_b;
    --pos_e;
  }
  return val;
}
} // namespace ptcl_utility

namespace ptcl_utility { namespace ansi_c12 { namespace dot18 {
enum
{
    STP = 0xEE  // Start of packet character
  , ACK = 0x06  // 肯定的确认
  , NAK = 0x15  // 否定的确认
};

enum FunctionCode_ET
{
    RSP_OK   = 0x00
  , RSP_ERR  = 0x01
  , RSP_SNS  = 0x02
  , RSP_ISC  = 0x03
  , RSP_ONP  = 0x04
  , RSP_IAR  = 0x05
  , RSP_BSY  = 0x06
  , RSP_DNR  = 0x07
  , RSP_DLK  = 0x08
  , RSP_RNO  = 0x09
  , RSP_ISSS = 0x0A

  , REQ_IDENT = 0x20
  , REQ_LOGON = 0x50
  , REQ_SECURITY = 0x51
  , REQ_LOGOFF = 0x52
  , REQ_NEGOTIATE_0 = 0x60    // 未列举所有的 Negotiate
  , REQ_WAIT = 0x70
  , REQ_TERMINATE = 0x21
  , REQ_PREAD_OFFSET = 0x3F
};

// Todo_Sxc: 去掉“FT_”
DEFINE_ENUM(FrameType_ET, 
  ((FT_NONE)  (0))
  ((FT_ACK)      )
  ((FT_DATA)     )
);

typedef boost::crc_optimal<16, 0x1021, 0xFFFF, 0xFFFF, true, true> Crc_AT;

#pragma pack(1)
class FrameHead_CT
{
public: inline void init(
            uint16_t len
          , uint8_t toggle_bit
          , bool is_multiple = false
          , byte_t seq = 0
          , bool is_first = false
        )
        {
          i_stp = STP;
          i_identity = 0;
          set_ctrl(toggle_bit, is_multiple, is_first);
          i_seq_nbr = seq;
          set_len(len);
        };

public: inline bool is_valid() const
        {
          return (STP == i_stp) 
              && (i_identity != 0xFF);  /* The value FF is reserved for ANSI C12.21 calling party use.*/
        };

public: inline uint16_t get_len() const 
        { return change_endian(i_length); };

public: inline void set_len(uint16_t len) 
        { i_length = change_endian(len); };

// toggle_bit 为非 0 的时候，当做 1 处理
public: inline void set_ctrl(uint8_t toggle_bit, bool is_multiple = false, bool is_first = false)
        {
          assert(!is_first || is_multiple);
          i_ctrl = 0;
          if (is_multiple)      { i_ctrl |= 0x80; }
          if (is_first)         { i_ctrl |= 0x40; }
          if (1 == toggle_bit)  { i_ctrl |= 0x20; }
        }

public: inline uint8_t get_toggle_bit() const
        { return (i_ctrl & 0x20) >> 5; }

DEFINE_MEMBER_ACCESSOR(byte_t, i_seq_nbr, seq, 0);

private:
  byte_t i_stp;
  byte_t i_identity;
  byte_t i_ctrl;    // 未用位域，防止不可移植
  byte_t i_seq_nbr;
  uint16_t i_length;
};
#pragma pack()

#pragma pack(1)
class PReadOffset_CT
{
public: inline void init( 
            uint16_t table_id
          , uint32_t offset
          , uint16_t count
        )
        {
          i_request_code = REQ_PREAD_OFFSET;
          i_tableid = change_endian(table_id);
          offset = change_endian(offset);
          uint8_t *p = reinterpret_cast<uint8_t *>(&offset) + 1;
          std::copy(p, p + i_offset.size(), i_offset.begin());
          i_octet_count = change_endian(count);
        };

private:
  byte_t i_request_code;
  uint16_t i_tableid;
  boost::array<uint8_t, 3> i_offset;
  uint16_t i_octet_count;
};
#pragma pack()

// 在缓冲区找出完整的报文
// 前置条件：输入迭代器指向区间的是连续的内存块
// range : 完整报文的起始、终止迭代器位置（返回 true 时）；
//         或不完整的报文的范围（返回 false 时）
// data_len : 报文中的“数据域”的字节数（返回 true 时）
template <typename ByteIt_T> 
FrameType_ET get_frame(
    ByteIt_T begin
  , ByteIt_T end
  , std::pair<ByteIt_T, ByteIt_T> &range
  , uint16_t &data_len
);
}}} // namespace ptcl_utility::ansi_c12::dot18

namespace ptcl_utility { namespace ansi_c12 { namespace dot19 {
DEFINE_ENUM(Endian_ET, 
  ((LITTLE) (0))
  ((BIG)    (1))
);

class Context_CT
{
public: Context_CT(Endian_ET data_order) : i_data_order(data_order) {};

DEFINE_MEMBER_ACCESSOR(Endian_ET, i_data_order, data_order, 0);
private:
  Endian_ET i_data_order;
};
static Context_CT const DEFAULT_CONTEXT(Endian_ET::BIG);

#pragma pack(1)
class STimeData02_CT
{
public: inline time_t get_time(int is_dst = -1) const;
private:
  uint8_t i_year;
  uint8_t i_month;
  uint8_t i_day;
  uint8_t i_hour;
  uint8_t i_minute;
};
#pragma pack()

#pragma pack(1)
class LpSetStatusRcd_CT
{
public: uint16_t get_nbr_valid_blocks(Context_CT const &context = DEFAULT_CONTEXT) const
        {
          return Endian_ET::LITTLE == context.get_data_order() ? 
            i_nbr_valid_blocks : change_endian(i_nbr_valid_blocks);
        };

public: uint16_t get_last_block_element(Context_CT const &context = DEFAULT_CONTEXT) const
        {
          return Endian_ET::LITTLE == context.get_data_order() ? 
            i_last_block_element : change_endian(i_last_block_element);
        };

public: uint16_t get_nbr_valid_int(Context_CT const &context = DEFAULT_CONTEXT) const
        {
          return Endian_ET::LITTLE == context.get_data_order() ? 
            i_nbr_valid_int : change_endian(i_nbr_valid_int);
        };

private:
  byte_t i_lp_set_status_flags;
  uint16_t i_nbr_valid_blocks;
  uint16_t i_last_block_element;
  uint32_t i_last_block_seq_nbr;
  uint16_t i_nbr_unread_blocks;
  uint16_t i_nbr_valid_int;
};
#pragma pack()
}}} // namespace ptcl_utility::ansi_c12::dot19



/** ------------------------------- INLINE ------------------------------- **/
namespace ptcl_utility { namespace ansi_c12 { namespace dot18 {
template <typename ByteIt_T> 
FrameType_ET get_frame(
    ByteIt_T begin
  , ByteIt_T end
  , std::pair<ByteIt_T, ByteIt_T> &range
  , uint16_t &data_len
)
{
  static_assert(sizeof(*begin) == 1, "The associated value type of iterator is not byte_t!");
  BOOST_CONCEPT_ASSERT((boost::RandomAccessIterator<ByteIt_T>));
  auto set_and_return = 
    [&](
        uint16_t len
      , ByteIt_T const &range_begin
      , ByteIt_T const &range_end
      , FrameType_ET ret_val
    ) -> FrameType_ET
    {
      data_len = len;
      range.first = range_begin;
      range.second = range_end;
      return ret_val;
    };

  ByteIt_T it = begin;
  while (it != end)
  {
    if (ACK == *it || NAK == *it)
      { return set_and_return(0, it, it + 1, FrameType_ET::FT_ACK); }
    else if (STP == *it) { break; }
    else { ++it; }
  }

  while ( std::distance(it, end) >= sizeof(FrameHead_CT) )
  {
    FrameHead_CT const *head = reinterpret_cast<FrameHead_CT const*>(&(*it));
    if (!head->is_valid()) { ++it; continue; }

    uint16_t d_len = head->get_len();
    int32_t len = sizeof(FrameHead_CT) + d_len + 2;

    // 判断是否包含了整个报文
    if (std::distance(it, end) < len) 
      { return set_and_return(0, it, it, FrameType_ET::FT_NONE); }

    // 验证“校验码”
    Crc_AT crc_func;
    auto crc_it = it + len - 2;
    crc_func = std::for_each(it, crc_it, crc_func);
    uint16_t const *crc = reinterpret_cast<uint16_t const*>(&*crc_it);
    if (*crc != crc_func()) { ++it; continue; }

    return set_and_return(d_len, it, it + len, FrameType_ET::FT_DATA); 
  }

  return set_and_return(0, it, it, FrameType_ET::FT_NONE);
}
}}} // namespace ptcl_utility::ansi_c12::dot18

namespace ptcl_utility { namespace ansi_c12 { namespace dot19 {
time_t STimeData02_CT::get_time(int is_dst) const
{
  tm t;
  t.tm_year = 2000 + i_year - 1900;
  t.tm_mon = i_month - 1;
  t.tm_mday = i_day;
  t.tm_hour = i_hour;
  t.tm_min = i_minute;
  t.tm_sec = 0;
  t.tm_isdst = is_dst;
  return mktime(&t);
}
}}} // namespace ptcl_utility::ansi_c12::dot19

#endif  // #ifndef LCLWH8V2L8S6M7N9_PUBLIC_PTCL_ANSI_C12
