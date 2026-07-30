// Round-trip test: create a config, save it, reload it, mutate it,
// save again, reload again, and verify everything persisted.
#include <rude/config.h>

#include <cstdio>
#include <cstring>
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
	const char *path = "roundtrip.ini";

	// Phase 1: create and save.
	{
		rude::Config cfg;

		cfg.setSection("database");
		cfg.setValue("host", "db.example.com");
		cfg.setIntValue("port", 5432);
		cfg.setBoolValue("ssl", true);

		cfg.setSection("logging");
		cfg.setValue("level", "info");
		cfg.setValue("file", "/var/log/app.log");

		cfg.setSection("cache");
		cfg.setIntValue("ttl_seconds", 300);
		cfg.setDoubleValue("max_mem_gb", 1.5);

		CHECK(cfg.save(path));
	}

	// Phase 2: reload, verify, mutate, save.
	{
		rude::Config cfg;
		CHECK(cfg.load(path));

		cfg.setSection("database");
		CHECK(std::strcmp(cfg.getValue("host"), "db.example.com") == 0);
		CHECK(cfg.getIntValue("port") == 5432);
		CHECK(cfg.getBoolValue("ssl"));

		cfg.setIntValue("port", 6543);
		cfg.setSection("logging");
		cfg.setValue("level", "debug");

		CHECK(cfg.save());
	}

	// Phase 3: reload and verify mutations persisted.
	{
		rude::Config cfg;
		CHECK(cfg.load(path));

		// 3 named sections plus the implicit unnamed first section.
		CHECK(cfg.getNumSections() == 4);

		cfg.setSection("database");
		CHECK(cfg.getIntValue("port") == 6543);

		cfg.setSection("logging");
		CHECK(std::strcmp(cfg.getValue("level"), "debug") == 0);

		cfg.setSection("cache");
		CHECK(cfg.getIntValue("ttl_seconds") == 300);
		CHECK(cfg.getDoubleValue("max_mem_gb") > 1.49 && cfg.getDoubleValue("max_mem_gb") < 1.51);

		// Missing keys fall back to defaults.
		CHECK(cfg.getValue("no_such_key") != nullptr);
		CHECK(cfg.getIntValue("no_such_key") == 0);
	}

	if(failures)
	{
		std::fprintf(stderr, "%d check(s) failed\n", failures);
		return 1;
	}
	std::printf("roundtrip OK\n");
	return 0;
}
