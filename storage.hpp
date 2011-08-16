/*
 * storage.hpp      -- lazy evaluating values storage
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

#ifndef __STORAGE_HPP_2011_08_15__
#define __STORAGE_HPP_2011_08_15__

#include <functional>
#include <memory>

namespace NReinventedWheels
{
    namespace NPrivate
    {
        template <class TValue, bool Trivial_>
        class TStorage
        {
            bool Initialized_;
            TValue Value_;

        public:
            inline TStorage()
                : Initialized_(false)
            {
            }

            inline TStorage(const TStorage& storage)
                : Initialized_(storage.Initialized_)
                , Value_(Initialized_ ? storage.Value_ : TValue())
            {
            }

            inline bool IsInitialized() const
            {
                return Initialized_;
            }

            inline TStorage Release()
            {
                return *this;
            }

            inline void Reset()
            {
                Initialized_ = false;
            }

            inline operator TValue&()
            {
                return Value_;
            }

            inline operator const TValue&() const
            {
                return Value_;
            }

            inline TStorage& operator = (const TValue& value)
            {
                Initialized_ = true;
                Value_ = value;
                return *this;
            }

            inline TStorage& operator = (TStorage&& storage)
            {
                if ((Initialized_ = storage.Initialized_))
                {
                    Value_ = storage.Value_;
                }
                return *this;
            }

            inline TStorage& operator = (
                const std::function<TValue(void)>& calculator)
            {
                Value_ = calculator();
                Initialized_ = true;
                return *this;
            }
        };

        template <class TValue>
        class TStorage<TValue, false>
        {
            std::unique_ptr<TValue> Value_;
            inline TStorage(TValue* value)
                : Value_(value)
            {
            }

        public:
            inline TStorage()
            {
            }

            inline TStorage(const TStorage& storage)
                : Value_(storage.Value_.get() ?
                    new TValue(*storage.Value_) : nullptr)
            {
            }

            inline TStorage(TStorage&& storage)
                : Value_(storage.Value_.release())
            {
            }

            inline bool IsInitialized() const
            {
                return Value_.get();
            }

            inline TStorage Release()
            {
                return TStorage(Value_.release());
            }

            inline void Reset()
            {
                Value_.reset();
            }

            inline operator TValue&()
            {
                return *Value_;
            }

            inline operator const TValue&() const
            {
                return *Value_;
            }

            inline TStorage& operator = (const TValue& value)
            {
                Value_.reset(new TValue(value));
                return *this;
            }

            inline TStorage& operator = (TStorage&& storage)
            {
                Value_.reset(storage.Value_.release());
                return *this;
            }

            inline TStorage& operator = (
                const std::function<TValue(void)>& calculator)
            {
                Value_.reset(new TValue(calculator()));
                return *this;
            }
        };
    }
}

#endif

