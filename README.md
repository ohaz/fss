# FSS

FSS is a tool that mirrors parts of `git`.

It's mainly meant as training for me to understand the concepts behind `git` better and to learn C++.
This is by no means clean code and it's also untested. It also doesn't support all `git` commands.

## Compiling

### Requirements
The following tools/libraries need to be installed:
* `scons` (https://scons.org/doc/production/HTML/scons-user/ch01s02.html)
* `boost.program_options` (https://www.boost.org/doc/libs/1_58_0/doc/html/program_options.html) including the dev libraries, usually installed via `apt install libboost-program-options-dev`
* `boost.filesystem` (https://www.boost.org/doc/libs/1_79_0/libs/filesystem/doc/index.htm) including the dev libraries, usually installed via `apt install libboost-filesystem-dev`
* `cryptopp` (https://cryptopp.com/) including the dev libraries, usually installed via `apt install libcrypto++-dev`
* `nlohmann json` (https://github.com/nlohmann/json) including the dev libraries, usually installed via `apt install nlohmann-json3-dev`

### Compilation
To compile, just run `scons` in the root directory of this repo

## Usage

All commands listed here presume that the `fss` binary has been added to the `PATH` variable.

### Initialisation

To initialise a `fss` repository, change into the directory you want under version control and run:

```bash
fss init
```

### Adding

To add a file, be in a directory under `fss` version control and run:

```bash
fss add <filename>
```
Where `<filename>` is the name of the file you want to add.

### Commiting

To create a new commit, be in a directory under `fss` version control and run:

```bash
fss commit This_is_a_commit_message
```

At the moment the commit message has to be a single word (no spaces).