#pragma once
#define NTFX_INLINE_TEMPLATE                                                           \
  template <typename signal_t>                                                         \
  __attribute__((always_inline)) static inline
#define NTFX_INLINE_MEMBER __attribute__((always_inline)) inline
#define NTFX_INLINE_STATIC __attribute__((always_inline)) inline static
#define SIGNAL(v) static_cast<signal_t>(v)
