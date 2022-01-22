#ifndef __JGTEXT_H__
#define __JGTEXT_H__

#include "jgdefs.h"

typedef struct TextTag {
    char *text;
    int length;
    int capacity;
} JGTEXT;

/**
 * Fills given buffer with the current text of given JGTEXT instance.
 *
 * @param JGTEXT         - JGTEXT instance, cannot be null
 * @param char *restrict - Buffer to fill
 * @param int            - Buffer length
 * @return Given buffer
 **/
char *JGGetString(JGTEXT*, char *restrict, int);

/**
 * Sets the text of given JGTEXT instance to given string.
 *
 * @param JGTEXT - JGTEXT instance, cannot be null
 * @param string_t - String to set the text to
 * @return If the text was cleared
 **/
bool JGSetString(JGTEXT*, string_t);

/**
 * Appends given JGTEXT instance with given string.
 *
 * @param JGTEXT - JGTEXT instance, cannot be null
 * @param string_t - String to append
 * @return If the text content changed
 **/
bool JGAppendString(JGTEXT*, string_t);

/**
 * Appends given JGTEXT instance with given string at given index.
 *
 * @param JGTEXT   - JGTEXT instance, cannot be null
 * @param string_t - String to insert
 * @param int      - Index of insertion
 * @return If the text content changed
 **/
bool JGInsertString(JGTEXT*, string_t, int);

#define JGInBounds(text, index) (index>=0&&index<(text)->len)

/**
 * Removes char of the given JGTEXT instance at given index.
 *
 * @param JGTEXT - JGTEXT instance, cannot be null
 * @param int    - Index to remove, must be in bounds, use macro JGInBounds for a bounds check
 * @return If the text content changed
 **/
bool JGRemoveCharAt(JGTEXT*, int);

/**
 * Removes a range of given length, starting from given index, from given JGTEXT instance.
 * Note: Range given must be in bounds, otherwise unexpected behavior like a crash could occur.
 *
 * @param JGTEXT - JGTEXT instance, cannot be null
 * @param int    - Start index
 * @param int    - Amount to remove
 * @return If the text content changed
 **/
bool JGRemoveRangeAt(JGTEXT*, int, int);

/**
 * Finds given char inside of the JGTEXT instance.
 * If the char was not found, -1 is returned.
 *
 * @param JGTEXT - JGTEXT instance, cannot be null
 * @param char   - Char to find
 * @param int    - Index to search from
 * @return Index of given char if found, otherwise -1
 **/
int JGFindChar(JGTEXT*, char, int);

/**
 * Finds given string inside of the JGTEXT instance.
 * If the string was not founds, -1 is returned.
 *
 * @param JGTEXT            - JGTEXT instance, cannot be null
 * @param string_t restrict - String to find
 * @param int               - Index to search from
 * @return Index of the beginning of the string if found, otherwise -1
 **/
int JGFindString(JGTEXT*, string_t restrict, int);

/**
 * Replaces found string with given string, the given strings must be unrelated to each other (restrict).
 *
 * @param JGTEXT            - JGTEXT instance, cannot be null
 * @param string_t restrict - String to find
 * @param int               - Index to search from
 * @param string_t restrict - String to replace
 * @return Index of the beginning of the string if found, otherwise -1
 **/
int JGReplaceString(JGTEXT*, string_t restrict, int, string_t restrict);

#endif // __JGTEXT_H__
