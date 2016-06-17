To work on this example, a compile_commands.json file needs to be generated. It should look similar to this:

``` json
[
  {
    "directory": "/refactorial/example",
    "command": "clang -I. -I/qt3/include example.cpp",
    "file": "/refactorial/example/example.cpp"
  }
]
```

This example uses Qt 3 types to simulate a port forward from that version. All paths must be absolute paths.

With that in place, the following command can be run from this "example" folder:

```
../build/refactorial -print example.cpp < refactors.yml
```

