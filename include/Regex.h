#pragma once

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <ctype.h>

/*
 * FORKED FROM: tiny-regex-c, see https://github.com/kokke/tiny-regex-c
 *
 * Supports:
 * ---------
 *   '.'        Dot, matches any character
 *   '^'        Start anchor, matches beginning of string
 *   '$'        End anchor, matches end of string
 *   '*'        Asterisk, match zero or more (greedy)
 *   '+'        Plus, match one or more (greedy)
 *   '*?'       Asterisk, match zero or more (lazy)
 *   '+?'       Plus, match one or more (lazy)
 *   '?'        Question, match zero or one (lazy)
 *   '{m,n}'    Quantifier, match min. 'm' and max. 'n' (greedy)
 *   '{m}'      Exactly 'm'
 *   '{m,}'     Match min 'm' and max.
 *   '[abc]'    Character class, match if one of {'a', 'b', 'c'}
 *   '[^abc]'   Inverted class, regexMatch if NOT one of {'a', 'b', 'c'}
 *   '[a-zA-Z]' Character ranges, the character set of the ranges { a-z | A-Z }
 *   '\s'       Whitespace, \t \f \r \n \v and spaces
 *   '\S'       Non-whitespace
 *   '\w'       Alphanumeric, [a-zA-Z0-9_]
 *   '\W'       Non-alphanumeric
 *   '\d'       Digits, [0-9]
 *   '\D'       Non-digits
 */

// Define to 0(false) if you DON'T want '.' to regexMatch '\r' + '\n'
#define REGEX_DOT_MATCH_NEWLINE true
#define MAX_REGEXP_OBJECTS  64    // Max number of regex symbols in expression.
#define MAX_CHAR_CLASS_LENGTH  64    // Max length of character-class buffer in. Determines the size of buffer for chars in all char-classes in the expression.

typedef enum RegexPatternType {
    REGEX_END_OF_PATTERN,     // is a sentinel used to indicate end-of-pattern
    REGEX_DOT,
    REGEX_BEGIN,
    REGEX_DOLLAR_END,
    REGEX_QUESTION_MARK,
    REGEX_STAR,
    REGEX_LAZY_STAR,
    REGEX_PLUS,
    REGEX_LAZY_PLUS,
    REGEX_REGULAR_CHAR,
    REGEX_CHAR_CLASS,
    REGEX_INVERSE_CHAR_CLASS,
    REGEX_DIGIT,
    REGEX_NOT_DIGIT,
    REGEX_ALPHA,
    REGEX_NOT_ALPHA,
    REGEX_WHITESPACE,
    REGEX_NOT_WHITESPACE,
    REGEX_QUANTIFIER
} RegexPatternType;

typedef struct RegexNode {
    RegexPatternType patternType;
    union {
        unsigned char regexChar;
        unsigned char *classCharPtr;
        uint16_t minMaxQuantifiers[2];  // {m,n} quantifier
    };
} RegexNode;

typedef struct Regex {
    struct RegexNode compiledRegexArray[MAX_REGEXP_OBJECTS];
    unsigned char classCharArray[MAX_CHAR_CLASS_LENGTH];
    bool isPatternValid;
    char *errorMessage;
} Regex;

typedef struct Matcher {
    int32_t foundAtIndex;
    int32_t matchLength;
    bool isFound;
} Matcher;

// Compile regex string pattern to a regex
void regexCompile(Regex *regex, const char *pattern);

// Find matches of the compiled pattern inside text.
Matcher regexMatch(Regex *regex, const char *text);