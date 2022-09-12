#ifndef AMOUNT_H
#define AMOUNT_H

/*

This file is part of VROOM.

Copyright (c) 2015-2022, Julien Coupey.
All rights reserved (see LICENSE).

*/

#include <algorithm>
#include <memory>
#include <vector>
#include <cassert>

#include "structures/typedefs.h"

namespace vroom {

class AmountImpl
{
    public:
        virtual ~AmountImpl() = default;

        virtual auto clone() const -> AmountImpl * = 0;

        virtual auto is_less(AmountImpl const & other) const -> bool = 0;
        virtual auto is_equal(AmountImpl const & other) const -> bool = 0;
        virtual auto add(AmountImpl const & other) -> void = 0;
        virtual auto sub(AmountImpl const & other) -> void = 0;
        virtual auto update_to_maxed(AmountImpl const & other) -> void = 0;
        virtual auto set_zero() -> void = 0;
};

class AmountDims : public AmountImpl
{
    public:
        AmountDims() = default;
        AmountDims(std::size_t size)
          : elems(size, 0)
        {
        }
        AmountDims(std::vector<Capacity> elems)
          : elems(elems)
        {
        }

        auto clone() const -> AmountDims * override
        {
            return new AmountDims(*this);
        }

        auto is_less(AmountImpl const & other) const -> bool override
        {
            assert(elems.size() == static_cast<AmountDims const &>(other).elems.size());

            if (elems.empty())
            {
                return false;
            }

            auto last_rank = elems.size() - 1;
            for (std::size_t i = 0; i < last_rank; ++i)
            {
                if (elems[i] < static_cast<AmountDims const &>(other).elems[i])
                {
                    return true;
                }
                if (elems[i] > static_cast<AmountDims const &>(other).elems[i])
                {
                    return false;
                }
            }
            return elems[last_rank] < static_cast<AmountDims const &>(other).elems[last_rank];
        }

        auto is_equal(AmountImpl const & other) const -> bool override
        {
            assert(elems.size() == static_cast<AmountDims const &>(other).elems.size());

            for (std::size_t i = 0; i < elems.size(); ++i)
            {
                if (elems[i] != static_cast<AmountDims const &>(other).elems[i])
                {
                    return false;
                }
            }
            return true;
        }

        auto add(AmountImpl const & other) -> void override
        {
            assert(elems.size() == static_cast<AmountDims const &>(other).elems.size());

            for (std::size_t i = 0; i < elems.size(); ++i)
            {
                elems[i] += static_cast<AmountDims const &>(other).elems[i];
            }
        }

        auto sub(AmountImpl const & other) -> void override
        {
            assert(elems.size() == static_cast<AmountDims const &>(other).elems.size());

            for (std::size_t i = 0; i < elems.size(); ++i)
            {
                elems[i] -= static_cast<AmountDims const &>(other).elems[i];
            }
        }

        auto update_to_maxed(AmountImpl const & other) -> void override
        {
            assert(elems.size() == static_cast<AmountDims const &>(other).elems.size());

            for (std::size_t i = 0; i < elems.size(); ++i)
            {
                elems[i] = std::max(elems[i], static_cast<AmountDims const &>(other).elems[i]);
            }
        }

        auto set_zero() -> void override
        {
            std::fill(elems.begin(), elems.end(), 0);
        }

    private:
        std::vector<Capacity> elems;
};

class AmountEmpty : public AmountImpl
{
    public:
        AmountEmpty() = default;

        auto clone() const -> AmountEmpty * override
        {
            return new AmountEmpty();
        }

        auto is_less(AmountImpl const & /* other */) const -> bool override
        {
            return false;
        }

        auto is_equal(AmountImpl const & /* other */) const -> bool override
        {
            return true;
        }

        auto add(AmountImpl const & /* other */) -> void override
        {
        }

        auto sub(AmountImpl const & /* other */) -> void override
        {
        }

        auto update_to_maxed(AmountImpl const & /* other */) -> void override
        {
        }

        auto set_zero() -> void override
        {
        }
};

class Amount
{
    public:
        Amount()
          : m_impl(std::make_unique<AmountEmpty>())
        {
        }

        Amount(std::unique_ptr<AmountImpl> impl)
          : m_impl(std::move(impl))
        {
        }

        Amount(Amount const & other)
          : m_impl(other.m_impl->clone())
        {
        }

        Amount & operator=(Amount const & other)
        {
            m_impl.reset(other.m_impl->clone());
            return *this;
        }

        Amount & operator+=(Amount const & other)
        {
            m_impl->add(*other.m_impl);
            return *this;
        }

        Amount & operator-=(Amount const & other)
        {
            m_impl->sub(*other.m_impl);
            return *this;
        }

        auto operator==(Amount const & other) const -> bool
        {
            return m_impl->is_equal(*other.m_impl);
        }

        auto operator<=(Amount const & other) const -> bool
        {
            return m_impl->is_less(*other.m_impl) || m_impl->is_equal(*other.m_impl);
        }

        auto operator<<(Amount const & other) const -> bool
        {
            return m_impl->is_less(*other.m_impl);
        }

        auto update_to_maxed(Amount const & other) -> void
        {
            m_impl->update_to_maxed(*other.m_impl);
        }

        Amount get_zero() const
        {
            auto copy(*this);
            copy.m_impl->set_zero();
            return copy;
        }

    private:
        std::unique_ptr<AmountImpl> m_impl;
};

inline Amount operator+(Amount const & lhs, Amount const & rhs)
{
    auto result(lhs);
    result += rhs;
    return result;
}

inline Amount operator-(Amount const & lhs, Amount const & rhs)
{
    auto result(lhs);
    result -= rhs;
    return result;
}

} // namespace vroom

#endif
