#define SOURCE_CODE

#include "print.h"

void __print(const char* str)
{
  SEGGER_RTT_printf(0, str);
}