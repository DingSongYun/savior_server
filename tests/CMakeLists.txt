# 测试模块
enable_testing()

# 查找Google Test（如果可用）
find_package(GTest QUIET)

if(GTest_FOUND)
    # 使用Google Test
    add_executable(savior_tests
        test_main.cpp
        # TODO: 添加测试文件
    )
    
    target_link_libraries(savior_tests
        PRIVATE
        savior_core
        savior_network
        savior_message
        savior_entity
        GTest::gtest
        GTest::gtest_main
    )
    
    add_test(NAME SaviorServerTests COMMAND savior_tests)
else()
    # 简单的测试程序
    add_executable(simple_test
        simple_test.cpp
    )
    
    target_link_libraries(simple_test
        PRIVATE
        savior_core
        savior_network
        savior_message
        savior_entity
    )
    
    add_test(NAME SimpleTest COMMAND simple_test)
endif()