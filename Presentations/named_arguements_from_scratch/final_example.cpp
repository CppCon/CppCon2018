#include <string>
#include <boost/hana.hpp>

namespace hana = boost::hana;
using namespace hana::literals;

static int foo(int a, float b, std::string const& c) {
    return (c.size() + a) / b;
}

template<char... Chars>
struct named_param {
    using name = hana::string<Chars...>;
    constexpr auto get_name() const { return name{}; }

    template<typename T>
    constexpr auto operator=(T&& value) {
        return hana::make_pair(get_name(), std::forward<T>(value));
    }
};

template<typename CharT, CharT... Chars>
constexpr auto operator"" _arg() -> named_param<Chars...> {
    return {};
}

constexpr auto get_names = [](auto arg_spec) {
    return hana::transform(arg_spec, [](auto key) {
        return key.get_name();        
    });
};

constexpr auto extract = [](auto arg_spec, auto args) {
    return hana::transform(arg_spec, [args](auto key) {
        return args[key];        
    });
};

constexpr auto adapt = [](auto& f, auto... input_arg_spec) {
    auto arg_spec = hana::make_tuple(input_arg_spec...);
    auto arg_names = get_names(arg_spec);
    return [&f, arg_names](auto... input_args) {
        auto args = hana::make_map(input_args...);
        auto arg_pack = extract(arg_names, args);
        return hana::unpack(arg_pack, f);
    };
};


// Want to write
// foo(c = "hello world", b = 0.5, a = 10);
int main() {
    auto my_foo = adapt(foo,
        "a"_arg,
        "b"_arg,
        "c"_arg
    );

    return my_foo(
        "c"_arg = "hello world",
        "b"_arg = 0.5,
        "a"_arg = 10
    );
}
