# TODO: tests for Windows.

if(CMAKE_SYSTEM_NAME NOT STREQUAL "Windows")
  if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    add_test(
      NAME ValidatePluginAU
      COMMAND pluginval
      --strictness-level 10
      --validate
      $<TARGET_BUNDLE_DIR:${NTFX_PLUGIN}_AU>
    )
  endif()

  add_test(
    NAME ValidatePluginVST3
    COMMAND pluginval
    --strictness-level 10
    --validate
    $<TARGET_BUNDLE_DIR:${NTFX_PLUGIN}_VST3>
  )
endif()


