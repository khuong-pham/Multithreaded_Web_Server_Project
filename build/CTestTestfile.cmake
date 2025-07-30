# CMake generated Testfile for 
# Source directory: /home/khuong/Backend/Project/Multithreaded_Server
# Build directory: /home/khuong/Backend/Project/Multithreaded_Server/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(unit_tests "/home/khuong/Backend/Project/Multithreaded_Server/build/unit_tests")
set_tests_properties(unit_tests PROPERTIES  _BACKTRACE_TRIPLES "/home/khuong/Backend/Project/Multithreaded_Server/CMakeLists.txt;67;add_test;/home/khuong/Backend/Project/Multithreaded_Server/CMakeLists.txt;70;add_gtest;/home/khuong/Backend/Project/Multithreaded_Server/CMakeLists.txt;0;")
add_test(test_integration "/home/khuong/Backend/Project/Multithreaded_Server/build/test_integration")
set_tests_properties(test_integration PROPERTIES  _BACKTRACE_TRIPLES "/home/khuong/Backend/Project/Multithreaded_Server/CMakeLists.txt;67;add_test;/home/khuong/Backend/Project/Multithreaded_Server/CMakeLists.txt;81;add_gtest;/home/khuong/Backend/Project/Multithreaded_Server/CMakeLists.txt;0;")
add_test(load_tests "/home/khuong/Backend/Project/Multithreaded_Server/build/load_tests")
set_tests_properties(load_tests PROPERTIES  _BACKTRACE_TRIPLES "/home/khuong/Backend/Project/Multithreaded_Server/CMakeLists.txt;67;add_test;/home/khuong/Backend/Project/Multithreaded_Server/CMakeLists.txt;85;add_gtest;/home/khuong/Backend/Project/Multithreaded_Server/CMakeLists.txt;0;")
