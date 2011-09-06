/*
 * storage.hpp              -- lazy evaluating values storage
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
#include <utility>

#include <reinvented-wheels/cheapcopy.hpp>

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

            inline TStorage(TStorage&& storage)
                : Initialized_(storage.Initialized_)
                , Value_(Initialized_ ? std::move(storage.Value_) : TValue())
            {
                storage.Initialized_ = false;
            }

            inline bool IsInitialized() const
            {
                return Initialized_;
            }

            inline void Reset()
            {
                Initialized_ = false;
            }

            inline operator TValue&()
            {
                return Value_;
            }

            inline operator typename TCheapCopy<TValue>::TValueType() const
            {
                return Value_;
            }

            inline TStorage& operator = (
                typename TCheapCopy<TValue>::TValueType value)
            {
                Initialized_ = true;
                Value_ = value;
                return *this;
            }

            inline TStorage& operator = (TStorage&& storage)
            {
                if ((Initialized_ = storage.Initialized_))
                {
                    Value_ = std::move(storage.Value_);
                }
                storage.Initialized_ = false;
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
                : Value_(std::move(storage.Value_))
            {
            }

            inline bool IsInitialized() const
            {
                return Value_.get();
            }

            inline void Reset()
            {
                Value_.reset();
            }

            inline operator TValue&()
            {
                return *Value_;
            }

            inline operator typename TCheapCopy<TValue>::TValueType() const
            {
                return *Value_;
            }

            inline TStorage& operator = (
                typename TCheapCopy<TValue>::TValueType value)
            {
                Value_.reset(new TValue(value));
                return *this;
            }

            inline TStorage& operator = (TStorage&& storage)
            {
                Value_ = std::move(storage.Value_);
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

