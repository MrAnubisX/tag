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
  // Compare the strings without the last 2 characters (new line and color code)
  // Attempts to use CFStringCompareWithOptions would not work for some reason.
  return CFStringCompareWithOptions(s1, s2,
                                    CFRangeMake(0, CFStringGetLength(s1) - 2),
                                    kCFCompareCaseInsensitive);
}