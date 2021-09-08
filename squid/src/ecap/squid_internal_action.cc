#include "squid_internal_action.h"
#include <dirent.h>
#include <string.h>
#include <stdio.h>

SquidInternalAction *SquidInternalAction::instance = NULL;

//forward from squid
class StoreEntry;
#define PRINTF_FORMAT_ARG2 __attribute__ ((format (printf, 2, 3)))
void storeAppendPrintf(StoreEntry *, const char *,...) PRINTF_FORMAT_ARG2;
#undef PRINTF_FORMAT_ARG2
namespace Mgr {
    void RegisterAction(char const *, char const *, void (*fnc)(StoreEntry *), int, int);
}



void read_file(StoreEntry *sentry, const char *directory_name, const char *d_name)
{
    std::string filename(directory_name);
    FILE *fd = NULL;
    int llstchar = -1;
    int lastchar = -1;
    int currchar = -1;
    bool drop = false;
    if (filename.c_str() && filename[strlen(filename.c_str()) - 1] != '/')
        filename += "/";
    filename += d_name;

    fd = fopen(filename.c_str(), "r");
    if (!fd) return ;

    storeAppendPrintf(sentry, "----------------------------------------------------------------------");
    storeAppendPrintf(sentry, "\nfilename: %s\n", filename.c_str());
    while ((currchar = fgetc(fd)) != EOF) {
        if ((llstchar=='\n' || llstchar=='\r') && (lastchar=='\n' || lastchar=='\r')
                && (currchar=='\n' || currchar=='\r')) {
            llstchar = lastchar;
            lastchar = currchar;
            continue;
        }
        if (currchar=='\n' || currchar=='\r') {
            drop = false;
        } else if (currchar=='#') {
            drop = true;
        }

        if (drop == false) storeAppendPrintf(sentry, "%c", currchar);

        llstchar = lastchar;
        lastchar = currchar;
    }

    fclose(fd);
    storeAppendPrintf(sentry, "\n");
}

void print_config(StoreEntry *sentry, const char *directory_name)
{
    DIR *fdd = opendir(directory_name);
    struct dirent *res = NULL;
    struct dirent entry = { 0 };
    if (!fdd) return ;

    while (!readdir_r(fdd, &entry, &res)) {
        if (res == NULL) break;
        if (!strcmp(entry.d_name, ".") || !strcmp(entry.d_name, "..")) continue;

        read_file(sentry, directory_name, entry.d_name);
    }


    closedir(fdd);
}

void transformation_list(StoreEntry *sentry)
{
    storeAppendPrintf(sentry, "Proxy-Cache Transformation List:");
    SquidInternalAction *sia = SquidInternalAction::getInstance();

    for (std::map<std::string, int>::iterator it = sia->begin();
         it != sia->end(); it++) {
        const char *directory = it->first.c_str();
        storeAppendPrintf(sentry, "\nDirectory %s\n", directory);
        print_config(sentry, directory);
    }
}

SquidInternalAction *SquidInternalAction::getInstance() {
    if (instance == NULL) {
        Mgr::RegisterAction("proxyList",
                            "Proxy-Cache Transformation List",
                            transformation_list, 0, 1);
        if (instance == NULL) {
            instance = new SquidInternalAction();
        }
    }
    return instance;
}

void SquidInternalAction::add(std::string dbpath)
{
   std::map<std::string, int>::iterator it;
   it = hash.find(dbpath);
   if (it != end()) {
     return ;
   } 

   hash[dbpath] = 1;
}

std::map<std::string, int>::iterator SquidInternalAction::begin()
{
    return hash.begin();
}

std::map<std::string, int>::iterator SquidInternalAction::end()
{
    return hash.end();
}
