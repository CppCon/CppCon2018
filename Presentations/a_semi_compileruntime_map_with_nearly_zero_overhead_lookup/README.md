**A Semi Compile/Run-time Map with (Nearly) Zero Overhead Lookup** by **Fabian Renn-Giles**

Animated version of the slides can be found at https://goo.gl/igwVxD 
Project source code can also be found at https://github.com/hogliux/semimap

semi::static_map and semi::map
==============================

This container was the topic of a cppcon talk:

https://www.youtube.com/watch?v=qNAbGpV1ZkU

(slides: https://goo.gl/igwVxD )


associative map containers with compile-time lookup!
----------------------------------------------------

Normally, associative containers require some runtime overhead when looking up their values from a key. However, when the key is known at compile-time (for example, when the key is a literal) then this run-time lookup could technically be avoided. This is exactly what the goal of `semi::static_map` and `semi::map` is.

In fact, when using `semi::static_map` and looking up a value with C++ literal as a key, then the value lookup is nearly as efficient as looking up a global variable (on x86/arm it will reduce to only three machine instructions: a cmp, jne and a direct load). As long as you use C++ literals as your keys, the computational time of the lookup will stay constant and, for example, will not increase with the number of keys in your container!

```c++
#include <iostream>
#include <string>

#include "semimap.h"

#define ID(x) []() constexpr { return x; }

int main()
{
  semi::map<std::string, std::string> map;

  // Using string literals to access the container is super fast:
  // computational complexity remains constant regardless of the number of key, value pairs!
  map.get(ID("food"))  = "pizza";
  map.get(ID("drink")) = "soda";
  std::cout << map.get(ID("drink")) << std::endl;


  // Values can also be looked-up with run-time keys
  // which will then use std::unordered_map as a fallback.
  std::string key;

  std::cin >> key;
  std::cout << map.get(key) << std::endl; // for example: outputs "soda" if key is "drink"

  // there is also a static version of the map where lookeup is even faster
  struct Tag {};
  using Map = semi::static_map<std::string, std::string, Tag>;

  // in fact, it is (nearly) as fast as looking up any plain old global variable
  Map::get(ID("food")) = "pizza";
  Map::get(ID("drink")) = "beer";
  
  return 0;
}
```

The containers are very simple and only have the methods `get`, `erase`, `contains` and `clear`. `get` can also take any number of extra optoinal parameters which will be passed to your value's constructor if the value is not already in the container. As such, `get` is very similar to `std::map`'s `try_emplace`. For example:

```c++
#define ID(x) []() constexpr { return x; }

semi::map<std::string, std::string> m;

m.get(ID("food"), "pizza");      // a value with key food is not already in the map so construct it with the parameter "pizza"
m.get(ID("food"), "spaghetti");  // a value with key food is in the map. The second parameter will not be used
```

There are two variants of the map:

1) `semi::map` behaves very similar to a normal associative container. It's methods are non-static, so that the key, value pairs are not shared between several instances of `semi::map`s (as one would expect).
2) `semi::static_map` is completely static. It's even faster than `semi::map`. However, to achieve this speed, it requires that all the methods are static. This means that two `semi::static_map`s, with same key and value types, will share their contents. To avoid this, there is a third optional "tag" template parameter. Only `semi::static_map`s that also have the same tag template type will share their contents. It's useful to use a local `struct` as the tag type, like follows:

```c++
void foo()
{
  struct Tag {};
  using map = semi::static_map<std::string, int, Tag>;

  map::get(ID("age")) = 18;
}
```

As this ensures that even if the same `struct` name "Tag" is used in another block, the `semi::static_map`s will not share their contents.

Also note, that as a static type, the contents of the `semi::static_map` will only be deleted when the program is exited. If you need to delete your key/values sooner then use the `clear` method.

-Fabian

@hogliux
