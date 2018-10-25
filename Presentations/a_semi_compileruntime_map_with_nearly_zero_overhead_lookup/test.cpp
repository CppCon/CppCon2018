#include <cassert>
#include <iostream>
#include <string>

#include "semimap.h"

#define ID(x) \
    []() constexpr { return x; }

//==============================================================================
void test_static_map()
{
    // Test compile-time only load/store
    {
        struct Tag {
        };
        using map = semi::static_map<std::string, std::string, Tag>;

        auto& food = map::get(ID("food"));
        assert(food.empty());

        food = "pizza";
        assert(map::get(ID("food")) == "pizza");

        auto& drink = map::get(ID("drink"));
        assert(drink.empty());

        drink = "beer";
        assert(map::get(ID("food")) == "pizza");
        assert(map::get(ID("drink")) == "beer");

        map::get(ID("food")) = "spaghetti";
        assert(map::get(ID("food")) == "spaghetti");
        assert(map::get(ID("drink")) == "beer");

        map::get(ID("drink")) = "soda";
        assert(map::get(ID("food")) == "spaghetti");
        assert(map::get(ID("drink")) == "soda");

        assert(map::get(ID("starter"), "soup") == "soup");
        assert(map::get(ID("starter"), "salad") == "soup");
    }

    // Test run-time only load/store
    {
        struct Tag {
        };
        using map = semi::static_map<std::string, std::string, Tag>;

        auto& food = map::get("food");
        assert(food.empty());

        food = "pizza";
        assert(map::get("food") == "pizza");

        auto& drink = map::get("drink");
        assert(drink.empty());

        drink = "beer";
        assert(map::get("food") == "pizza");
        assert(map::get("drink") == "beer");

        map::get("food") = "spaghetti";
        assert(map::get("food") == "spaghetti");
        assert(map::get("drink") == "beer");

        map::get("drink") = "soda";
        assert(map::get("food") == "spaghetti");
        assert(map::get("drink") == "soda");

        assert(map::get("starter", "soup") == "soup");
        assert(map::get("starter", "salad") == "soup");
    }

    // Test mixed compile-time/run-time load/store
    {
        struct Tag {
        };
        using map = semi::static_map<std::string, std::string, Tag>;

        // compile-time first, then run-time
        map::get(ID("food")) = "pizza";
        assert(map::get("food") == "pizza");

        // runtime-time first, then compile-time
        map::get("drink") = "beer";
        assert(map::get(ID("drink")) == "beer");

        assert(map::get(ID("food")) == "pizza");
        assert(map::get("drink") == "beer");

        assert(map::get(ID("starter"), "soup") == "soup");
        assert(map::get("starter", "salad") == "soup");

        assert(map::get("side", "rice") == "rice");
        assert(map::get(ID("side"), "peas") == "rice");
    }

    // test clear & contains
    {
        struct Tag {
        };
        using map = semi::static_map<std::string, std::string, Tag>;

        assert(!map::contains(ID("food")));
        assert(!map::contains("food"));

        // check again to see if contains by mistake add the element
        assert(!map::contains(ID("food")));
        assert(!map::contains("food"));

        map::get(ID("food")) = "pizza";
        assert(map::contains(ID("food")));
        assert(map::contains("food"));

        map::get("drink") = "beer";
        assert(map::contains("drink"));
        assert(map::contains(ID("drink")));

        map::get(ID("dessert")) = "icecream";
        assert(map::contains("dessert"));
        assert(map::contains(ID("dessert")));

        map::get("starter") = "salad";
        assert(map::contains(ID("starter")));
        assert(map::contains("starter"));

        map::clear();

        assert(!map::contains(ID("food")));
        assert(!map::contains("food"));
        assert(!map::contains("drink"));
        assert(!map::contains(ID("drink")));
    }

    // test erase
    {
        struct Tag {
        };
        using map = semi::static_map<std::string, std::string, Tag>;

        map::get(ID("food")) = "pizza";
        map::get(ID("drink")) = "beer";
        map::get(ID("dessert")) = "icecream";
        map::get(ID("starter")) = "soup";
        map::get(ID("side")) = "salad";

        // erase first
        map::erase(ID("food"));
        assert((!map::contains(ID("food"))) && map::contains(ID("drink")) && map::contains(ID("dessert")) && map::contains(ID("starter")) && map::contains(ID("side")));

        // erase last
        map::erase("side");
        assert((!map::contains(ID("food"))) && map::contains(ID("drink")) && map::contains(ID("dessert")) && map::contains(ID("starter")) && (!map::contains(ID("side"))));

        // try adding something
        map::get("bill") = "too much";
        assert((!map::contains(ID("food"))) && map::contains(ID("drink")) && map::contains(ID("dessert")) && map::contains(ID("starter")) && (!map::contains(ID("side"))) && map::contains(ID("bill")));

        // erase middle
        map::erase(ID("dessert"));
        assert((!map::contains(ID("food"))) && map::contains(ID("drink")) && (!map::contains(ID("dessert"))) && map::contains(ID("starter")) && (!map::contains(ID("side"))) && map::contains(ID("bill")));
    }

    // test independent maps do not influence each other
    {
        struct TagA {
        };
        struct TagB {
        };
        using mapA = semi::static_map<std::string, std::string, TagA>;
        using mapB = semi::static_map<std::string, std::string, TagB>;

        mapA::get(ID("food")) = "pizza";
        assert(mapA::get("food") == "pizza");
        assert(!mapB::contains("food"));

        mapB::get(ID("food")) = "spaghetti";
        assert(mapA::get("food") == "pizza");
        assert(mapB::get("food") == "spaghetti");

        mapB::get("drink") = "beer";
        assert(mapB::get(ID("drink")) == "beer");
        assert(!mapA::contains(ID("drink")));
        assert(mapA::contains("food"));

        mapA::get("drink") = "soda";
        assert(mapA::get(ID("drink")) == "soda");
        assert(mapB::get(ID("drink")) == "beer");

        mapA::get(ID("starter")) = "salad";
        mapB::get("starter") = "soup";

        mapB::erase("drink");
        assert(mapA::contains("drink"));
        assert(mapA::contains(ID("drink")));
        assert(!mapB::contains("drink"));
        assert(!mapB::contains(ID("drink")));

        mapB::clear();
        assert(mapA::get(ID("starter")) == "salad");
        assert(mapA::get("food") == "pizza");
        assert(mapA::get(ID("drink")) == "soda");
        assert(!mapB::contains("food"));
        assert(!mapB::contains(ID("drink")));
    }
}

void test_map()
{
    // Test compile-time only load/store
    {
        semi::map<std::string, std::string> map;

        auto& food = map.get(ID("food"));
        assert(food.empty());

        food = "pizza";
        assert(map.get(ID("food")) == "pizza");

        auto& drink = map.get(ID("drink"));
        assert(drink.empty());

        drink = "beer";
        assert(map.get(ID("food")) == "pizza");
        assert(map.get(ID("drink")) == "beer");

        map.get(ID("food")) = "spaghetti";
        assert(map.get(ID("food")) == "spaghetti");
        assert(map.get(ID("drink")) == "beer");

        map.get(ID("drink")) = "soda";
        assert(map.get(ID("food")) == "spaghetti");
        assert(map.get(ID("drink")) == "soda");

        assert(map.get(ID("starter"), "soup") == "soup");
        assert(map.get(ID("starter"), "salad") == "soup");
    }

    // Test run-time only load/store
    {
        semi::map<std::string, std::string> map;

        auto& food = map.get("food");
        assert(food.empty());

        food = "pizza";
        assert(map.get("food") == "pizza");

        auto& drink = map.get("drink");
        assert(drink.empty());

        drink = "beer";
        assert(map.get("food") == "pizza");
        assert(map.get("drink") == "beer");

        map.get("food") = "spaghetti";
        assert(map.get("food") == "spaghetti");
        assert(map.get("drink") == "beer");

        map.get("drink") = "soda";
        assert(map.get("food") == "spaghetti");
        assert(map.get("drink") == "soda");

        assert(map.get("starter", "soup") == "soup");
        assert(map.get("starter", "salad") == "soup");
    }

    // Test mixed compile-time/run-time load/store
    {
        semi::map<std::string, std::string> map;

        // compile-time first, then run-time
        map.get(ID("food")) = "pizza";
        assert(map.get("food") == "pizza");

        // runtime-time first, then compile-time
        map.get("drink") = "beer";
        assert(map.get(ID("drink")) == "beer");

        assert(map.get(ID("food")) == "pizza");
        assert(map.get("drink") == "beer");

        assert(map.get(ID("starter"), "soup") == "soup");
        assert(map.get("starter", "salad") == "soup");

        assert(map.get("side", "rice") == "rice");
        assert(map.get(ID("side"), "peas") == "rice");
    }

    // test clear & contains
    {
        semi::map<std::string, std::string> map;

        assert(!map.contains(ID("food")));
        assert(!map.contains("food"));

        // check again to see if contains by mistake add the element
        assert(!map.contains(ID("food")));
        assert(!map.contains("food"));

        map.get(ID("food")) = "pizza";
        assert(map.contains(ID("food")));
        assert(map.contains("food"));

        map.get("drink") = "beer";
        assert(map.contains("drink"));
        assert(map.contains(ID("drink")));

        map.get(ID("dessert")) = "icecream";
        assert(map.contains("dessert"));
        assert(map.contains(ID("dessert")));

        map.get("starter") = "salad";
        assert(map.contains(ID("starter")));
        assert(map.contains("starter"));

        map.clear();

        assert(!map.contains(ID("food")));
        assert(!map.contains("food"));
        assert(!map.contains("drink"));
        assert(!map.contains(ID("drink")));
    }

    // test erase
    {
        semi::map<std::string, std::string> map;

        map.get(ID("food")) = "pizza";
        map.get(ID("drink")) = "beer";
        map.get(ID("dessert")) = "icecream";
        map.get(ID("starter")) = "soup";
        map.get(ID("side")) = "salad";

        // erase first
        map.erase(ID("food"));
        assert((!map.contains(ID("food"))) && map.contains(ID("drink")) && map.contains(ID("dessert")) && map.contains(ID("starter")) && map.contains(ID("side")));

        // erase last
        map.erase("side");
        assert((!map.contains(ID("food"))) && map.contains(ID("drink")) && map.contains(ID("dessert")) && map.contains(ID("starter")) && (!map.contains(ID("side"))));

        // try adding something
        map.get("bill") = "too much";
        assert((!map.contains(ID("food"))) && map.contains(ID("drink")) && map.contains(ID("dessert")) && map.contains(ID("starter")) && (!map.contains(ID("side"))) && map.contains(ID("bill")));

        // erase middle
        map.erase(ID("dessert"));
        assert((!map.contains(ID("food"))) && map.contains(ID("drink")) && (!map.contains(ID("dessert"))) && map.contains(ID("starter")) && (!map.contains(ID("side"))) && map.contains(ID("bill")));
    }

    // test independent maps do not influence each other
    {
        semi::map<std::string, std::string> mapA;
        semi::map<std::string, std::string> mapB;

        mapA.get(ID("food")) = "pizza";
        assert(mapA.get("food") == "pizza");
        assert(!mapB.contains("food"));

        mapB.get(ID("food")) = "spaghetti";
        assert(mapA.get("food") == "pizza");
        assert(mapB.get("food") == "spaghetti");

        mapB.get("drink") = "beer";
        assert(mapB.get(ID("drink")) == "beer");
        assert(!mapA.contains(ID("drink")));
        assert(mapA.contains("food"));

        mapA.get("drink") = "soda";
        assert(mapA.get(ID("drink")) == "soda");
        assert(mapB.get(ID("drink")) == "beer");

        mapA.get(ID("starter")) = "salad";
        mapB.get("starter") = "soup";

        mapB.erase("drink");
        assert(mapA.contains("drink"));
        assert(mapA.contains(ID("drink")));
        assert(!mapB.contains("drink"));
        assert(!mapB.contains(ID("drink")));

        mapB.clear();
        assert(mapA.get(ID("starter")) == "salad");
        assert(mapA.get("food") == "pizza");
        assert(mapA.get(ID("drink")) == "soda");
        assert(!mapB.contains("food"));
        assert(!mapB.contains(ID("drink")));
    }
}

int main()
{
    test_static_map();
    test_map();

    return 0;
}
