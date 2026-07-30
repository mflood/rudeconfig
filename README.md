# rudeconfig

A small C++ library for reading and writing configuration / `.ini` files.

RudeConfig preserves what other ini libraries throw away: comments, blank
lines, and file ordering all survive a load → modify → save round trip, so
config files that humans edit stay human-shaped.

```ini
# server settings
[database]
host=db.example.com
port=5432       # changing this value programmatically keeps this comment
ssl=true
```

First released in 2000 and battle-tested for years as part of the
[RudeServer](https://github.com/mflood) C++ CGI library family; modernized in
2026 (CMake, C++17, CI, ARM fixes).

## Quick start

```cpp
#include <rude/config.h>
#include <iostream>

int main()
{
    rude::Config cfg;
    cfg.load("app.ini");                       // false on failure — see cfg.getError()

    cfg.setSection("database");
    std::cout << cfg.getValue("host") << "\n"; // "" if missing
    int port = cfg.getIntValue("port");        // 0 if missing

    cfg.setIntValue("port", 6543);             // comments & layout are preserved
    cfg.save();
}
```

Compile with:

```sh
c++ -std=c++17 app.cpp $(pkg-config --cflags --libs rudeconfig)
```

## Building

Requires CMake ≥ 3.16 and any C++17 compiler. No other dependencies.

```sh
git clone https://github.com/mflood/rudeconfig
cd rudeconfig
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build            # run the tests
cmake --install build             # add --prefix ~/some/dir for a local install
```

This builds a static library by default; add `-DBUILD_SHARED_LIBS=ON` for a
shared library. An example program lives in [`examples/demo.cpp`](examples/demo.cpp)
(built as `build/examples/demo`).

### Using from CMake

```cmake
find_package(rudeconfig REQUIRED)
target_link_libraries(myapp PRIVATE rudeconfig::rudeconfig)
```

Or vendor it with `FetchContent`:

```cmake
include(FetchContent)
FetchContent_Declare(rudeconfig
    GIT_REPOSITORY https://github.com/mflood/rudeconfig
    GIT_TAG v6.0.1)
FetchContent_MakeAvailable(rudeconfig)
target_link_libraries(myapp PRIVATE rudeconfig::rudeconfig)
```

## API notes

- The full API is documented in [`src/config.h`](src/config.h) and the
  `rudeconfig(3)` man page.
- **Sections**: `setSection("name")` sets the *current* section (creating it
  if needed); getters and setters then operate within it. Values before the
  first `[section]` header live in an unnamed section, which counts toward
  `getNumSections()` — a file with three `[...]` headers reports four sections.
- **Missing keys** are safe: `getValue` returns `""` (never `nullptr`),
  `getIntValue` returns `0`, `getBoolValue` returns `false`.
- **Comments travel with their line**: `key=value # note` keeps the note when
  the value is rewritten.

## History

- **5.1.0** (2026) — CMake build; C++17; fixed a char-signedness bug that made
  `load()` hang on platforms with unsigned `char` (Linux/ARM); UTF-8-safe
  whitespace handling; CI on Linux (x86_64 + ARM), macOS, and Windows. The
  legacy autotools files are still present but no longer maintained.
- **5.0.5** (2005) — last release of the original autotools era; packaged in
  Fedora and FreeBSD ports.

## License

GPL-2.0-or-later — see [COPYING](COPYING).
