#ifndef FUNCTION_H_
#define FUNCTION_H_

#include <exception>

#include "storage.h"
#include "type_descriptor.h"

template<typename F>
struct function;

template<typename R, typename... Args>
struct function<R (Args...)>
{
  function() noexcept = default;

  function(function const& other) = default;
  function(function&& other) noexcept = default;

  template<typename T>
  function(T val);

  function & operator=(function const& rhs) = default;
  function & operator=(function&& rhs) noexcept = default;

  ~function() = default;

  explicit operator bool() const noexcept;

  R operator()(Args... args) const;

  template <typename T>
  T * target() noexcept;

  template<typename T>
  T const* target() const noexcept;

private:
  storage<R, Args...> stg;
};

template<typename R, typename... Args>
template<typename T>
function<R (Args...)>::function(T val) : stg(std::move(val))
{
}

template<typename R, typename... Args>
function<R (Args...)>::operator bool() const noexcept
{
  return stg.operator bool();
}

template<typename R, typename... Args>
R function<R (Args...)>::operator()(Args... args) const
{
  return stg.invoke(std::forward<Args>(args)...);
}

template<typename R, typename... Args>
template<typename T>
T * function<R (Args...)>::target() noexcept
{
  return stg.template check_type<T>() && operator bool() ? function_traits<T>::get_target(&stg) : nullptr;
}

template<typename R, typename... Args>
template<typename T>
const T * function<R (Args...)>::target() const noexcept
{
  return stg.template check_type<T>() && operator bool() ? function_traits<T>::get_target(&stg) : nullptr;
}

#endif /* FUNCTION_H_ */
