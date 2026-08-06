#include <rude/config.h>

#include <string>

int main()
{
	rude::Config config;
	config.setSection("consumer");
	config.setValue("package", "rudeconfig");
	return std::string(config.getValue("package")) == "rudeconfig" &&
	               std::string(rude::Config::version()) == "6.1.0"
	           ? 0
	           : 1;
}
