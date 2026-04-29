if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  add_test(
    NAME ValidatePluginVST3
    COMMAND pluginval
    --strictness-level 10
    --validate
    $<TARGET_BUNDLE_DIR:${NTFX_PLUGIN}_VST3>
  )
  add_test(
    NAME ValidatePluginAU
    COMMAND pluginval
    --strictness-level 10
    --validate
    $<TARGET_BUNDLE_DIR:${NTFX_PLUGIN}_AU>
  )
else()
  add_test(
    NAME ValidatePluginVST3Linux
    COMMAND pluginval
    --strictness-level 10
    --validate
    ../../build/${NTFX_PLUGIN}_artefacts/${CMAKE_BUILD_TYPE}/VST3/${NTFX_PLUGIN}.vst3
  )
endif()
