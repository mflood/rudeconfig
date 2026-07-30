// demo.cpp - create an ini file, save, re-open, modify, print.
#include <rude/config.h>
#include <iostream>
#include <cstdlib>

int main()
{
    const char *path = "demo.ini";

    // --- Phase 1: create and save ---
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

        if (!cfg.save(path)) {
            std::cerr << "save failed: " << cfg.getError() << "\n";
            return 1;
        }
        std::cout << "[phase1] saved " << path << "\n";
    }

    // --- Phase 2: re-open, read, modify ---
    {
        rude::Config cfg;
        if (!cfg.load(path)) {
            std::cerr << "load failed: " << cfg.getError() << "\n";
            return 1;
        }

        cfg.setSection("database");
        std::cout << "[phase2] database.host = " << cfg.getValue("host") << "\n";
        std::cout << "[phase2] database.port = " << cfg.getIntValue("port") << "\n";
        std::cout << "[phase2] database.ssl  = " << (cfg.getBoolValue("ssl") ? "true" : "false") << "\n";

        // modify a value
        cfg.setIntValue("port", 6543);

        cfg.setSection("logging");
        cfg.setValue("level", "debug");

        if (!cfg.save()) {
            std::cerr << "re-save failed: " << cfg.getError() << "\n";
            return 1;
        }
        std::cout << "[phase2] modified port -> 6543, level -> debug, saved\n";
    }

    // --- Phase 3: verify modifications persisted ---
    {
        rude::Config cfg;
        if (!cfg.load(path)) {
            std::cerr << "reload failed: " << cfg.getError() << "\n";
            return 1;
        }
        std::cout << "[phase3] sections: " << cfg.getNumSections() << "\n";
        for (int i = 0; i < cfg.getNumSections(); i++) {
            std::cout << "  section[" << i << "] = '" << cfg.getSectionNameAt(i) << "'\n";
        }
        cfg.setSection("database");
        int port = cfg.getIntValue("port");
        cfg.setSection("logging");
        const char *level = cfg.getValue("level");
        std::cout << "[phase3] database.port = " << port << " (expect 6543)\n";
        std::cout << "[phase3] logging.level = " << level << " (expect debug)\n";

        if (port != 6543 || std::string(level) != "debug") {
            std::cerr << "VERIFY FAILED\n";
            return 1;
        }
        std::cout << "VERIFY OK\n";
    }
    return 0;
}
