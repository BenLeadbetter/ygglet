#include <cmajor/COM/cmaj_Library.h>

namespace ygglet::audio::cmajor {

void init()
{
    if constexpr (cmaj::Library::isUsingDLL)
    {
        std::string dllName = cmaj::Library::getDLLName();

        // On macOS with conan, the library should be in the RPATH
        // We can just pass the name and let the system find it
        if (!cmaj::Library::initialise(dllName))
        {
            // TODO:
            // If that doesn't work, it might need an absolute path
            // which would be set up by CMake
        }
    }
}

void deinit()
{
    cmaj::Library::shutdown();
}

} // namespace ygglet::audio::cmajor
