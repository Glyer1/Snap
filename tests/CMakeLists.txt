cmake_minimum_required(VERSION 3.16)

# ---- Test Infrastructure ----
# 注意：Qt6::Test 已在父 CMakeLists.txt 通过 find_package 加载（BUILD_TESTS=ON 时）
enable_testing()

# 测试辅助函数：注册一个测试 target
function(add_snap_test TEST_NAME TEST_SOURCE)
    add_executable(${TEST_NAME}
        ${TEST_SOURCE}
    )

    target_include_directories(${TEST_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../resources/src
        ${CMAKE_CURRENT_SOURCE_DIR}/../resources/thirdparty/qtkeychain
    )

    target_link_libraries(${TEST_NAME}
        PRIVATE Qt6::Core
        PRIVATE Qt6::Quick
        PRIVATE Qt6::Test
        PRIVATE qt6keychain      # 复用父作用域定义的 qtkeychain 静态库
    )

    target_compile_definitions(${TEST_NAME} PRIVATE
        QTKEYCHAIN_NO_EXPORT
        USE_CREDENTIAL_STORE=1
        UNICODE
    )

    # 将项目源码与测试源码一起编译
    target_sources(${TEST_NAME} PRIVATE
        # appcore
        ${CMAKE_CURRENT_SOURCE_DIR}/../resources/src/appcore.cpp
        # types
        ${CMAKE_CURRENT_SOURCE_DIR}/../resources/src/types/types.cpp
        # utils
        ${CMAKE_CURRENT_SOURCE_DIR}/../resources/src/utils/jsonutils.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../resources/src/utils/passwordutils.cpp
        # workers
        ${CMAKE_CURRENT_SOURCE_DIR}/../resources/src/workers/apiworker.cpp
    )

    set_target_properties(${TEST_NAME} PROPERTIES
        AUTOMOC ON
    )

    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endfunction()

# ---- Test Targets ----

add_snap_test(tst_jsonutils     tst_jsonutils.cpp)
add_snap_test(tst_types         tst_types.cpp)
add_snap_test(tst_appcore       tst_appcore.cpp)
add_snap_test(tst_passwordutils tst_passwordutils.cpp)
