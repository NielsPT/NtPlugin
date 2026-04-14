# TODO: tests for Windows.

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  add_test(
    NAME ValidatePluginAU
    COMMAND pluginval
    --strictness-level 10
    --validate
    $<TARGET_BUNDLE_DIR:${NTFX_PLUGIN}_AU>
  )
endif()

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Windows")
  add_test(
    NAME ValidatePluginVST3
    COMMAND pluginval
    --strictness-level 10
    --validate
    $<TARGET_BUNDLE_DIR:${NTFX_PLUGIN}_VST3>
  )
endif()


