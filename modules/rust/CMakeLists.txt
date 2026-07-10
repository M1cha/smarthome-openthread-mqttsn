set(features)
if (CONFIG_SMARTHOME_RUST_LOGGER)
set(features "logger")
endif()

set(target thumbv7m-none-eabi)

if (CONFIG_SMARTHOME_DEBUG_PROFILE)
set(lib_file "${CMAKE_CURRENT_BINARY_DIR}/target/${target}/debug/libsmarthome.a")
set(profile "")
else()
set(lib_file "${CMAKE_CURRENT_BINARY_DIR}/target/${target}/release/libsmarthome.a")
set(profile "--release")
endif()

set(include_dir "${CMAKE_CURRENT_BINARY_DIR}/include")
set(include_file "${include_dir}/smarthome-rust.h")

add_custom_target(smarthome_rust-build
	BYPRODUCTS "${lib_file}"
	WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
	COMMAND cargo build -Z build-std=core,panic_abort -Z build-std-features=panic_immediate_abort --target "${target}" --no-default-features ${profile} --target-dir "${CMAKE_CURRENT_BINARY_DIR}/target"  --features "${features}"
	VERBATIM
	USES_TERMINAL
)

add_custom_target(smarthome_rust-cbindgen
	BYPRODUCTS "${include_file}"
	WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
	COMMAND cbindgen --config smarthome/cbindgen.toml --crate smarthome --output "${include_file}" -q
	VERBATIM
)

add_dependencies(zephyr_interface smarthome_rust-cbindgen)
zephyr_include_directories("${include_dir}")

zephyr_library_import(smarthome_rust "${lib_file}")
add_dependencies(smarthome_rust smarthome_rust-build)
