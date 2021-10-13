#include "../buffer/buffer.h"
#include <stdio.h>
#include <string.h>

static int main_ret = 0;
static int test_count = 0;
static int test_pass = 0;

#define EXPECT_EQ(actual, expect, format) \
    do {\
        ++test_count;\
        if ((actual == expect)) {\
            ++test_pass;\
        } else {\
            fprintf(stderr, "%s:%d: expect: " format " actual: " format "\n", \
                    __FILE__, __LINE__, expect, actual);\
            main_ret = 1;\
        }\
    } while(0)

#define EXPECT_STREQ(actual, expect) \
    do {\
        ++test_count;\
        if (!(strcmp(actual, expect))) {\
            ++test_pass;\
        } else {\
            fprintf(stderr, "%s:%d: expect: %s actual: %s\n", \
                    __FILE__, __LINE__, expect, actual);\
            main_ret = 1;\
        }\
    } while(0)

static void test_buffer() {
    Buffer buffer(100);
    const char *log = "word\n"; // 5个char
    const char *testLog;

    buffer.append(log, strlen(log));
    EXPECT_EQ(buffer.getReadableSize(), 5, "%d");
    EXPECT_EQ(buffer.getWritableSize(), 95, "%d");
    testLog = "word\n";
    EXPECT_STREQ(buffer.begin(), testLog);
    EXPECT_STREQ(buffer.getReadPtr(), testLog);
    testLog = "";
    EXPECT_STREQ(buffer.getWritePtr(), testLog);

    buffer.append(log, strlen(log));
    EXPECT_EQ(buffer.getReadableSize(), 10, "%d");
    EXPECT_EQ(buffer.getWritableSize(), 90, "%d");
    testLog = "word\nword\n";
    EXPECT_STREQ(buffer.begin(), testLog);
    EXPECT_STREQ(buffer.getReadPtr(), testLog);
    testLog = "";
    EXPECT_STREQ(buffer.getWritePtr(), testLog);

    return;
}

int main() {
    test_buffer();
    printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
    return main_ret;
}