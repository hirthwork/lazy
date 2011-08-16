/*
 * main.cpp    -- lazy evaluating variables tests
 *
 * Copyright (C) 2011 Dmitry Potapov <potapov.d@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <lazy.hpp>
using NReinventedWheels::TLazy;

#define BOOST_TEST_MODULE LazyEvaluationTest
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(single_object)
{
    TLazy<int> one([](){ return 1.7f; });
    BOOST_REQUIRE_EQUAL(one, 1);

    bool flag = false;
    TLazy<float> setFlag([&flag](){ return (flag = true, 5.5f); });
    BOOST_REQUIRE_EQUAL(flag, false);
    BOOST_REQUIRE_EQUAL(setFlag, 5.5f);
    BOOST_REQUIRE_EQUAL(flag, true);

    bool castFlag = false;
    TLazy<int> cast([&castFlag](){ return (castFlag = true, 1); });
    BOOST_REQUIRE_EQUAL(castFlag, false);
    static_cast<void>(static_cast<int>(cast));
    BOOST_REQUIRE_EQUAL(castFlag, true);
}

BOOST_AUTO_TEST_CASE(recursive_calls)
{
    int firstFlag = 0, secondFlag = 0, thirdFlag = 0;
    TLazy<int> first([&firstFlag](){ return (++firstFlag, 5); });
    TLazy<int> second([&secondFlag, &first]()
        { return (++secondFlag, first + 1); });
    TLazy<double> third([&thirdFlag, &first, &second]()
        { return (++thirdFlag, first + second); });

    BOOST_REQUIRE_EQUAL(firstFlag, 0);
    BOOST_REQUIRE_EQUAL(secondFlag, 0);
    BOOST_REQUIRE_EQUAL(thirdFlag, 0);

    BOOST_REQUIRE_EQUAL(third, 11);
    BOOST_REQUIRE_EQUAL(second, 6);
    BOOST_REQUIRE_EQUAL(first, 5);

    BOOST_REQUIRE_EQUAL(firstFlag, 1);
    BOOST_REQUIRE_EQUAL(secondFlag, 1);
    BOOST_REQUIRE_EQUAL(thirdFlag, 1);
}

BOOST_AUTO_TEST_CASE(value_access)
{
    bool flag = 0;
    TLazy<int> one([&flag](){ return (++flag, 1); });
    int& i = one;
    BOOST_REQUIRE_EQUAL(i, 1);
    BOOST_REQUIRE_EQUAL(flag, 1);
    i = 2;
    BOOST_REQUIRE_EQUAL(one, 2);
    BOOST_REQUIRE_EQUAL(flag, 1);

    const TLazy<double> pi([](){ return 3.14f; });
    BOOST_REQUIRE_EQUAL(pi, 3.14f);
}

class TCounter
{
    int& Flag_;
    TCounter(const TCounter&) = delete;

public:
    inline TCounter(int& flag)
        : Flag_(flag)
    {
        ++Flag_;
    }

    inline TCounter(TCounter&& counter)
        : Flag_(counter.Flag_)
    {
        ++Flag_;
    }
};

TLazy<TCounter> CreateCounter(int& flag)
{
    return TLazy<TCounter>([&flag](){ return TCounter(flag); });
}

TLazy<TCounter> CreateUsedCounter(int& flag)
{
    TLazy<TCounter> counter(CreateCounter(flag));
    static_cast<void>(static_cast<TCounter&>(counter));
    return counter;
}

BOOST_AUTO_TEST_CASE(constuctors)
{
    int firstFlag = 0;
    TLazy<int> one([&firstFlag](){ return (++firstFlag, 1); });
    TLazy<int> copy(one);
    BOOST_REQUIRE_EQUAL(firstFlag, 0);
    BOOST_REQUIRE_EQUAL(copy, 1);
    BOOST_REQUIRE_EQUAL(firstFlag, 1);

    int secondFlag = 0;
    TLazy<TCounter> first(CreateCounter(secondFlag));
    BOOST_REQUIRE_EQUAL(secondFlag, 0);
    static_cast<void>(static_cast<TCounter&>(first));
    BOOST_REQUIRE_EQUAL(secondFlag, 1);

    int thirdFlag = 0;
    TLazy<TCounter> second(CreateUsedCounter(thirdFlag));
    BOOST_REQUIRE_EQUAL(thirdFlag, 1);
}

BOOST_AUTO_TEST_CASE(assignments)
{
    bool firstFlag = false;
    TLazy<int> first([&firstFlag](){ return (firstFlag = true, 1); });
    first = 5.5;
    BOOST_REQUIRE_EQUAL(first, 5);
    BOOST_REQUIRE_EQUAL(firstFlag, false);

    int secondFlag = 0, thirdFlag = 0;
    TLazy<int> second([&secondFlag](){ return (++secondFlag, 1); });
    TLazy<int> third([&thirdFlag](){ return (++thirdFlag, 2); });
    third = second;
    BOOST_REQUIRE_EQUAL(secondFlag, 0);
    BOOST_REQUIRE_EQUAL(thirdFlag, 0);
    BOOST_REQUIRE_EQUAL(third, 1);
    BOOST_REQUIRE_EQUAL(secondFlag, 1);
    BOOST_REQUIRE_EQUAL(thirdFlag, 0);

    int forthFlag = 0;
    int dummyFlag = 0;
    TLazy<TCounter> forth([&dummyFlag](){ return TCounter(dummyFlag); });
    forth = CreateUsedCounter(forthFlag);
    static_cast<void>(static_cast<TCounter&>(forth));
    BOOST_REQUIRE_EQUAL(forthFlag, 1);
    BOOST_REQUIRE_EQUAL(dummyFlag, 0);
}

struct TPod
{
    int A_;
    double B_;
    char C_[16];
};

struct TCopyable
{
    TCopyable& operator = (const TCopyable&)
    {
        return *this;
    }
};

BOOST_AUTO_TEST_CASE(storage)
{
    TLazy<int> first([](){ return 1; });
    BOOST_REQUIRE_LE(&first, static_cast<void*>(&static_cast<int&>(first)));
    BOOST_REQUIRE_GT(&first + 1,
        static_cast<void*>(&static_cast<int&>(first)));

    TLazy<TPod> second([](){ return TPod(); });
    BOOST_REQUIRE_LE(&second,
        static_cast<void*>(&static_cast<TPod&>(second)));
    BOOST_REQUIRE_GT(&second + 1,
        static_cast<void*>(&static_cast<TPod&>(second)));

    TLazy<TCopyable> third([](){ return TCopyable(); });
    BOOST_REQUIRE_LE(&third,
        static_cast<void*>(&static_cast<TCopyable&>(third)));
    BOOST_REQUIRE_GT(&third + 1,
        static_cast<void*>(&static_cast<TCopyable&>(third)));

    int flag = 0;
    TLazy<TCounter> forth([&flag](){ return TCounter(flag); });
    BOOST_REQUIRE(static_cast<void*>(&static_cast<TCounter&>(forth)) < &forth
        || static_cast<void*>(&static_cast<TCounter&>(forth)) > &forth + 1);
}

