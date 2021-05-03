# FUSE FTFS: Family Tree File System

[Fuse](https://en.wikipedia.org/wiki/Filesystem_in_Userspace) allows one
to create their own file system without editing kernel code. We're using in-memory approach to evaluate and implement a simple File System structure, based on file nodes' parenthood from the root node.

### Requisites

For compiling, testing and mounting this project you'll need to install the following packages:

- C compilers: gcc (different versions from 7. to 10 i.e. gcc-10), clang 6-9
- C build systems: cmake: 3.19.6 autoconf: 2.69, automake: 1.15.1
- [libfuse](https://github.com/libfuse/libfuse)
- Python 3

## Compiling

[Makefile](Makefile) has been created to easily compile and make a `./memfs` executable.

```console
$ make all
```

You might want to change Makefile instruction if you're using Clang instead of gcc.

## Tests

Each test program is a python3 script and can be run individually, i.e.:

```console
python3 ./tests/test_mount.py
```

For convenience our Makefile also comes with `check` target which will run all tests in serial:

```console
$ make check
```

### Implementation

- It should be possible to mount the filesystem as follows:

``` console
./memfs [mount point]
```

Where the argument "mount point" is the directory where the filesystem will be mounted at.

- creation of flat files and directories in the in-memory filesystem i.e, all files and directories stored in a single directory which is the root of the filesystem.

- creation of hierarchical files and directories in the in-memory filesystem i.e, create files and directories in directories inside the root directory of the filesystem.

- writing data to and reading data from files is supported. 

- appending data to an existing file is supported.

### Future work

- Implementation of every basic file management method, as file deletion.
- Lookup table to reduce file access workload in long paths.
- ?????? you want something to be implemented? ask to [me](https://instagram.com/instamisu) here or open an 'Issue'

## GG WP

I've struggled to find something appropriate on the internet that talks and treat FUSE in-memory implementation of File Systems, so I made one for future students that want to improve their system programming skills.

You can take this code, share it, use it, do what you want x

Ad maiora,
[Misu](https://instagram.com/instamisu)


