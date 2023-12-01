#include <gtest/gtest.h>

#include "../include/bej_decoder.h"

TEST(decode, invalidFile)
{
    char* dictionary = "invalid_dict.dict";
    char* bejFile = "invalid_file.bin";
    EXPECT_TRUE(bejDecode(dictionary, &bejFile, 1));
}

int main()
{
    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}