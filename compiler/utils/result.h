#ifndef PICKC_UTILS_RESULT_H_
#define PICKC_UTILS_RESULT_H_

#include <cassert>
#include <utility>

namespace pickc
{
  template<typename OK, typename Error>
  class Result
  {
    enum struct Tag
    {
      None,
      OK,
      Error
    } tag;
    union
    {
      OK _ok;
      Error _error;
    };
  public:
    Result(const Result& result) : tag(result.tag)
    {
      switch(tag) {
        case Tag::None:
          break;
        case Tag::OK:
          new (&_ok) OK(result._ok);
          break;
        case Tag::Error:
          new (&_error) Error(result._error);
          break;
        default:
          assert(false);
      }
    }
    Result(Result&& result) : tag(result.tag)
    {
      switch(tag) {
        case Tag::None:
          break;
        case Tag::OK:
          new (&_ok) OK(std::move(result._ok));
          break;
        case Tag::Error:
          new (&_error) Error(std::move(result._error));
          break;
        default:
          assert(false);
      }
      result.tag = Tag::None;
    }
    ~Result()
    {
      switch(tag) {
        case Tag::None:
          break;
        case Tag::OK:
          _ok.~OK();
          break;
        case Tag::Error:
          _error.~Error();
          break;
        default:
          assert(false);
      }
      tag = Tag::None;
    }
    Result& operator=(const Result& result) &
    {
      if(this != &result) {
        switch(result.tag) {
          case Tag::None:
            this->~Result();
            break;
          case Tag::OK:
            *this = result._ok;
            break;
          case Tag::Error:
            *this = result._error;
            break;
          default:
            assert(false);
        }
      }
      return *this;
    }
    Result& operator=(Result&& result) & noexcept
    {
      assert(this != &result);
      switch (result.tag) {
        case Tag::None:
          this->~Result();
          break;
        case Tag::OK:
          *this = std::move(result._ok);
          break;
        case Tag::Error:
          *this = std::move(result._error);
          break;
        default:
          assert(false);
      }
      result.tag = Tag::None;
      return *this;
    }
    explicit Result(const OK& ok) : tag(Tag::OK), _ok(ok) {}
    explicit Result(OK&& ok) : tag(Tag::OK), _ok(std::move(ok)) {}
    Result& operator=(const OK& ok)
    {
      if(tag != Tag::OK) {
        this->~Result();
        new (this) Result(ok);
      }
      else {
        this->_ok = ok;
      }
      return *this;
    }
    Result& operator=(OK&& ok)
    {
      if(tag != Tag::OK) {
        this->~Result();
        new (this) Result(std::move(ok));
      }
      else {
        this->_ok = std::move(ok);
      }
      return *this;
    }
    explicit Result(const Error& error) : tag(Tag::Error), _error(error) {}
    explicit Result(Error&& error) : tag(Tag::Error), _error(std::move(error)) {}
    Result& operator=(const Error& error)
    {
      if(tag != Tag::Error) {
        this->~Result();
        new (this) Result(error);
      }
      else {
        this->_error = error;
      }
      return *this;
    }
    Result& operator=(Error&& error)
    {
      if(tag != Tag::Error) {
        this->~Result();
        new (this) Result(std::move(error));
      }
      else {
        this->_error = std::move(error);
      }
      return *this;
    }
    operator bool() const noexcept
    {
      assert(tag != Tag::None);
      return tag == Tag::OK;
    }
    bool operator!() const noexcept
    {
      assert(tag != Tag::None);
      return tag == Tag::Error;
    }
    const OK& get() const
    {
      assert(tag == Tag::OK);
      return _ok;
    }
    OK& get()
    {
      assert(tag == Tag::OK);
      return _ok;
    }
    const Error& err() const
    {
      assert(tag == Tag::Error);
      return _error;
    }
    Error& err()
    {
      assert(tag == Tag::Error);
      return _error;
    }
  };

  template<typename T>
  class ok_t
  {
    T ok;
  public:
    explicit ok_t(T ok) : ok(ok) {}
    template<typename OK, typename Error>
    operator Result<OK, Error>() const
    {
      return Result<OK, Error>(ok);
    }
  };
  template<typename OK>
  ok_t<OK> ok(OK ok)
  {
    return ok_t<OK>(ok);
  }

  template<typename T>
  class error_t
  {
    T error;
  public:
    explicit error_t(T error) : error(error) {}
    template<typename OK, typename Error>
    operator Result<OK, Error>() const
    {
      return Result<OK, Error>(error);
    }
  };
  template<typename Error>
  error_t<Error> error(Error error)
  {
    return error_t<Error>(error);
  }
}

#endif // PICKC_UTILS_RESULT_H_