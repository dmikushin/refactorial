# Refactorial

Refactorial is a Clang-based refactoring tool for C, C++, Objective-C, and Objective-C++. It has a number of nice properties:

1.  It runs independent of any IDE
2.  It refactors C, C++, Objective-C and Objective-C++ programs
3.  It can refactor both a library and its clients using the same script, so that library developers can upgrade libraries more easily by distributing the script
4.  It is extensible

Refactorial provides the following transforms:
 
*   [**Accessor**](doc/TRANSFORM-accessors.md): Synthesize getters and setters for designated member variables
*   **MethodMove**: Move inlined member function bodies to the implementation file
*   **ExtractParameter**: promote a function variable to a parameter to that function
*   **TypeRename**: Rename types, including tag types (enum, struct, union, class), template classes, Objective-C types (class and protocol), typedefs and     even bulit-in types (e.g. `unsigned` to `uint32_t`)
*   **RecordFieldRename**: Rename record (struct, union) fields, including C++ member variables
*   **FunctionRename**: Rename functions, including C++ member functions

This is an update of the [original refactorial](https://github.com/lukhnos/refactorial), which supports LLVM & Clang 12.

## Prerequisites

```
sudo apt install llvm-12-dev clang-12 libclang-12-dev libtbb-dev
```

## Building

```
mkdir build
cd build
cmake ..
make
```

## Using Refactorial

### Generating `compile_commands.json`

Clang-based tools (to be specific, those that use LibTooling) use the
"compilation database" to know which source files to parse with which
compiler options. It can be seen as a condensed Makefile.

CMake, which is a popular GNU Autotools replacement ("`./configure; make`"),
will happily generate the compilation database for your CMake project:

```
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS:STRING=ON <your build dir>
```

LLVM incidentally also uses CMake, and so do many popular open source projects.

There is no way to generate a compilation database out of a
Makefile or an IDE project (e.g. Microsoft `.vcproj` or Xcode's `.xcodeproj`).

You tell Refactorial using a YAML config file. For example, to rename all
classes with the prefix `Tree` to `Trie`, you can write a `refactor.yml` like
this:

    ---
    Transforms:        
      TypeRename:
        Ignore:
          - /usr/.*
          - /opt/.*
        Types:
          - class Tree(.*): Trie\1
          
Here `\1` is the regular expression capture directive.

Then, in your build directory (where you have the compilation database), run:

    refactorial < refactor.yml
    
Refactorial will then run the TypeRename transform on all source files in your
project.

If you only need to refactor some of the files, you can say:

    ---
    Files:
      - foo.cpp
      - bar.cpp
    Transforms:        
      TypeRename:
        Ignore:
          - /usr/.*
          - /opt/.*
        Types:
          - class Tree(.*): Trie\1

More documentation upcoming. Before that, take a look at our test cases in
`tests/`. You can get an idea what each source transform does and which
parameters they take.

## Known Issues

- The result field initialization in the Example constructor is not changed.
- `QValueList<QVariant>` in the iterator loop is not changed.

## Copyright and License

Copyright © 2012 Lukhnos Liu and Thomas Minor.

Released under the MIT License.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

