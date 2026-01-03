#include <ygglet/processor/kernal_manager.hpp>
#include <ygglet/processor/logger.hpp>

#include <boost/functional/hash.hpp>

namespace ygglet::processor {

namespace {

void logDiagnostics(const auto& messages)
{
    for (const auto& message : messages.messages)
    {
        using Type = cmaj::DiagnosticMessage::Type;
        switch (message.type)
        {
            case Type::error:
            case Type::internalCompilerError:
                Logger::error("[{}] {}", message.getCategory(), message.getFullDescription());
                break;
            case Type::warning:
                Logger::warn("[{}] {}", message.getCategory(), message.getFullDescription(),
                             message.getAnnotatedSourceLine());
                break;
            case Type::note:
                Logger::info("[{}] {}", message.getCategory(), message.getFullDescription(),
                             message.getAnnotatedSourceLine());
                break;
            default:
                BOOST_ASSERT_MSG(false, "Unhandled message type");
                break;
        }
        if (auto location = message.getAnnotatedSourceLine(); !location.empty())
        {
            Logger::info("{}", location);
        }
    }
};

} // namespace
KernalManager::KernalManager(double sampleRate, std::uint32_t maxBlockSize)
: m_sampleRate(sampleRate)
, m_maxBlockSize(maxBlockSize)
{
}

KernalManager::Key KernalManager::hash(const Patch& patch) const
{
    auto hash = std::size_t{0};
    boost::hash_combine(hash, patch.source);
    boost::hash_combine(hash, patch.filename);
    boost::hash_combine(hash, m_maxBlockSize);
    boost::hash_combine(hash, m_sampleRate);
    return Key{hash};
}

std::optional<std::shared_ptr<Kernal>> KernalManager::aquire(Key key) const
{
    auto lock = std::lock_guard{m_mutex};
    if (auto itr = m_cache.find(key); itr != m_cache.cend())
    {
        if (auto instance = itr->second.lock())
        {
            return instance;
        }
    }
    return {};
}

std::optional<std::shared_ptr<Kernal>> KernalManager::aquire(const Patch& patch) const
{
    return aquire(hash(patch));
}

tl::expected<std::pair<KernalManager::Key, std::shared_ptr<Kernal>>, engine_manager::Error>
KernalManager::load(Patch patch)
{
    auto engine = std::make_shared<Kernal>();
    engine->engine = cmaj::Engine::create();

    if (!engine)
    {
        return tl::unexpected{engine_manager::FailedToCreateEngine{}};
    }

    cmaj::Program program;

    {
        cmaj::DiagnosticMessageList messages;
        const auto result = program.parse(messages, patch.filename ? *patch.filename : "", patch.source);
        logDiagnostics(messages);
        if (!result)
        {
            return tl::unexpected{std::move(messages)};
        }
    }

    {
        cmaj::BuildSettings buildSettings;
        buildSettings.setFrequency(m_sampleRate);
        buildSettings.setMaxBlockSize(m_maxBlockSize);
        engine->engine.setBuildSettings(buildSettings);
    }

    {
        cmaj::DiagnosticMessageList messages;
        const auto result = engine->engine.load(messages, program, nullptr, nullptr);
        logDiagnostics(messages);
        if (!result)
        {
            return tl::unexpected{std::move(messages)};
        }
    }

    // note endpoints need to be cached before linking (as per docs)

    auto inputs = engine->engine.getInputEndpoints();
    for (auto& endpoint : inputs.endpoints)
    {
        if (endpoint.isStream() && endpoint.getNumAudioChannels() > 0)
        {
            auto idString = endpoint.endpointID.toString();
            auto handle = engine->engine.getEndpointHandle(idString.c_str());
            engine->inputs.push_back(handle);
        }
    }

    auto outputs = engine->engine.getOutputEndpoints();
    for (auto& endpoint : outputs.endpoints)
    {
        if (endpoint.isStream() && endpoint.getNumAudioChannels() > 0)
        {
            auto idString = endpoint.endpointID.toString();
            auto handle = engine->engine.getEndpointHandle(idString.c_str());
            engine->outputs.push_back(handle);
        }
    }

    {
        cmaj::DiagnosticMessageList messages;
        const auto result = engine->engine.link(messages);
        logDiagnostics(messages);
        if (!result)
        {
            return tl::unexpected{std::move(messages)};
        }
    }

    const auto key = hash(patch);
    m_cache.insert(std::make_pair(key, std::weak_ptr{engine}));

    return std::make_pair(key, std::move(engine));
}

void KernalManager::cleanup()
{
    auto lock = std::lock_guard{m_mutex};
    std::vector<Key> remove{};
    for (auto [key, weak] : m_cache)
    {
        if (!weak.lock())
        {
            remove.push_back(key);
        }
    }
    for (auto key : remove)
    {
        m_cache.erase(key);
    }
}

} // namespace ygglet::processor
