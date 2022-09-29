# tag

**tag** is a command line tool to manipulate tags on Mac OS X files (10.7.5 Lion and above), and to query for files with those tags.

## Usage

### Synopsis

    tag - A tool for manipulating and querying file tags.
      usage:
        tag -a | --add <tags> <path>...     Add tags to file
        tag -r | --remove <tags> <path>...  Remove tags from file
        tag -s | --set <tags> <path>...     Set tags on file
        tag -m | --match <tags> <path>...   Display files with matching tags
        tag -l | --list <path>...           List the tags on file
      <tags> is a comma-separated list of tag names; use * to match/find any tag. Follow the tag name with ":[0-7]" or ":Color" to apply a color  
      additional options:
            -v | --version      Display version
            -h | --help         Display this help
            -A | --all          Display invisible files while enumerating
            -R | --recursive    Recursively process directories
            -n | --name         Turn on filename display in output (default)
            -N | --no-name      Turn off filename display in output (list, match)
            -t | --tags         Turn on tags display in output (find, match)
            -T | --no-tags      Turn off tags display in output (list)
            -g | --garrulous    Display tags each on own line (list, match)
            -G | --no-garrulous Display tags comma-separated after filename (default)
            -c | --color        Display tags in color
            -p | --slash        Terminate each directory name with a slash
            -0 | --nul          Terminate lines with NUL (\0) for use with xargs -0

### Add tags to a file

The *add* operation adds one or more tags to the specified files without modifying any tags already there.

    tag --add tagname file
    tag --add tagname1,tagname2,... file1 file2...
    tag --add tagname1:Red,tagname2:6,... file1 file2...

### Remove tags from a file

The *remove* operation removes one or more tags from the specified files.

    tag --remove tagname file
    tag --remove tagname1,tagname2,... file1 file2...

To remove all tags from a file, use the wildcard \* to match all tags:

    tag --remove \* file

### Set tags on a file

The *set* operation replaces all tags on the specified files with one or more new tags.

    tag --set tagname file
    tag --set tagname1:Green,tagname2:Purple,... file1 file2...

### Show files matching tags

The *match* operation prints the file names that match the specified tags. Matched files must have at least *all* of the tags specified. Note that *match* matches only against the files that are provided as parameters (and those that it encounters if you use the --enter or --recursive options). To search for tagged files across your filesystem, see the *find* operation.

    tag --match tagname file
    tag --match tagname1,tagname2,... file1 file2...

You can use a wildcard (*) character in the tags list to match against any/all tags. Note, however, that you'll need to quote that * against shell expansion. To display all files in the current directory that have any combination of tags (but not *no* tags), use:

    tag --match '*' *

Conversely, to match against paths that have *no* tags, use an empty tag expression:

    tag --match '' *

Turn off --tags display mode for this operation to not show the tags on the file:

    tag --match '*' --no-tags *

Turn on garrulous output to format those tags onto multiple lines:

    tag --match '*' --tags --garrulous *

You may use short options as well. The following is equivalent to the previous command:

    tag -tgm '*' *

You may use the --recursive options to match the contents of, or recursively process, any directories provided.

    tag --match '*' --recursive .

If no file arguments are given, *match* will enumerate and match against the contents of the current directory:

    tag --match tagname

### List the tags on a file

This *list* operation lists the given files, displaying the tags on each:

    tag --list file
    tag --list file1 file2...

*list* is the default operation, so you may omit the list option:

    tag file1 file2...

As with *match*, if no file arguments are given, *list* will display the contents of the current directory and any tags on those files:

    tag

You can turn on garrulous mode for *list* as well:

    tag -g *

If you just want to see tags, but not filenames, turn off display of files:

    tag --no-name *

You may use the --enter or --recursive options to list the contents of, or recursively process, any directories provided:

    tag --list --enter .
    tag --list --recursive .
    tag -R .

### Colored Output

If your terminal supports ANSI color sequences, you may pass the -c/--color option.

### Get help

The --help option will show you the command synopsis:

```bash
tag --help
```

## Building and Installing


### Homebrew

```bash
brew install sneadc/personal/tag
```

Alternatively, you may add this repository to your taps before installing the
formula:

```bash
brew tap sneadc/personal
brew install tag
```

Building and Installing
---
You must have Xcode or the Command Line Tools installed to build/install.

To build without installing:

```
make
```

This will build **tag** into ./bin/tag

To build and install onto your system:

```
make && sudo make install
```

This will install **tag** at /usr/local/bin/tag and the man page at /usr/local/share/man/man1/tag.1

## Advanced Usage

- Wherever a "tagname" is expected, a list of tags may be provided. They must be comma-separated.
- Tag names may include spaces, but the entire tag list must be provided as one parameter: "tag1,a multiword tag name,tag3".
- Because the comma is used to separate tag names, it may not be used in tags (we don't support escaping that comma yet).
- For *match*, and *remove*, a tag name of '\*' is the wildcard and will match any tag. An empty tag expression '' will match only files with no tags.
- Wherever a "file" is expected, a list of files may be used instead. These are provided as separate parameters.
- Note that directories can be tagged as well, so directories may be specified instead of files.
- The --all, and --recursive options apply to --match, and --list, and control whether hidden files are processed and whether directories are entered and/or processed recursively. If a directory is supplied, but neither of --enter or --recursive, then the operation will apply to the directory itself, rather than to its contents.
- The operation selector --add, --remove, --set, --match, or --list may be abbreviated as -a, -r, -s, -m, or -l respectively. All of the options have a short version, in fact. See see the synopsis above, or output from help.
- If no operation selector is given, the operation will default to *list*.
- A *list* operation will default to the current directory if no directory is given.
- For compatibility with Finder, tags are compared in a case-insensitive manner.
- If you plan to pipe the output of **tag** through **xargs**, you might want to use the -0 option of each.
