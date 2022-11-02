# Regex

[![tests](https://github.com/ximtech/Regex/actions/workflows/cmake-ci.yml/badge.svg)](https://github.com/ximtech/Regex/actions/workflows/cmake-ci.yml)
[![codecov](https://codecov.io/gh/ximtech/Regex/branch/main/graph/badge.svg?token=3WSE6BR59E)](https://codecov.io/gh/ximtech/Regex)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/10afb9b293e546679157ccb93cd069b3)](https://www.codacy.com/gh/ximtech/Regex/dashboard)

***Forked from*** [tiny-regex-c](https://github.com/kokke/tiny-regex-c/blob/master/README.md)

Fully refactored Regex library. Fixed all issues and added lazy operators. 
Also improved Regex compilation and removed static allocation. Code coverage with Perl tests.

### Features
- Small and portable
- Designed for small embedded applications
- Provides matching characters length and first match occurrence index
- Compiled with (MinGW.org GCC-6.3.0-1) 6.3.0
  ```
  > gcc -Os -c Regex.c
  > size Regex.o
     text    data     bss     dec     hex filename
     3732       0       0    3732    125c Regex.o
      
  ```

### Add as CPM project dependency
How to add CPM to the project, check the [link](https://github.com/cpm-cmake/CPM.cmake)
```cmake
CPMAddPackage(
        NAME Regex
        GITHUB_REPOSITORY ximtech/Regex
        GIT_TAG origin/main)

target_link_libraries(${PROJECT_NAME} Regex)
```
```cmake
add_executable(${PROJECT_NAME}.elf ${SOURCES} ${LINKER_SCRIPT})
# For Clion STM32 plugin generated Cmake use 
target_link_libraries(${PROJECT_NAME}.elf Regex)
```

### Supported regex-operators
The following features / regex-operators are supported by this library.

-   `.`        Dot, matches any character
-   `^`        Start anchor, matches beginning of string
-   `$`        End anchor, matches end of string
-   `*`        Asterisk, match zero or more (greedy)
-   `+`        Plus, match one or more (greedy)
-   `*?`       Asterisk, match zero or more (lazy)
-   `+?`       Plus, match one or more (lazy)
-   `?`        Question, match zero or one (lazy)
-   `{m,n}`    Quantifier, match min. 'm' and max. 'n' (greedy)
-   `{m}`      Exactly 'm'
-   `{m,}`     Match min 'm' and max.
-   `[abc]`    Character class, match if one of {'a', 'b', 'c'}
-   `[^abc]`   Inverted class, regexMatch if NOT one of {'a', 'b', 'c'}
-   `[a-zA-Z]` Character ranges, the character set of the ranges { a-z | A-Z }
-   `\s`       Whitespace, \t \f \r \n \v and spaces
-   `\S`       Non-whitespace
-   `\w`       Alphanumeric, [a-zA-Z0-9_]
-   `\W`       Non-alphanumeric
-   `\d`       Digits, [0-9]
-   `\D`       Non-digits

### Usage
```c
Regex regex;
regexCompile(&regex, "[Hh]ello [Ww]orld\\s*[!]?");
if (!regex.isPatternValid) {
    printf("Error: %s\n", regex.errorMessage);
    return 1;
}

Matcher matcher = regexMatch(&regex, "ahem.. 'hello world !' ..");
printf("Is found: %s\n", matcher.isFound ? "Yes" : "No");
printf("At index: %d\n", matcher.foundAtIndex);
printf("Length: %d\n", matcher.matchLength);
```
