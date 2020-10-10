#ifndef TYPE_DESCRIPTOR_H_
#define TYPE_DESCRIPTOR_H_

#include "storage.h"

template<typename R, typename... Args>
struct type_descriptor
{
  using storage_t = storage<R, Args...>;

  void (*copy)(const storage_t *src, storage_t *dest);
  void (*move)(storage_t *src, storage_t *dest);
  R (*invoke)(const storage_t *src, Args...);
  void (*destroy)(storage_t *);
};

template<typename R, typename... Args>
const type_descriptor<R, Args...> * empty_type_descriptor()
{
  using storage_t = storage<R, Args...>;

  constexpr static type_descriptor<R, Args...> impl =
  {
    // Copy
    [](const storage_t *src, storage_t *dest)
    {
      dest->desc = src->desc;
    },
    // Move
    [](storage_t *src, storage_t *dest)
    {
      dest->desc = src->desc;
    },
    // Invoke
    [](const storage_t *src, Args...) -> R
    {
      throw bad_function_call();
    },
    // Destroy
    [](storage_t *)
    {
    }
  };

  return &impl;
}

template<typename T, bool is_small>
struct function_traits_impl
{
  template<typename R, typename... Args>
  static void initialize_storage(storage<R, Args...> &src, T &&obj)
  {
    src.set_dynamic<T>(new T(std::move(obj)));
  }

  template<typename R, typename... Args>
  static T * get_target(const storage<R, Args...> *src)
  {
    return src->template get_dynamic<T>();
  }

  template<typename R, typename... Args>
  static void set_target(storage<R, Args...> *src, T *value)
  {
    return src->template set_dynamic<T>(value);
  }

  template<typename R, typename... Args>
  static const type_descriptor<R, Args...> * get_type_descriptor() noexcept
  {
    using storage_t = storage<R, Args...>;
    constexpr static type_descriptor<R, Args...> impl =
    {
      // Copy
      [](const storage_t *src, storage_t *dest)
      {
        set_target<R, Args...>(dest, new T(*get_target<R, Args...>(src)));
        dest->desc = src->desc;
      },
      // Move
      [](storage_t *src, storage_t *dest)
      {
        set_target<R, Args...>(dest, get_target<R, Args...>(src));
        set_target<R, Args...>(src, nullptr);
        dest->desc = src->desc;
        src->desc = empty_type_descriptor<R, Args...>();
      },
      // Invoke
      [](const storage_t *src, Args... args)
      {
        return (*get_target<R, Args...>(src))(std::forward<Args>(args)...);
      },
      // Destroy
      [](storage_t *src)
      {
        delete get_target<R, Args...>(src);
      }
    };

    return &impl;
  }
};

template<typename T>
struct function_traits_impl<T, true>
{
  template<typename R, typename... Args>
  static void initialize_storage(storage<R, Args...> &src, T &&obj)
  {
    new(&src.buf) T(std::move(obj));
  }

  template<typename R, typename... Args>
  static T * get_target(storage<R, Args...> *src)
  {
    return &src->template get_static<T>();
  }

  template<typename R, typename... Args>
  static const T * get_target(const storage<R, Args...> *src)
  {
    return &src->template get_static<T>();
  }

  template<typename R, typename... Args>
  static const type_descriptor<R, Args...> * get_type_descriptor() noexcept
  {
    using storage_t = storage<R, Args...>;
    constexpr static type_descriptor<R, Args...> impl =
    {
      // Copy
      [](const storage_t *src, storage_t *dest)
      {
        new(&dest->buf) T(*get_target<R, Args...>(src));
        dest->desc = src->desc;
      },
      // Move
      [](storage_t *src, storage_t *dest)
      {
        new(&dest->buf) T(std::move(*get_target<R, Args...>(src)));
        dest->desc = src->desc;
      },
      // Invoke
      [](const storage_t *src, Args... args) -> R
      {
        return (*get_target<R, Args...>(src))(std::forward<Args>(args)...);
      },
      // Destroy
      [](storage_t *src)
      {
        get_target<R, Args...>(src)->~T();
      }
    };

    return &impl;
  }
};

template<typename T>
using function_traits = function_traits_impl<T, fits_small_storage<T>>;

#endif /* TYPE_DESCRIPTOR_H_ */
