#include <algorithm>
#include <cassert>
#include <cstddef>
#include <chrono>
#include <cstring>
#include <experimental/net>
#include <functional>
#include <iostream>
#include <string_view>
#include <system_error>
#include <type_traits>
#include <utility>

namespace std::net {

using namespace ::std::experimental::net;
using ::std::experimental::net::buffer;

const_buffer buffer(const ::std::string_view& sv) noexcept { return const_buffer(sv.data(), sv.size()); }

}

template<typename WaitableTimer, typename AsyncWriteStream, typename ConstBufferSequence, typename Handler>
void async_wait_then_write(WaitableTimer& timer, AsyncWriteStream& s,
  typename WaitableTimer::duration dur, ConstBufferSequence bufs, Handler&& h)
{
  timer.expires_after(dur);
  auto f = [&s, bufs = std::move(bufs), h = std::forward<Handler>(h)]
    (std::error_code ec) mutable
  {
    if (ec) h(ec, 0);
    else std::net::async_write(s, std::move(bufs), std::move(h));
  };
  timer.async_wait(std::move(f));
}

struct mock_async_write_stream {
  std::net::io_context::executor_type ex_;
  std::byte* ptr_;
  std::size_t size_;
  std::net::io_context::executor_type get_executor() noexcept { return ex_; }
  template<class ConstBufferSequence, class Handler>
  void async_write_some(ConstBufferSequence cb, Handler&& h);
};

template<class ConstBufferSequence, class Handler>
void mock_async_write_stream::async_write_some(ConstBufferSequence cb, Handler&& h) {
  std::error_code ec;
  std::size_t bytes = 0;
  if (std::net::buffer_size(cb)) {
    bytes = std::net::buffer_copy(std::net::buffer(ptr_, size_), std::move(cb));
    size_ -= bytes;
    ptr_ += bytes;
    if (!bytes) {
      ec = make_error_code(std::net::stream_errc::eof);
    }
  }
  std::net::post(std::net::bind_executor(ex_, std::bind(std::forward<Handler>(h), ec, bytes)));
}

class mock_waitable_timer {
private:
  using function_type = std::function<void(std::error_code)>;
public:
  using duration = std::chrono::milliseconds;
  explicit mock_waitable_timer(const std::net::io_context::executor_type& exec) noexcept : exec_(exec) {}
  template<typename Token>
  decltype(auto) async_wait(Token&& t) {
    using result_type = std::net::async_result<std::decay_t<Token>, void(std::error_code)>;
    using handler_type = typename result_type::completion_handler_type;
    handler_type h(std::forward<Token>(t));
    result_type r(h);
    expire();
    func_ = std::move(h);
    return r.get();
  }
  void expires_after(duration d) {
    expire();
    d_ = d;
  }
  duration expires_after() const noexcept {
    return d_;
  }
  void expire() {
    if (!func_) {
      return;
    }
    function_type func;
    using std::swap;
    swap(func, func_);
    post(bind_executor(exec_, std::bind(std::move(func), std::error_code())));
  }
private:
  std::net::io_context::executor_type exec_;
  function_type                       func_;
  duration                            d_;
};

int main(int argc,
         char** argv)
{
  std::byte out[1024];
  std::string_view in("GET / HTTP/1.1\r\n\r\n");
  std::net::io_context ctx;
  mock_waitable_timer timer{ctx.get_executor()};
  mock_async_write_stream stream{ctx.get_executor(), out, sizeof(out)};
  bool i = false;
  std::error_code ec;
  std::size_t bytes = 0;
  async_wait_then_write(timer, stream, std::chrono::milliseconds(1000), std::net::buffer(in), [&] (auto e, auto b) { i = true; ec = e; bytes = b; });
  assert(!i);
  std::size_t handlers = ctx.poll();
  assert(!handlers);
  timer.expire();
  ctx.restart();
  handlers = ctx.poll();
  assert(handlers);
  assert(i);
  assert(!ec);
  assert(bytes == in.size());
  std::size_t written(sizeof(out) - stream.size_);
  assert(written == in.size());
  assert(std::equal(reinterpret_cast<const char*>(out), reinterpret_cast<const char*>(out) + written, in.begin(), in.end()));
}
