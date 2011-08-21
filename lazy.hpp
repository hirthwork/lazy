/*
 * lazy.hpp                 -- lazy evaluating variables wrapper
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

#ifndef __LAZY_HPP_2011_08_07__
#define __LAZY_HPP_2011_08_07__

#include <functional>
#include <type_traits>

#include <reinvented-wheels/cheapcopy.hpp>

#include "storage.hpp"

namespace NReinventedWheels
{
    template <class TValue>
    class TLazy
    {
        mutable NPrivate::TStorage<TValue,
            std::is_copy_assignable<TValue>::value
            && std::has_trivial_default_constructor<TValue>::value> Value_;
        // Uncomment this line, when this trait will be implemented by gcc
        //    && std::is_trivially_default_constructible<TValue>::value> Value_;
        std::function<TValue(void)> Calculator_;

        inline void Calculate() const
        {
            if (!Value_.IsInitialized())
            {
                Value_ = Calculator_;
            }
        }

    public:
        inline TLazy(const std::function<TValue(void)>& calculator)
            : Calculator_(calculator)
        {
        }

        inline TLazy(const TLazy& lazy)
            : Value_(lazy.Value_)
            , Calculator_(Value_.IsInitialized() ? nullptr : lazy.Calculator_)
        {
        }

        inline TLazy(TLazy&& lazy)
            : Value_(lazy.Value_.Release())
            , Calculator_(Value_.IsInitialized() ? nullptr : lazy.Calculator_)
        {
        }

        inline operator TValue&()
        {
            Calculate();
            return Value_;
        }

        inline operator typename TCheapCopy<TValue>::TValueType() const
        {
            Calculate();
            return Value_;
        }

        inline TLazy& operator = (
            typename TCheapCopy<TValue>::TValueType value)
        {
            Value_ = value;
            return *this;
        }

        inline TLazy& operator = (const TLazy& lazy)
        {
            if (this != &lazy)
            {
                if (lazy.Value_.IsInitialized())
                {
                    Value_ = static_cast<
                        typename TCheapCopy<TValue>::TValueType>(lazy.Value_);
                }
                else
                {
                    Value_.Reset();
                    Calculator_ = lazy.Calculator_;
                }
            }
            return *this;
        }

        inline TLazy& operator = (TLazy&& lazy)
        {
            Value_ = lazy.Value_.Release();
            if (!Value_.IsInitialized())
            {
                Calculator_ = lazy.Calculator_;
            }
            return *this;
        }
    };
}

#endif

