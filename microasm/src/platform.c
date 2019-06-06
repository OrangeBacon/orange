#include <stdlib.h>
#include <limits.h>
#include "platform.h"

const char* resolvePath(const char* path) {
#ifdef _WIN32
    return _fullpath(NULL, path, _MAX_PATH);
#else
    char buf[PATH_MAX + 1];
    char* ret = realpath(path, buf);
    if(ret) {
        return buf;
    } else {
        return path;
    }
#endif
}