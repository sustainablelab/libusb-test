## ToC

Return to [README.md](README.md)

- [Build an executable that uses libusb](HELPME.md#build-an-executable-that-uses-libusb)
- [A really brief `make` primer](HELPME.md#a-really-brief-make-primer)
- [Data types](HELPME.md#data-types)
    - [`ssize_t`](HELPME.md#ssize_t)
    - [`libusb_context`](HELPME.md#libusb_context)
    - [`libusb_device`](HELPME.md#libusb_device)
- [What is a list in C](HELPME.md#what-is-a-list-in-c)
- [What is and is not a string constant](HELPME.md#what-is-and-is-not-a-string-constant)

# libusb 

## Build an executable that uses libusb

Compiling looks like this:

```compiling-example
g++ `pkg-config --cflags libusb-1.0` -c -o main.o main.cpp
```

Linking looks like this:

```linking-example
g++ -o bob main.o `pkg-config --cflags libusb-1.0` `pkg-config --libs-only-l libusb-1.0`
```

Here are the specific compiler and linker flags for my
environment:

```pkg-config-output
$ pkg-config --cflags libusb-1.0
-IC:/msys64/mingw64/include/libusb-1.0

$ pkg-config --libs-only-l libusb-1.0
-lusb-1.0 
```

I want all of the above and more to happen when I type `make` at
the terminal:

```make-at-terminal
$ make
g++ -g -Wall -Wformat `pkg-config --cflags libusb-1.0` -c -o main.o main.cpp
g++ -o bob main.o -g -Wall -Wformat `pkg-config --cflags libusb-1.0` `pkg-config --libs-only-l libusb-1.0`
Build complete for "MinGW"
```

Here is a `Makefile` to do that:

```make
EXE = bob
SOURCES = main.cpp
CXXFLAGS =
LIBS =

OBJS = $(addsuffix .o, $(basename $(notdir $(SOURCES))))
CXXFLAGS += -g -Wall -Wformat

LIBS += `pkg-config --libs-only-l libusb-1.0`
CXXFLAGS += `pkg-config --cflags libusb-1.0`

%.o:%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

all: $(EXE)
	@echo Build complete for MinGW

$(EXE): $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)
```

## A really brief `make` primer

*Read this bit if you want to understand how the Makefile works.*

- The `make` language has variables. `$(BOB)` is a Makefile
  variable named BOB that is defined somewhere in the Makefile
  like this `BOB = whatever-bob-is`
- `$(CXXFLAGS)` are the CFLAGS (compiler flags). These are OS
  dependent. I don't have anything insightful to say about them.
- `$(CXX)` is g++ (the gcc C++ compiler) or clang++ (the clang
  C++ compiler) depending on what compiler is on your system.

`CXX` isn't defined in the Makefile like `CXXFLAGS` is. Run
command `make what-compiler` to inspect `$(CXX)`. This is a handy
trick for any `make` variable you want to inspect. Here is the
recipe:

```make
.PHONY: what-compiler
what-compiler:
	@echo $(CXX)
```

These are the build recipes (the stuff that happens when you run
the command `make`):

```make
%.o:%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

all: $(EXE)
	@echo Build complete for MinGW

$(EXE): $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)

clean:
	rm -f $(EXE) $(OBJS)
	rm -f libs.txt
```

Make has a terse language for describing the build. The terseness
is from **pattern matching** and **automatic variables**.

First, know that you can run `make -n` to see what these recipes
look like when `make` expands them. The `-n` flag tells `make` to
print the recipe without actually doing anything. It's a handy
way to preview what the command `make` will do without actually
running it.

```bash
make -n
```

Now that you know what the expanded recipes look like, the rest
of this will make sense.

`%` is a pattern matching rule. `%.o` means any object file that
needs to get built will use the `%.o` recipe. The `%` now holds
the file stem, so `%.o: %.c` means that `bob.c` is a
pre-requisite to build target `bob.o`.

**pre-requisite** means that if `bob.c` has not been touched,
`bob.o` does not need to be rebuilt. But if `bob.c` has been
touched, `make` will rebuild `bob.o`.

The `$@` is the name of the target, `bob.o`. The `$<` is the
prerequisite(s). The order of `$@` and `$<` don't matter -- the
compiler figures out which is the output file (the target) and
which are the input files.



# Data types

There are exotic data types in the code. This section details the
**where**, **what**, and **why**.

- [`ssize_t`](HELPME.md#ssize_t)
- [`libusb_context`](HELPME.md#libusb_context)
- [`libusb_device`](HELPME.md#libusb_device)

## `ssize_t`

[`ssize_t` (click to view Microsoft docs)](https://docs.microsoft.com/en-us/windows/win32/winprog/windows-data-types#ssize_t)

### where is `ssize_t` defined

- `SSIZE_T` is a Windows type from `BaseTsd.h`
- `libusb.h` includes `basetsd.h` on MinGW:

```c
#include <basetsd.h>
```

- `basetsd.h` defines the **lower case** version used by
  `libusb`:

```c
typedef SSIZE_T ssize_t;
```

### what is `ssize_t`

- `SIZE_T` is the maximum number of bytes which a pointer
  can point to:

```c
typedef ULONG_PTR SIZE_T
```

- `SSIZE_T` is a "signed" version of
  [`SIZE_T` (click to view Microsoft docs)](https://docs.microsoft.com/en-us/windows/win32/winprog/windows-data-types#size_t)

```c
typedef LONG_PTR SSIZE_T;
```

### why does libusb use `ssize_t`

I don't know.


## `libusb_context`

Make a **context** to avoid interfering with other `libusb`
applications: [contexts (click to view libusb docs)](https://libusb.sourceforge.io/api-1.0/libusb_contexts.html).

```c
libusb_context *ctx;
```

Initialize *before* calling any `libusb` functions.

*Initialize the context:*

```c
libusb_init(&ctx)
```


**Do not** call any `libusb` functions *after* the context is
deinitialized and destroyed (freed).

*Deinitialize and destroy the context:*

```c
libusb_exit(ctx);
```

## `libusb_device`

[`libusb_device` (click to view libusb docs)](https://libusb.sourceforge.io/api-1.0/group__libusb__dev.html#ga77eedd00d01eb7569b880e861a971c2b)

From the docs:

> Structure representing a USB device detected on the system.

It is a `struct`, but **do not** use it by trying to access
`libusb_device` member variables!

This is how you use the `libusb_device` data type:

- Create a pointer to a `libusb_device`
    - pass that *pointer* to `libusb` functions like
      [`libusb_get_device_speed()`](https://libusb.sourceforge.io/api-1.0/group__libusb__dev.html#ga58c4e448ecd5cd4782f2b896ec40b22b)
- Create a list of USB devices
    - pass that list to functions like [`libusb_free_device_list()`](https://libusb.sourceforge.io/api-1.0/group__libusb__dev.html#gad3b8561d064bb3e1b8851ddeed3cd7d6)
    - pass the address of that list to `libusb` functions like
      [`libusb_get_device_list()`](https://libusb.sourceforge.io/api-1.0/group__libusb__dev.html#gac0fe4b65914c5ed036e6cbec61cb0b97)
    - a [list](HELPME.md#what-is-a-list-in-c) of USB devices is a
      pointer to a pointer to a `libusb_device`

# What is a list in C

Wait, what is a list? C has no lists...

C has arrays. But arrays aren't lists. An array is a chunk of
memory. To make an array I either need to give it a size or I
need to tell C the size indirectly by defining the values in the
array. Array size is not mutable.

A list only declares the size of its elements. A list **does
not** declare its overall size. I don't need to tell C how a big
a list is. I just need a sentinel at the end of the list to know
when I've reached the end.

Convention is that the sentinel evaluates to `false` so I can
iterate over the list elements in a loop and exit the loop when I
get to the end of the list.

Here's an example where I do that with a list of USB devices:

```c
static void print_devs(libusb_device **devs)

    libusb_device *dev;
    // Keep going until you run out of devices
    u8 i = 0; // device index
    while ( (dev = devs[i++]) )
    {
        ...
    }
```

When I hit the NULL device, the `while` conditional evaluates to
`false` and the loop terminates.

Let me explain lists by talking about strings, which is another
data type C doesn't have.

- a *list* of anything in C is a pointer to a pointer
- or I find it easier to visualize as a pointer to an array
- the common example is a **list of strings**:
- first, what is a string?
    - C has no string datatype, just [string constants](HELPME.md#what-is-and-is-not-a-string-constant):

    - in practice, a string variable in C is an array of
      `char` that contains a '\0' somewhere to denote
      where the string ends
    - so a *list* of strings is a *pointer* to an array of `char`
        - Why? Imagine incrementing the pointer. Now the pointer
          points to the *next* string (the next array of `char`).
- More abstractly, if you forget about the requirement that a C
  string contains a '\0':
    - then a C string is just a pointer to a `char`
    - so then a *list* of strings is a pointer to "a pointer to a
      `char`"
    - and that is exactly the notation used for passing a list of
      strings to a function, like this common function signature
      for `main()`:

    ```c
    int main(int argv, char** argc)
    ```

# What is and is not a string constant

To make a string constant, you write this: `"string constant"`
<--- double quotes matter!

The C compiler knows to treat this as an array of characters and
to add a `'\0'` character at the end.

Note that `'string constant'` is **not a string constant**:

- single quotes are for characters
- you *cannot* make a constant array of characters with
  this `'string constant'` notation
- you *can* make a constant character like this `'s'`
- but `'s'` is not `"s"`
- `"s"` is `{'s','\0'}`
- `'s'` is just `{'s'}`

Who cares? You gotta care. C doesn't hold your hand. The compiler
might throw warnings about this, but the executable still builds,
resulting in a program that's touching memory it shouldn't be as
string functions go searching for `'\0'` terminators.

Try this "safe" example.

First the working version. This should print `X`:

```c
printf("%s", "X");
```

Now the broken version. This *builds* but running the executable
causes a Segmentation fault:

```c
printf("%s", 'X');
```

*See if you can figure out why the second version causes a
Segmentation fault.*

The correct way to do the second version is like this:

```c
char x[2] = {'X','\0'};
printf("%s",x );
```
