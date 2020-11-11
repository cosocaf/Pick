#ifndef PICKC_UTILS_OPTION_H_
#define PICKC_UTILS_OPTION_H_

#include <cassert>
#include <utility>

namespace pickc
{
  template<typename T>
  class Option
  {
    enum struct Tag
    {
      None,
      Some
    } tag;
    union
    {
      struct {} _none;
      T _some;
    };
  public:
    Option() : tag(Tag::None) {};
    Option(const Option& option) : tag(option.tag)
    {
      switch(tag) {
        case Tag::None:
          break;
        case Tag::Some:
          new (&_some) T(option._some);
          break;
        default:
          assert(false);
      }
    }
    Option(Option&& option) : tag(option.tag)
    {
      switch(tag) {
        case Tag::None:
          break;
        case Tag::Some:
          new (&_some) T(std::move(option._some));
          break;
        default:
          assert(false);
      }
      option.tag = Tag::None;
    }
    ~Option()
    {
      switch(tag) {
        case Tag::None:
          break;
        case Tag::Some:
          _some.~T();
          break;
        default:
          assert(false);
      }
      tag = Tag::None;
    }
    Option& operator=(const Option& option) &
    {
      if(this != &option) {
        switch(option.tag) {
          case Tag::None:
            this->~Option();
            break;
          case Tag::Some:
            *this = option._some;
            break;
          default:
            assert(false);
        }
      }
      return *this;
    }
    Option& operator=(Option&& option) & noexcept
    {
      assert(this != &option);
      switch(option.tag) {
        case Tag::None:
          this->~Option();
          break;
        case Tag::Some:
          *this = std::move(option._some);
          break;
        default:
          assert(false);
      }
      option.tag = Tag::None;
      return *this;
    }
    explicit Option(const T& some) : tag(Tag::Some), _some(some) {}
    explicit Option(T&& some) : tag(Tag::Some), _some(std::move(some)) {}
    Option& operator=(const T& some)
    {
      this->_some = some;
      return *this;
    }
    Option& operator=(T&& some)
    {
      this->_some = std::move(some);
      return *this;
    }

    operator bool() const noexcept
    {
      return tag == Tag::Some;
    }
    bool operator!() const noexcept
    {
      return tag == Tag::None;
    }
    const T& get() const
    {
      assert(tag == Tag::Some);
      return _some;
    }
    T& get()
    {
      assert(tag == Tag::Some);
      return _some;
    }
  };

  template<typename T>
  class some_t
  {
    T some;
  public:
    explicit some_t(T some) : some(some) {}
    template<typename Some>
    operator Option<Some>() const
    {
      return Option<Some>(some);
    }
  };
  template<typename T>
  some_t<T> some(T some)
  {
    return some_t<T>(some);
  }

  struct none_t
  {
    template<typename T>
    constexpr operator Option<T>() const
    {
      return Option<T>();
    }
  };
  constexpr none_t none{};
}

#endif // PICKC_UTILS_OPTION_H_