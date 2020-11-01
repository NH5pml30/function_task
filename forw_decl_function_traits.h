#ifndef FORW_DECL_FUNCTION_TRAITS_H_
#define FORW_DECL_FUNCTION_TRAITS_H_

template<typename T, bool is_small>
struct function_traits_impl;

constexpr static size_t INPLACE_BUFFER_SIZE = sizeof(void *);
constexpr static size_t INPLACE_BUFFER_ALIGNMENT = alignof(void *);

template<typename T>
constexpr static bool fits_small_storage =
  sizeof(T) < INPLACE_BUFFER_SIZE &&
  INPLACE_BUFFER_ALIGNMENT % alignof(T) == 0 &&
  std::is_nothrow_move_constructible_v<T>;

template<typename T>
using function_traits = function_traits_impl<T, fits_small_storage<T>>;

#endif /* FORW_DECL_FUNCTION_TRAITS_H_ */
