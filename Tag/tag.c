//
// tag_tests.c
// Tag
//
// Created by Christopher Snead on 9/11/22.
//

#include "tag.h"

#include "array.h"
#include <CoreFoundation/CoreFoundation.h>
#include <dirent.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/xattr.h>

int parseCommandLine(int argc, char *const argv[]) {
  // Command line arguments
  static struct option options[] = {
    // Operations
    {"set", required_argument, 0, OperationModeSet},
    {"add", required_argument, 0, OperationModeAdd},
    {"remove", required_argument, 0, OperationModeRemove},
    {"match", required_argument, 0, OperationModeMatch},
    {"list", no_argument, 0, OperationModeList},
    // Format options
    {"name", no_argument, 0, 'n'},
    {"no-name", no_argument, 0, 'N'},
    {"tags", no_argument, 0, 't'},
    {"no-tags", no_argument, 0, 'T'},
    {"garrulous", no_argument, 0, 'g'},
    {"no-garrulous", no_argument, 0, 'G'},
    {"color", no_argument, 0, 'c'},
    {"slash", no_argument, 0, 'p'},
    {"nul", no_argument, 0, '0'},
    // Directory enumeration options
    {"all", no_argument, 0, 'A'},
    {"recursive", no_argument, 0, 'R'},
    // Other
    {"help", no_argument, 0, 'h'},
    {"version", no_argument, 0, 'v'},
    {0, 0, 0, 0}};

  // Option character
  int opt;

  // Operation mode initialized to a known state
  OperationMode operationMode = OperationModeUnknown;

  // Output flags defaults
  OutputFlags outputFlags = OutputFlagsName | OutputFlagsTags;

  // User tags container, for set, add, etc...
  UserTag *tags = NULL;

  // Number of argument tags
  int tagCount = 0;

  // Tag extended attribute property list binary data
  UInt8 *plBin = NULL;

  // Tag extended attribute property list binary data length
  size_t plLen = 0;

  // Parse options
  int ndx = 0;
  while ((opt = getopt_long(argc, argv, "s:a:r:lnNtTgGcp0ARhv", options,
                            &ndx)) != -1) {
    switch (opt) {
      case OperationModeSet:
      case OperationModeAdd:
      case OperationModeRemove:
      case OperationModeMatch:
      case OperationModeList:
        if (operationMode) {
          reportError("%s\n", "Operation mode cannot be respecified");
          freeUserTags(tags, tagCount);
          return EXIT_FAILURE;
        }
        operationMode = opt;
        tags = parseTagsArgument(optarg, &tagCount);
        break;
      case 'n':
        outputFlags |= OutputFlagsName;
        break;
      case 'N':
        outputFlags &= ~OutputFlagsName;
        break;
      case 't':
        outputFlags |= OutputFlagsTags;
        break;
      case 'T':
        outputFlags &= ~OutputFlagsTags;
        break;
      case 'g':
        outputFlags |= OutputFlagsGarrulous;
        break;
      case 'G':
        outputFlags &= ~OutputFlagsGarrulous;
        break;
      case 'c':
        outputFlags |= OutputFlagsColor;
        break;
      case 'p':
        outputFlags |= OutputFlagsSlashDirectory;
        break;
      case '0':
        outputFlags |= OutputFlagsNulTerminate;
        break;
      case 'A':
        outputFlags |= OutputFlagsShowHidden;
        break;
      case 'R':
        outputFlags |= OutputFlagsRecurseDirectory;
        break;
      case '?':
      case 'h':
        operationMode = OperationModeNone;
        displayUsage(argv[0]);
        break;
      case 'v':
        operationMode = OperationModeNone;
        displayVersion();
        break;
      default:
        break;
    }
  }

  // Default the operation mode to list if it was not set
  if (operationMode == OperationModeUnknown) operationMode = OperationModeList;

  // Process any remaining arguments as file paths
  if (operationMode > OperationModeNone) {
    if (operationMode == OperationModeSet) {
      // Create a binary format property list from the tags argument
      plBin = createPlistBinary(&plLen, tags, tagCount);
    }
    // Default to CWD if no filenames entered
    if ((operationMode == OperationModeList ||
         operationMode == OperationModeMatch) &&
        (argc - optind) < 1) {
      char cwd[PATH_MAX];
      struct dirent *dir;
      DIR *_d;

      getcwd(cwd, sizeof(cwd));
      if ((_d = opendir(cwd)) != NULL) {
        while ((dir = readdir(_d)) != NULL) {
          if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0)
            continue;
          if (*(dir->d_name) == '.' && !(outputFlags & OutputFlagsShowHidden))
            continue;
          if (operationMode == OperationModeList)
            listTags(dir->d_name, outputFlags);
          if (operationMode == OperationModeMatch)
            matchTags(dir->d_name, tags, tagCount, outputFlags);
        }
        closedir(_d);
      }
    }
    // Process directories and filenames
    for (int i = 0; i < (argc - optind); ++i) {
      // Filepath to process
      char *path = argv[optind + i];

      // Skip empty path requests
      if (!strlen(path)) continue;

      switch (operationMode) {
        case OperationModeSet:
          // Apply the attr data on each path
          setxattr(path, TAG_NAME, plBin, plLen, 0, 0);
          break;
        case OperationModeAdd:
          addTags(path, tags, tagCount);
          break;
        case OperationModeRemove:
          removeTags(path, tags, tagCount);
          break;
        case OperationModeMatch:
          matchTags(path, tags, tagCount, outputFlags);
          break;
        case OperationModeList:
          listTags(path, outputFlags);
          break;
        default:
          break;
      }
    }
  }

  // Cleanup
  if (plBin) free(plBin);
  freeUserTags(tags, tagCount);

  return EXIT_SUCCESS;
}

UserTag *parseTagsArgument(char *arg, int *tagCount) {
  // separated portion of the string
  char *tok;

  // temporary string for holding the name of tag
  char *nam;

  // used in tracking the position in the array
  int index = 0;

  // array of tags
  UserTag *userTags = calloc(ALLOC_AMOUNT, sizeof(*userTags));

  // Process the comma delimited string of tags
  while ((tok = strtok_r(arg, ",", &arg))) {
    // If the tag name contains a color, split, and use the first part for the
    // name, second part for the color
    nam = strtok_r(tok, ":", &tok);
    (userTags + index)->name = malloc(strlen(nam) + 1);
    strcpy((userTags + index)->name, nam);

    // Color code from the remaining string, fallback to name if nothing remains
    (userTags + index)->color =
      getColorCode(tok ? tok : (userTags + index)->name);

    // Increment the index and allocate additional memory if necessary
    if ((++index % ALLOC_AMOUNT) == 0) {
      size_t amount = ((index / ALLOC_AMOUNT) + 1) * ALLOC_AMOUNT;
      userTags = realloc(userTags, sizeof(*userTags) * amount);
    }
  }

  // Update the tag count value
  *tagCount = index;

  // Return the memory address of the UserTag array
  return userTags;
}

void addTags(char *path, UserTag *userTags, int tagCount) {
  // Paths' existing tags
  UserTag *existingTags;

  // Merged tags from the argument tags and existing path tags
  UserTag *mergedTags;

  // Paths' existing tags count
  int existingTagsCount;

  // Merged tags count
  int mergedTagsCount;

  // Binary property list buffer
  UInt8 *mergedBytes;

  // Binary property list size
  size_t mergedBytesLen = 0;

  // Get the tags for the path if they exists
  existingTags = createUserTagsFromPath(path, &existingTagsCount);

  // Merged tags count
  mergedTagsCount = tagCount + existingTagsCount;

  // Merge the existing and argument tags
  mergedTags = calloc(mergedTagsCount, sizeof(*mergedTags));
  memcpy(mergedTags, existingTags, sizeof(*mergedTags) * existingTagsCount);
  memcpy((mergedTags + existingTagsCount), userTags,
         sizeof(*mergedTags) * tagCount);

  // Sort the tags
  qsort(mergedTags, mergedTagsCount, sizeof(*mergedTags), tagCompare);

  // Create the binary property list
  mergedBytes = createPlistBinary(&mergedBytesLen, mergedTags, mergedTagsCount);

  // Apply the merged property list as an extended attribute to the path
  setxattr(path, TAG_NAME, mergedBytes, mergedBytesLen, 0, 0);

  // Cleanup
  free(mergedBytes);
  free(mergedTags);
  freeUserTags(existingTags, existingTagsCount);
}

void removeTags(char *path, UserTag *userTags, int tagCount) {
  // Path's existing tags
  UserTag *existingTags;

  // Path's existing tags count
  int existingCount;

  // Wildcard remove all tags
  if (*(userTags->name) == '*') {
    removexattr(path, TAG_NAME, 0);
    return;
  }

  // Get the tags for the path if they exist
  existingTags = createUserTagsFromPath(path, &existingCount);

  if (!existingTags || !existingCount) return;

  // Sort the tags before searching
  qsort(existingTags, existingCount, sizeof(*existingTags), tagCompare);

  // Iterate over the tags requested to be removed
  for (int j = 0; j < tagCount; ++j) {
    UserTag *found = NULL;
    found = bsearch((userTags + j), existingTags, existingCount,
                    sizeof(*existingTags), tagCompare);
    if (found) {
      // Set the name of the found to an empty string
      *(found->name) = '\0';
    }
  }

  // Replacement property list binary
  UInt8 *bin;

  // Property list size
  size_t siz;

  // Apply the remaining tags
  if (existingCount) {
    // Create a binary property list of the remaining existing tags
    bin = createPlistBinary(&siz, existingTags, existingCount);

    // Set the extended attribute tag using the binary property list
    setxattr(path, TAG_NAME, bin, siz, 0, 0);

    // Release resources used by the binary
    if (bin) free(bin);
  } else {
    // Remove the extended attribute altogether if there are no remaining tags
    removexattr(path, TAG_NAME, 0);
  }

  // Cleanup
  freeUserTags(existingTags, existingCount);
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "misc-no-recursion"
// Enumerate directory contents, recursively entering subdirectories
void listTags(char *path, OutputFlags outputFlags) {
  DIR *pDir;
  struct dirent *dir;

  // Existing tags for the filename, if there are any
  UserTag *existingTags;

  // Count of existing tags
  int existingTagsCount;

  // Get the tags for the path if they exists
  existingTags = createUserTagsFromPath(path, &existingTagsCount);

  // Print the filename and tags according to the output flags
  printPath(path, existingTags, existingTagsCount, outputFlags);

  // Memory cleanup
  freeUserTags(existingTags, existingTagsCount);

  // Is directory?
  if ((pDir = opendir(path)) != NULL) {
    // Recurse directory ?
    if (outputFlags & OutputFlagsRecurseDirectory) {
      while ((dir = readdir(pDir)) != NULL) {
        char _p[PATH_MAX];

        // Ignore current and parent dir entries
        if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0)
          continue;

        // Ignore dot paths if the show hidden flag is not set
        if (*(dir->d_name) == '.' && !(outputFlags & OutputFlagsShowHidden))
          continue;

        // Combine the parent path, entry name, and enumerate
        snprintf(_p, sizeof(_p), "%s%s%s", path, PATH_SEPARATOR, dir->d_name);
        listTags(_p, outputFlags);
      }
    }
    closedir(pDir);
  }
}
#pragma clang diagnostic pop

#pragma clang diagnostic push
#pragma ide diagnostic ignored "misc-no-recursion"
bool matchTags(char *path, UserTag *userTags, int tagCount,
               OutputFlags outputFlags) {
  DIR *pDir;
  struct dirent *dir;
  bool matched = false;

  // Existing tags for the filename, if there are any
  UserTag *existingTags;

  // Count of existing tags
  int existingTagsCount;

  // Get the tags for the path if they exists
  existingTags = createUserTagsFromPath(path, &existingTagsCount);

  // Sort the tags before searching
  qsort(existingTags, existingTagsCount, sizeof(*existingTags), tagCompare);

  if (userTags->name) {
    // Wildcard match any tags
    matched = (*(userTags->name) == '*' && existingTagsCount > 0);
  } else {
    // Match none
    matched = (existingTagsCount == 0);
  }

  // Iterate the tags to match against
  int foundCount = 0;
  for (int j = 0; j < tagCount && !matched; ++j) {
    UserTag *found = NULL;
    found = bsearch((userTags + j), existingTags, existingTagsCount,
                    sizeof(*existingTags), tagCompare);
    if (found) matched = (++foundCount == tagCount);
  }

  if (matched) printPath(path, existingTags, existingTagsCount, outputFlags);

  // Cleanup
  freeUserTags(existingTags, existingTagsCount);

  // Is path a directory?
  if ((pDir = opendir(path)) != NULL) {
    // Recurse directory ?
    if (outputFlags & OutputFlagsRecurseDirectory) {
      while ((dir = readdir(pDir)) != NULL) {
        char _p[PATH_MAX];

        // Ignore current and parent dir entries
        if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0)
          continue;

        // Ignore dot paths if the show hidden flag is not set
        if (*(dir->d_name) == '.' && !(outputFlags & OutputFlagsShowHidden))
          continue;

        // Combine the parent path, entry name, and enumerate
        snprintf(_p, sizeof(_p), "%s%s%s", path, PATH_SEPARATOR, dir->d_name);
        matchTags(_p, userTags, tagCount, outputFlags);
      }
    }
    closedir(pDir);
  }

  return matched;
}
#pragma clang diagnostic pop

UInt8 *createPlistBinary(size_t *length, UserTag *userTags, int tagCount) {
  // Return byte array
  UInt8 *bin;

  // Property list array
  CFMutableArrayRef arr;

  // String encoding format
  CFPropertyListFormat fmt = kCFPropertyListBinaryFormat_v1_0;

  // CallBack for the CFArray
  CFArrayCallBacks cfTagCallBacks = {
    .version = 0,
    .retain = TagCallBackRetain,
    .release = TagCallBackRelease,
    .copyDescription = TagCallBackCopyDescription,
    .equal = TagCallBacksEqual};

  // Create a string array of tags for the property list
  arr = CFArrayCreateMutable(NULL, tagCount, &cfTagCallBacks);

  // Populate the array
  for (int i = 0; i < tagCount; i++) {
    // Avoid appending a tag with an empty name
    if (!strnlen((userTags + i)->name, 2)) continue;

    // CFString val containing the "tag name + \n + color code"
    CFStringRef val = CFStringCreateWithFormat(
      NULL, NULL, CFSTR("%s\n%d"), (userTags + i)->name, (userTags + i)->color);

    // Try to prevent adding duplicates
    if (!CFArrayContainsValue(arr, CFRangeMake(0, CFArrayGetCount(arr)), val)) {
      CFArrayAppendValue(arr, val);
    }

    // Cleanup
    CFRelease(val);
  }

  // Data reference from the foundation array
  CFDataRef dat = CFPropertyListCreateData(NULL, arr, fmt, 0, NULL);

  // Release the foundation array
  CFRelease(arr);

  // Update the bin' length value
  *length = CFDataGetLength(dat);

  // Property list in binary format
  bin = calloc(*length, sizeof(*bin));
  CFDataGetBytes(dat, CFRangeMake(0, (CFIndex)*length), bin);

  // Is this safe to release, or is the bin not copied
  CFRelease(dat);

  // Return the binary data address
  return bin;
}

UserTag *createUserTagsFromPath(char *path, int *tagCount) {
  UserTag *userTags = NULL;
  UInt8 buf[EXT_ATTR_SIZE];
  ssize_t len;

  // Default the tag count to zero
  *tagCount = 0;

  // Get the binary property list user tag extended attribute if it exists
  len = getxattr(path, TAG_NAME, buf, EXT_ATTR_SIZE, 0, 0);

  // Process only the buffer contains more than the binary plist header
  if (len > 8) {
    CFArrayRef cfArray;
    CFMutableDataRef cfData = CFDataCreateMutable(NULL, 0);
    CFStringEncoding enc = CFStringGetSystemEncoding();

    // Pack the binary property list into the core foundation data reference
    CFDataAppendBytes(cfData, buf, len);

    // Create a core foundation array of strings from the cfData
    cfArray = CFPropertyListCreateWithData(NULL, cfData, 0, NULL, NULL);

    // Release the core foundation data reference
    if (cfData) CFRelease(cfData);

    // Update the tag count
    *tagCount = (int)CFArrayGetCount(cfArray);

    // Allocate and zero the memory for the user tags
    userTags = calloc(*tagCount, sizeof(*userTags));

    // Set the values for each UserTag struct
    for (int i = 0; i < *tagCount; ++i) {
      // Split the string on the new line. tag name + new line + color code
      CFArrayRef parts = CFStringCreateArrayBySeparatingStrings(
        NULL, CFArrayGetValueAtIndex(cfArray, i), CFSTR("\n"));

      // Allocate memory for the tag name string
      CFStringRef tagName = CFArrayGetValueAtIndex(parts, 0);
      CFIndex nameLength =
        CFStringGetMaximumSizeForEncoding(CFStringGetLength(tagName), enc);
      (userTags + i)->name = malloc(nameLength + 1);

      // Set the name from the first part, and color value from the second
      CFStringGetCString(tagName, (userTags + i)->name, nameLength + 1, enc);
      (userTags + i)->color =
        CFStringGetIntValue(CFArrayGetValueAtIndex(parts, 1));

      // Cleanup, make way for the next iteration
      CFRelease(parts);
    }

    // Property list array is no longer needed?
    CFRelease(cfArray);
  }

  return userTags;
}

void printPath(char *path, UserTag *userTags, long tagCount,
               OutputFlags outputFlags) {
  char *fileName = NULL;
  struct stat pathStat;

  // Populate path stat
  stat(path, &pathStat);

  if (outputFlags & OutputFlagsName) {
    char *suffix = "";
    if (outputFlags & OutputFlagsSlashDirectory) {
      if (S_ISDIR(pathStat.st_mode)) suffix = PATH_SEPARATOR;
    }
    fileName = malloc(FILENAME_MAX);
    snprintf(fileName, FILENAME_MAX, "%s%s", path, suffix);
  }

  bool tagsOnSeparateLines = !!(outputFlags & OutputFlagsGarrulous);
  bool printTags = (outputFlags & OutputFlagsTags) && tagCount;
  char lineTerminator = (outputFlags & OutputFlagsNulTerminate) ? '\0' : '\n';

  if (fileName) {
    if (printTags && !tagsOnSeparateLines) {
      printf("%-31s", fileName);
    } else {
      printf("%s", fileName);
    }
  }

  if (printTags) {
    bool needLineTerm = false;

    // Sort the array alphabetically by name
    qsort(userTags, tagCount, sizeof(*userTags), tagCompare);

    char *tagSeparator;
    char *startingSeparator;
    if (tagsOnSeparateLines) {
      needLineTerm = !!fileName;
      tagSeparator = fileName ? "    " : "";
      startingSeparator = tagSeparator;
    } else {
      tagSeparator = ",";
      startingSeparator = fileName ? "\t" : "";
    }

    // Print colorized tag name
    char *buf = NULL;
    char *sep = startingSeparator;
    buf = (char *)malloc(TAG_BUF_SIZE);
    for (int i = 0; i < tagCount; ++i) {
      if (needLineTerm) putc(lineTerminator, stdout);
      displayStringForTag(buf, (userTags + i), outputFlags);
      printf("%s%s", sep, buf);

      sep = tagSeparator;
      needLineTerm = tagsOnSeparateLines;
    }
    free(buf);
  }

  // Print out the ending line terminator
  if (fileName || printTags) putc(lineTerminator, stdout);

  // Cleanup
  if (fileName) free(fileName);
}

void displayStringForTag(char *buf, UserTag *tag, OutputFlags outputFlags) {
  char *colorSequence = NULL;

  if (outputFlags & OutputFlagsColor) {
    switch (tag->color) {
      case TagColorBlue:
        colorSequence = COLORS_BLUE;
        break;
      case TagColorGray:
        colorSequence = COLORS_GRAY;
        break;
      case TagColorGreen:
        colorSequence = COLORS_GREEN;
        break;
      case TagColorOrange:
        colorSequence = COLORS_ORANGE;
        break;
      case TagColorPurple:
        colorSequence = COLORS_PURPLE;
        break;
      case TagColorRed:
        colorSequence = COLORS_RED;
        break;
      case TagColorYellow:
        colorSequence = COLORS_YELLOW;
        break;
      default:
        colorSequence = COLORS_NONE;
        break;
    }

    snprintf(buf, TAG_BUF_SIZE, "%s%s%s", colorSequence, tag->name,
             COLORS_NONE);
  } else {
    strncpy(buf, tag->name, strlen(tag->name) + 1);
  }
}

TagColor getColorCode(char *color) {
  switch (toupper(*color)) {
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
      return strtol(color, NULL, 10);
    case 'G':
      if (strncasecmp(color, "gray", 4) == 0) return TagColorGray;
      if (strncasecmp(color, "green", 5) == 0) return TagColorGreen;
      break;
    case 'P':
      if (strncasecmp(color, "purple", 6) == 0) return TagColorPurple;
      break;
    case 'B':
      if (strncasecmp(color, "blue", 4) == 0) return TagColorBlue;
      break;
    case 'Y':
      if (strncasecmp(color, "yellow", 6) == 0) return TagColorYellow;
      break;
    case 'R':
      if (strncasecmp(color, "red", 3) == 0) return TagColorRed;
      break;
    case 'O':
      if (strncasecmp(color, "orange", 6) == 0) return TagColorOrange;
      break;
  }
  return TagColorNone;
}

int tagCompare(const void *a, const void *b) {
  return strcmp(((UserTag *)a)->name, ((UserTag *)b)->name);
}

void freeUserTags(UserTag *userTags, int tagCount) {
  if (userTags) {
    for (int i = 0; i < tagCount; ++i) {
      if ((userTags + i)->name) free((userTags + i)->name);
    }
    free(userTags);
    userTags = NULL;
  }
}

void displayUsage(char *programName) {
  printf(
    "%s - %s", programName,
    "A tool for manipulating and querying file tags.\n"
    "  displayUsage:\n"
    "    tag -a | --add <tags> <path>...     Add tags to file\n"
    "    tag -r | --remove <tags> <path>...  Remove tags from file\n"
    "    tag -s | --set <tags> <path>...     Set tags on file\n"
    "    tag -m | --match <tags> <path>...   Display files with matching tags\n"
    "    tag -f | --find <tags> <path>...    Find all files with tags (-A, -e, "
    "-R ignored)\n"
    "    tag -u | --usage <tags> <path>...   Display tags used, with "
    "displayUsage "
    "counts\n"
    "    tag -l | --list <path>...           List the tags on file\n"
    "  <tags> is a comma-separated list of tag names; use * to match/find any "
    "tag.\n"
    "  additional options:\n"
    "        -v | --version      Display version\n"
    "        -h | --help         Display this help\n"
    "        -A | --all          Display invisible files while enumerating\n"
    "        -e | --enter        Enter and enumerate directories provided\n"
    "        -R | --recursive    Recursively process directories\n"
    "        -n | --name         Turn on filename display in output (default)\n"
    "        -N | --no-name      Turn off filename display in output (list, "
    "find, match)\n"
    "        -t | --tags         Turn on tags display in output (find, match)\n"
    "        -T | --no-tags      Turn off tags display in output (list)\n"
    "        -g | --garrulous    Display tags each on own line (list, find, "
    "match)\n"
    "        -G | --no-garrulous Display tags comma-separated after filename "
    "(default)\n"
    "        -c | --color        Display tags in color\n"
    "        -p | --slash        Terminate each directory name with a slash\n"
    "        -0 | --nul          Terminate lines with NUL (\\0) for use with "
    "xargs -0\n"
    "             --home         Find tagged files in user home directory\n"
    "             --local        Find tagged files in home + local "
    "filesystems\n"
    "             --network      Find tagged files in home + local + network "
    "filesystems\n");
}

void displayVersion() { printf("%s v%s\n", PROGRAM_NAME, PROGRAM_VERSION); }

void reportError(const char *fmt, ...) {
  va_list args;

  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
}
