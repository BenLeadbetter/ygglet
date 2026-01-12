#pragma once

#include <strong_type/strong_type.hpp>

#include <boost/uuid/generators.hpp>
#include <boost/uuid/uuid.hpp>

namespace ygglet::engine::detail {

template <typename Tag> using IdBase = strong::type<boost::uuids::uuid, Tag, strong::regular, strong::ordered>;

template <typename Tag> struct Id : detail::IdBase<Tag>
{
    using detail::IdBase<Tag>::IdBase;
    static Id generate() { return Id{boost::uuids::random_generator{}()}; }
};

} // namespace ygglet::engine::detail
