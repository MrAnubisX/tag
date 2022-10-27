//
// array.c
// Tag
//
// Created by Christopher Snead on 2022/09/28.
//

#include "array.h"

const void *TagCallBackRetain(CFAllocatorRef allocator, const void *value) {
  return CFRetain(value);
}

void TagCallBackRelease(CFAllocatorRef allocator, const void *value) {
  CFRelease(value);
}

CFStringRef TagCallBackCopyDescription(const void *value) {
  return (CFStringRef)value;
}

// Compare the strings without the last 2 characters (new line and color code)
// Attempts to use CFStringCompareWithOptions would not work for some reason.
Boolean TagCallBacksEqual(const void *s1, const void *s2) {
  CFIndex l1 = CFStringGetLength(s1);
  CFIndex l2 = CFStringGetLength(s2);
  CFStringEncoding enc = CFStringGetSystemEncoding();
  char *p1 = calloc(l1 + 1, sizeof(char));
  char *p2 = calloc(l2 + 1, sizeof(char));

  CFStringGetCString(s1, p1, l1 + 1, enc);
  CFStringGetCString(s2, p2, l2 + 1, enc);

  long result = strncasecmp(p1, p2, l1 - 2);

  free(p1);
  free(p2);

  return (result == 0);
}