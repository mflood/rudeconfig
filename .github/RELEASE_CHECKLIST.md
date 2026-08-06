# Release checklist

Use this checklist for every tagged release so the source, documentation, and
downstream package recipes continue to describe the same artifact.

## Before tagging

- [ ] Update `project(... VERSION ...)`, `Config::version()` expectations, and
      the package-consumer version assertion together.
- [ ] Add the release and date to `NEWS`; verify README API examples against the
      public header and runnable example.
- [ ] Pin README clone and `FetchContent` examples to the release tag.
- [ ] Run a clean configure, build, and `ctest --output-on-failure` on a
      supported platform. Confirm both `package_smoke_static` and
      `package_smoke_shared` pass.
- [ ] Confirm CI passes on Linux x86-64/ARM, macOS, and Windows, including
      sanitizers, format, and tidy.
- [ ] Review the SPDX license identifier and package metadata in
      `docs/PACKAGING.md`.

## Publish and hand off

- [ ] Create and push the signed/annotated `vX.Y.Z` tag from the intended
      commit.
- [ ] Create the GitHub release from the matching `NEWS` section and attach or
      link the source archive.
- [ ] Generate the vcpkg and Conan checksums from the final release archive
      and record them in the submissions.
- [ ] Open or update the vcpkg and Conan submission PRs, linking their CI runs
      from the packaging tracking issue.
- [ ] Re-test README install commands using the published tag and verify the
      GitHub release, default README, and package recipes all show the same
      version.
