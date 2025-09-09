#ifndef __SPI_BASE_HPP__
#define __SPI_BASE_HPP__

#include "direct_device.hpp"
#include "signal.hpp"

/// @brief 名称空间 QAQ
namespace QAQ
{
/// @brief 名称空间 基本对象
namespace base
{
/// @brief 名称空间 SPI
namespace spi
{
/// @brief SPI 工作模式
enum class Spi_Type : uint8_t
{
  Normal,    /* 正常模式 */
  Interrupt, /* 中断模式 */
  DMA,       /* DMA 模式 */
};

class Config
{
public:
};



} /* namespace spi */
} /* namespace base */
} /* namespace QAQ */

#endif /* __SPI_BASE_HPP__ */
