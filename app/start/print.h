#ifndef __PRINT_H__
#define __PRINT_H__

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef SOURCE_CODE

  #define PRINT(str) __print(str)

#else
  #include "SEGGER_RTT.h"
#endif

  void __print(const char* str);

#ifdef __cplusplus
}
#endif

#endif /* __PRINT_H__ */
