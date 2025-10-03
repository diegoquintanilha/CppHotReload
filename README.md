# CppHotReload

You need a command line C++ compiler (both g++ and clang++ work fine).

You can compile and link everything regularly:

```
g++ main.cpp ReloadableFunction.cpp -o App.exe
```

Alternatively, you can enable hot reload by defining the macro HOT_RELOAD.
First compile the static files:

```
g++ -DHOT_RELOAD main.cpp FunctionLoader.cpp -o App.exe
```

Then compile the reloadable function file to a DLL with the same name as the function inside of it:

```
g++ -DHOT_RELOAD -shared ReloadableFunction.cpp -o DrawAscii.dll
```

TODO: add support for Linux .so files.

