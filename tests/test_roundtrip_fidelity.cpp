// Round-trip fidelity tests, from the Phase 2 backlog.
//
// 1. An empty value used to be written as a bare "key" with no delimiter.
//    That reloaded correctly through this library, so it looked fine, but
//    other ini readers treat a line with no delimiter as malformed or as a
//    valueless flag -- so an empty string did not survive leaving rudeconfig.
//
// 2. load() on a directory returned true. A directory opens successfully as
//    an ifstream on POSIX and then reads as empty, so the caller was handed a
//    silently empty configuration with no error.
//
// Blank-line preservation is also checked here. The round-2 report suspected
// the blank line before a [section] was being eaten; it is not, and these
// cases pin that down so it stays that way.
#include <rude/config.h>

#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>

static int failures = 0;

#define CHECK(cond) \
    do { \
        if (!(cond)) { \
            std::fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
            ++failures; \
        } \
    } while (0)

static std::string slurp(const char *path)
{
    std::ifstream in(path, std::ios::binary);
    std::stringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

static void write(const char *path, const std::string &content)
{
    std::ofstream out(path, std::ios::binary);
    out << content;
}

// Strips carriage returns so line-ending conventions do not enter into the
// comparison. The library writes through a text-mode stream, so on Windows a
// saved "\n" comes back as "\r\n"; these tests are about which lines survive
// a round trip, not about which terminator the platform uses.
static std::string normalizeNewlines(const std::string &text)
{
    std::string out;
    out.reserve(text.size());
    for (std::string::size_type i = 0; i < text.size(); ++i) {
        if (text[i] != '\r')
            out += text[i];
    }
    return out;
}

// Loads and saves 'input', returning what ended up on disk, newline-normalized.
static std::string roundtrip(const std::string &input)
{
    const char *path = "fidelity.ini";
    write(path, input);
    {
        rude::Config config;
        config.load(path);
        config.save();
    }
    return normalizeNewlines(slurp(path));
}

int main()
{
    // ---- empty values keep their delimiter ----
    {
        const std::string out = roundtrip("[s]\nk=\nother=v\n");
        CHECK(out.find("k = \n") != std::string::npos ||
              out.find("k =\n") != std::string::npos);
        // The bare-key form must not come back.
        CHECK(out.find("\nk\n") == std::string::npos);
        if (out.find("\nk\n") != std::string::npos)
            std::fprintf(stderr, "  got: [%s]\n", out.c_str());

        // ...and it must still read back as an empty value.
        write("fidelity2.ini", out);
        rude::Config config;
        CHECK(config.load("fidelity2.ini"));
        CHECK(config.setSection("s", false));
        CHECK(config.exists("k"));
        CHECK(config.getStringValue("k") != 0);
        CHECK(std::strcmp(config.getStringValue("k"), "") == 0);
        CHECK(std::strcmp(config.getStringValue("other"), "v") == 0);
    }

    // ---- blank lines survive, in every position ----
    {
        // A blank line between a global value and the first section.
        CHECK(roundtrip("a = 1\n\n[s1]\nb = 2\n") == "a = 1\n\n[s1]\nb = 2\n");
        // Two consecutive blank lines.
        CHECK(roundtrip("a = 1\n\n\n[s1]\nb = 2\n") == "a = 1\n\n\n[s1]\nb = 2\n");
        // Leading blank lines before the first section.
        CHECK(roundtrip("\n\n[s1]\nb = 2\n") == "\n\n[s1]\nb = 2\n");
        // A trailing blank line at end of file.
        CHECK(roundtrip("[s1]\nb = 2\n\n") == "[s1]\nb = 2\n\n");
    }

    // ---- comments survive alongside data ----
    {
        const std::string in = "# leading comment\n[s1]\nb = 2\t # trailing\n";
        CHECK(roundtrip(in) == in);
    }

    // ---- load() rejects a directory instead of reporting success ----
    {
        rude::Config config;
        CHECK(!config.load("."));
        CHECK(config.getError() != 0);
        CHECK(config.getError()[0] != '\0');
        std::printf("load(\".\") error: %s\n", config.getError());
    }

    // ---- load() still reports a missing file as an error ----
    {
        rude::Config config;
        CHECK(!config.load("no-such-file-here.ini"));
        CHECK(config.getError()[0] != '\0');
    }

    std::remove("fidelity.ini");
    std::remove("fidelity2.ini");

    if (failures) {
        std::fprintf(stderr, "%d check(s) failed\n", failures);
        return 1;
    }
    std::printf("roundtrip fidelity OK\n");
    return 0;
}
