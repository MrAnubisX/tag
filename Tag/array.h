//
// array.h
// Tag
//
// Created by Christopher Snead on 2022/09/28.
//

#ifndef TAG__ARRAY_H_
#define TAG__ARRAY_H_

#include <CoreFoundation/CoreFoundation.h>

const void *TagCallBackRetain(CFAllocatorRef, const void *);
void TagCallBackRelease(CFAllocatorRef, const void *);
CFStringRef TagCallBackCopyDescription(const void *);
Boolean TagCallBacksEqual(const void *, const void *);

#endif  // TAG__ARRAY_H_