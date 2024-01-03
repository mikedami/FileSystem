#include <string>
#include <iostream>
#include <vector>
#include <cstdint>
#include <fstream>
#include <stdio.h>
#include <cstring>
#include <algorithm>
using namespace std;

class Wad {
private:

    struct Descriptor{ //can be a file or a directory
        vector<char> lumpData; 
        unsigned int offset = 0;
        unsigned int length = 0;
        string name = ""; //max 8 bytes
        string path = "";
        bool isThisADirectory = false;
        bool isThisAFile = false;
        bool isEndingNamespace = false;
    };

    vector<Descriptor> fileSystem;
    string magic = "";
    unsigned int numDescriptors = 0;
    unsigned int descriptorOffset = 0;
    string wadPath;

public:

    Wad();

    static Wad* loadWad(const string &path);

    bool isDigit(char ch);

    bool isMapMarker(const string name);

    bool isNamespaceMarker(const string name);

    void format();

    int setDirectory(const string &path);

    void setFile(const string &path);

    void createDirectory(const string &path);

    void createFile(const string &path);

    string getMagic();

    bool isContent(const string &path);

    bool isDirectory(const string &path);

    int getSize(const string &path);

    int getContents(const string &path, char* buffer, int length, int offset = 0);

    int getDirectory(const string &path, vector<string> *directory);

    int writeToFile(const string &path, const char *buffer, int length, int offset = 0);

    void updateWadFileDir(Descriptor elementStart, Descriptor elementEnd);

    void updateWadFile(Descriptor element, int control);
};