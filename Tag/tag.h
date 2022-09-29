// tag.h
// Tag
//
// Created by Christopher Snead on 2022/09/28.
//

#ifndef TAG_TAG_H
#define TAG_TAG_H

// clang-format off
#define PROGRAM_NAME    "tag"
#define PROGRAM_VERSION "2022.3.1"

// Extended attribute binary property list buffer size
#define EXT_ATTR_SIZE   10000

// Extended attribute key name
#define TAG_NAME        "com.apple.metadata:_kMDItemUserTags"

// Tag string buffer size
#define TAG_BUF_SIZE    512

// Amount to use when allocating memory for argument tags
#define ALLOC_AMOUNT    5

// Ansi color escapes
#define COLORS_ESCAPE   "\033["
#define COLORS_NONE     COLORS_ESCAPE "m"
#define COLORS_GRAY     COLORS_ESCAPE "48;5;241m"
#define COLORS_GREEN    COLORS_ESCAPE "42m"
#define COLORS_PURPLE   COLORS_ESCAPE "48;5;129m"
#define COLORS_BLUE     COLORS_ESCAPE "44m"
#define COLORS_YELLOW   COLORS_ESCAPE "43m"
#define COLORS_RED      COLORS_ESCAPE "41m"
#define COLORS_ORANGE   COLORS_ESCAPE "48;5;208m"

#define PATH_SEPARATOR  "/"

/**
 * @typedef Type of operation to perform
 * @enum    -1  None
 * @enum     0  Unknown
 * @enum    's' Set tag extended attribute on a file or directory
 * @enum    'a' Append to the existing tags, or set if no prior tags exists
 * @enum    'r' Remove tags
 */
typedef enum OperationMode {
  OperationModeNone     = -1,
  OperationModeUnknown  = 0,
  OperationModeSet      = 's',
  OperationModeAdd      = 'a',
  OperationModeRemove   = 'r',
  OperationModeMatch    = 'm',
  OperationModeList     = 'l'
} OperationMode;

/**
 * @typedef Options to control how and what is printed via bit operations
 * @enum 0b00000001 Turn on filename display in output?
 * @enum 0b00000010 Turn on tags display in output?
 * @enum 0b00000100 Display tags each on own line?
 * @enum 0b00001000 Terminate each directory name with a slash
 * @enum 0b00010000 Terminate lines with NUL (\0)
 * @enum 0b00100000 Display tags in color
 */
typedef enum OutputFlags {
  OutputFlagsName             = (1 << 0),
  OutputFlagsTags             = (1 << 1),
  OutputFlagsGarrulous        = (1 << 2),
  OutputFlagsSlashDirectory   = (1 << 3),
  OutputFlagsNulTerminate     = (1 << 4),
  OutputFlagsColor            = (1 << 5),
  OutputFlagsShowHidden       = (1 << 6),
  OutputFlagsRecurseDirectory = (1 << 7)
} OutputFlags;
// clang-format on

/**
 * @typedef Available tag colors
 * @enum 0 None
 * @enum 1 Gray
 * @enum 2 Green
 * @enum 3 Purple
 * @enum 4 Blue
 * @enum 5 Yellow
 * @enum 6 Red
 * @enum 7 Orange
 */
typedef enum TagColor {
  TagColorNone,
  TagColorGray,
  TagColorGreen,
  TagColorPurple,
  TagColorBlue,
  TagColorYellow,
  TagColorRed,
  TagColorOrange
} TagColor;

/**
 * @typedef Tag data structure containing the two primary pieces of a user tag
 */
typedef struct UserTag {
  char *name;
  TagColor color;
} UserTag;

/**
 * @brief Parse command line arguments, main entry point
 * @param argc
 * @param argv
 * @return
 */
int parseCommandLine(int, char *const *);

/**
 * @brief Parse comma delimited tags input a tag array
 * @param arg Comma delimited tags with ':<0-7>' to indicate color
 * @param tagCount Count of the number of tags in the array
 * @return Pointer to the memory address of a UserTag array
 */
UserTag *parseTagsArgument(char *, int *);

/**
 * @brief Append tags to an existing or non-existing set of tags for a filename
 * or directory
 * @param path Path to the filename or directory
 * @param userTags Tags to be added
 * @param tagCount Count of tags to be added
 */
void addTags(char *, UserTag *, int);

/**
 * @brief Remove individual tags from an existing set for a filename or
 * directory
 * @param path Path to the filename or directory
 * @param userTags Tags to be removed
 * @param tagCount Count of tags to be removed
 */
void removeTags(char *path, UserTag *userTags, int tagCount);

/**
 * @brief Print a list of tags for a specified filename or directory
 * @param path Path to the filename or directory
 * @param outputFlags Output flags for output printing control
 * @note recursive chain
 */
void listTags(char *path, OutputFlags outputFlags);

/**
 * @brief Check file for matching tags
 * @param path
 * @param userTags
 * @param tagCount
 * @param outputFlags
 * @return
 */
_Bool matchTags(char *, UserTag *, int, OutputFlags);

/**
 * @brief Print a specific path and it's tags in a formatted output
 * @param path The path to process
 * @param userTags Array of user tags applied to the path
 * @param tagCount Count of the tags applied to the path
 * @param outputFlags Output options
 */
void printPath(char *path, UserTag *userTags, long tagCount,
               OutputFlags outputFlags);

/**
 * @brief Report formatted error message
 * @param fmt
 */
void reportError(const char *fmt, ...);

/**
 * @brief Print usage information
 * @param programName Name and or location of the executable
 */
void displayUsage(char *);

/**
 * @brief Print the version
 */
void displayVersion();

/**
 * @brief Create a formatted ansi color sequence for the given tag
 * @param buf The string buffer to receive the formatted output
 * @param tag The tag struct containing the label and color code
 */
void displayStringForTag(char *, UserTag *, OutputFlags);

/**
 * @brief Convert color string to TagColor integer code
 * @param color Color string
 * @return TagColor color code integer in the range of 0 to 7
 */
TagColor getColorCode(char *color);

/**
 * @brief Create a data pointer to a property list in binary format
 * @param length pointer to receive the length of the data
 * @param userTags Source user tags
 * @param tagCount Number of sourced user tags
 * @return Address of the property list in binary format
 * @note Free the memory after use
 */
unsigned char *createPlistBinary(unsigned long *length, UserTag *userTags,
                                 int tagCount);

/**
 * @brief Get any user tags applied to a particular path as a UserTag array
 * @param path The path to the filename or directory
 * @param tagCount Reference to receive the count of the tags in the array
 * @return pointer to a UserTag array
 */
UserTag *createUserTagsFromPath(char *, int *);

/**
 * @brief Free the dynamically memory allocated to the name member of UserTag
 * structs
 * @param userTags
 * @param tagCount
 */
void freeUserTags(UserTag *, int);

/**
 * @brief Compare callback used in sorting elements in an array UserTag
 * structs
 * @param a
 * @param b
 * @return
 */
int tagCompare(const void *, const void *);

#endif  // TAG_TAG_H
