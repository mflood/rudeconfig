// Asserts Config::version() reports the project version CMake was configured
// with. Before 6.0.0 it returned a hardcoded "3.0" that had not tracked a
// release since 2005, so callers could not tell what they had linked.
#include <rude/config.h>

#include <cstdio>
#include <cstring>

int main()
{
	const char *reported = rude::Config::version();
	if(!reported)
	{
		std::fprintf(stderr, "FAIL: version() returned NULL\n");
		return 1;
	}
	if(std::strcmp(reported, EXPECTED_VERSION) != 0)
	{
		std::fprintf(stderr, "FAIL: version() reported \"%s\", expected \"%s\"\n",
					 reported, EXPECTED_VERSION);
		return 1;
	}
	std::printf("version OK (%s)\n", reported);
	return 0;
}
