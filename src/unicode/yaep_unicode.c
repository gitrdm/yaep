/* YAEP Unicode Support Wrapper Implementation
 *
 * This module wraps utf8proc to provide Unicode text handling for YAEP.
 * See yaep_unicode.h for API documentation.
 *
 * Copyright (c) 2025
 * Licensed under the same terms as YAEP (MIT License).
 */

#include "yaep_unicode.h"
#include "utf8proc.h"
#include <string.h>

/* UTF-8 Iterator with ASCII Fast Path
 *
 * For maximum performance on predominantly-ASCII grammars (the common case),
 * we check for single-byte characters first before calling into utf8proc.
 * This avoids the overhead of the full Unicode property lookup for simple
 * ASCII text.
 */
yaep_codepoint_t yaep_utf8_next(const char **str_ptr)
{
  const unsigned char *str = (const unsigned char *)*str_ptr;
  
  /* Handle NULL pointer gracefully */
  if (str == NULL || *str == '\0') {
    return YAEP_CODEPOINT_EOS;
  }
  
  /* ASCII fast path: single-byte characters (0x00-0x7F) */
  if (*str < 0x80) {
    *str_ptr = (const char *)(str + 1);
    return (yaep_codepoint_t)*str;
  }
  
  /* Multi-byte UTF-8 sequence: delegate to utf8proc */
  utf8proc_int32_t codepoint;
  utf8proc_ssize_t bytes_read = utf8proc_iterate(str, -1, &codepoint);
  
  if (bytes_read < 0) {
    /* Decoding error: advance by 1 byte to allow recovery and return invalid */
    *str_ptr = (const char *)(str + 1);
    return YAEP_CODEPOINT_INVALID;
  }
  
  /* Success: advance pointer past the decoded sequence */
  *str_ptr = (const char *)(str + bytes_read);
  return (yaep_codepoint_t)codepoint;
}

/* UTF-8 Validation
 *
 * Walks the entire string using yaep_utf8_next to ensure all sequences
 * are well-formed. Also counts the number of code points for the caller's
 * convenience (useful for pre-allocating buffers or progress reporting).
 */
int yaep_utf8_validate(const char *str, size_t *len)
{
  const char *p = str;
  size_t count = 0;
  yaep_codepoint_t cp;
  
  /* Iterate through the string until we hit EOS */
  while ((cp = yaep_utf8_next(&p)) != YAEP_CODEPOINT_EOS) {
    if (cp == YAEP_CODEPOINT_INVALID) {
      /* Invalid UTF-8 sequence encountered */
      if (len != NULL) {
        *len = count;
      }
      return 0;
    }
    count++;
  }
  
  /* All sequences were valid */
  if (len != NULL) {
    *len = count;
  }
  return 1;
}

/* Character Classification: Alphabetic
 *
 * Uses utf8proc's category lookup to identify letters. The Unicode
 * standard defines several letter categories (Lu, Ll, Lt, Lm, Lo)
 * which we treat uniformly as "alphabetic".
 */
int yaep_utf8_isalpha(yaep_codepoint_t cp)
{
  /* ASCII fast path: A-Z and a-z */
  if (cp >= 0 && cp < 128) {
    return (cp >= 'A' && cp <= 'Z') || (cp >= 'a' && cp <= 'z');
  }
  
  /* Invalid or EOS code points are not alphabetic */
  if (cp < 0) {
    return 0;
  }
  
  /* Query the Unicode Character Database via utf8proc */
  const utf8proc_property_t *prop = utf8proc_get_property(cp);
  
  /* Check if the category is any of the Letter categories:
   * Lu (Uppercase Letter), Ll (Lowercase Letter), Lt (Titlecase Letter),
   * Lm (Modifier Letter), Lo (Other Letter)
   */
  return (prop->category == UTF8PROC_CATEGORY_LU ||
          prop->category == UTF8PROC_CATEGORY_LL ||
          prop->category == UTF8PROC_CATEGORY_LT ||
          prop->category == UTF8PROC_CATEGORY_LM ||
          prop->category == UTF8PROC_CATEGORY_LO);
}

/* Character Classification: Decimal Digit
 *
 * Identifies decimal digit characters from any script. This includes
 * ASCII 0-9 as well as digits from other writing systems.
 */
int yaep_utf8_isdigit(yaep_codepoint_t cp)
{
  /* ASCII fast path: 0-9 */
  if (cp >= '0' && cp <= '9') {
    return 1;
  }
  
  /* Invalid or EOS code points are not digits */
  if (cp < 0) {
    return 0;
  }
  
  /* Query the Unicode Character Database via utf8proc */
  const utf8proc_property_t *prop = utf8proc_get_property(cp);
  
  /* Check if the category is Nd (Decimal Number) */
  return (prop->category == UTF8PROC_CATEGORY_ND);
}

/* Character Classification: Alphanumeric
 *
 * Combines alphabetic and digit checks. This is the typical predicate
 * for identifier continuation characters in most programming languages.
 */
int yaep_utf8_isalnum(yaep_codepoint_t cp)
{
  /* ASCII fast path: combine the ASCII checks inline */
  if (cp >= 0 && cp < 128) {
    return ((cp >= 'A' && cp <= 'Z') || 
            (cp >= 'a' && cp <= 'z') || 
            (cp >= '0' && cp <= '9'));
  }
  
  /* Invalid or EOS code points are not alphanumeric */
  if (cp < 0) {
    return 0;
  }
  
  /* Query the Unicode Character Database via utf8proc */
  const utf8proc_property_t *prop = utf8proc_get_property(cp);
  
  /* Check for Letter or Decimal Number categories */
  return (prop->category == UTF8PROC_CATEGORY_LU ||
          prop->category == UTF8PROC_CATEGORY_LL ||
          prop->category == UTF8PROC_CATEGORY_LT ||
          prop->category == UTF8PROC_CATEGORY_LM ||
          prop->category == UTF8PROC_CATEGORY_LO ||
          prop->category == UTF8PROC_CATEGORY_ND);
}

/* Character Classification: Whitespace
 *
 * Identifies Unicode whitespace characters. This includes the ASCII
 * whitespace characters (space, tab, newline, etc.) plus Unicode
 * space separators and line/paragraph separators.
 */
int yaep_utf8_isspace(yaep_codepoint_t cp)
{
  /* ASCII fast path: common whitespace characters
   * Space (0x20), Tab (0x09), LF (0x0A), VT (0x0B), FF (0x0C), CR (0x0D)
   */
  if (cp >= 0 && cp < 128) {
    return (cp == ' ' || cp == '\t' || cp == '\n' || 
            cp == '\v' || cp == '\f' || cp == '\r');
  }
  
  /* Invalid or EOS code points are not whitespace */
  if (cp < 0) {
    return 0;
  }
  
  /* Query the Unicode Character Database via utf8proc */
  const utf8proc_property_t *prop = utf8proc_get_property(cp);
  
  /* Check for Space Separator categories or explicit whitespace property
   * Zs (Space Separator), Zl (Line Separator), Zp (Paragraph Separator)
   */
  return (prop->category == UTF8PROC_CATEGORY_ZS ||
          prop->category == UTF8PROC_CATEGORY_ZL ||
          prop->category == UTF8PROC_CATEGORY_ZP);
}

/* UTF-8 String Hashing with Unsigned Byte Treatment
 *
 * This hash function treats all bytes as unsigned to avoid sign-extension
 * issues. When a signed char with the high bit set is promoted to int,
 * it becomes negative on most platforms, which can cause hash instability.
 *
 * We use a simple multiplicative hash (FNV-1a style) which provides
 * reasonable distribution for short strings like grammar symbols.
 */
unsigned yaep_utf8_hash(const char *str)
{
  unsigned hash = 2166136261u;  /* FNV-1a offset basis (32-bit) */
  const unsigned char *p = (const unsigned char *)str;
  
  /* Process each byte as unsigned to avoid sign extension */
  while (*p != '\0') {
    hash ^= *p++;
    hash *= 16777619u;  /* FNV-1a prime (32-bit) */
  }
  
  return hash;
}

/* Error Message Lookup
 *
 * Translates utf8proc error codes to human-readable messages.
 * This is primarily used for diagnostic output when invalid UTF-8
 * is encountered during grammar parsing.
 */
const char *yaep_utf8_error_message(int error_code)
{
  /* utf8proc returns negative error codes on failure.
   * The utf8proc_errmsg function provides descriptive text.
   */
  if (error_code >= 0) {
    return "No error";
  }
  
  /* Delegate to utf8proc's error message function */
  return utf8proc_errmsg(error_code);
}
