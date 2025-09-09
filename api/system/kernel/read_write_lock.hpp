#ifndef __READ_WRITE_LOCK_HPP__
#define __READ_WRITE_LOCK_HPP__

#include "mutex.hpp"
#include "semaphore.hpp"

/// @brief 名称空间 QAQ
namespace QAQ
{
/// @brief 名称空间 系统
namespace system
{
/// @brief 名称空间 内核
namespace kernel
{
/**
 * @brief 读写锁类（写者优先）
 *
 * 该读写锁确保：
 *   1. 多个读者可以同时读取
 *   2. 写者独占访问
 *   3. 写者优先（新读者在有写者等待时会被阻塞）
 */
class Read_Write_Lock final
{
  // 禁止拷贝和移动
  NO_COPY_MOVE(Read_Write_Lock)

private:
  /// @brief 读写锁 资源访问信号量
  Semaphore   m_resource_sem;
  /// @brief 读写锁 读尝试信号量
  Semaphore   m_read_try_sem;
  /// @brief 读写锁 读者锁
  Mutex<true> m_read_count_mtx;
  /// @brief 读写锁 写者锁
  Mutex<true> m_write_count_mtx;
  /// @brief 读写锁 读者数量
  uint32_t    m_read_count  = 0;
  /// @brief 读写锁 写者数量
  uint32_t    m_write_count = 0;

public:
  /// @brief 读写锁 状态
  enum class Status
  {
    SUCCESS = 0, /* 操作成功 */
    TIMEOUT,     /* 操作超时 */
    ERROR        /* 操作失败 */
  };

  /**
   * @brief 读写锁 构造函数
   *
   * @param name 读写锁名称
   */
  explicit Read_Write_Lock(const char* name = "Read_Write_Lock") : m_resource_sem(1, (std::string(name) + "_Resource").c_str()), m_read_try_sem(1, (std::string(name) + "_ReadTry").c_str()) {}

  /**
   * @brief 读写锁 获取读锁
   *
   * @param timeout 超时时间（毫秒）
   * @return Status 操作状态
   */
  Status lock_read(uint32_t timeout = TX_WAIT_FOREVER) noexcept
  {
    // 尝试获取读权限
    if (auto status = m_read_try_sem.acquire(timeout); status != Semaphore::Status::SUCCESS)
    {
      return (status == Semaphore::Status::TIMEOUT) ? Status::TIMEOUT : Status::ERROR;
    }

    {
      // 保护读者计数
      Mutex_Guard<Mutex<true>> lock(m_read_count_mtx);
      if (++m_read_count == 1)
      {
        // 第一个读者需要获取资源锁
        if (auto status = m_resource_sem.acquire(timeout); status != Semaphore::Status::SUCCESS)
        {
          m_read_count--;
          m_read_try_sem.release();
          return (status == Semaphore::Status::TIMEOUT) ? Status::TIMEOUT : Status::ERROR;
        }
      }
    }

    m_read_try_sem.release();
    return Status::SUCCESS;
  }

  /**
   * @brief 读写锁 释放读锁
   */
  void unlock_read() noexcept
  {
    Mutex_Guard<Mutex<true>> lock(m_read_count_mtx);
    if (--m_read_count == 0)
    {
      // 最后一个读者释放资源锁
      m_resource_sem.release();
    }
  }

  /**
   * @brief 读写锁 获取写锁
   *
   * @param timeout 超时时间（毫秒）
   * @return Status 操作状态
   */
  Status lock_write(uint32_t timeout = TX_WAIT_FOREVER) noexcept
  {
    // 更新写者计数
    {
      Mutex_Guard<Mutex<true>> lock(m_write_count_mtx);
      if (++m_write_count == 1)
      {
        // 第一个写者阻塞新读者
        if (auto status = m_read_try_sem.acquire(timeout); status != Semaphore::Status::SUCCESS)
        {
          m_write_count--;
          return (status == Semaphore::Status::TIMEOUT) ? Status::TIMEOUT : Status::ERROR;
        }
      }
    }

    // 获取资源独占访问
    if (auto status = m_resource_sem.acquire(timeout); status != Semaphore::Status::SUCCESS)
    {
      Mutex_Guard<Mutex<true>> lock(m_write_count_mtx);
      if (--m_write_count == 0)
      {
        m_read_try_sem.release();
      }
      return (status == Semaphore::Status::TIMEOUT) ? Status::TIMEOUT : Status::ERROR;
    }

    return Status::SUCCESS;
  }

  /**
   * @brief 读写锁 释放写锁
   */
  void unlock_write() noexcept
  {
    m_resource_sem.release();

    Mutex_Guard<Mutex<true>> lock(m_write_count_mtx);
    if (--m_write_count == 0)
    {
      // 最后一个写者允许新读者
      m_read_try_sem.release();
    }
  }

  /**
   * @brief 读写锁 析构函数
   */
  ~Read_Write_Lock()
  {
    // 确保所有读者和写者都释放了锁
    while (m_read_count > 0)
    {
      unlock_read();
    }
    while (m_write_count > 0)
    {
      unlock_write();
    }
  }
};

/**
 * @brief 读锁保护器（RAII）
 */
class Read_Guard
{
  // 禁止拷贝和移动
  NO_COPY_MOVE(Read_Guard)

private:
  /// 读写锁
  Read_Write_Lock& m_lock;

public:
  explicit Read_Guard(Read_Write_Lock& lock, uint32_t timeout = TX_WAIT_FOREVER) : m_lock(lock)
  {
    m_lock.lock_read(timeout);
  }

  ~Read_Guard()
  {
    m_lock.unlock_read();
  }
};

/**
 * @brief 写锁保护器（RAII）
 */
class Write_Guard
{
  // 禁止拷贝和移动
  NO_COPY_MOVE(Write_Guard)

private:
  /// 读写锁
  Read_Write_Lock& m_lock;

public:
  explicit Write_Guard(Read_Write_Lock& lock, uint32_t timeout = TX_WAIT_FOREVER) : m_lock(lock)
  {
    m_lock.lock_write(timeout);
  }

  ~Write_Guard()
  {
    m_lock.unlock_write();
  }
};
} /* namespace kernel */
} /* namespace system */
} /* namespace QAQ */

#endif /* __READ_WRITE_LOCK_HPP__ */
