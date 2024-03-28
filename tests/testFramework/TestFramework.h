//
// Created by Ethan Horowitz on 11/4/23.
//

#ifndef SIMLED_TESTFRAMEWORK_H
#define SIMLED_TESTFRAMEWORK_H

#include <cstdio>
#include <string>

class __TFTest{
public:
    __TFTest(void (*test)(), const char* group, const char* name);
};
void __addTest(void (*test)(), const char* name);
void __setErrorFlag();
void RUN_TESTS();
void RUN_TEST_GROUP(std::string groupName);

#define TEST(group, name) void name(); __TFTest _##group_##name(name, #group, #name); void name()

#define ASSERT_EQ(a, b, display_specifier) if(a != b){printf("Assertion failed in file %s, line %d: " #display_specifier " != " #display_specifier "\n", __FILE__, __LINE__, a, b); exit(EXIT_FAILURE);}
#define ASSERT_NE(a, b, display_specifier) if(a == b){printf("Assertion failed in file %s, line %d: " #display_specifier " == " #display_specifier "\n", __FILE__, __LINE__, a, b); exit(EXIT_FAILURE);}
#define ASSERT_TRUE(a) if(!(a)){printf("Assertion failed in file %s, line %d: false\n", __FILE__, __LINE__); exit(EXIT_FAILURE);}
#define ASSERT_FALSE(a) if(a){printf("Assertion failed in file %s, line %d: true\n", __FILE__, __LINE__); exit(EXIT_FAILURE);}
#define ASSERT_LE(a, b) if(a > b){printf("Assertion failed in file %s, line %d: %d > %d\n", __FILE__, __LINE__, a, b); exit(EXIT_FAILURE);}
#define ASSERT_LT(a, b) if(a >= b){printf("Assertion failed in file %s, line %d: %d >= %d\n", __FILE__, __LINE__, a, b); exit(EXIT_FAILURE);}
#define ASSERT_GE(a, b) if(a < b){printf("Assertion failed in file %s, line %d: %d < %d\n", __FILE__, __LINE__, a, b); exit(EXIT_FAILURE);}
#define ASSERT_GT(a, b) if(a <= b){printf("Assertion failed in file %s, line %d: %d <= %d\n", __FILE__, __LINE__, a, b); exit(EXIT_FAILURE);}


#define EXPECT_EQ(a, b, display_specifier) if(a != b){printf("Assertion failed in file %s, line %d: " #display_specifier " != " #display_specifier "\n", __FILE__, __LINE__, a, b); __setErrorFlag();}
#define EXPECT_EQ_SPECIAL(a, b, display_specifier, dotOp1, dotOp2) if(a != b){printf("Assertion failed in file %s, line %d: " #display_specifier " != " #display_specifier "\n", __FILE__, __LINE__, a dotOp1, b dotOp2); __setErrorFlag();}
#define EXPECT_NE(a, b, display_specifier) if(a == b){printf("Assertion failed in file %s, line %d: " #display_specifier " == " #display_specifier "\n", __FILE__, __LINE__, a, b); __setErrorFlag();}
#define EXPECT_TRUE(a) if(!(a)){printf("Assertion failed in file %s, line %d: false\n", __FILE__, __LINE__); __setErrorFlag();}
#define EXPECT_FALSE(a) if(a){printf("Assertion failed in file %s, line %d: true\n", __FILE__, __LINE__); __setErrorFlag();}
#define EXPECT_LE(a, b) if(a > b){printf("Assertion failed in file %s, line %d: %d > %d\n", __FILE__, __LINE__, a, b); __setErrorFlag();}
#define EXPECT_LT(a, b) if(a >= b){printf("Assertion failed in file %s, line %d: %d >= %d\n", __FILE__, __LINE__, a, b); __setErrorFlag();}
#define EXPECT_GE(a, b) if(a < b){printf("Assertion failed in file %s, line %d: %d < %d\n", __FILE__, __LINE__, a, b); __setErrorFlag();}
#define EXPECT_GT(a, b) if(a <= b){printf("Assertion failed in file %s, line %d: %d <= %d\n", __FILE__, __LINE__, a, b); __setErrorFlag();}

#define FAIL() printf("Test failed in file %s, line %d\n", __FILE__, __LINE__); __setErrorFlag(); exit(EXIT_FAILURE);

#endif //SIMLED_TESTFRAMEWORK_H
