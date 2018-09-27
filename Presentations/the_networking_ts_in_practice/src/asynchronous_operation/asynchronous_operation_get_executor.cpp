#include <chrono>
#include <cstddef>
#include <cstring>
#include <experimental/net>
#include <iostream>
#include <system_error>
#include <type_traits>
#include <utility>

namespace std::net {

using namespace experimental::net;

}

template<typename WaitableTimer, typename AsyncWriteStream, typename ConstBufferSequence, typename Handler>
struct write_op {
  WaitableTimer& timer_;
  AsyncWriteStream& sock_;
  ConstBufferSequence cb_;
  Handler h_;
  void operator()(std::error_code ec) {
    if (ec) h_(ec, 0);
    else async_write(sock_, std::move(cb_), std::move(h_));
  }
  using executor_type = std::net::associated_executor_t<Handler, decltype(std::declval<WaitableTimer>().get_executor())>;
  auto get_executor() const noexcept {
    return std::net::get_associated_executor(h_, timer_.get_executor());
  }
};

template<typename WaitableTimer, typename AsyncWriteStream, typename ConstBufferSequence, typename Handler>
void async_wait_then_write(WaitableTimer& timer, AsyncWriteStream& s,
  typename WaitableTimer::duration dur, ConstBufferSequence bufs, Handler&& h)
{
  timer.expires_after(dur);
  write_op<WaitableTimer, AsyncWriteStream, ConstBufferSequence, std::decay_t<Handler>> op{timer, s, std::move(bufs), std::forward<Handler>(h)};
  timer.async_wait(std::move(op));
}

struct heartbeat {
  std::net::system_timer& timer_;
  std::net::ip::tcp::socket& socket_;
  const std::byte* buffer_;
  std::size_t size_;
  void initiate() {
    async_wait_then_write(timer_, socket_, std::chrono::seconds(5), std::net::buffer(buffer_, size_), *this);
  }
  void operator()(std::error_code ec, std::size_t written) {
    if (ec) throw std::system_error(ec);
    initiate();
  }
};

struct process {
  std::net::ip::tcp::socket& socket_;
  std::byte* buffer_;
  std::size_t size_;
  void initiate() {
    socket_.async_read_some(std::net::buffer(buffer_, size_), *this);
  }
  void operator()(std::error_code ec, std::size_t bytes) {
    if (ec) throw std::system_error(ec);
    //  Process bytes received...
    initiate();
  }
};

int main(int argc,
         char** argv)
{
  const char str[] = "GET / HTTP/1.1\r\n\r\n";
  std::byte write_buffer[sizeof(str) - 1U];
  std::byte read_buffer[1024];
  std::memcpy(write_buffer,
              str,
              sizeof(write_buffer));
  std::net::io_context ctx;
  std::net::ip::tcp::socket socket(ctx);
  std::net::system_timer timer(ctx);
  socket.connect(std::net::ip::tcp::endpoint(std::net::ip::make_address_v4("172.217.7.142"),
                                             80));
  heartbeat{timer, socket, write_buffer, sizeof(write_buffer)}.initiate();
  process{socket, read_buffer, sizeof(read_buffer)}.initiate();
  // Only one thread available to run work
  // therefore thread safe
  ctx.run();
}
