cmake_minimum_required(VERSION 3.7)
project(test)

find_package(Boost 1.49 COMPONENTS
    unit_test_framework
    filesystem
)

if(NOT Boost_FOUND)
  message(WARNING "Boost not found, don't build tests")
  return()
endif(NOT Boost_FOUND)

add_executable(test_metadata
  test_metadata.cpp
)

target_link_libraries(test_metadata
  PUBLIC
    Boost::boost

  PRIVATE
    rvnbinresource
    Boost::unit_test_framework
)

target_compile_definitions(test_metadata PRIVATE "BOOST_TEST_DYN_LINK")

add_test(rvnbinresource::metadata test_metadata)

add_executable(test_reader
  test_reader.cpp
)

target_link_libraries(test_reader
  PUBLIC
    Boost::boost

  PRIVATE
    rvnbinresource
    Boost::unit_test_framework
)

target_compile_definitions(test_reader PRIVATE "BOOST_TEST_DYN_LINK")

add_test(rvnbinresource::reader test_reader)

add_executable(test_writer
  test_writer.cpp
)

target_link_libraries(test_writer
  PUBLIC
    Boost::boost

  PRIVATE
    rvnbinresource
    Boost::unit_test_framework
)

target_compile_definitions(test_writer PRIVATE "BOOST_TEST_DYN_LINK")

add_test(rvnbinresource::writer test_writer)

add_executable(test_read_write
  test_read_write.cpp
)

target_link_libraries(test_read_write
  PUBLIC
    Boost::boost

  PRIVATE
    rvnbinresource
    Boost::unit_test_framework
    Boost::filesystem
)

target_compile_definitions(test_read_write PRIVATE "BOOST_TEST_DYN_LINK")

add_test(rvnbinresource::read_write test_read_write)
