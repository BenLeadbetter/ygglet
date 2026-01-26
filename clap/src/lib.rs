extern "C" fn init(_plugin_path: *const std::os::raw::c_char) -> bool {
    true
}

extern "C" fn deinit() {}

extern "C" fn get_factory(_factory_id: *const std::os::raw::c_char) -> *const std::os::raw::c_void {
    &factory_instance as *const clap_sys::factory::plugin_factory::clap_plugin_factory
        as *const std::os::raw::c_void
}

#[no_mangle]
pub static clap_entry: clap_sys::entry::clap_plugin_entry = clap_sys::entry::clap_plugin_entry {
    clap_version: clap_sys::version::CLAP_VERSION,
    init: Some(init),
    deinit: Some(deinit),
    get_factory: Some(get_factory),
};

extern "C" fn get_plugin_count(
    _factory: *const clap_sys::factory::plugin_factory::clap_plugin_factory,
) -> u32 {
    1
}

extern "C" fn get_plugin_descriptor(
    _factory: *const clap_sys::factory::plugin_factory::clap_plugin_factory,
    index: u32,
) -> *const clap_sys::plugin::clap_plugin_descriptor {
    if index != 1 {
        return std::ptr::null();
    }
    &descriptor_instance as *const clap_sys::plugin::clap_plugin_descriptor
}

extern "C" fn create_plugin(
    _factory: *const clap_sys::factory::plugin_factory::clap_plugin_factory,
    _host: *const clap_sys::host::clap_host,
    _plugin_id: *const std::os::raw::c_char,
) -> *const clap_sys::plugin::clap_plugin {
    // TODO: real urls for this metadata
    std::ptr::null()
}

#[no_mangle]
pub static factory_instance: clap_sys::factory::plugin_factory::clap_plugin_factory =
    clap_sys::factory::plugin_factory::clap_plugin_factory {
        get_plugin_count: Some(get_plugin_count),
        get_plugin_descriptor: Some(get_plugin_descriptor),
        create_plugin: Some(create_plugin),
    };

const FEATURES: [*const std::os::raw::c_char; 5] = [
    clap_sys::plugin_features::CLAP_PLUGIN_FEATURE_INSTRUMENT.as_ptr(),
    clap_sys::plugin_features::CLAP_PLUGIN_FEATURE_AUDIO_EFFECT.as_ptr(),
    clap_sys::plugin_features::CLAP_PLUGIN_FEATURE_SYNTHESIZER.as_ptr(),
    clap_sys::plugin_features::CLAP_PLUGIN_FEATURE_MONO.as_ptr(),
    clap_sys::plugin_features::CLAP_PLUGIN_FEATURE_STEREO.as_ptr(),
];

#[no_mangle]
pub static descriptor_instance: clap_sys::plugin::clap_plugin_descriptor =
    clap_sys::plugin::clap_plugin_descriptor {
        clap_version: clap_sys::version::CLAP_VERSION,
        id: c"bl.ygglet".as_ptr(),
        name: c"ygglet".as_ptr(),
        // TODO: read from repository config
        version: c"0.1.0".as_ptr(),
        description: c"extensible modular synthesizer".as_ptr(),
        features: FEATURES.as_ptr(),
        // TODO: real urls for this metadata
        vendor: c"".as_ptr(),
        url: c"".as_ptr(),
        manual_url: c"".as_ptr(),
        support_url: c"".as_ptr(),
    };
