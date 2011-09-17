/*
 * main.cpp                 -- lazy evaluating variables tests
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

#include <utility>

#include <lazy.hpp>
using NReinventedWheels::TLazy;

#define BOOST_TEST_MODULE LazyTest
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(single_object1)
{
    TLazy<int> one([](){ return 1.7f; });
    BOOST_REQUIRE_EQUAL(one, 1);
}

BOOST_AUTO_TEST_CASE(single_object2)
{
    bool flag = false;
    TLazy<float> setFlag([&flag](){ return (flag = true, 5.5f); });
    BOOST_REQUIRE_EQUAL(flag, false);
    BOOST_REQUIRE_EQUAL(setFlag, 5.5f);
    BOOST_REQUIRE_EQUAL(flag, true);
}

BOOST_AUTO_TEST_CASE(single_object3)
{
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

BOOST_AUTO_TEST_CASE(value_access1)
{
    bool flag = 0;
    TLazy<int> one([&flag](){ return (++flag, 1); });
    int& i = one;
    BOOST_REQUIRE_EQUAL(i, 1);
    BOOST_REQUIRE_EQUAL(flag, 1);
    i = 2;
    BOOST_REQUIRE_EQUAL(one, 2);
    BOOST_REQUIRE_EQUAL(flag, 1);
}

BOOST_AUTO_TEST_CASE(value_access2)
{
    const TLazy<double> pi([](){ return 3.14f; });
    BOOST_REQUIRE_EQUAL(pi, 3.14f);
}

BOOST_AUTO_TEST_CASE(constuctor1)
{
    int flag = 0;
    TLazy<int> one([&flag](){ return (++flag, 1); });
    TLazy<int> copy(one);
    BOOST_REQUIRE_EQUAL(flag, 0);
    BOOST_REQUIRE_EQUAL(copy, 1);
    BOOST_REQUIRE_EQUAL(flag, 1);
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
        Flag_ += 4;
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

BOOST_AUTO_TEST_CASE(constuctor2)
{
    int flag = 0;
    TLazy<TCounter> lazy(CreateCounter(flag));
    BOOST_REQUIRE_EQUAL(flag, 0);
    static_cast<void>(static_cast<TCounter&>(lazy));
    BOOST_REQUIRE_EQUAL(flag, 1);
}

struct TIntConstructible
{
    inline TIntConstructible(int)
    {
    }
};

BOOST_AUTO_TEST_CASE(constuctor3)
{
    int flag = 0;
    TLazy<TCounter> lazy(CreateUsedCounter(flag));
    BOOST_REQUIRE_EQUAL(flag, 1);
    TCounter counter(std::forward<TCounter>(lazy));
    BOOST_REQUIRE_EQUAL(flag, 5);
}

BOOST_AUTO_TEST_CASE(constuctor4)
{
    TLazy<TIntConstructible> lazy([](){ return int(); });
    static_cast<void>(static_cast<TIntConstructible&>(lazy));
}

BOOST_AUTO_TEST_CASE(constuctor5)
{
    int flag = 0;
    TLazy<TCounter> lazy([&flag]()->int&{ return flag;});
    BOOST_REQUIRE_EQUAL(flag, 0);
    TCounter counter(std::forward<TCounter>(lazy));
    BOOST_REQUIRE_EQUAL(flag, 5);
}

TLazy<int> MakeLazy(int value)
{
    return TLazy<int>([value](){ return value; });
}

BOOST_AUTO_TEST_CASE(constuctor6)
{
    TLazy<int> lazy(MakeLazy(1));
    TLazy<int> copy(std::move(lazy));
    BOOST_REQUIRE_EQUAL(copy, 1);
}

TLazy<int> MakeUsedLazy(int value)
{
    TLazy<int> lazy(MakeLazy(value));
    static_cast<void>(static_cast<int>(lazy));
    return std::move(lazy);
}

BOOST_AUTO_TEST_CASE(constuctor7)
{
    TLazy<int> lazy(MakeUsedLazy(1));
    TLazy<int> copy(std::move(lazy));
    BOOST_REQUIRE_EQUAL(copy, 1);
}

BOOST_AUTO_TEST_CASE(constuctor8)
{
    TLazy<int> lazy(MakeUsedLazy(1));
    TLazy<int> copy(lazy);
    BOOST_REQUIRE_EQUAL(copy, 1);
}

BOOST_AUTO_TEST_CASE(assignment1)
{
    bool flag = false;
    TLazy<int> lazy([&flag](){ return (flag = true, 1); });
    lazy = 5.5;
    BOOST_REQUIRE_EQUAL(lazy, 5);
    BOOST_REQUIRE_EQUAL(flag, false);
    lazy = 6.5;
    BOOST_REQUIRE_EQUAL(lazy, 6);
    BOOST_REQUIRE_EQUAL(flag, false);
}

BOOST_AUTO_TEST_CASE(assignment2)
{
    int firstFlag = 0, secondFlag = 0;
    TLazy<int> first([&firstFlag](){ return (++firstFlag, 1); });
    TLazy<int> second([&secondFlag](){ return (++secondFlag, 2); });
    second = first;
    BOOST_REQUIRE_EQUAL(firstFlag, 0);
    BOOST_REQUIRE_EQUAL(secondFlag, 0);
    BOOST_REQUIRE_EQUAL(second, 1);
    BOOST_REQUIRE_EQUAL(firstFlag, 1);
    BOOST_REQUIRE_EQUAL(secondFlag, 0);
}

BOOST_AUTO_TEST_CASE(assignment3)
{
    int flag = 0, dummyFlag = 0;
    TLazy<TCounter> lazy([&dummyFlag](){ return TCounter(dummyFlag); });
    lazy = CreateUsedCounter(flag);
    BOOST_REQUIRE_EQUAL(flag, 5);
    static_cast<void>(static_cast<TCounter&>(lazy));
    BOOST_REQUIRE_EQUAL(flag, 5);
    BOOST_REQUIRE_EQUAL(dummyFlag, 0);
    lazy = CreateUsedCounter(flag);
    BOOST_REQUIRE_EQUAL(flag, 10);
}

BOOST_AUTO_TEST_CASE(assignment4)
{
    int firstFlag = 0, secondFlag = 0;
    TLazy<int> first([&firstFlag](){ return (++firstFlag, 1); });
    TLazy<int> second([&secondFlag](){ return (secondFlag += 3, 2); });
    BOOST_REQUIRE_EQUAL(first, 1);
    second = first;
    BOOST_REQUIRE_EQUAL(firstFlag, 1);
    BOOST_REQUIRE_EQUAL(secondFlag, 0);
    BOOST_REQUIRE_EQUAL(second, 1);
    BOOST_REQUIRE_EQUAL(firstFlag, 1);
    BOOST_REQUIRE_EQUAL(secondFlag, 0);
}

BOOST_AUTO_TEST_CASE(assignment5)
{
    int firstFlag = 0, secondFlag = 0;
    TLazy<int> first([&firstFlag](){ return (++firstFlag, 1); });
    TLazy<int> second([&secondFlag](){ return (secondFlag += 3, 2); });
    BOOST_REQUIRE_EQUAL(second, 2);
    second = first;
    BOOST_REQUIRE_EQUAL(firstFlag, 0);
    BOOST_REQUIRE_EQUAL(secondFlag, 3);
    BOOST_REQUIRE_EQUAL(second, 1);
    BOOST_REQUIRE_EQUAL(firstFlag, 1);
    BOOST_REQUIRE_EQUAL(secondFlag, 3);
}

BOOST_AUTO_TEST_CASE(assignment6)
{
    int flag = 0;
    TLazy<int> lazy([&flag](){ return (++flag, 1); });
    lazy = MakeLazy(3);
    BOOST_REQUIRE_EQUAL(lazy, 3);
    BOOST_REQUIRE_EQUAL(flag, 0);
}

BOOST_AUTO_TEST_CASE(assignment7)
{
    int flag = 0;
    TLazy<int> lazy([&flag](){ return (++flag, 1); });
    lazy = MakeUsedLazy(3);
    BOOST_REQUIRE_EQUAL(lazy, 3);
    BOOST_REQUIRE_EQUAL(flag, 0);
}

BOOST_AUTO_TEST_CASE(assignment8)
{
    int flag = 0;
    TLazy<int> lazy([&flag](){ return (++flag, 1); });
    BOOST_REQUIRE_EQUAL(lazy, 1);
    BOOST_REQUIRE_EQUAL(flag, 1);
    lazy = MakeLazy(3);
    BOOST_REQUIRE_EQUAL(lazy, 3);
    BOOST_REQUIRE_EQUAL(flag, 1);
}

BOOST_AUTO_TEST_CASE(assignment9)
{
    int flag = 0;
    TLazy<int> lazy([&flag](){ return (++flag, 1); });
    BOOST_REQUIRE_EQUAL(lazy, 1);
    BOOST_REQUIRE_EQUAL(flag, 1);
    lazy = MakeUsedLazy(3);
    BOOST_REQUIRE_EQUAL(lazy, 3);
    BOOST_REQUIRE_EQUAL(flag, 1);
}

BOOST_AUTO_TEST_CASE(assignment10)
{
    int firstFlag = 0, secondFlag = 0;
    TLazy<int> first([&firstFlag](){ return (++firstFlag, 1); });
    BOOST_REQUIRE_EQUAL(first, 1);
    TLazy<int> second([&secondFlag](){ return (secondFlag += 3, 2); });
    BOOST_REQUIRE_EQUAL(second, 2);
    second = first;
    BOOST_REQUIRE_EQUAL(second, 1);
    BOOST_REQUIRE_EQUAL(firstFlag, 1);
    BOOST_REQUIRE_EQUAL(secondFlag, 3);
}

BOOST_AUTO_TEST_CASE(swap1)
{
    int firstFlag = 0;
    TLazy<int> first([&firstFlag](){ return (++firstFlag, 1); });
    int secondFlag = 0;
    TLazy<int> second([&secondFlag](){ return (secondFlag += 3, 2); });
    std::swap(first, second);
    BOOST_REQUIRE_EQUAL(firstFlag, 0);
    BOOST_REQUIRE_EQUAL(secondFlag, 0);
    BOOST_REQUIRE_EQUAL(first, 2);
    BOOST_REQUIRE_EQUAL(firstFlag, 0);
    BOOST_REQUIRE_EQUAL(secondFlag, 3);
    BOOST_REQUIRE_EQUAL(second, 1);
    BOOST_REQUIRE_EQUAL(firstFlag, 1);
    BOOST_REQUIRE_EQUAL(secondFlag, 3);
}

BOOST_AUTO_TEST_CASE(swap2)
{
    int firstFlag = 0;
    TLazy<int> first([&firstFlag](){ return (++firstFlag, 1); });
    BOOST_REQUIRE_EQUAL(first, 1);
    BOOST_REQUIRE_EQUAL(firstFlag, 1);
    int secondFlag = 0;
    TLazy<int> second([&secondFlag](){ return (secondFlag += 3, 2); });
    std::swap(first, second);
    BOOST_REQUIRE_EQUAL(second, 1);
    BOOST_REQUIRE_EQUAL(secondFlag, 0);
    BOOST_REQUIRE_EQUAL(first, 2);
    BOOST_REQUIRE_EQUAL(firstFlag, 1);
    BOOST_REQUIRE_EQUAL(secondFlag, 3);
}

BOOST_AUTO_TEST_CASE(swap3)
{
    int firstFlag = 0;
    TLazy<int> first([&firstFlag](){ return (++firstFlag, 1); });
    int secondFlag = 0;
    TLazy<int> second([&secondFlag](){ return (secondFlag += 3, 2); });
    BOOST_REQUIRE_EQUAL(second, 2);
    BOOST_REQUIRE_EQUAL(secondFlag, 3);
    std::swap(first, second);
    BOOST_REQUIRE_EQUAL(first, 2);
    BOOST_REQUIRE_EQUAL(firstFlag, 0);
    BOOST_REQUIRE_EQUAL(second, 1);
    BOOST_REQUIRE_EQUAL(firstFlag, 1);
    BOOST_REQUIRE_EQUAL(secondFlag, 3);
}

BOOST_AUTO_TEST_CASE(swap4)
{
    int firstFlag = 0;
    TLazy<int> first([&firstFlag](){ return (++firstFlag, 1); });
    BOOST_REQUIRE_EQUAL(first, 1);
    BOOST_REQUIRE_EQUAL(firstFlag, 1);
    int secondFlag = 0;
    TLazy<int> second([&secondFlag](){ return (secondFlag += 3, 2); });
    BOOST_REQUIRE_EQUAL(second, 2);
    BOOST_REQUIRE_EQUAL(secondFlag, 3);
    std::swap(first, second);
    BOOST_REQUIRE_EQUAL(first, 2);
    BOOST_REQUIRE_EQUAL(second, 1);
    BOOST_REQUIRE_EQUAL(firstFlag, 1);
    BOOST_REQUIRE_EQUAL(secondFlag, 3);
}

/* TODO: uncomment this once alingas will be implemented in compiler
BOOST_AUTO_TEST_CASE(refs)
{
    int firstFlag = 0;
    TLazy<int&> first([&firstFlag](){ return ++firstFlag; });
    BOOST_REQUIRE_EQUAL(value, 0);
    BOOST_REQUIRE_EQUAL(first, 1);
    BOOST_REQUIRE_EQUAL(++value, 2);
    BOOST_REQUIRE_EQUAL(++first, 3);
}*/

