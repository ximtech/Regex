#pragma once

#include "BaseTestTemplate.h"
#include "Regex.h"

#define OK    true
#define FAIL   false

typedef struct RegexPatternTestObject {
    bool isOK;
    char *pattern;
    char *text;
    int32_t matchLength;
    int32_t foundAtIndex;
} RegexPatternTestObject;

typedef struct RegexCompileTestObject {
    bool isOK;
    char *pattern;
    char *errorMessage;
} RegexCompileTestObject;


static const RegexPatternTestObject REGEX_PATTERN_TEST_ARRAY[] = {
        // isOK   pattern                  text                                 matchLength  foundAtIndex
        // Tests from Perl
        {FAIL, "",                          "",                                          0,  0},
        {OK,   "abc",                       "abc",                                       3,  0},
        {FAIL, "abc",                       "xbc",                                       0,  0},
        {FAIL, "abc",                       "axc",                                       0,  0},
        {FAIL, "abc",                       "abx",                                       0,  0},
        {OK,   "abc",                       "xabcy",                                     3,  1},
        {OK,   "abc",                       "ababc",                                     3,  2},
        {OK,   "ab*c",                      "abc",                                       3,  0},
        {OK,   "ab*bc",                     "abc",                                       3,  0},
        {OK,   "ab*bc",                     "abbc",                                      4,  0},
        {OK,   "ab*bc",                     "abbbbc",                                    6,  0},
        {OK,   "ab{0,}bc",                  "abbbbc",                                    6,  0},
        {OK,   "ab+bc",                     "abbc",                                      4,  0},
        {FAIL, "ab+bc",                     "abc",                                       0,  0},
        {FAIL, "ab+bc",                     "abq",                                       0,  0},
        {FAIL, "ab{1,}bc",                  "abq",                                       0,  0},
        {OK,   "ab+bc",                     "abbbbc",                                    6,  0},
        {OK,   "ab{1,}bc",                  "abbbbc",                                    6,  0},
        {OK,   "ab{1,3}bc",                 "abbbbc",                                    6,  0},
        {OK,   "ab{3,4}bc",                 "abbbbc",                                    6,  0},
        {FAIL, "ab{4,5}bc",                 "abbbbc",                                    0,  0},
        {OK,   "ab?bc",                     "abbc",                                      4,  0},
        {OK,   "ab?bc",                     "abc",                                       3,  0},
        {OK,   "ab{0,1}bc",                 "abc",                                       3,  0},
        {FAIL, "ab?bc",                     "abbbbc",                                    0,  0},
        {OK,   "ab?c",                      "abc",                                       3,  0},
        {OK,   "ab{0,1}c",                  "abc",                                       3,  0},
        {OK,   "^abc$",                     "abc",                                       3,  0},
        {FAIL, "^abc$",                     "abcc",                                      0,  0},
        {OK,   "^abc",                      "abcc",                                      3,  0},
        {FAIL, "^abc$",                     "aabc",                                      0,  0},
        {OK,   "abc$",                      "aabc",                                      3,  1},
        {FAIL, "\\^",                       "abc",                                       0,  0},
        {FAIL, "\\$",                       "abc",                                       0,  0},
        {OK,   "a.c",                       "abc",                                       3,  0},
        {OK,   "a.c",                       "axc",                                       3,  0},
        {OK,   "a.*c",                      "axyzc",                                     5,  0},
        {FAIL, "a.*c",                      "axyzd",                                     0,  0},
        {FAIL, "a[bc]d",                    "abc",                                       0,  0},
        {OK,   "a[bc]d",                    "abd",                                       3,  0},
        {FAIL, "a[b-d]e",                   "abd",                                       0,  0},
        {OK,   "a[b-d]e",                   "ace",                                       3,  0},
        {OK,   "a[b-d]",                    "aac",                                       2,  1},
        {OK,   "a[-d]",                     "a-",                                        2,  0},
        {OK,   "a[b-]",                     "a-",                                        2,  0},
        {OK,   "a[b-]",                     "a-",                                        2,  0},
        {OK,   "a]",                        "a]",                                        2,  0},
        {OK,   "a[\\]]b",                   "a]b",                                       3,  0},
        {OK,   "a[^bc]d",                   "aed",                                       3,  0},
        {FAIL, "a[^bc]d",                   "abd",                                       0,  0},
        {OK,   "a[^-b]c",                   "adc",                                       3,  0},
        {FAIL, "a[^-b]c",                   "a-c",                                       0,  0},
        {FAIL, "a[^\\]]c",                  "a]c",                                       0,  0},
        {OK,   "a[^\\]b]c",                 "adc",                                       3,  0},
        {FAIL, "$b",                        "b",                                         0,  0},
        {OK,   "a\\(b",                     "a(b",                                       3,  0},
        {OK,   "a\\(*b",                    "ab",                                        2,  0},
        {OK,   "a\\(*b",                    "a((b",                                      4,  0},
        {OK,   "a\\\\b",                    "a\\b",                                      3,  0},
        {OK,   "a+b+c",                     "aabbabc",                                   3,  4},
        {OK,   "a{1,}b{1,}c",               "aabbabc",                                   3,  4},
        {OK,   "a.+?c",                     "abcabc",                                    3,  0},
        {OK,   "[^ab]*",                    "cde",                                       3,  0},
        {FAIL, "abc",                       "",                                          0,  0},
        {FAIL, "a*",                        "",                                          0,  0},    // this can be ok
        {OK,   "a[bcd]*dcdcde",             "adcdcde",                                   7,  0},
        {FAIL, "a[bcd]+dcdcde",             "adcdcde",                                   0,  0},
        {OK,   "[a-zA-Z_][a-zA-Z0-9_]*",    "alpha",                                     5,  0},
        {FAIL, "multiple words of text",    "uh-uh",                                     0,  0},
        {OK,   "multiple words",            "multiple words, yeah",                      14, 0},
        {FAIL, "[k]",                       "ab",                                        0,  0},
        {OK,   "a[-]?c",                    "ac",                                        2,  0},
        {OK,   "a[bcd]*dcdcde",             "adcdcde",                                   7,  0},
        {FAIL, "a[bcd]+dcdcde",             "adcdcde",                                   0,  0},
        {OK,   "[abhgefdc]ij",              "hij",                                       3,  0},
        {OK,   "abcd*efg",                  "abcdefg",                                   7,  0},
        {OK,   "ab*",                       "xabyabbbz",                                 2,  1},
        {OK,   "ab*",                       "xayabbbz",                                  1,  1},
        {OK,   "ab*",                       "xsyabbbz",                                  4,  3},

        // All tests from tiny-regex
        {OK,   "\\d",                       "5",                                         1,  0},
        {OK,   "\\w+",                      "hej",                                       3,  0},
        {OK,   "\\s",                       "\t \n",                                     1,  0},
        {FAIL, "\\S",                       "\t \n",                                     0,  0},
        {OK,   "[\\s]",                     "\t \n",                                     1,  0},
        {FAIL, "[\\S]",                     "\t \n",                                     0,  0},
        {FAIL, "\\D",                       "5",                                         0,  0},
        {FAIL, "\\W+",                      "hej",                                       0,  0},
        {OK,   "[0-9]+",                    "12345",                                     5,  0},
        {OK,   "\\D",                       "hej",                                       1,  0},
        {FAIL, "\\d",                       "hej",                                       0,  0},
        {OK,   "[^\\w]",                    "\\",                                        1,  0},
        {OK,   "[\\W]",                     "\\",                                        1,  0},
        {FAIL, "[\\w]",                     "\\",                                        0,  0},
        {OK,   "[^\\d]",                    "d",                                         1,  0},
        {FAIL, "[\\d]",                     "d",                                         0,  0},
        {FAIL, "[^\\D]",                    "d",                                         0,  0},
        {OK,   "[\\D]",                     "d",                                         1,  0},
        {OK,   "^.*\\\\.*$",                "c:\\Tools",                                 8,  0},
        {OK,   "^[\\+-]*[\\d]+$",           "+27",                                       3,  0},
        {OK,   "[abc]",                     "1c2",                                       1,  1},
        {OK,   "^abc",                      "abcde",                                     3,  0},
        {FAIL, "[abc]",                     "1C2",                                       0,  0},
        {OK,   "[1-5]+",                    "0123456789",                                5,  1},
        {OK,   "[.2]",                      "1C2",                                       1,  2},
        {OK,   "a*$",                       "Xaa",                                       2,  1},
        {OK,   "[a-h]+",                    "abcdefghxxx",                               8,  0},
        {FAIL, "[a-h]+",                    "ABCDEFGH",                                  0,  0},
        {OK,   "[A-H]+",                    "ABCDEFGH",                                  8,  0},
        {FAIL, "[A-H]+",                    "abcdefgh",                                  0,  0},
        {OK,   "[^\\s]+",                   "abc def",                                   3,  0},
        {OK,   "[^fc]+",                    "abc def",                                   2,  0},
        {OK,   "[^d\\sf]+",                 "abc def",                                   3,  0},
        {OK,   "\n",                        "abc\ndef",                                  1,  3},
        {OK,   "b.\\s*\n",                  "aa\r\nbb\r\ncc\r\n\r\n",                    4,  4},
        {OK,   ".*c",                       "abcabc",                                    6,  0},
        {OK,   ".+c",                       "abcabc",                                    6,  0},
        {OK,   "[b-z].*",                   "ab",                                        1,  1},
        {OK,   "b[k-z]*",                   "ab",                                        1,  1},
        {FAIL, "[0-9]",                     "  - ",                                      0,  0},
        {OK,   "[^0-9]",                    "  - ",                                      1,  0},
        {OK,   "0|",                        "0|",                                        2,  0},
        {FAIL, "\\d\\d:\\d\\d:\\d\\d",      "0s:00:00",                                  0,  0},
        {FAIL, "\\d\\d:\\d\\d:\\d\\d",      "000:00",                                    0,  0},
        {FAIL, "\\d\\d:\\d\\d:\\d\\d",      "00:0000",                                   0,  0},
        {FAIL, "\\d\\d:\\d\\d:\\d\\d",      "100:0:00",                                  0,  0},
        {FAIL, "\\d\\d:\\d\\d:\\d\\d",      "00:100:00",                                 0,  0},
        {FAIL, "\\d\\d:\\d\\d:\\d\\d",      "0:00:100",                                  0,  0},
        {OK,   "\\d\\d?:\\d\\d?:\\d\\d?",   "0:0:0",                                     5,  0},
        {OK,   "\\d\\d?:\\d\\d?:\\d\\d?",   "0:00:0",                                    6,  0},
        {OK,   "\\d\\d?:\\d\\d?:\\d\\d?",   "0:0:00",                                    5,  0},
        {OK,   "\\d\\d?:\\d\\d?:\\d\\d?",   "00:0:0",                                    6,  0},
        {OK,   "\\d\\d?:\\d\\d?:\\d\\d?",   "00:00:0",                                   7,  0},
        {OK,   "\\d\\d?:\\d\\d?:\\d\\d?",   "00:0:00",                                   6,  0},
        {OK,   "\\d\\d?:\\d\\d?:\\d\\d?",   "0:00:00",                                   6,  0},
        {OK,   "\\d\\d?:\\d\\d?:\\d\\d?",   "00:00:00",                                  7,  0},
        {OK,   "[Hh]ello [Ww]orld\\s*[!]?", "Hello world !",                             12, 0},
        {OK,   "[Hh]ello [Ww]orld\\s*[!]?", "hello world !",                             12, 0},
        {OK,   "[Hh]ello [Ww]orld\\s*[!]?", "Hello World !",                             12, 0},
        {OK,   "[Hh]ello [Ww]orld\\s*[!]?", "Hello world!   ",                           11, 0},
        {OK,   "[Hh]ello [Ww]orld\\s*[!]?", "Hello world  !",                            13, 0},
        {OK,   "[Hh]ello [Ww]orld\\s*[!]?", "hello World    !",                          15, 0},
        {FAIL, "\\d\\d?:\\d\\d?:\\d\\d?",   "a:0",                                       0,  0},
        {OK,   "[^\\w][^-1-4]",             ")T",                                        2,  0},
        {OK,   "[^\\w][^-1-4]",             ")^",                                        2,  0},
        {OK,   "[^\\w][^-1-4]",             "*)",                                        2,  0},
        {OK,   "[^\\w][^-1-4]",             "!.",                                        2,  0},
        {OK,   "[^\\w][^-1-4]",             " x",                                        2,  0},
        {OK,   "[^\\w][^-1-4]",             "$b",                                        2,  0},
        {OK,   ".?bar",                     "real_bar",                                  4,  4},
        {FAIL, ".?bar",                     "real_foo",                                  0,  0},
        {FAIL, "X?Y",                       "Z",                                         0,  0},
        {OK,   "[a-z]+\nbreak",             "blahblah\nbreak",                           14, 0},
        {OK,   "[a-z\\s]+\nbreak",          "bla bla \nbreak",                           14, 0},
        {OK,   "\\d+? \\d+?",               "123 456",                                   5,  0},
        {OK,   "ba+?",                      "b gh jj  badd fa",                          2,  9},
        {FAIL, "ba+?",                      "b gh jj  bdd fa",                           0,  0},
        {OK,   "\".*?\"",                   "a \"witch\" and her \"broom\" is one",      7,  2},
        {OK,   "\".*\"",                    "a \"witch\" and her \"broom\" is one",      23, 2},
        {OK,   "<.*?>",                     "<p> <b> Wikipedia </b> encyclopedia. </p>", 3,  0},
        {OK,   "<[^>]*>",                   "<p> <b> Wikipedia </b> encyclopedia. </p>", 3,  0},
        {OK,   "<.*>",                      "<p> <b> Wikipedia </b> encyclopedia. </p>", 41, 0},
        {OK,   "[^sfdgk]ello",              "ahem.. 'hello world !' ..",                 5,  8},
        {OK,   "[^56789]{1,5}/test",        "1234/test",                                 9,  0},
        {OK,   ".*a.*a",                    "aa",                                        2,  0}, // https://github.com/kokke/tiny-regex-c/issues/53
        {OK,   ".*a.*a",                    "Xaa",                                       3,  0},
        {OK,   "ab?bc",                     "abc",                                       3,  0},
        {OK,   "\\{\\w+\\}",                "{{FW}_TEST",                                4,  1}, // https://github.com/kokke/tiny-regex-c/issues/69
        {OK,   "\\d{15}",                   "869759002514931",                           15, 0}, // https://github.com/kokke/tiny-regex-c/issues/21
        {OK,   "[0-9]{15}",                 "869759002514931",                           15, 0},
        {OK,   "^[0-9]{15}$",               "869759002514931",                           15, 0},
};


static const RegexCompileTestObject REGEX_COMPILER_TEST_ARRAY[] = {
        {OK, "^abc.*$", "Success"},
        {FAIL, NULL, "NULL pattern string"},
        {FAIL, "abc$*", "Non-quantifiable before '*'"},
        {FAIL, "abc$+", "Non-quantifiable before '+'"},
        {FAIL, "abc$?", "Non-quantifiable before '?'"},
        {FAIL, "\\\x01[^\\\xff][^", "Incomplete pattern, missing non-zero char after '^'"},
        {FAIL, "\\\x01[^\\\xff][\\", "Incomplete pattern, missing non-zero char after '\\'"},
        {FAIL, "abc[\\wa+", "Non terminated class ']'"},
        {FAIL, "a+${1,2}", "Non-quantifiable before '{m,n}'"},
        {FAIL, "abc{", "Dangling '{' quantifier"},
        {FAIL, "abc{a,1}", "Non-digit min value in quantifier"},
        {FAIL, "abc{1,a}", "Non-digit max value in quantifier"},
        {FAIL, "abc{13454545345}", "Min value too big in quantifier"},
        {FAIL, "abc{6,", "Dangling ',' quantifier"},
        {FAIL, "abc{6,345346546768}", "Max value too big or less than min value in quantifier"},
};


static MunitResult regexPatternTest(const MunitParameter params[], void *data) {
    for (int i = 0; i < ARRAY_SIZE(REGEX_PATTERN_TEST_ARRAY); i++) {
        Regex regex = {0};
        RegexPatternTestObject testObject = REGEX_PATTERN_TEST_ARRAY[i];
        regexCompile(&regex, testObject.pattern);
        Matcher matcher = regexMatch(&regex, testObject.text);

        bool isMatchLengthValid = testObject.isOK == OK ? matcher.matchLength == testObject.matchLength : true;
        bool isFound = testObject.isOK == OK ? matcher.foundAtIndex == testObject.foundAtIndex : true;

        if (matcher.isFound != testObject.isOK || !isMatchLengthValid || !isFound) {
            printf(". Test No: [%d], Pattern: [%s], Text: [%s]", i, testObject.pattern, testObject.text);
            if (!isMatchLengthValid) {
                printf(", Matched. Actual: [%d], Expected: [%d]", matcher.matchLength, testObject.matchLength);
            }

            if (!isFound) {
                printf(", FoundAt. Actual: [%d], Expected: [%d]", matcher.foundAtIndex, testObject.foundAtIndex);
            }
            return MUNIT_FAIL;
        }
    }

    return MUNIT_OK;
}


static MunitResult regexCompileTest(const MunitParameter params[], void *data) {
    for (int i = 0; i < ARRAY_SIZE(REGEX_COMPILER_TEST_ARRAY); i++) {
        Regex regex = {0};
        RegexCompileTestObject compileTestObject = REGEX_COMPILER_TEST_ARRAY[i];
        regexCompile(&regex, compileTestObject.pattern);

        assert_true(regex.isPatternValid == compileTestObject.isOK);
        assert_string_equal(regex.errorMessage, compileTestObject.errorMessage);
    }

    return MUNIT_OK;
}


static MunitTest regexTests[] = {
        {.name = "Test OK regexMatch() - Check pattern match", .test = regexPatternTest},
        {.name = "Test OK regexCompile() - Check pattern errors", .test = regexCompileTest},

        END_OF_TESTS
};

static const MunitSuite regexTestSuite = {
        .prefix = "Regex: ",
        .tests = regexTests,
        .suites = NULL,
        .iterations = 1,
        .options = MUNIT_SUITE_OPTION_NONE
};
