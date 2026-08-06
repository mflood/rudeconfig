# Packaging handoff

RudeConfig 6.1.0 is ready to be used as the Rude SDK's vcpkg and Conan pilot.
The submission work is tracked separately so registry review can proceed
without coupling this repository's release documentation to external PRs.

## Package metadata

| Field | Value |
| --- | --- |
| Package name | `rudeconfig` |
| Version | `6.1.0` |
| Source tag | `v6.1.0` |
| Project URL | `https://github.com/mflood/rudeconfig` |
| License | `GPL-2.0-or-later` |
| Build system | CMake 3.16+ |
| Language level | C++17 |
| CMake target | `rudeconfig::rudeconfig` |
| pkg-config module | `rudeconfig` |
| Options | `BUILD_SHARED_LIBS` |
| Dependencies | None |

The repository's `COPYING` file contains GPL version 2 and the source headers
carry the "version 2 or later" grant, so registry metadata must use
`GPL-2.0-or-later`, not `GPL-2.0-only`.

## Evidence supplied by this repository

The CTest package smoke tests create clean static and shared builds, install
each to a temporary prefix, relocate the whole prefix, and then compile an
out-of-tree consumer against:

1. `find_package(rudeconfig 6.1 CONFIG)` and the exported
   `rudeconfig::rudeconfig` target on every platform.
2. The relocated `rudeconfig.pc` through CMake's pkg-config imported target on
   Unix platforms.

This exercises the same package boundaries used by vcpkg and Conan: public
headers, library artifacts, exported usage requirements, version metadata,
static/shared selection, and relocatability. Run it with:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

## Registry submission notes

- vcpkg: use `vcpkg_from_github`, pass `-DRUDECONFIG_BUILD_EXAMPLES=OFF`, let
  vcpkg control `BUILD_SHARED_LIBS`, run `vcpkg_cmake_config_fixup` with package
  name `rudeconfig`, install `COPYING`, and remove debug headers/share files.
- Conan Center: use a CMake-based Conan 2 recipe, expose `shared` and `fPIC`,
  disable examples/tests, publish the CMake target as
  `rudeconfig::rudeconfig`, and publish the pkg-config name as `rudeconfig`.
- Compute registry checksums from the final GitHub `v6.1.0` source archive at
  submission time. Registry PR URLs and their platform results belong on
  SIX-153, which SIX-144 blocks.

The release procedure that keeps these values synchronized is in
`.github/RELEASE_CHECKLIST.md`.
