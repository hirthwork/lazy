/*
 * lazy.hpp    -- lazy evaluations variables wrapper
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
    (void)(int)cast;
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

    const TLazy<double> pi([](){ return 3.14f; });
    BOOST_REQUIRE_EQUAL(pi, 3.14f);
}

class TCounter
{
    int& Flag;
    TCounter(const TCounter&) = delete;

public:
    inline TCounter(int& flag)
        : Flag(flag)
    {
        ++Flag;
    }

    inline TCounter(TCounter&& counter)
        : Flag(counter.Flag)
    {
        ++Flag;
    }
};

TLazy<TCounter> CreateCounter(int& flag)
{
    return TLazy<TCounter>([&flag](){ return TCounter(flag); });
}

TLazy<TCounter> CreateUsedCounter(int& flag)
{
    TLazy<TCounter> counter(CreateCounter(flag));
    (void)(TCounter&)counter;
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
    (void)(TCounter&)first;
    BOOST_REQUIRE_EQUAL(secondFlag, 1);

    int thirdFlag = 0;
    TLazy<TCounter> second(CreateUsedCounter(thirdFlag));
    BOOST_REQUIRE_EQUAL(thirdFlag, 1);
}

BOOST_AUTO_TEST_CASE(assignments)
{
    bool firstFlag = false;
    TLazy<int> one([&firstFlag](){ return (firstFlag = true, 1); });
    one = 5.5;
    BOOST_REQUIRE_EQUAL(one, 5);
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
    (void)(TCounter&)forth;
    BOOST_REQUIRE_EQUAL(forthFlag, 1);
    BOOST_REQUIRE_EQUAL(dummyFlag, 0);
}

