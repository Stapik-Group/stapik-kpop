include(FetchContent)

FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/releases/download/v1.17.0/googletest-1.17.0.tar.gz
        FIND_PACKAGE_ARGS NAMES GTest
)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

add_executable(stapikkpop_tests
        CollectionControllerTest.cpp
        CollectionDocumentTest.cpp
        CollectionStatisticsTest.cpp
        EnumCatalogTest.cpp
        ItemFilterTest.cpp
        LocaleResourcesTest.cpp
        PartialDateTest.cpp
        support/TestFixtures.hpp
)
target_link_libraries(stapikkpop_tests PRIVATE kpopcore GTest::gtest_main)
target_compile_definitions(stapikkpop_tests PRIVATE
        STAPIK_KPOP_RESOURCES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../resources"
        STAPIK_KPOP_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../src")
stapik_enable_warnings(stapikkpop_tests)
gtest_discover_tests(stapikkpop_tests)
