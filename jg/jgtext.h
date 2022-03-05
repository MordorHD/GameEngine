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
char *JGText_Get(JGTEXT*, char *restrict, int);

/**
 * Sets the text of given JGTEXT instance to given string.
 *
 * @param JGTEXT - JGTEXT instance, cannot be null
 * @param string_t - String to set the text to
 * @return If the text was cleared
 **/
bool JGText_Set0(JGTEXT*, string_t);
bool JGText_Set(JGTEXT*, string_t, int);

/**
 * Appends given JGTEXT instance with given string.
 *
 * @param JGTEXT - JGTEXT instance, cannot be null
 * @param string_t - String to append
 * @return If the text content changed
 **/
bool JGText_AppendString0(JGTEXT*, string_t);
bool JGText_AppendString(JGTEXT*, string_t, int);
#define JGText_AppendText(_text, atext) JGText_AppendString((_text), (atext)->text, (atext)->length)

/**
 * Appends given JGTEXT instance with given character.
 *
 * @param JGTEXT - JGTEXT instance, cannot be null
 * @param char - Character to append
 * @return If the text content changed
 **/
bool JGText_AppendChar(JGTEXT*, char);

/**
 * Appends given JGTEXT instance with given string at given index.
 *
 * @param JGTEXT   - JGTEXT instance, cannot be null
 * @param string_t - String to insert
 * @param int      - Index of insertion
 * @return If the text content changed
 **/
bool JGText_InsertString0(JGTEXT*, string_t, int);
bool JGText_InsertString(JGTEXT*, string_t, int, int);
#define JGText_InsertText(text, atext, index) JGText_InsertString((text), (atext)->text, (atext)->length, index)

#define JGInBounds(text, index) (index>=0&&index<(text)->len)

bool JGText_InsertChar(JGTEXT*, char, int);

/**
 * Removes char of the given JGTEXT instance at given index.
 *
 * @param JGTEXT - JGTEXT instance, cannot be null
 * @param int    - Index to remove, must be in bounds, use macro JGInBounds for a bounds check
 * @return If the text content changed
 **/
bool JGText_RemoveCharAt(JGTEXT*, int);

/**
 * Removes a range of given length, starting from given index, from given JGTEXT instance.
 * Note: Range given must be in bounds, otherwise unexpected behavior like a crash could occur.
 *
 * @param JGTEXT - JGTEXT instance, cannot be null
 * @param int    - Start index
 * @param int    - Amount to remove
 * @return If the text content changed
 **/
bool JGText_RemoveRangeAt(JGTEXT*, int, int);

/**
 * Finds given char inside of the JGTEXT instance.
 * If the char was not found, -1 is returned.
 *
 * @param JGTEXT - JGTEXT instance, cannot be null
 * @param char   - Char to find
 * @param int    - Index to search from
 * @return Index of given char if found, otherwise -1
 **/
int JGText_FindChar(JGTEXT*, char, int);

/**
 * Finds given string inside of the JGTEXT instance.
 * If the string was not founds, -1 is returned.
 *
 * @param JGTEXT            - JGTEXT instance, cannot be null
 * @param string_t restrict - String to find
 * @param int               - Index to search from
 * @return Index of the beginning of the string if found, otherwise -1
 **/
int JGText_FindString(JGTEXT*, string_t restrict, int);

/**
 * Replaces found string with given string, the given strings must be unrelated to each other (restrict).
 *
 * @param JGTEXT            - JGTEXT instance, cannot be null
 * @param string_t restrict - String to find
 * @param int               - Index to search from
 * @param string_t restrict - String to replace
 * @return Index of the beginning of the string if found, otherwise -1
 **/
int JGText_FindReplaceString(JGTEXT*, string_t restrict, int, string_t restrict);

#endif // __JGTEXT_H__
