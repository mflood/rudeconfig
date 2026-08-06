# RudeConfig

RudeConfig is a lightweight C++ library for reading, modifying, and writing
configuration and `.ini` files while preserving their human-friendly
structure.

- Preserves comments, blank lines, section order, and key order across a
  load-modify-save round trip.
- Provides typed access to strings, integers, booleans, doubles, and
  base64-backed binary values.
- Supports custom delimiters and comment characters.
- Has no external dependencies beyond a C++17 standard library.
- Builds as a static or shared library on Linux, macOS, and Windows.

First released in 2000, RudeConfig has been updated for modern C++ projects
with CMake, C++17, cross-platform CI, and package-consumer coverage.

```ini
# server settings
[database]
host=db.example.com
port=5432       # this comment survives a programmatic update
ssl=true
```

## Quick start

```cpp
#include <rude/config.h>

#include <iostream>

int main()
{
    rude::Config config;
    if (!config.load("app.ini")) {
        std::cerr << config.getError() << "\n";
        return 1;
    }

    config.setSection("database");
    std::cout << config.getValue("host") << "\n";
    std::cout << config.getIntValue("port") << "\n";

    config.setIntValue("port", 6543);
    if (!config.save()) {
        std::cerr << config.getError() << "\n";
        return 1;
    }
}
```

Compile an installed copy with pkg-config:

```sh
c++ -std=c++17 app.cpp $(pkg-config --cflags --libs rudeconfig)
```

## Build and install

RudeConfig 6.1.0 requires CMake 3.16 or newer and a C++17 compiler.

```sh
git clone --branch v6.1.0 https://github.com/mflood/rudeconfig.git
cd rudeconfig
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
cmake --install build --prefix ./install
```

The default build is static. Pass `-DBUILD_SHARED_LIBS=ON` to build a shared
library. For an install outside the system prefix, point CMake consumers at it
with `-DCMAKE_PREFIX_PATH=/path/to/install`. For pkg-config, add
`/path/to/install/lib/pkgconfig` to `PKG_CONFIG_PATH` (the library directory
may be `lib64` on some platforms).

The complete runnable example is in
[`examples/demo.cpp`](examples/demo.cpp).

## Use from CMake

With an installed copy:

```cmake
find_package(rudeconfig 6.1 REQUIRED)
target_link_libraries(myapp PRIVATE rudeconfig::rudeconfig)
```

Or include the stable release directly with `FetchContent`:

```cmake
include(FetchContent)
FetchContent_Declare(rudeconfig
    GIT_REPOSITORY https://github.com/mflood/rudeconfig.git
    GIT_TAG v6.1.0)
FetchContent_MakeAvailable(rudeconfig)
target_link_libraries(myapp PRIVATE rudeconfig::rudeconfig)
```

## Core concepts

### Current section

`setSection("name")` selects a section, creating it when needed. Subsequent
getters and setters operate in that section until another is selected.

Values before the first section header belong to an unnamed section. The
unnamed section is included in `getNumSections()`, so a file containing three
named section headers reports four sections.

### Missing values

`getValue()` returns `""` for a missing key, `getIntValue()` returns `0`, and
`getBoolValue()` returns `false`. Use the enumeration APIs when you need to
distinguish a missing value from one containing its type's default value.

### Structure preservation

Comments remain attached to their lines when values change. Unmodified
spacing, blank lines, and ordering are retained when the file is saved.

## Documentation and support

- Public API: [`src/config.h`](src/config.h)
- Runnable example: [`examples/demo.cpp`](examples/demo.cpp)
- Manual page: `rudeconfig(3)`
- Release notes: [`NEWS`](NEWS)
- Packaging notes: [`docs/PACKAGING.md`](docs/PACKAGING.md)
- Bug reports: [GitHub Issues](https://github.com/mflood/rudeconfig/issues)

## License

GPL-2.0-or-later. See [`COPYING`](COPYING).
