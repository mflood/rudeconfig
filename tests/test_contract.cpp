// Contract tests: every assertion here comes from a documented promise in
// config.h, not from observed behaviour.
//
// Round 2 found three defects in documented APIs while CI stayed green across
// five platforms, because the suite only exercised happy paths. This file
// walks the public header and asserts what it says, including the worked
// examples in the doc comments.
#include <rude/config.h>

#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>

static int failures = 0;

#define CHECK(cond)                                                              \
	do                                                                           \
	{                                                                            \
		if(!(cond))                                                              \
		{                                                                        \
			std::fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
			++failures;                                                          \
		}                                                                        \
	} while(0)

#define CHECK_STR(actual, expected)                                           \
	do                                                                        \
	{                                                                         \
		const char *a_ = (actual);                                            \
		if(!a_ || std::strcmp(a_, (expected)) != 0)                           \
		{                                                                     \
			std::fprintf(stderr, "FAIL %s:%d: expected \"%s\", got \"%s\"\n", \
						 __FILE__, __LINE__, (expected), a_ ? a_ : "(null)"); \
			++failures;                                                       \
		}                                                                     \
	} while(0)

static void write(const char *path, const char *content)
{
	std::ofstream out(path, std::ios::binary);
	out << content;
}

// ---------------------------------------------------------------------------
// getNumSections() / getSectionNameAt()
//
// "This number includes the default section - "" ... the return value will
// always be >= 1." and "Returns the section name at the given index, or NULL
// if the index is out of range. If the section has no name, but is a valid
// index, then it will return the empty string ("")".
// ---------------------------------------------------------------------------
static void testSectionEnumeration()
{
	write("c_sections.ini", "global = 1\n[alpha]\na = 1\n[beta]\nb = 2\n");

	rude::Config config;
	CHECK(config.load("c_sections.ini"));

	// Two named sections plus the implicit unnamed one.
	CHECK(config.getNumSections() == 3);
	CHECK_STR(config.getSectionNameAt(0), "");
	CHECK_STR(config.getSectionNameAt(1), "alpha");
	CHECK_STR(config.getSectionNameAt(2), "beta");

	// Out of range is NULL, both directions.
	CHECK(config.getSectionNameAt(3) == 0);
	CHECK(config.getSectionNameAt(-1) == 0);

	// Always >= 1, even for an entirely empty configuration.
	rude::Config empty;
	CHECK(empty.getNumSections() >= 1);
	CHECK_STR(empty.getSectionNameAt(0), "");

	// A file that opens with a named section still has the unnamed one.
	write("c_sections2.ini", "[only]\nx = 1\n");
	rude::Config leading;
	CHECK(leading.load("c_sections2.ini"));
	CHECK(leading.getNumSections() == 2);
	CHECK_STR(leading.getSectionNameAt(0), "");

	std::remove("c_sections.ini");
	std::remove("c_sections2.ini");
}

// ---------------------------------------------------------------------------
// setSection() / deleteSection()
//
// "Whitespace surrounding the section name is ignored... Section names are
// case sensitive." and "If the new section cannot be found, and shouldCreate
// is false, then the current section remains unchanged, and the method
// returns false." deleteSection "Returns false if the section does not exist
// or has already been deleted."
// ---------------------------------------------------------------------------
static void testSectionSelection()
{
	write("c_select.ini", "[  Padded  ]\np = 1\n[Case]\nc = 2\n");

	rude::Config config;
	CHECK(config.load("c_select.ini"));

	// Whitespace around the section name is ignored on lookup.
	CHECK(config.setSection("Padded", false));
	CHECK_STR(config.getValue("p"), "1");

	// Section names are case sensitive: "case" must not match "Case".
	CHECK(config.setSection("Case", false));
	CHECK(!config.setSection("case", false));

	// A failed lookup with shouldCreate=false leaves the current section
	// unchanged -- so the previously selected section's data is still here.
	CHECK_STR(config.getValue("c"), "2");

	// shouldCreate=true creates it.
	CHECK(config.setSection("Brand New", true));
	CHECK(config.getNumDataMembers() == 0);
	CHECK(config.setSection("Brand New", false));

	// deleteSection: true once, false thereafter, false for the unknown.
	CHECK(config.deleteSection("Brand New"));
	CHECK(!config.deleteSection("Brand New"));
	CHECK(!config.deleteSection("Never Existed"));

	std::remove("c_select.ini");
}

// ---------------------------------------------------------------------------
// getBoolValue()
//
// The header lists exactly which spellings are true and which are false:
// "Yes, yes, On, on, True, true, 1, T, t, Y, y" are true;
// "No, no, Off, off, False, false, 0, F, f, N, n" are false.
// The worked example also has "sand = Nope" as false.
// ---------------------------------------------------------------------------
static void testBoolContract()
{
	write("c_bool.ini",
		  "[b]\n"
		  "t1 = Yes\nt2 = yes\nt3 = On\nt4 = on\nt5 = True\nt6 = true\n"
		  "t7 = 1\nt8 = T\nt9 = t\nt10 = Y\nt11 = y\n"
		  "f1 = No\nf2 = no\nf3 = Off\nf4 = off\nf5 = False\nf6 = false\n"
		  "f7 = 0\nf8 = F\nf9 = f\nf10 = N\nf11 = n\nf12 = Nope\n");

	rude::Config config;
	CHECK(config.load("c_bool.ini"));
	CHECK(config.setSection("b", false));

	const char *truthy[] = {"t1", "t2", "t3", "t4", "t5", "t6", "t7", "t8", "t9", "t10", "t11"};
	for(unsigned i = 0; i < sizeof(truthy) / sizeof(truthy[0]); ++i)
	{
		if(!config.getBoolValue(truthy[i]))
		{
			std::fprintf(stderr, "FAIL %s:%d: %s (\"%s\") should be true\n",
						 __FILE__, __LINE__, truthy[i], config.getValue(truthy[i]));
			++failures;
		}
	}

	const char *falsy[] = {"f1", "f2", "f3", "f4", "f5", "f6", "f7", "f8", "f9", "f10", "f11", "f12"};
	for(unsigned i = 0; i < sizeof(falsy) / sizeof(falsy[0]); ++i)
	{
		if(config.getBoolValue(falsy[i]))
		{
			std::fprintf(stderr, "FAIL %s:%d: %s (\"%s\") should be false\n",
						 __FILE__, __LINE__, falsy[i], config.getValue(falsy[i]));
			++failures;
		}
	}

	// A name that does not exist is false, not a crash.
	CHECK(!config.getBoolValue("no-such-key"));

	std::remove("c_bool.ini");
}

// ---------------------------------------------------------------------------
// getIntValue() / getDoubleValue()
//
// "Returns 0 if the data does not exist or if an integer value cannot be
// determined." / "Returns the double value, or 0 if a double value cannot be
// determined."
// ---------------------------------------------------------------------------
static void testNumericContract()
{
	write("c_num.ini",
		  "[n]\ni = 42\nneg = -7\nzero = 0\nword = banana\nempty =\n"
		  "d = 3.5\ndneg = -0.25\ndword = not-a-number\n");

	rude::Config config;
	CHECK(config.load("c_num.ini"));
	CHECK(config.setSection("n", false));

	CHECK(config.getIntValue("i") == 42);
	CHECK(config.getIntValue("neg") == -7);
	CHECK(config.getIntValue("zero") == 0);

	// Documented fallbacks.
	CHECK(config.getIntValue("word") == 0);
	CHECK(config.getIntValue("empty") == 0);
	CHECK(config.getIntValue("no-such-key") == 0);

	CHECK(config.getDoubleValue("d") == 3.5);
	CHECK(config.getDoubleValue("dneg") == -0.25);
	CHECK(config.getDoubleValue("dword") == 0.0);
	CHECK(config.getDoubleValue("no-such-key") == 0.0);

	std::remove("c_num.ini");
}

// ---------------------------------------------------------------------------
// getValue()
//
// "Unless the value is quoted, leading and trailing whitespace is stripped.
// If the value is quoted, the quotes are removed, and leading and trailing
// whitespace within the quotes are preserved."
// ---------------------------------------------------------------------------
static void testValueWhitespaceContract()
{
	write("c_ws.ini",
		  "[w]\n"
		  "plain =    spaced out   \n"
		  "quoted = \"   kept   \"\n"
		  "innerquote = \"a b\"\n");

	rude::Config config;
	CHECK(config.load("c_ws.ini"));
	CHECK(config.setSection("w", false));

	CHECK_STR(config.getValue("plain"), "spaced out");
	CHECK_STR(config.getValue("quoted"), "   kept   ");
	CHECK_STR(config.getValue("innerquote"), "a b");

	// getStringValue is documented as the same call.
	CHECK_STR(config.getStringValue("plain"), "spaced out");
	CHECK_STR(config.getStringValue("quoted"), "   kept   ");

	std::remove("c_ws.ini");
}

// ---------------------------------------------------------------------------
// exists() / deleteData() / setters
// ---------------------------------------------------------------------------
static void testDataLifecycle()
{
	rude::Config config;
	CHECK(config.setSection("data", true));

	CHECK(!config.exists("k"));
	config.setStringValue("k", "v");
	CHECK(config.exists("k"));
	CHECK_STR(config.getValue("k"), "v");

	// Typed setters round-trip through their matching getters.
	config.setIntValue("i", -12);
	CHECK(config.getIntValue("i") == -12);
	config.setDoubleValue("d", 1.25);
	CHECK(config.getDoubleValue("d") == 1.25);
	config.setBoolValue("btrue", true);
	CHECK(config.getBoolValue("btrue"));
	config.setBoolValue("bfalse", false);
	CHECK(!config.getBoolValue("bfalse"));

	// deleteData: true once, false thereafter; false for an unknown name.
	CHECK(config.deleteData("k"));
	CHECK(!config.exists("k"));
	CHECK(!config.deleteData("k"));
	CHECK(!config.deleteData("never-existed"));
}

// ---------------------------------------------------------------------------
// Class-level defaults, and setConfigFile()/getConfigFile().
// ---------------------------------------------------------------------------
static void testDefaultsAndAccessors()
{
	rude::Config::setDefaultCommentCharacter(';');
	CHECK(rude::Config::getDefaultCommentCharacter() == ';');
	rude::Config::setDefaultDelimiter(':');
	CHECK(rude::Config::getDefaultDelimiter() == ':');
	rude::Config::setDefaultPreserveDeleted(false);
	CHECK(!rude::Config::getDefaultPreserveDeleted());
	rude::Config::setDefaultPreserveDeleted(true);
	CHECK(rude::Config::getDefaultPreserveDeleted());

	rude::Config::setDefaultConfigFile("some-default.ini");
	CHECK_STR(rude::Config::getDefaultConfigFile(), "some-default.ini");

	// Restore library defaults for any test that runs after this one.
	rude::Config::setDefaultCommentCharacter('#');
	rude::Config::setDefaultDelimiter('=');

	rude::Config config;
	config.setConfigFile("explicit.ini");
	CHECK_STR(config.getConfigFile(), "explicit.ini");
}

// ---------------------------------------------------------------------------
// clear() and getError().
// ---------------------------------------------------------------------------
static void testClearAndError()
{
	write("c_clear.ini", "[s]\nk = v\n");

	rude::Config config;
	CHECK(config.load("c_clear.ini"));
	CHECK(config.setSection("s", false));
	CHECK(config.exists("k"));

	config.clear();
	// Back to an empty configuration: only the implicit unnamed section.
	CHECK(config.getNumSections() == 1);

	// A failed load sets an error message.
	rude::Config bad;
	CHECK(!bad.load("definitely-not-here.ini"));
	CHECK(bad.getError() != 0);
	CHECK(bad.getError()[0] != '\0');

	std::remove("c_clear.ini");
}

int main()
{
	testSectionEnumeration();
	testSectionSelection();
	testBoolContract();
	testNumericContract();
	testValueWhitespaceContract();
	testDataLifecycle();
	testDefaultsAndAccessors();
	testClearAndError();

	if(failures)
	{
		std::fprintf(stderr, "%d contract check(s) failed\n", failures);
		return 1;
	}
	std::printf("contract OK\n");
	return 0;
}
