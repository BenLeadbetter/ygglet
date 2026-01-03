#pragma once

#include <ygglet/processor/kernal.hpp>
#include <ygglet/processor/patch.hpp>

#include <boost/container/flat_map.hpp>
#include <cmajor/API/cmaj_DiagnosticMessages.h>
#include <cmajor/API/cmaj_Engine.h>
#include <cmajor/API/cmaj_Performer.h>
#include <strong_type/strong_type.hpp>
#include <tl/expected.hpp>

#include <memory>
#include <mutex>

namespace ygglet::processor {

namespace engine_manager {
struct FailedToCreateEngine
{
};
using Error = std::variant<FailedToCreateEngine, cmaj::DiagnosticMessageList>;
} // namespace engine_manager

struct Node;

struct KernalManager
{
    KernalManager(double sampleRate, std::uint32_t maxBlockSize);

    using Key = strong::type<std::size_t, struct key_, strong::ordered>;

    tl::expected<std::pair<Key, std::shared_ptr<Kernal>>, engine_manager::Error> load(Patch);
    Key hash(const Patch&) const;
    std::optional<std::shared_ptr<Kernal>> aquire(Key) const;
    std::optional<std::shared_ptr<Kernal>> aquire(const Patch&) const;
    void cleanup();

private:
    using Cache = boost::container::flat_map<Key, std::weak_ptr<Kernal>>;

    Cache m_cache;
    double m_sampleRate;
    mutable std::mutex m_mutex;
    std::uint32_t m_maxBlockSize;
};

} // namespace ygglet::processor
