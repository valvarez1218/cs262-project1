cmake_minimum_required(VERSION 3.2)
project(my_project)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 14)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/5376968f6948923e2411081fd9372e71a59d8e77.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
  storageTests
  ./tests/storageTests.cc
) 

add_executable(
  clientTests
  ./tests/clientTests.cc
)

add_executable(
  integrationTest
  ./tests/integrationTest.cc
)

target_link_libraries(
  storageTests
  gtest
  gtest_main
)

target_link_libraries(
  clientTests
  gtest
  gtest_main
)

target_link_libraries(
  integrationTest
  gtest
  gtest_main
)

include(GoogleTest)
# gtest_discover_tests(storageTests)
gtest_add_tests(TARGET storageTests)
gtest_add_tests(TARGET clientTests)
gtest_add_tests(TARGET integrationTest)