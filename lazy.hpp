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

#ifndef __LAZY_HPP_2011_08_07__
#define __LAZY_HPP_2011_08_07__

#include <functional>
#include <memory>

namespace NReinventedWheels
{
    template <class TValue>
    class TLazy
    {
        mutable std::unique_ptr<TValue> Value_;
        std::function<TValue(void)> Calculator_;

        inline void Calculate() const
        {
            if (!Value_.get())
            {
                Value_.reset(new TValue(Calculator_()));
            }
        }

    public:
        inline TLazy(const std::function<TValue(void)>& calculator)
            : Calculator_(calculator)
        {
        }

        inline TLazy(const TLazy& lazy)
            : Value_(lazy.Value_.get() ? new TValue(*lazy.Value_) : 0)
            , Calculator_(Value_.get() ? 0 : lazy.Calculator_)
        {
        }

        inline TLazy(TLazy&& lazy)
            : Value_(lazy.Value_.release())
            , Calculator_(Value_.get() ? 0 : lazy.Calculator_)
        {
        }

        inline operator TValue&()
        {
            Calculate();
            return *Value_;
        }

        inline operator const TValue&() const
        {
            Calculate();
            return *Value_;
        }

        inline TLazy& operator = (const TValue& value)
        {
            Value_.reset(new TValue(value));
            return *this;
        }

        inline TLazy& operator = (const TLazy& lazy)
        {
            if (this != &lazy)
            {
                if (lazy.Value_.get())
                {
                    Value_.reset(new TValue(*lazy.Value_));
                }
                else
                {
                    Value_.reset();
                    Calculator_ = lazy.Calculator_;
                }
            }
            return *this;
        }

        inline TLazy& operator = (TLazy&& lazy)
        {
            Value_.reset(lazy.Value_.release());
            if (!Value_.get())
            {
                Calculator_ = lazy.Calculator_;
            }
            return *this;
        }
    };
}

#endif

