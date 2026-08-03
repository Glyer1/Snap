cmake_minimum_required(VERSION 3.16)

project(Snap VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- 测试模块（可选）----
option(BUILD_TESTS "Build unit tests" ON)

set(QT_COMPONENTS Quick Core)
if(BUILD_TESTS)
    list(APPEND QT_COMPONENTS Test)
endif()
find_package(Qt6 REQUIRED COMPONENTS ${QT_COMPONENTS})
qt_standard_project_setup(REQUIRES 6.8)

# Build qtkeychain from thirdparty source (Windows)
set(QTKEYCHAIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/resources/thirdparty/qtkeychain/qt6keychain")
add_library(qt6keychain STATIC
    ${QTKEYCHAIN_DIR}/keychain.cpp
    ${QTKEYCHAIN_DIR}/keychain_win.cpp
    ${QTKEYCHAIN_DIR}/keychain.h
    ${QTKEYCHAIN_DIR}/keychain_p.h
)
# Parent dir so <qt6keychain/keychain.h> resolves correctly
target_include_directories(qt6keychain PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/resources/thirdparty/qtkeychain")
target_compile_definitions(qt6keychain PUBLIC
    QTKEYCHAIN_NO_EXPORT
    USE_CREDENTIAL_STORE=1
    UNICODE
)
target_link_libraries(qt6keychain PUBLIC Qt6::Core PRIVATE crypt32)
set_target_properties(qt6keychain PROPERTIES
    AUTOMOC ON
    DEBUG_POSTFIX "d"
)


qt_add_executable(appSnap
    resources/src/main.cpp
)

qt_add_qml_module(appSnap
    URI Snap
    VERSION 0.1
    QML_FILES
        resources/qml/Main.qml
    SOURCES
        resources/src/appcore.cpp
        resources/src/appcore.h
        SOURCES resources/src/types/types.h resources/src/types/types.cpp
        SOURCES resources/src/workers/apiworker.h resources/src/workers/apiworker.cpp
        SOURCES resources/src/utils/jsonutils.h resources/src/utils/jsonutils.cpp
        SOURCES resources/src/utils/passwordutils.h resources/src/utils/passwordutils.cpp
        QML_FILES resources/qml/pages/SettingsPage.qml
)

target_include_directories(appSnap PRIVATE
    resources/src
)

target_link_libraries(appSnap
    PRIVATE Qt6::Quick
    qt6keychain
)

set_target_properties(appSnap PROPERTIES
    WIN32_EXECUTABLE TRUE
)

install(TARGETS appSnap
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# ---- 测试 Target ----
if(BUILD_TESTS)
    enable_testing()

    set(TEST_INCLUDES
        resources/src
        resources/thirdparty/qtkeychain
    )
    set(TEST_DEFINES
        QTKEYCHAIN_NO_EXPORT
        USE_CREDENTIAL_STORE=1
        UNICODE
    )
    set(TEST_SOURCES
        resources/src/appcore.cpp
        resources/src/appcore.h
        resources/src/types/types.h
        resources/src/types/types.cpp
        resources/src/workers/apiworker.h
        resources/src/workers/apiworker.cpp
        resources/src/utils/jsonutils.h
        resources/src/utils/jsonutils.cpp
        resources/src/utils/passwordutils.h
        resources/src/utils/passwordutils.cpp
    )
    set(TEST_LIBS Qt6::Core Qt6::Quick Qt6::Test qt6keychain)

    add_executable(tst_jsonutils
        ${TEST_SOURCES}
        tests/tst_jsonutils.cpp
    )
    target_include_directories(tst_jsonutils PRIVATE ${TEST_INCLUDES})
    target_compile_definitions(tst_jsonutils PRIVATE ${TEST_DEFINES})
    target_link_libraries(tst_jsonutils PRIVATE ${TEST_LIBS})
    set_target_properties(tst_jsonutils PROPERTIES AUTOMOC ON)
    add_test(NAME tst_jsonutils COMMAND tst_jsonutils)

    add_executable(tst_types
        ${TEST_SOURCES}
        tests/tst_types.cpp
    )
    target_include_directories(tst_types PRIVATE ${TEST_INCLUDES})
    target_compile_definitions(tst_types PRIVATE ${TEST_DEFINES})
    target_link_libraries(tst_types PRIVATE ${TEST_LIBS})
    set_target_properties(tst_types PROPERTIES AUTOMOC ON)
    add_test(NAME tst_types COMMAND tst_types)

    add_executable(tst_appcore
        ${TEST_SOURCES}
        tests/tst_appcore.cpp
    )
    target_include_directories(tst_appcore PRIVATE ${TEST_INCLUDES})
    target_compile_definitions(tst_appcore PRIVATE ${TEST_DEFINES})
    target_link_libraries(tst_appcore PRIVATE ${TEST_LIBS})
    set_target_properties(tst_appcore PROPERTIES AUTOMOC ON)
    add_test(NAME tst_appcore COMMAND tst_appcore)

    add_executable(tst_passwordutils
        ${TEST_SOURCES}
        tests/tst_passwordutils.cpp
    )
    target_include_directories(tst_passwordutils PRIVATE ${TEST_INCLUDES})
    target_compile_definitions(tst_passwordutils PRIVATE ${TEST_DEFINES})
    target_link_libraries(tst_passwordutils PRIVATE ${TEST_LIBS})
    set_target_properties(tst_passwordutils PROPERTIES AUTOMOC ON)
    add_test(NAME tst_passwordutils COMMAND tst_passwordutils)
endif()
