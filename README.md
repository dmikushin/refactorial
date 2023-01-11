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

This is an update of the [original Refactorial](https://github.com/lukhnos/refactorial), which supports LLVM & Clang 12.

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
compiler options.

In order to generate a compilation database for a CMake-based project,
add the following option:

```
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS:STRING=ON <your build dir>
```

Only CMake is supported, there is no way to generate a compilation database out of a
Makefile or an IDE project (e.g. Microsoft `.vcproj` or Xcode's `.xcodeproj`).

Furthermore, the C++ compiler chosen for the CMake project should be Clang,
in order to ensure the best possible compatibility of compilation options.
The compiler could be set by adding `-DCMAKE_CXX_COMPILER=clang++` to the cmake
command line above.

You tell Refactorial what to do using a YAML config file. For example, to remove the
"get" prefix of all SampleNameSpace::Foo class functions names, prepare the
following `refactor.yml` config file:

```
---
Transforms:
  FunctionRename:
    Renames:
      - { From: 'SampleNameSpace::Foo::get(.+)', To: '\1' }
```

Here `\1` is the regular expression capture directive.

Then, in your build directory (where you have the compilation database), run:

```
./refactorial --spec refactor.yml --print --apply ./compile_commands.json
```

Refactorial will then run the `FunctionRename` transform on all source files in your
project.

If you only need to refactor some of the files, you can say:

```
---
Transforms:
  FunctionRename:
    WithinPaths:
      - foo.cpp
    Renames:
      - { From: 'SampleNameSpace::Foo::get(.+)', To: '\1' }
```

For more examples, take a look at our test cases in `tests/`. You can get an idea
what each source transform does and which parameters they take.

## Known Issues

- The result field initialization in the Example constructor is not changed.
- `QValueList<QVariant>` in the iterator loop is not changed.

## Copyright and License

Copyright © 2012 Lukhnos Liu and Thomas Minor  
Copyright © 2014 Martin Zenzes  
Copyright © 2014, 2016 Artem Kotsynyak  
Copyright © 2016 Ryan Wersal  
Copyright © 2023 Dmitry Mikushin

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

