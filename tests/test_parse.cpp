// Parser tests against a hand-written .ini file.
//
// This is also the regression test for the platform-signedness bug fixed in
// 5.1.0: ParserJuly2004 stored istream::peek() results in `char`, so on
// platforms where char is unsigned (Linux/ARM, Linux/PowerPC) EOF was never
// detected and Config::load() looped forever. On those platforms this test
// hangs (and trips the CTest timeout) if the bug regresses.
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

int main()
{
	const char *path = "parse.ini";

	{
		std::ofstream out(path, std::ios::binary);
		out << "# top-of-file comment\n"
			<< "globalkey=globalvalue\n"
			<< "\n"
			<< "[server] # section comment\n"
			<< "hostname=example.com\n"
			<< "greeting=\"  hello world  \" # quoted, keeps inner spaces\n"
			<< "motto=caf\xC3\xA9 au lait\n" // UTF-8 high-bit bytes
			<< "empty=\n"
			<< "\n"
			<< "[flags]\n"
			<< "enabled=true\n"
			<< "retries = 7\n"
			<< "ratio = 0.25\n";
		// Note: file deliberately has no trailing newline after the last line
		// in earlier revisions of this test; keep one section ending at EOF.
	}

	rude::Config cfg;
	CHECK(cfg.load(path));

	// Value in the implicit unnamed section.
	CHECK(std::strcmp(cfg.getValue("globalkey"), "globalvalue") == 0);

	CHECK(cfg.setSection("server", false));
	CHECK(std::strcmp(cfg.getValue("hostname"), "example.com") == 0);
	CHECK(std::strcmp(cfg.getValue("greeting"), "  hello world  ") == 0);
	CHECK(std::strcmp(cfg.getValue("motto"), "caf\xC3\xA9 au lait") == 0);
	CHECK(std::strcmp(cfg.getValue("empty"), "") == 0);

	CHECK(cfg.setSection("flags", false));
	CHECK(cfg.getBoolValue("enabled"));
	CHECK(cfg.getIntValue("retries") == 7);
	CHECK(cfg.getDoubleValue("ratio") > 0.249 && cfg.getDoubleValue("ratio") < 0.251);

	// A file whose last line has no trailing newline must still terminate.
	{
		std::ofstream out("noeol.ini", std::ios::binary);
		out << "[last]\nkey=value";
	}
	rude::Config cfg2;
	CHECK(cfg2.load("noeol.ini"));
	CHECK(cfg2.setSection("last", false));
	CHECK(std::strcmp(cfg2.getValue("key"), "value") == 0);

	if(failures)
	{
		std::fprintf(stderr, "%d check(s) failed\n", failures);
		return 1;
	}
	std::printf("parse OK\n");
	return 0;
}
