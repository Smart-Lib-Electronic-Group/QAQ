#ifndef __DMA_HPP__
#define __DMA_HPP__

#include "dma_base.hpp"

/// @brief 名称空间 QAQ
namespace QAQ
{
/// @brief 名称空间 基本对象
namespace base
{
/// @brief 名称空间 DMA
namespace dma
{
/**
 * @brief  DMA 模板类 单缓存模式特化
 *
 * @tparam Config DMA配置模版类
 */
template <typename Config>
class Dma final : public base_internal::dma_internal::Dma_Base

{
  // 禁止拷贝和移动
  NO_COPY_MOVE(Dma)

public:
  /**
   * @brief  DMA 类 构造函数
   */
  explicit Dma() {}

  /**
   * @brief  DMA 类 打开通道
   *
   * @tparam type            DMA 中断类型
   * @param  request         DMA 请求通道编码
   * @param  function        DMA 回调函数
   * @param  arg             DMA 回调函数参数
   * @param  priority        DMA 中断优先级
   * @param  subpriority     DMA 中断子优先级
   * @return Dma_Error_Code  DMA 错误码
   */
  template <Interrupt_Type type>
  Dma_Error_Code open(uint32_t request, Dma_Callback_Func_t function, Dma_Callback_Args_t arg, uint32_t priority = 0, uint32_t subpriority = 0)
  {
    static_assert((type == Interrupt_Type::Direct || type == Interrupt_Type::Queue), "This type function only for Direct or Queue mode");

    Dma_Error_Code error_code = Dma_Error_Code::OK;

    if (State::CLOSEED != m_state)
    {
      error_code = Dma_Error_Code::ALREADY_OPENED;
    }
    else
    {
      if (false == distribution_channel())
      {
        error_code = Dma_Error_Code::NO_AVAILABLE_CHANNEL;
      }
      else
      {
        if (DMA1 == get_dma())
        {
          LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);
        }
        else if (DMA2 == get_dma())
        {
          LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA2);
        }

        LL_DMA_InitTypeDef dma_init     = { 0 };

        dma_init.PeriphRequest          = request;
        dma_init.Direction              = Config::get_direction();
        dma_init.Mode                   = Config::get_mode();
        dma_init.PeriphOrM2MSrcIncMode  = Config::get_periph_increment();
        dma_init.MemoryOrM2MDstIncMode  = Config::get_memory_increment();
        dma_init.PeriphOrM2MSrcDataSize = Config::get_periph_data_size();
        dma_init.MemoryOrM2MDstDataSize = Config::get_memory_data_size();
        dma_init.Priority               = Config::get_priority();
        dma_init.FIFOMode               = LL_DMA_FIFOMODE_DISABLE;

        if constexpr (Config::is_double_buffer())
        {
          dma_init.DoubleBufferMode = LL_DMA_DOUBLEBUFFER_MODE_ENABLE;
        }
        else
        {
          dma_init.DoubleBufferMode = LL_DMA_DOUBLEBUFFER_MODE_DISABLE;
        }

        if (SUCCESS != LL_DMA_Init(get_dma(), get_stream(), &dma_init))
        {
          error_code = Dma_Error_Code::ERROR;
        }
        else
        {
          if constexpr (Interrupt_Type::Direct == type)
          {
            register_interrupt(function, nullptr, arg, type, priority, subpriority);
          }
          else if constexpr (Interrupt_Type::Queue == type)
          {
            register_interrupt(nullptr, function, arg, type, priority, subpriority);
          }

          m_state = State::OPENED;
        }
      }
    }

    return error_code;
  }

  /**
   * @brief  DMA 类 打开通道
   *
   * @tparam type            DMA 中断类型
   * @param  request         DMA 请求通道编码
   * @param  direct_function DMA 直接回调函数
   * @param  queue_function  DMA 队列回调函数
   * @param  arg             DMA 回调函数参数
   * @param  priority        DMA 中断优先级
   * @param  subpriority     DMA 中断子优先级
   * @return Dma_Error_Code  DMA 错误码
   */
  template <Interrupt_Type type>
  Dma_Error_Code open(uint32_t request, Dma_Callback_Func_t direct_function, Dma_Callback_Func_t queue_function, Dma_Callback_Args_t arg, uint32_t priority = 0, uint32_t subpriority = 0)
  {
    static_assert((type == Interrupt_Type::Mixed || type == Interrupt_Type::Device), "This type function only for Mixed or Device mode");

    Dma_Error_Code error_code = Dma_Error_Code::OK;

    if (State::CLOSEED != m_state)
    {
      error_code = Dma_Error_Code::ALREADY_OPENED;
    }
    else
    {
      if ((false == distribution_channel()))
      {
        error_code = Dma_Error_Code::NO_AVAILABLE_CHANNEL;
      }
      else
      {
        if (DMA1 == get_dma())
        {
          LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);
        }
        else if (DMA2 == get_dma())
        {
          LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA2);
        }

        LL_DMA_InitTypeDef dma_init     = { 0 };

        dma_init.PeriphRequest          = request;
        dma_init.Direction              = Config::get_direction();
        dma_init.Mode                   = Config::get_mode();
        dma_init.PeriphOrM2MSrcIncMode  = Config::get_periph_increment();
        dma_init.MemoryOrM2MDstIncMode  = Config::get_memory_increment();
        dma_init.PeriphOrM2MSrcDataSize = Config::get_periph_data_size();
        dma_init.MemoryOrM2MDstDataSize = Config::get_memory_data_size();
        dma_init.Priority               = Config::get_priority();
        dma_init.FIFOMode               = LL_DMA_FIFOMODE_DISABLE;

        if constexpr (Config::is_double_buffer())
        {
          dma_init.DoubleBufferMode = LL_DMA_DOUBLEBUFFER_MODE_ENABLE;
        }
        else
        {
          dma_init.DoubleBufferMode = LL_DMA_DOUBLEBUFFER_MODE_DISABLE;
        }

        if (SUCCESS != LL_DMA_Init(get_dma(), get_stream(), &dma_init))
        {
          error_code = Dma_Error_Code::ERROR;
        }
        else
        {
          register_interrupt(direct_function, queue_function, arg, type, priority, subpriority);
        }

        m_state = State::OPENED;
      }
    }

    return error_code;
  }

  /**
   * @brief  DMA 类 启动传输
   *
   * @param  src_address      DMA 源地址
   * @param  dst_address      DMA 目的地址
   * @param  memory_size      DMA 数据长度
   * @return Dma_Error_Code   DMA 错误码
   */
  template <bool enable = !Config::is_double_buffer(), typename = std::enable_if_t<enable>>
  Dma_Error_Code start(uint32_t src_address, uint32_t dst_address, uint32_t memory_size)
  {
    Dma_Error_Code error_code = Dma_Error_Code::OK;

    if (State::CLOSEED == m_state)
    {
      error_code = Dma_Error_Code::CHANNEL_NOT_OPEN;
    }
    else
    {
      m_memory_size = memory_size;

      LL_DMA_DisableIT_TC(get_dma(), get_stream());
      LL_DMA_DisableStream(get_dma(), get_stream());

      while (LL_DMA_IsEnabledStream(get_dma(), get_stream()))
      {
      }

      clear_interrupt_flag();

      LL_DMA_ConfigAddresses(get_dma(), get_stream(), src_address, dst_address, Config::get_direction());
      LL_DMA_SetDataLength(get_dma(), get_stream(), memory_size);

      LL_DMA_EnableIT_TC(get_dma(), get_stream());
      LL_DMA_EnableStream(get_dma(), get_stream());

      m_state = State::WORKING;
    }

    return error_code;
  }

  /**
   * @brief  DMA 类 启动传输
   *
   * @param  src_address          DMA 数据源地址
   * @param  dst_memory_1_address DMA 目标内存1地址
   * @param  dst_memory_2_address DMA 目标内存2地址
   * @param  memory_size          DMA 内存大小
   * @return Dma_Error_Code       DMA 错误码
   */
  template <bool enable = Config::is_double_buffer(), typename = std::enable_if_t<enable>>
  Dma_Error_Code start(uint32_t src_address, uint32_t dst_memory_1_address, uint32_t dst_memory_2_address, uint32_t memory_size)
  {
    Dma_Error_Code error_code = Dma_Error_Code::OK;

    if (State::CLOSEED == m_state)
    {
      error_code = Dma_Error_Code::CHANNEL_NOT_OPEN;
    }
    else
    {
      m_memory_size = memory_size;

      LL_DMA_DisableIT_TC(get_dma(), get_stream());
      LL_DMA_DisableStream(get_dma(), get_stream());

      while (LL_DMA_IsEnabledStream(get_dma(), get_stream()))
      {
      }

      clear_interrupt_flag();

      LL_DMA_SetPeriphAddress(get_dma(), get_stream(), src_address);
      LL_DMA_SetMemoryAddress(get_dma(), get_stream(), dst_memory_1_address);
      LL_DMA_SetMemory1Address(get_dma(), get_stream(), dst_memory_2_address);
      LL_DMA_SetDataLength(get_dma(), get_stream(), memory_size);
      LL_DMA_SetCurrentTargetMem(get_dma(), get_stream(), LL_DMA_CURRENTTARGETMEM0);

      LL_DMA_EnableIT_TC(get_dma(), get_stream());
      LL_DMA_EnableStream(get_dma(), get_stream());

      m_state = State::WORKING;
    }

    return error_code;
  }

  /**
   * @brief  DMA 类 析构函数
   */
  virtual ~Dma() {}
};
} /* namespace dma */
} /* namespace base */
} /* namespace QAQ */

#endif /* __DMA_HPP__ */
