#include "Regex.h"

#define END_LINE '\0'
#define MAX_QUANTIFICATION_VALUE  1024  // Max in {M,N} - Denotes the minimum M and the maximum N regexMatch count.

typedef struct InnerRegexCompiler {     // The sizes of the two static arrays below substantiates the static RAM usage of this module.
    Regex *regex;
    uint16_t regexIndex;
    uint16_t patternIndex;
    uint16_t classCharIndex;
    bool isQuantifiable;    // is the last node quantifiable
} RegexCompiler;

static inline void setBeginMetaChar(RegexCompiler *regexCompiler);
static inline void setDollarEndMetaChar(RegexCompiler *regexCompiler);
static inline void setDotMetaChar(RegexCompiler *regexCompiler);
static inline void setStarMetaChar(RegexCompiler *regexCompiler, const char *pattern);
static inline void setPlusMetaChar(RegexCompiler *regexCompiler, const char *pattern);
static inline void setQuestionMarkMetaChar(RegexCompiler *regexCompiler);
static inline void setRegexPatternType(RegexPatternType patternType, RegexCompiler *regexCompiler);
static void resolveEscapedCharacterClasses(RegexCompiler *regexCompiler, const char *pattern);
static void resolveCharacterClass(RegexCompiler *regexCompiler, const char *pattern);
static void resolveQuantification(RegexCompiler *regexCompiler, const char *pattern);
static inline void setRegularChar(RegexCompiler *regexCompiler, char charInPattern);

static bool matchPattern(RegexNode *regex, Matcher *matcher, const char *text);
static bool matchQuestionMark(RegexNode *regex, RegexNode *pattern, const char *text, Matcher *matcher);
static bool matchQuantifier(RegexNode *regex, RegexNode *pattern, const char *text, Matcher *matcher);

static bool matchStar(RegexNode *regex, RegexNode *pattern, const char *text, Matcher *matcher);
static bool matchStarLazy(RegexNode *regex, RegexNode *pattern, const char *text, Matcher *matcher);

static bool matchPlus(RegexNode *regex, RegexNode *pattern, const char *text, Matcher *matcher);
static bool matchPlusLazy(RegexNode *regex, RegexNode *pattern, const char *text, Matcher *matcher);

static bool matchOne(RegexNode *regex, char character);
static inline bool isMatchingDot(unsigned char character);
static bool matchCharClass(unsigned char character, const unsigned char *metaCharString);
static inline bool isMatchingRange(unsigned char character, const unsigned char *string);
static bool isMatchingMetaChar(unsigned char character, const unsigned char *metaCharString);


void regexCompile(Regex *regex, const char *pattern) {
    if (regex == NULL) return;
    regex->isPatternValid = true;
    regex->errorMessage = "Success";
    RegexCompiler regexCompiler = {
            .classCharIndex = 0,
            .patternIndex = 0,
            .regexIndex = 0,
            .isQuantifiable = false,
            .regex = regex
    };

    if (pattern == NULL) {
        regex->isPatternValid = false;
        regex->errorMessage = "NULL pattern string";
        return;
    }

    while (pattern[regexCompiler.patternIndex] != END_LINE && ((regexCompiler.regexIndex + 1) < MAX_REGEXP_OBJECTS)) {
        char charInPattern = pattern[regexCompiler.patternIndex];

        switch (charInPattern) {
            case '^':   // Meta-characters
                setBeginMetaChar(&regexCompiler);
                break;
            case '$':
                setDollarEndMetaChar(&regexCompiler);
                break;
            case '.':
                setDotMetaChar(&regexCompiler);
                break;
            case '*':
                setStarMetaChar(&regexCompiler, pattern);
                break;
            case '+':
                setPlusMetaChar(&regexCompiler, pattern);
                break;
            case '?':
                setQuestionMarkMetaChar(&regexCompiler);
                break;
            case '\\':  // Escaped characters
                resolveEscapedCharacterClasses(&regexCompiler, pattern);
                break;
            case '[':   // Character class
                resolveCharacterClass(&regexCompiler, pattern);
                break;
            case '{':   // Quantifier
                resolveQuantification(&regexCompiler, pattern);
                break;
            default:    // Regular characters
                setRegularChar(&regexCompiler, charInPattern);
        }

        if (!regex->isPatternValid) {
            return;
        }
        regexCompiler.patternIndex++;
        regexCompiler.regexIndex++;
    }

    setRegexPatternType(REGEX_END_OF_PATTERN, &regexCompiler);
}

Matcher regexMatch(Regex *regex, const char *text) {
    Matcher matcher = {.foundAtIndex = 0, .matchLength = 0, .isFound = false};
    if (regex == NULL || !regex->isPatternValid) return matcher;

    if (regex->compiledRegexArray[0].patternType == REGEX_BEGIN) {
        matcher.isFound = matchPattern(regex->compiledRegexArray + 1, &matcher, text);
        return matcher;
    }

    do {
        if (matchPattern(regex->compiledRegexArray, &matcher, text)) {
            if (*text == END_LINE) {
                matcher.isFound = false;
                return matcher;
            }
            matcher.isFound = true;
            return matcher;
        }
        matcher.foundAtIndex++;
    } while (*text++ != END_LINE);

    return matcher;
}


static inline void setBeginMetaChar(RegexCompiler *regexCompiler) {
    regexCompiler->isQuantifiable = false;
    setRegexPatternType(REGEX_BEGIN, regexCompiler);
}

static inline void setDollarEndMetaChar(RegexCompiler *regexCompiler) {
    regexCompiler->isQuantifiable = false;
    setRegexPatternType(REGEX_DOLLAR_END, regexCompiler);
}

static inline void setDotMetaChar(RegexCompiler *regexCompiler) {
    regexCompiler->isQuantifiable = true;
    setRegexPatternType(REGEX_DOT, regexCompiler);
}

static inline void setStarMetaChar(RegexCompiler *regexCompiler, const char *pattern) {
    if (!regexCompiler->isQuantifiable) {
        regexCompiler->regex->isPatternValid = false;
        regexCompiler->regex->errorMessage = "Non-quantifiable before '*'";
        return;
    }
    regexCompiler->isQuantifiable = false;

    if (pattern[regexCompiler->patternIndex + 1] == '?') {
        setRegexPatternType(REGEX_LAZY_STAR, regexCompiler);
        regexCompiler->patternIndex++;
    } else {
        setRegexPatternType(REGEX_STAR, regexCompiler);
    }
}

static inline void setPlusMetaChar(RegexCompiler *regexCompiler, const char *pattern) {
    if (!regexCompiler->isQuantifiable) {
        regexCompiler->regex->isPatternValid = false;
        regexCompiler->regex->errorMessage = "Non-quantifiable before '+'";
        return;
    }

    if (pattern[regexCompiler->patternIndex + 1] == '?') {
        setRegexPatternType(REGEX_LAZY_PLUS, regexCompiler);
        regexCompiler->patternIndex++;
    } else {
        setRegexPatternType(REGEX_PLUS, regexCompiler);
    }
}

static inline void setQuestionMarkMetaChar(RegexCompiler *regexCompiler) {
    if (!regexCompiler->isQuantifiable) {
        regexCompiler->regex->isPatternValid = false;
        regexCompiler->regex->errorMessage = "Non-quantifiable before '?'";
        return;
    }
    setRegexPatternType(REGEX_QUESTION_MARK, regexCompiler);
}

static inline void setRegexPatternType(RegexPatternType patternType, RegexCompiler *regexCompiler) {
    regexCompiler->regex->compiledRegexArray[regexCompiler->regexIndex].patternType = patternType;
}

static void resolveEscapedCharacterClasses(RegexCompiler *regexCompiler, const char *pattern) {
    if (pattern[regexCompiler->patternIndex + 1] != END_LINE) {
        regexCompiler->isQuantifiable = true;
        regexCompiler->patternIndex++;   // Skip the escape-char

        RegexPatternType patternType;
        switch (pattern[regexCompiler->patternIndex]) {
            case 's':
                patternType = REGEX_WHITESPACE;
                break;
            case 'S':
                patternType = REGEX_NOT_WHITESPACE;
                break;
            case 'w':
                patternType = REGEX_ALPHA;
                break;
            case 'W':
                patternType = REGEX_NOT_ALPHA;
                break;
            case 'd':
                patternType = REGEX_DIGIT;
                break;
            case 'D':
                patternType = REGEX_NOT_DIGIT;
                break;
            default:
                patternType = 0;
                break;
        }

        if (patternType > 0) {  // Check the next Meta-character:
            setRegexPatternType(patternType, regexCompiler);
        } else {
            setRegexPatternType(REGEX_REGULAR_CHAR, regexCompiler); // Escaped character, e.g. '.' or '$'
            regexCompiler->regex->compiledRegexArray[regexCompiler->regexIndex].regexChar = pattern[regexCompiler->patternIndex];
        }
    }
}

static void resolveCharacterClass(RegexCompiler *regexCompiler, const char *pattern) {
    uint16_t bufferBegin = regexCompiler->classCharIndex;    // Remember where the char-buffer starts.
    regexCompiler->patternIndex++;  // Skip '['
    regexCompiler->isQuantifiable = true;

    if (pattern[regexCompiler->patternIndex] == '^') {    // Look-ahead to determine if negated
        setRegexPatternType(REGEX_INVERSE_CHAR_CLASS, regexCompiler);
        regexCompiler->patternIndex++; // Increment index to avoid including '^' in the char-buffer
        if (pattern[regexCompiler->patternIndex] == END_LINE) {
            regexCompiler->regex->isPatternValid = false;
            regexCompiler->regex->errorMessage = "Incomplete pattern, missing non-zero char after '^'";
            return;
        }
    } else {
        setRegexPatternType(REGEX_CHAR_CLASS, regexCompiler);
    }

    while (pattern[regexCompiler->patternIndex] != END_LINE && pattern[regexCompiler->patternIndex] != ']') {
        char charInPattern = pattern[regexCompiler->patternIndex];

        if (charInPattern == '\\') {
            if (regexCompiler->classCharIndex >= MAX_CHAR_CLASS_LENGTH - 1 || pattern[regexCompiler->patternIndex + 1] == END_LINE) {
                regexCompiler->regex->isPatternValid = false;
                regexCompiler->regex->errorMessage = "Incomplete pattern, missing non-zero char after '\\'";
                return;
            }
            regexCompiler->regex->classCharArray[regexCompiler->classCharIndex] = pattern[regexCompiler->patternIndex];
            regexCompiler->classCharIndex++;
            regexCompiler->patternIndex++;

        } else if (regexCompiler->classCharIndex >= MAX_CHAR_CLASS_LENGTH) {
            regexCompiler->regex->isPatternValid = false;
            regexCompiler->regex->errorMessage = "Exceeded internal buffer";
            return;
        }
        regexCompiler->regex->classCharArray[regexCompiler->classCharIndex] = pattern[regexCompiler->patternIndex++];
        regexCompiler->classCharIndex++;
    }

    if (regexCompiler->classCharIndex >= MAX_CHAR_CLASS_LENGTH) { // Check for too long patterns. Such as [00000000000000000000000000000000000000][
        regexCompiler->regex->isPatternValid = false;
        regexCompiler->regex->errorMessage = "Too long char class pattern";
        return;
    }

    if (pattern[regexCompiler->patternIndex] != ']') {
        regexCompiler->regex->isPatternValid = false;
        regexCompiler->regex->errorMessage = "Non terminated class ']'";
        return;
    }
    regexCompiler->regex->classCharArray[regexCompiler->classCharIndex] = END_LINE;// Null-terminate string end
    regexCompiler->regex->compiledRegexArray[regexCompiler->regexIndex].classCharPtr = &regexCompiler->regex->classCharArray[bufferBegin];
    regexCompiler->classCharIndex++;
}

static void resolveQuantification(RegexCompiler *regexCompiler, const char *pattern) {
    if (!regexCompiler->isQuantifiable) {
        regexCompiler->regex->isPatternValid = false;
        regexCompiler->regex->errorMessage = "Non-quantifiable before '{m,n}'";
        return;
    }
    regexCompiler->patternIndex++;  // Skip '{'

    if (pattern[regexCompiler->patternIndex] == END_LINE) {
        regexCompiler->regex->isPatternValid = false;
        regexCompiler->regex->errorMessage = "Dangling '{' quantifier";
        return;
    }

    uint32_t minQuantifierValue = 0;
    do
    {
        char quantifierValueChar = pattern[regexCompiler->patternIndex];
        if (!isdigit(quantifierValueChar)) {
            regexCompiler->regex->isPatternValid = false;
            regexCompiler->regex->errorMessage = "Non-digit min value in quantifier";
            return;
        }
        minQuantifierValue = 10 * minQuantifierValue + (unsigned) (quantifierValueChar - '0');
        regexCompiler->patternIndex++;
    }
    while (pattern[regexCompiler->patternIndex] != ',' && pattern[regexCompiler->patternIndex] != '}');

    if (minQuantifierValue > MAX_QUANTIFICATION_VALUE) {
        regexCompiler->regex->isPatternValid = false;
        regexCompiler->regex->errorMessage = "Min value too big in quantifier";
        return;
    }
    regexCompiler->regex->compiledRegexArray[regexCompiler->regexIndex].minMaxQuantifiers[0] = minQuantifierValue;

    if (pattern[regexCompiler->patternIndex] == ',') {
        regexCompiler->patternIndex++;  // Skip ','
        if (pattern[regexCompiler->patternIndex] == END_LINE) {
            regexCompiler->regex->isPatternValid = false;
            regexCompiler->regex->errorMessage = "Dangling ',' quantifier";
            return;
        }

        if (pattern[regexCompiler->patternIndex] == '}') {
            regexCompiler->regex->compiledRegexArray[regexCompiler->regexIndex].minMaxQuantifiers[1] = MAX_QUANTIFICATION_VALUE;
        } else {

            uint32_t maxQuantifierValue = 0;
            while (pattern[regexCompiler->patternIndex] != '}') {
                char quantifierValueChar = pattern[regexCompiler->patternIndex];
                if (quantifierValueChar == END_LINE || !isdigit(quantifierValueChar)) {
                    regexCompiler->regex->isPatternValid = false;
                    regexCompiler->regex->errorMessage = "Non-digit max value in quantifier";
                    return;
                }

                maxQuantifierValue = 10 * maxQuantifierValue + (unsigned) (quantifierValueChar - '0');
                regexCompiler->patternIndex++;
            }

            if (maxQuantifierValue > MAX_QUANTIFICATION_VALUE || maxQuantifierValue < minQuantifierValue) {
                regexCompiler->regex->isPatternValid = false;
                regexCompiler->regex->errorMessage = "Max value too big or less than min value in quantifier";
                return;
            }
            regexCompiler->regex->compiledRegexArray[regexCompiler->regexIndex].minMaxQuantifiers[1] = maxQuantifierValue;
        }
    }

    setRegexPatternType(REGEX_QUANTIFIER, regexCompiler);
}

static inline void setRegularChar(RegexCompiler *regexCompiler, char charInPattern) {
    regexCompiler->isQuantifiable = true;
    setRegexPatternType(REGEX_REGULAR_CHAR, regexCompiler);
    regexCompiler->regex->compiledRegexArray[regexCompiler->regexIndex].regexChar = charInPattern;
}

static bool matchPattern(RegexNode *regex, Matcher *matcher, const char *text) {
    int32_t previousMatch = matcher->matchLength;

    do {
        if (regex[0].patternType == REGEX_END_OF_PATTERN || regex[1].patternType == REGEX_QUESTION_MARK) {
            return matchQuestionMark(regex, (regex + 2), text, matcher);

        } else if (regex[1].patternType == REGEX_QUANTIFIER) {
            return matchQuantifier(regex, (regex + 1), text, matcher);

        } else if (regex[1].patternType == REGEX_STAR) {
            return matchStar(regex, (regex + 2), text, matcher);

        } else if (regex[1].patternType == REGEX_LAZY_STAR) {
            return matchStarLazy(regex, (regex + 2), text, matcher);

        } else if (regex[1].patternType == REGEX_PLUS) {
            return matchPlus(regex, (regex + 2), text, matcher);

        } else if (regex[1].patternType == REGEX_LAZY_PLUS) {
            return matchPlusLazy(regex, (regex + 2), text, matcher);

        } else if ((regex[0].patternType == REGEX_DOLLAR_END) && regex[1].patternType == REGEX_END_OF_PATTERN) {
            return (text[0] == END_LINE);
        }

        matcher->matchLength++;
    } while (text[0] != END_LINE && matchOne(regex++, *text++));

    matcher->matchLength = previousMatch;
    return false;
}


static bool matchQuestionMark(RegexNode *regex, RegexNode *pattern, const char *text, Matcher *matcher) {
    if (regex->patternType == REGEX_END_OF_PATTERN || matchPattern(pattern, matcher, text)) {
        return true;
    }

    if (*text && matchOne(regex, *text++)) {
        if (matchPattern(pattern, matcher, text)) {
            matcher->matchLength++;
            return true;
        }
    }
    return false;
}

static bool matchQuantifier(RegexNode *regex, RegexNode *pattern, const char *text, Matcher *matcher) {
    int32_t preLength = matcher->matchLength;
    uint16_t minQuantifier = pattern->minMaxQuantifiers[0];
    int32_t maxQuantifier = pattern->minMaxQuantifiers[1] - minQuantifier;

    while (text[0] != END_LINE && minQuantifier > 0 && matchOne(regex, *text)) {
        matcher->matchLength++;
        minQuantifier--;
        text++;
    }

    if (minQuantifier > 0) {
        return false;
    }

    do {
        if (matchPattern(pattern + 1, matcher, text)) {
            return true;
        }
        matcher->matchLength++;
    } while (text[0] != END_LINE && maxQuantifier-- > 0 && matchOne(regex, *text++));

    matcher->matchLength = preLength;
    return false;
}

static bool matchStar(RegexNode *regex, RegexNode *pattern, const char *text, Matcher *matcher) {
    const char *prePoint = text;
    while (text[0] != END_LINE && matchOne(regex, *text)) {
        matcher->matchLength++;
        text++;
    }

    if (matcher->matchLength == 0) {
        return false;
    }

    while (text >= prePoint) {
        if (matchPattern(pattern, matcher, text--)) {
            return true;
        }
        matcher->matchLength--;
    }
    return false;
}

static bool matchStarLazy(RegexNode *regex, RegexNode *pattern, const char *text, Matcher *matcher) {
    int32_t preLength = matcher->matchLength;

    do {
        if (matchPattern(pattern, matcher, text)) {
            return true;
        }
        matcher->matchLength++;
    } while (text[0] != END_LINE && matchOne(regex, *text++));

    matcher->matchLength = preLength;
    return false;
}

static bool matchPlus(RegexNode *regex, RegexNode *pattern, const char *text, Matcher *matcher) {
    const char *prePoint = text;
    while ((text[0] != END_LINE) && matchOne(regex, *text)) {
        matcher->matchLength++;
        text++;
    }

    while (text > prePoint) {   // match one or more
        if (matchPattern(pattern, matcher, text--)) {
            return true;
        }
        matcher->matchLength--;
    }
    matcher->matchLength = 0;
    return false;
}

static bool matchPlusLazy(RegexNode *regex, RegexNode *pattern, const char *text, Matcher *matcher) {
    while ((text[0] != END_LINE) && matchOne(regex, *text)) {
        matcher->matchLength++;
        text++;

        if (matchPattern(pattern, matcher, text)) {
            return true;
        }
    }
    matcher->matchLength--;
    return false;
}

static bool matchOne(RegexNode *regex, char character) {
    switch (regex->patternType) {
        case REGEX_DOT:
            return isMatchingDot(character);
        case REGEX_CHAR_CLASS:
            return matchCharClass(character, regex->classCharPtr);
        case REGEX_INVERSE_CHAR_CLASS:
            return !matchCharClass(character, regex->classCharPtr);
        case REGEX_DIGIT:
            return isdigit(character);
        case REGEX_NOT_DIGIT:
            return !isdigit(character);
        case REGEX_ALPHA:
            return isalnum(character);
        case REGEX_NOT_ALPHA:
            return !isalnum(character);
        case REGEX_WHITESPACE:
            return isspace(character);
        case REGEX_NOT_WHITESPACE:
            return !isspace(character);
        case REGEX_REGULAR_CHAR:
            return (regex->regexChar == character);
        default:
            return false;
    }
}

static inline bool isMatchingDot(unsigned char character) {
#if defined(REGEX_DOT_MATCH_NEWLINE) && (REGEX_DOT_MATCH_NEWLINE == true)
    (void) character;
    return true;
#else
    return (character != '\n' && character != '\r');
#endif
}

static bool matchCharClass(unsigned char character, const unsigned char *metaCharString) {
    do {
        if (isMatchingRange(character, metaCharString)) {
            return true;
        } else if (metaCharString[0] == '\\') { /* Escape-char: increment metaCharString-ptr and regexMatch on next char */
            metaCharString++;
            if (isMatchingMetaChar(character, metaCharString)) {
                return true;
            }
        } else if (character == metaCharString[0]) {
            return (character) == '-' ? (metaCharString[-1] == END_LINE) || (metaCharString[1] == END_LINE) : true;
        }
    } while (*metaCharString++ != END_LINE);

    return false;
}

static inline bool isMatchingRange(unsigned char character, const unsigned char *string) {
    return ((character != '-')
            && (string[0] != END_LINE)
            && (string[0] != '-')
            && (string[1] == '-')
            && (string[2] != END_LINE)
            && ((character >= string[0]) && (character <= string[2])));
}

static bool isMatchingMetaChar(unsigned char character, const unsigned char *metaCharString) {
    unsigned char metaChar = metaCharString[0];
    switch (metaChar) {
        case 'd':
            return isdigit(character);
        case 'D':
            return !isdigit(character);
        case 'w':
            return isalnum(character);
        case 'W':
            return !isalnum(character);
        case 's':
            return isspace(character);
        case 'S':
            return !isspace(character);
        default:
            return (character == metaChar);
    }
}