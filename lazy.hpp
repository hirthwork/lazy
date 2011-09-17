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
#include <new>
#include <type_traits>
#include <utility>

namespace NReinventedWheels
{
    template <class TValue>
    struct TLazyBase
    {
        // use alignas(TValue) instead of union once it will be implemented in
        // compiler
        union TStorage {
            constexpr TStorage()
            {
            }

            inline ~TStorage()
            {
            }

            TValue Data_;
        } Storage_;
        TValue& Value_;
        mutable bool Initialized_;

        inline TLazyBase()
            : Value_(Storage_.Data_)
            , Initialized_(false)
        {
        }

        inline ~TLazyBase()
        {
            if (Initialized_) {
                Value_.~TValue();
            }
        }

        inline void Destroy()
        {
            Value_.~TValue();
            Initialized_ = false;
        }
    };

    template <class TValue>
    class TLazy : TLazyBase<TValue>
    {
        constexpr void ValidateCopyTraits()
        {
            static_assert(std::is_copy_constructible<TValue>::value ||
                (std::is_default_constructible<TValue>::value &&
                    std::is_copy_assignable<TValue>::value),
                "Stored type should be either copy constructible or "
                "default constructible and copy assignable");
        }
        typedef TLazyBase<TValue> TBase;
        using TBase::Value_;
        using TBase::Initialized_;
        typedef std::function<TValue(void)> TCalculator;
        TCalculator Calculator_;

        inline void Calculate(std::true_type) const
        {
            // TODO: check that move c'tor called here if present
            new(&Value_) TValue(Calculator_());
        }

        inline void Calculate(std::false_type) const
        {
            new(&Value_) TValue;
            Value_ = this->Calculator();
        }

        inline void Calculate() const
        {
            if (!Initialized_)
            {
                Calculate(std::__or_<std::is_copy_constructible<TValue>,
                    std::is_move_constructible<TValue>>());
                Initialized_ = true;
            }
        }

        inline void MoveNewValue(std::true_type, TValue&& value)
        {
            new(&Value_) TValue;
            Value_ = std::move(value);
        }

        inline void MoveNewValue(std::false_type, TValue&& value)
        {
            ConstructValue(value);
        }

        inline void MoveNewValue(TValue&& value)
        {
            MoveNewValue(std::is_move_assignable<TValue>(), std::move(value));
        }

        inline void ConstructValue(std::true_type, const TValue& value)
        {
            new(&Value_) TValue(value);
        }

        inline void ConstructValue(std::false_type, const TValue& value)
        {
            new(&Value_) TValue;
            Value_ = value;
        }

        inline void ConstructValue(const TValue& value)
        {
            ConstructValue(std::is_copy_constructible<TValue>(), value);
        }

        inline void ConstructValue(std::true_type, TValue&& value)
        {
            new(&Value_) TValue(std::move(value));
        }

        inline void ConstructValue(std::false_type, TValue&& value)
        {
            MoveNewValue(std::move(value));
        }

        inline void ConstructValue(TValue&& value)
        {
            ConstructValue(std::is_move_constructible<TValue>(),
                std::move(value));
        }

        inline void CopyValue(std::true_type, const TValue& value)
        {
            Value_ = value;
        }

        inline void CopyValue(std::false_type, const TValue& value)
        {
            // TODO: provide strong guarantees here
            Value_.~TValue();
            new(&Value_) TValue(value);
        }

        inline void CopyValue(const TValue& value)
        {
            CopyValue(std::is_copy_assignable<TValue>(), value);
        }

        // TODO: rewrite next five functions, to have less functions
        inline void MoveValue(std::true_type, TValue&& value)
        {
            Value_ = std::move(value);
        }

        inline void MoveValue(std::false_type, TValue&& value)
        {
            // TODO: provide strong guarantees here
            Value_.~TValue();
            ConstructValue(std::move(value));
        }

        inline void MoveAssign(std::true_type, TValue&& value)
        {
            MoveValue(std::is_move_assignable<TValue>(), std::move(value));
        }

        inline void MoveAssign(std::false_type, TValue&& value)
        {
            CopyValue(value);
        }

        inline void MoveValue(TValue&& value)
        {
            MoveAssign(std::__or_<std::is_move_constructible<TValue>,
                std::is_move_assignable<TValue>>(), std::move(value));
        }

    public:
        inline explicit TLazy(const TCalculator& calculator)
            : Calculator_(calculator)
        {
        }

        inline explicit TLazy(TCalculator&& calculator)
            : Calculator_(std::move(calculator))
        {
        }

        inline TLazy(const TLazy& lazy)
        {
            ValidateCopyTraits();
            if (lazy.Initialized_)
            {
                ConstructValue(lazy.Value_);
                Initialized_ = true;
            }
            else
            {
                Calculator_ = lazy.Calculator_;
            }
        }

        inline TLazy(TLazy&& lazy)
        {
            if (lazy.Initialized_)
            {
                ConstructValue(std::move(lazy.Value_));
                Initialized_ = true;
            }
            else
            {
                Calculator_ = std::move(lazy.Calculator_);
            }
        }

        inline operator TValue&()
        {
            Calculate();
            return Value_;
        }

        inline operator const TValue&() const
        {
            Calculate();
            return Value_;
        }

        inline TLazy& operator = (const TValue& value)
        {
            ValidateCopyTraits();
            if (Initialized_)
            {
                CopyValue(value);
            }
            else
            {
                ConstructValue(value);
                Initialized_ = true;
            }
            return *this;
        }

        inline TLazy& operator = (TValue&& value)
        {
            if (Initialized_)
            {
                MoveValue(std::move(value));
            }
            else
            {
                ConstructValue(std::move(value));
                Initialized_ = true;
            }
            return *this;
        }

        inline TLazy& operator = (const TLazy& lazy)
        {
            ValidateCopyTraits();
            if (this != &lazy)
            {
                if (lazy.Initialized_)
                {
                    if (Initialized_)
                    {
                        CopyValue(lazy.Value_);
                    }
                    else
                    {
                        ConstructValue(lazy.Value_);
                        Initialized_ = true;
                    }
                }
                else
                {
                    Calculator_ = lazy.Calculator_;
                    if (Initialized_)
                    {
                        this->Destroy();
                    }
                }
            }
            return *this;
        }

        inline TLazy& operator = (TLazy&& lazy)
        {
            if (lazy.Initialized_)
            {
                if (Initialized_) {
                    MoveValue(std::move(lazy.Value_));
                } else {
                    ConstructValue(std::move(lazy.Value_));
                    Initialized_ = true;
                }
            }
            else
            {
                if (Initialized_) {
                    this->Destroy();
                }
                Calculator_ = std::move(lazy.Calculator_);
            }
            return *this;
        }

        inline void Swap(TLazy& lazy)
        {
            if (Initialized_)
            {
                if (lazy.Initialized_)
                {
                    std::swap(Value_, lazy.Value_);
                }
                else
                {
                    lazy.MoveNewValue(std::move(Value_));
                    lazy.Initialized_ = true;
                    Calculator_ = std::move(lazy.Calculator_);
                    this->Destroy();
                }
            }
            else
            {
                if (lazy.Initialized_)
                {
                    MoveNewValue(std::move(lazy.Value_));
                    Initialized_ = true;
                    lazy.Calculator_ = std::move(Calculator_);
                    lazy.Destroy();
                }
                else
                {
                    std::swap(Calculator_, lazy.Calculator_);
                }
            }
        }
    };
}

namespace std {
    template <class TValue>
    void swap(NReinventedWheels::TLazy<TValue>& lhs,
        NReinventedWheels::TLazy<TValue>& rhs)
    {
        lhs.Swap(rhs);
    }
}

#endif

