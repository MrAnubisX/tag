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

Boolean TagCallBacksEqual(const void *s1, const void *s2) {
  CFIndex len = CFStringGetLength(s1);
  CFStringEncoding enc = CFStringGetSystemEncoding();

  // Compare the strings without the last 2 characters (new line and color code)
  // Attempts to use CFStringCompareWithOptions would not work for some reason.
  return strncasecmp(CFStringGetCStringPtr(s1, enc),
                     CFStringGetCStringPtr(s2, enc), len - 2) == 0;
}