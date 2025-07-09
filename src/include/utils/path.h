#ifndef PATH_H
#define PATH_H

#include <string>

class Path {
private:
    Path();
    ~Path();
    static Path* instance;

public:
    std::string rootDirectory;
    std::string pluginDirectory;
    std::string aircraftDirectory;
    std::string aircraftFilename;
    
    static Path* getInstance();
    void reloadPaths();
};

#endif
