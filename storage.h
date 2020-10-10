#ifndef STORAGE_H_
#define STORAGE_H_

struct bad_function_call : std::exception
{
  const char * what() const noexcept override
  {
    return "bad function call";
  }
};

constexpr static size_t INPLACE_BUFFER_SIZE = sizeof(void *);
constexpr static size_t INPLACE_BUFFER_ALIGNMENT = alignof(void *);

using inplace_buffer =
  std::aligned_storage<INPLACE_BUFFER_SIZE, INPLACE_BUFFER_ALIGNMENT>::type;

template<typename T>
constexpr static bool fits_small_storage =
  sizeof(T) < INPLACE_BUFFER_SIZE &&
  INPLACE_BUFFER_ALIGNMENT % alignof(T) == 0 &&
  std::is_nothrow_move_constructible_v<T>;

template<typename R, typename... Args>
struct type_descriptor; // Move, Copy, Invoke, Destroy

template<typename R, typename... Args>
const type_descriptor<R, Args...> * empty_type_descriptor();

template<typename R, typename... Args>
struct storage
{
  storage() = default;
  storage(const storage &other)
  {
    other.desc->copy(&other, this);
  }

  storage(storage &&other) noexcept
  {
    other.desc->move(&other, this);
  }

  template<typename T>
  storage(T val) : desc(function_traits<T>::template get_type_descriptor<R, Args...>())
  {
    function_traits<T>::initialize_storage(*this, std::move(val));
  }

  storage & operator=(const storage &other)
  {
    if (this != &other)
      storage(other).swap(*this);
    return *this;
  }

  void swap(storage &other) noexcept
  {
    std::swap(desc, other.desc);
    std::swap(buf, other.buf);
  }

  storage & operator=(storage &&other) noexcept
  {
    if (this != &other)
    {
      desc->destroy(this);
      other.desc->move(&other, this);
    }
    return *this;
  }

  template<typename T>
  T & get_static() noexcept
  {
    return *reinterpret_cast<T *>(&buf);
  }

  template<typename T>
  const T & get_static() const noexcept
  {
    return *reinterpret_cast<const T *>(&buf);
  }

  template<typename T>
  void set_dynamic(void *value) noexcept
  {
    reinterpret_cast<void *&>(buf) = value;
  }

  template<typename T>
  T * get_dynamic() const noexcept
  {
    return reinterpret_cast<T * const &>(buf);
  }

  explicit operator bool() const noexcept
  {
    return desc != empty_type_descriptor<R, Args...>();
  }

  R invoke(Args... args) const
  {
    return desc->invoke(this, std::forward<Args>(args)...);
  }

  template<typename T>
  bool check_type() const noexcept
  {
    return desc == function_traits<T>::template get_type_descriptor<R, Args...>();
  }

  ~storage()
  {
    desc->destroy(this);
  }
private:
  template<typename T, bool>
  friend struct function_traits_impl;

  friend const type_descriptor<R, Args...> * empty_type_descriptor<R, Args...>();

  inplace_buffer buf;
  const type_descriptor<R, Args...> *desc = empty_type_descriptor<R, Args...>();
};

#endif /* STORAGE_H_ */
