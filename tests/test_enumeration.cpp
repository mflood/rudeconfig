// Regression tests for the data-member enumeration API, fixed in 5.1.1.
//
// Section::getDataNameAt()/getDataValueAt() indexed the underlying vector with
// no bounds check, so an out-of-range index was a heap-buffer-overflow read
// rather than the documented NULL return. The sentinel loop recommended by
// config.h's own doc comment ("while ((n = getDataNameAt(i++)) != NULL)")
// therefore read unowned memory on every use. Run under ASan to see the
// original failure.
//
// Also asserts that rude::Config is non-copyable: it owns its ConfigImpl
// through a raw pointer, so the implicit copy constructor produced two owners
// and a double free on destruction.
#include <rude/config.h>

#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>
#include <type_traits>

static int failures = 0;

#define CHECK(cond) \
    do { \
        if (!(cond)) { \
            std::fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
            ++failures; \
        } \
    } while (0)

int main()
{
    // rude::Config must not be copyable -- copying double-freed ConfigImpl.
    static_assert(!std::is_copy_constructible<rude::Config>::value,
                  "rude::Config must not be copy constructible");
    static_assert(!std::is_copy_assignable<rude::Config>::value,
                  "rude::Config must not be copy assignable");

    const char *path = "enumeration.ini";

    {
        std::ofstream out(path, std::ios::binary);
        out << "[Contact Info]\n"
            << "name= Mark Twain\n"
            << "email address = mark@twain\n"
            << "phone = 123.456.789\n";
    }

    rude::Config config;
    CHECK(config.load(path));
    CHECK(config.setSection("Contact Info", false));

    // The in-range lookups from config.h's own worked example.
    CHECK(config.getNumDataMembers() == 3);
    CHECK(config.getDataNameAt(0) != 0 && std::strcmp(config.getDataNameAt(0), "name") == 0);
    CHECK(config.getDataNameAt(2) != 0 && std::strcmp(config.getDataNameAt(2), "phone") == 0);

    // Out of range must be NULL, not a wild read. config.h documents exactly
    // this call: "config->getDataNameAt(3); // returns NULL (out of range)".
    CHECK(config.getDataNameAt(3) == 0);
    CHECK(config.getDataNameAt(4096) == 0);
    CHECK(config.getDataNameAt(-1) == 0);

    // The sentinel loop from the header's doc comment must terminate, and must
    // visit exactly the members reported by getNumDataMembers(). Each name it
    // yields must still resolve to a value.
    int visited = 0;
    int index = 0;
    const char *name = 0;
    while ((name = config.getDataNameAt(index++)) != 0)
    {
        CHECK(config.exists(name));
        CHECK(config.getStringValue(name) != 0);
        ++visited;
        if (visited > 100)
        {
            std::fprintf(stderr, "FAIL %s:%d: sentinel loop did not terminate\n",
                         __FILE__, __LINE__);
            ++failures;
            break;
        }
    }
    CHECK(visited == config.getNumDataMembers());

    // Enumeration on a freshly created empty section is still well behaved.
    CHECK(config.setSection("Nonexistent", true));
    CHECK(config.getNumDataMembers() == 0);
    CHECK(config.getDataNameAt(0) == 0);

    std::remove(path);

    if (failures)
    {
        std::fprintf(stderr, "%d check(s) failed\n", failures);
        return 1;
    }
    std::printf("enumeration OK\n");
    return 0;
}
