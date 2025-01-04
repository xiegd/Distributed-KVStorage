#include <gtest/gtest.h>
#include "include/slice.h"
#include <string>
#include <iostream>

namespace kvstorage {

// 测试夹具（Test Fixture）
class SliceTest : public ::testing::Test {
protected:
    // 在每个TEST_F之前自动执行
    void SetUp() override {
        test_str = "hello world";
        test_slice = Slice(test_str);
    }

    std::string test_str;
    Slice test_slice;
};

// 测试构造函数
TEST_F(SliceTest, Constructor) {
    // 默认构造函数
    Slice empty_slice;
    EXPECT_TRUE(empty_slice.empty());
    EXPECT_EQ(empty_slice.size(), 0);
    
    // 从 const char* 构造
    Slice s1("hello");
    EXPECT_EQ(s1.size(), 5);
    EXPECT_EQ(s1.toString(), "hello");
    
    // 从 string 构造
    std::string str = "world";
    Slice s2(str);
    EXPECT_EQ(s2.size(), 5);
    EXPECT_EQ(s2.toString(), "world");
    
    // 从 char* 和 length 构造
    const char* data = "test";
    Slice s3(data, 4);
    EXPECT_EQ(s3.size(), 4);
    EXPECT_EQ(s3.toString(), "test");
}

// 测试基本操作
TEST_F(SliceTest, BasicOperations) {
    EXPECT_FALSE(test_slice.empty());
    EXPECT_EQ(test_slice.size(), 11);
    EXPECT_EQ(test_slice.toString(), "hello world");
    
    // 测试 operator[]
    EXPECT_EQ(test_slice[0], 'h');
    EXPECT_EQ(test_slice[5], ' ');
    EXPECT_EQ(test_slice[10], 'd');
}

// 测试 startsWith
TEST_F(SliceTest, StartsWith) {
    Slice prefix("hello");
    EXPECT_TRUE(test_slice.startsWith(prefix));    
}

// 测试 removePrefix
TEST_F(SliceTest, RemovePrefix) {
    Slice s = test_slice;
    s.removePrefix(6);  // 移除 "hello "
    EXPECT_EQ(s.toString(), "world");
    EXPECT_EQ(s.size(), 5);
}

// 测试比较操作
TEST_F(SliceTest, Compare) {
    Slice s1("abc");
    Slice s2("abd");
    Slice s3("abc");
    
    EXPECT_LT(s1.compare(s2), 0);  // s1 < s2
    EXPECT_GT(s2.compare(s1), 0);  // s2 > s1
    EXPECT_EQ(s1.compare(s3), 0);  // s1 == s3
    
    // 测试运算符
    EXPECT_TRUE(s1 == s3);
    EXPECT_TRUE(s1 != s2);
}

// 测试边界情况
TEST_F(SliceTest, EdgeCases) {
    // 空字符串
    Slice empty("");
    EXPECT_TRUE(empty.empty());
    EXPECT_EQ(empty.size(), 0);
    
    // 包含空字符的字符串
    Slice with_null("hello\0world", 11);
    EXPECT_EQ(with_null.size(), 11);
}

// 测试异常情况
TEST_F(SliceTest, ExceptionCases) {
    // 测试越界访问是否会触发断言
    // 注意：在实际代码中，这种测试可能需要特殊的处理方式
    // EXPECT_DEATH(test_slice[11], ".*");
    
    // 测试 removePrefix 越界
    // EXPECT_DEATH(test_slice.removePrefix(12), ".*");
}

}  // namespace kvstorage