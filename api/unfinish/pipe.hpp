#ifndef __PIPE_HPP__
#define __PIPE_HPP__

/*
template <uint32_t In_Buf_Size, uint32_t Out_Buf_Size, memory::Ring_Buffer_Mode In_Buf_Mode>
class Stream_Device<Stream_Type::STORAGE, In_Buf_Size, Out_Buf_Size, In_Buf_Mode> : public internal::device_internal::Storage_Device_Base
{
  static_assert(In_Buf_Size == 0, "Storage device does not support input buffer");
  static_assert(Out_Buf_Size == 0, "Storage device does not support output buffer");

public:
  static constexpr uint32_t input_buffer_size  = 0;
  static constexpr uint32_t output_buffer_size = 0;
};

template <uint32_t In_Buf_Size, uint32_t Out_Buf_Size, memory::Ring_Buffer_Mode In_Buf_Mode>
class Stream_Device<Stream_Type::DIRECTORY, In_Buf_Size, Out_Buf_Size, In_Buf_Mode> : public internal::device_internal::Direct_Device_Base
{
  static_assert(In_Buf_Size == 0, "Storage device does not support input buffer");
  static_assert(Out_Buf_Size == 0, "Storage device does not support output buffer");

public:
  static constexpr uint32_t input_buffer_size  = 0;
  static constexpr uint32_t output_buffer_size = 0;
};
*/

#endif /* __PIPE_HPP__ */
