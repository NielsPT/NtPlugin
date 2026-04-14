add_test(
    NAME ValidatePluginAU
    COMMAND pluginval
        --strictness-level 10
        --validate
        $<TARGET_BUNDLE_DIR:${NTFX_PLUGIN}_AU>
)

add_test(
    NAME ValidatePluginVST3
    COMMAND pluginval
        --strictness-level 10
        --validate
        $<TARGET_BUNDLE_DIR:${NTFX_PLUGIN}_VST3>
)


