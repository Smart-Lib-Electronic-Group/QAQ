#ifndef __QSTRING_MEMORY_HPP__
#define __QSTRING_MEMORY_HPP__

#include "memory_pool.hpp"

namespace QAQ
{
namespace container
{
namespace container_internal
{
namespace qstring_internal
{
class QString_Memory_Pool final
{
private:
  system::memory::Struct_Memory_Pool<512, std::atomic<uint32_t>> m_counter_pool;
  system::memory::Block_Memory_Pool<256, 32>                     m_small_pool;
  system::memory::Block_Memory_Pool<128, 64>                     m_medium_pool;
  system::memory::Block_Memory_Pool<64, 128>                     m_large_pool;
  system::memory::Byte_Memory_Pool<8192>                         m_huge_pool;

protected:
  explicit QString_Memory_Pool() : m_counter_pool("QString Counter Memory Pool"), m_small_pool("QString Small Memory Pool"), m_medium_pool("QString Medium Memory Pool"), m_large_pool("QString Large Memory Pool"), m_huge_pool("QString Huge Memory Pool") {}
  ~QString_Memory_Pool() {}

public:
  static QString_Memory_Pool& instance()
  {
    static QString_Memory_Pool instance;
    return instance;
  }

  char* allocate(uint32_t size)
  {
    void* ptr = nullptr;

    if (size <= 32)
    {
      ptr = m_small_pool.allocate();
    }
    else if (size <= 64)
    {
      ptr = m_medium_pool.allocate();
    }
    else if (size <= 128)
    {
      ptr = m_large_pool.allocate();
    }
    else
    {
      ptr = m_huge_pool.allocate(size);
    }

    return static_cast<char*>(ptr);
  }

  std::atomic<uint32_t>* counter_malloc(void)
  {
    return m_counter_pool.allocate();
  }

  uint32_t get_capacity(uint32_t size) const
  {
    if (size <= 32)
    {
      return 32;
    }
    else if (size <= 64)
    {
      return 64;
    }
    else if (size <= 128)
    {
      return 128;
    }
    else
    {
      return size;
    }
  }

  void deallocate(char* ptr, uint32_t size)
  {
    if (nullptr != ptr)
    {
      if (size <= 32)
      {
        m_small_pool.deallocate(static_cast<void*>(ptr));
      }
      else if (size <= 64)
      {
        m_medium_pool.deallocate(static_cast<void*>(ptr));
      }
      else if (size <= 128)
      {
        m_large_pool.deallocate(static_cast<void*>(ptr));
      }
      else
      {
        m_huge_pool.deallocate(static_cast<void*>(ptr));
      }
    }
  }

  void counter_free(std::atomic<uint32_t>* ptr)
  {
    m_counter_pool.deallocate(ptr);
  }
};
} /* namespace qstring_internal */
} /* namespace container_internal */
} /* namespace container */
} /* namespace QAQ */

#endif /* __QSTRING_MEMORY_HPP__ */
