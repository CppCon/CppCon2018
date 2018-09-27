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

template<typename ConstBufferSequence, typename Handler>
void async_wait_then_write(std::net::system_timer& timer, std::net::ip::tcp::socket& sock,
  std::net::system_timer::duration dur, ConstBufferSequence bufs, Handler&& h)
{
  timer.expires_after(dur);
  auto f = [&sock, bufs = std::move(bufs), h = std::forward<Handler>(h)]
    (std::error_code ec) mutable
  {
    if (ec) h(ec, 0);
    else std::net::async_write(sock, std::move(bufs), std::move(h));
  };
  timer.async_wait(std::move(f));
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

int main(int argc,
         char** argv)
{
  const char str[] = "GET / HTTP/1.1\r\n\r\n";
  std::byte buffer[sizeof(str) - 1U];
  std::memcpy(buffer,
              str,
              sizeof(buffer));
  std::net::io_context ctx;
  std::net::ip::tcp::socket socket(ctx);
  std::net::system_timer timer(ctx);
  socket.connect(std::net::ip::tcp::endpoint(std::net::ip::make_address_v4("172.217.7.142"),
                                             80));
  // ...
  heartbeat{timer, socket, buffer, sizeof(buffer)}.initiate();
  ctx.run();
}
