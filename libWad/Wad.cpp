#include <string>
#include <iostream>
#include <vector>
#include <cstdint>
#include <fstream>
#include <stdio.h>
#include <cstring>
#include <algorithm>
#include "Wad.h"
using namespace std;

Wad::Wad(){
    magic = "";
    numDescriptors = 0;
    descriptorOffset = 0;
    wadPath = "";
}

Wad* Wad::loadWad(const string &path){

    Wad* ret = new Wad;
    ret->wadPath = path;

    ifstream file(path, ios::binary);
    if(!file.is_open()){
        return nullptr;
    }

    //Setting magic, number of descriptors, and descriptor offset

    char temp[9];
    string tempS;
    file.read(temp, 4);
    temp[4] = '\0';
    tempS = temp;
    ret->magic = tempS;

    file.read(reinterpret_cast<char*>(&ret->numDescriptors), sizeof(ret->numDescriptors));
    file.read(reinterpret_cast<char*>(&ret->descriptorOffset), sizeof(ret->descriptorOffset));

    //For number of descriptors, set offset, length, and name of each one
    file.seekg(ret->descriptorOffset);
    for(unsigned int i = 0; i < ret->numDescriptors; i++){

        Descriptor element;

        file.read(reinterpret_cast<char*>(&element.offset), sizeof(element.offset)); //4
        file.read(reinterpret_cast<char*>(&element.length), sizeof(element.length)); //4

        file.read(temp, 8);
        temp[8] = '\0';
        tempS = temp;
        element.name = tempS;
        //all paths are already initialized to be ""

        ret->fileSystem.push_back(element);
    }

    //Call format to read in data from lumps and organize correctly
    ret->format();
    file.close();
    return ret;
}

bool Wad::isDigit(char ch){
    int check = static_cast<int>(ch);
    if(check >= 48 && check <= 57){
        return true;
    } else{
        return false;
    }
}

bool Wad::isMapMarker(const string name){
    if(name.size() != 4){
        return false;
    }

    int count = 0;

    if(name.at(0) == 'E'){
        count++;
    }
    if(isDigit(name.at(1))){
        count++;
    }
    if(name.at(2) == 'M'){
        count++;
    }
    if(isDigit(name.at(3))){
        count++;
    }
    if(count == 4){
        return true;
    } else {
        return false;
    }
}

bool Wad::isNamespaceMarker(const string name){
    if((name.find("_START") != string::npos && (name.length() == 7 || name.length() == 8))){
        return true;
    }
    else if((name.find("_END") != string::npos && (name.length() == 5 || name.length() == 6))){
        return true;
    } else {
        return false;
    }
}

void Wad::format(){

    string rootPath = "/";
    string currentPath = "";
    string currentName = "";

    for(unsigned int i = 0; i < fileSystem.size(); i++) { //looping through descriptors
        Descriptor debug = fileSystem.at(i);        
        currentPath = "/";
        currentName = fileSystem.at(i).name;

        if(isMapMarker(currentName) || isNamespaceMarker(currentName)){ //Setting path if it is a directory
        
            if(fileSystem.at(i).isThisADirectory == false && currentName.find("_END") == string::npos){ 
                //In case createDirectory already recursively made descriptor a directory, nothing will happen
                
                if(isNamespaceMarker(currentName)){ 
                    if(currentName.find("_START") != string::npos){
                        currentName.erase(currentName.find("_START"), 6); 
                    } 
                }
                
                currentPath += currentName;
                i = setDirectory(currentPath);
            }

        } else { //only case if we're adding files inside a Namespace directory, not a Map directory
            if(fileSystem.at(i).isThisAFile == false){
                if(currentPath == "/"){ //in case we're adding files in root directory
                    currentPath += currentName;
                    setFile(currentPath);
                } 
            }
        }
    }
    return;
}

int Wad::setDirectory(const string &path){

    //Called in loadWad to set pre-set directories
    string descripName;
    string currentPath = path;

    descripName = path.substr(path.find_last_of("/")+1);
    if(descripName.find("_START") != string::npos){
        descripName.erase(descripName.find("_START"), 6);
    }

    if(isMapMarker(descripName)){
            for(unsigned int i = 0; i < fileSystem.size(); i++){

            if(fileSystem.at(i).name == descripName && fileSystem.at(i).isThisADirectory == false){
                fileSystem.at(i).isThisADirectory = true;
                fileSystem.at(i).path = path;

                Descriptor debug = fileSystem.at(i);
                for(unsigned int j = i+1; j < i+11; j++){
                    //For next 10 descriptors, set lumpdata by calling setFile
                    currentPath = path;
                    currentPath += ("/" + fileSystem.at(j).name);
                    setFile(currentPath);
                    Descriptor debug2 = fileSystem.at(j);
                }
                return 0;
            } 
        } //We never recursively call setDirectory again inside a Map Marker

    } else{
        string descripEnd = descripName + "_END";
        descripName += "_START";
        int endIndex = 0;
        int startIndex = 0;

        for(unsigned int i = 0; i < fileSystem.size(); i++){

            if(fileSystem.at(i).name == descripName && fileSystem.at(i).isThisADirectory == false){
                fileSystem.at(i).isThisADirectory = true;
                fileSystem.at(i).path = path;
                startIndex = i+1;
                for(unsigned int j = i; j < fileSystem.size(); j++){
                    if(fileSystem.at(j).name == descripEnd){
                        fileSystem.at(j).path = path + "END"; //for duplicate named directories to find by path
                        fileSystem.at(j).isEndingNamespace = true;
                        Descriptor debug = fileSystem.at(j);
                        endIndex = j;
                        break;
                    }
                }
                break;
            }
        }

        for(int i = startIndex; i < endIndex; i++){

            Descriptor debug = fileSystem.at(i);

            currentPath = fileSystem.at(startIndex-1).path + "/" + fileSystem.at(i).name;
            if(currentPath.find("_START") != string::npos){
                currentPath.erase(currentPath.find("_START"), 6);
            }
            //Recursive call if we are in nested namespace

            if((isMapMarker(fileSystem.at(i).name) || isNamespaceMarker(fileSystem.at(i).name)) && fileSystem.at(i).isThisADirectory == false){
                int retIndex = setDirectory(currentPath);
                fileSystem.at(i).isThisADirectory = true;
                i = retIndex;

            } else if(fileSystem.at(i).isThisAFile == false){
                setFile(currentPath);
            }
        }
        return endIndex;
    }
    return 0;
}

void Wad::setFile(const string &path){
    
    //On loadWad initializes all files with content already in WadFile
    ifstream file;
    file.open(wadPath);
    if(!file.is_open()){
        return;
    }

    string descripName;
    int tempOffset;
    int tempLength;
    descripName = path.substr(path.find_last_of("/")+1);

    for(unsigned int i = 0; i < fileSystem.size(); i++){

        //Root case
        if(path == "/" && fileSystem.at(i).isThisAFile == false && fileSystem.at(i).isThisADirectory == false){
            fileSystem.at(i).isThisAFile = true;
            fileSystem.at(i).path = path + fileSystem.at(i).name;
            fileSystem.at(i).isThisAFile = true;
            tempOffset = fileSystem.at(i).offset;
            tempLength = fileSystem.at(i).length;
            char buffer[tempLength];
            file.seekg(tempOffset);
            file.read(buffer, tempLength);
            
            for(int j = 0; j < tempLength; j++){
                fileSystem.at(i).lumpData.push_back(buffer[j]);
            }

            file.close();

            return;
        }

        else if(fileSystem.at(i).name == descripName && fileSystem.at(i).isThisAFile == false){
            
            //Setting lump data based on offset of where content descriptor is, along with correct file path
            fileSystem.at(i).isThisAFile = true;
            fileSystem.at(i).path = path;
            tempOffset = fileSystem.at(i).offset;
            tempLength = fileSystem.at(i).length;
            char buffer[tempLength];
            file.seekg(tempOffset);
            file.read(buffer, tempLength);
            
            for(int j = 0; j < tempLength; j++){
                fileSystem.at(i).lumpData.push_back(buffer[j]);
            }

            file.close();
            return;
        }
    }
}

void Wad::createDirectory(const string &path){ 
//Can only be called from a namespace marker directory

    string directoryName;
    string directoryNameEnd;
    string currentPath = path;
    string tempPath;
    string parentDir;
    string parentEndPath;
    string parentDirEnd;
    bool control = false;
    if(path.find_last_of("/") == path.length()-1){
        currentPath.erase(path.find_last_of("/"));
        directoryName = currentPath.substr(currentPath.find_last_of("/")+1);
    } else {
        directoryName = path.substr(path.find_last_of("/")+1);
    }
    //Finds name of what we are adding

    //Too large a name
    if(directoryName.size() > 2){
        return;
    }

    //Root case
    if(currentPath == "/" + directoryName){
        parentDir = "/";
        parentEndPath = currentPath + "END";
        control = true;
    }
    if(!control){
        
        //Otherwise sets parentDir to look for
        tempPath = currentPath;
        tempPath = tempPath.erase(path.find(directoryName), directoryName.length());
        if(tempPath.find_last_of("/") == tempPath.length()-1){
            int place = tempPath.find_last_of("/");
            tempPath.erase(place);
        }
        parentDir = tempPath.substr(tempPath.find_last_of("/")+1);

        //If parent directory doesn't exist
        parentDirEnd = parentDir + "_END";
        parentDir += "_START";

        bool parentExist = false;
        parentEndPath = tempPath + "END";

        for(unsigned int i = 0; i < fileSystem.size(); i++){
            if(tempPath == fileSystem.at(i).path){
                parentExist = true;
                break;
            }
        }
        if(parentExist == false){
            return;
        }

    }

    if(!isMapMarker(directoryName)){
        directoryNameEnd += directoryName + "_END";
        directoryName += "_START";
    }

    //Finds where we are adding based on end index of namespace dir we are inside
    int index;
    for(unsigned int i = 0; i < fileSystem.size(); i++){

        if(parentDir == "/"){
            index = fileSystem.size();
            break;
        } 

        if(fileSystem.at(i).path == parentEndPath){
            index = i;
            break;
        }
    }

    //Create new descriptors and update file
    Descriptor dirStart;
    dirStart.name = directoryName;
    dirStart.isThisADirectory = true;
    dirStart.path = currentPath;

    Descriptor dirEnd;
    dirEnd.name = directoryNameEnd;
    dirEnd.path = currentPath+"END";
    dirEnd.isEndingNamespace = true;
    
    vector<Descriptor>::iterator it = fileSystem.begin() + index;
    fileSystem.insert(it, dirStart);
    index++;
    it = fileSystem.begin() + index;
    fileSystem.insert(it, dirEnd);

    //Update WAD File
    updateWadFileDir(dirStart, dirEnd);

    return;
}

void Wad::createFile(const string &path){ //Only can happen in namespace marker directories
    string currentPath = path;
    string fileName = path.substr(path.find_last_of("/")+1);
    currentPath = currentPath.erase(path.find(fileName), fileName.length());

    if(isContent(path) || fileName.length() > 8 || isMapMarker(fileName) || isNamespaceMarker(fileName)){
        return;
    }    

    //Root case
    if(currentPath == "/"){
        Descriptor element;
        element.name = fileName;
        element.isThisAFile = true;
        element.path = currentPath + element.name;
        fileSystem.push_back(element);
        updateWadFile(element, 0);
        return;
    }

    string parentDir = currentPath.erase(currentPath.size()-1) + "END";
    //Specific parentDirEnd is based on start path added with "END" for unique indentification

    int index;
    for(unsigned int i = 0; i < fileSystem.size(); i++){
        if(fileSystem.at(i).path == parentDir){ 
            Descriptor debug = fileSystem.at(i);
            index = i;
            Descriptor element;
            element.name = fileName;
            element.isThisAFile = true;
            element.path = path;
            vector<Descriptor>::iterator it = fileSystem.begin() + index;
            fileSystem.insert(it, element);
            updateWadFile(element, 0);
            return;
        }
        //Found correct index, insert new element at that index and push everything forward
    }
}

string Wad::getMagic(){
    return magic;
}

bool Wad::isContent(const string &path){
    if(path.find_last_of("/") == path.length()-1){
        return false;
    } else{

        //Uses built in file bool and if path is valid (not directory)
        for(unsigned int i = 0; i < fileSystem.size(); i++){
            if(fileSystem.at(i).path == path){
                if(fileSystem.at(i).isThisAFile == true){
                    return true;
                } else{
                    return false;
                }
            }
        }
        return false;
    }
}

bool Wad::isDirectory(const string &path){

    string currentPath = path;

    if(path == "/"){
        return true;
    } else if(path == ""){
        return false;
    }

    if(path.find_last_of("/") == path.length()-1){
        currentPath = currentPath.erase(path.find_last_of("/"));
    }

    //Utilizes built in isDirectory bool and universal directory rules
    for(unsigned int i = 0; i < fileSystem.size(); i++){
        if(fileSystem.at(i).path == currentPath){
            if(fileSystem.at(i).isThisADirectory == true){
                return true;
            }
        }
    }
    return false;
}

int Wad::getSize(const string &path){
    if(!isContent(path)){
        return -1;
    }
    for(unsigned int i = 0; i < fileSystem.size(); i++){
        if(path == fileSystem.at(i).path){
            return fileSystem.at(i).lumpData.size();
        }
    }
}

int Wad::getContents(const string &path, char* buffer, int length, int offset){
    if(!isContent(path)){
        return -1;
    }
    int ans = 0;
    for(unsigned int i = 0; i < fileSystem.size(); i++){
        Descriptor debug = fileSystem.at(i);
        if(fileSystem.at(i).path == path){
            const vector<char> &data = fileSystem.at(i).lumpData;
            if(offset < 0 || offset >= static_cast<int>(data.size())){
                return 0;
            }
            ans = min(length, static_cast<int>(data.size()-offset));
            copy(data.begin() + offset, data.begin() + offset + ans, buffer);
            return ans;
            //For length specified, if valid content pass in from lumpdata
        }
    }
    return -1;
}

int Wad::getDirectory(const string &path, vector<string> *directory){
    if(!isDirectory(path)){
        return -1;
    }
    int ans = 0;
    string childElementPath;
    string currentPath = path;

    for(unsigned int i = 0; i < fileSystem.size(); i++){
        if(fileSystem.at(i).path == currentPath || (fileSystem.at(i).path+"/") == currentPath || currentPath == "/" ){

            if(path.find_last_of("/") != path.length()-1){
                currentPath += "/";
            }

            for(unsigned int j = 0; j < fileSystem.size(); j++){
                if(fileSystem.at(j).isEndingNamespace == false){
                    
                    //If child has same path as current directory we're searching inside
                    childElementPath = fileSystem.at(j).path;
                    if((childElementPath.find_last_of("/") == childElementPath.length()-1) && childElementPath != ""){
                        childElementPath.erase(childElementPath.find_last_of("/"));
                    }
                    childElementPath.erase(childElementPath.find_last_of("/")+1);

                    if(childElementPath == currentPath){
                        string currName = fileSystem.at(j).name;

                        //For namespace directories
                        if(currName.find("_START") != string::npos && fileSystem.at(j).isThisADirectory == true){
                            currName.erase(currName.find("_START"), 6);
                        } else if(currName.find("_END") != string::npos && fileSystem.at(j).isThisADirectory == true){
                            currName.erase(currName.find("_END"), 4);
                        }
                        
                        directory->push_back(currName);
                        ans++;
                    }
                }
            }
            return ans;
        }
    }
}

int Wad::writeToFile(const string &path, const char *buffer, int length, int offset){

    if(!isContent(path)){
        return -1;
    }

    int tempOffset = descriptorOffset + offset;
    //This is where we're writing in new lump data

    int ans = 0;
    for(unsigned int i = 0; i < fileSystem.size(); i++){

        if(fileSystem.at(i).path == path){
            if(fileSystem.at(i).lumpData.size() != 0){
                return 0;
            }

            //Write data to actual wadFile
            fstream file(wadPath, ios::binary | ios::in | ios::out);
            if(!file.is_open()){
                return 0;
            }

            //Copy descriptor list
            streampos copySize = numDescriptors*16;
            file.seekg(tempOffset);
            vector<char> existingData(copySize);
            file.read(existingData.data(), copySize);
            
            //Write new lump data
            file.seekp(tempOffset);
            file.write(buffer, length);
            
            //Write back descriptor list moved forward
            file.write(existingData.data(), existingData.size());

            //Copy data into fileSystem
            fileSystem.at(i).offset = tempOffset;
            fileSystem.at(i).length = length;
            for(int j = offset; j < (offset+length); j++){
                fileSystem.at(i).lumpData.push_back(buffer[j]);
                ans++;
            }

            Descriptor element = fileSystem.at(i);

            //Update descriptor offset inside header
            file.seekp(8);
            descriptorOffset += length;
            file.write(reinterpret_cast<const char*>(&descriptorOffset), sizeof(descriptorOffset));

            //Update file info inside wadfile
            int prevOffset = (i*16) + descriptorOffset;
            file.seekp(prevOffset);
            file.write(reinterpret_cast<const char*>(&element.offset), sizeof(element.offset));
            file.write(reinterpret_cast<const char*>(&element.length), sizeof(element.length));
            file.write(element.name.c_str(), 8);

            file.close();

            return ans;
        }
    }
    return -1;
}

void Wad::updateWadFileDir(Descriptor elementStart, Descriptor elementEnd){
    int prevOffset;
    fstream file(wadPath, ios::binary | ios::in | ios::out);
    if(!file.is_open()){
        return;
    }

    for(unsigned int i = 0; i < fileSystem.size(); i++){
        if(fileSystem.at(i).path == elementStart.path){
            //Copy exisiting descriptor data based on item before what we are adding
            Descriptor prev = fileSystem.at(i-1);

            prevOffset = (i*16) + descriptorOffset;
            file.seekp(0, ios::end);
            streampos totalSize = file.tellp();
            streampos copySize = totalSize - static_cast<streampos>(prevOffset);

            file.seekg(prevOffset);
            vector<char> exisitingData(copySize);
            file.read(exisitingData.data(), copySize);

            file.seekp(prevOffset);
            //write new descriptor data
            file.write(reinterpret_cast<const char*>(&elementStart.offset), sizeof(elementStart.offset));
            file.write(reinterpret_cast<const char*>(&elementStart.length), sizeof(elementStart.length));
            file.write(elementStart.name.c_str(), 8);

            file.write(reinterpret_cast<const char*>(&elementEnd.offset), sizeof(elementEnd.offset));
            file.write(reinterpret_cast<const char*>(&elementEnd.length), sizeof(elementEnd.length));
            file.write(elementEnd.name.c_str(), 8);

            //write back rest of descriptor data in front
            file.write(exisitingData.data(), exisitingData.size());

            //Update header data (number of descriptors)
            numDescriptors += 2;
            file.seekp(4);
            file.write(reinterpret_cast<const char*>(&numDescriptors), sizeof(numDescriptors));
            file.close();

            return;
        }
    }
}

void Wad::updateWadFile(Descriptor element, int control){
    int prevOffset;
    fstream file(wadPath, ios::binary | ios::in | ios::out);
    if(!file.is_open()){
        return;
    }

    for(unsigned int i = 0; i < fileSystem.size(); i++){
        if(fileSystem.at(i).path == element.path){
            //Copy exisiting descriptor data based on item before what we are adding


            prevOffset = (i*16) + descriptorOffset;
            file.seekp(0, ios::end);
            streampos totalSize = file.tellp();
            streampos copySize = totalSize - static_cast<streampos>(prevOffset);

            file.seekg(prevOffset);
            vector<char> exisitingData(copySize);
            file.read(exisitingData.data(), copySize);

            file.seekp(prevOffset);
            //write new descriptor data
            file.write(reinterpret_cast<const char*>(&element.offset), sizeof(element.offset));
            file.write(reinterpret_cast<const char*>(&element.length), sizeof(element.length));
            file.write(element.name.c_str(), 8);

            //write back rest of descriptor data in front
            file.write(exisitingData.data(), exisitingData.size());

            //Update header data (number of descriptors)
            if(control == 0){
                numDescriptors++;
                file.seekp(4);
                file.write(reinterpret_cast<const char*>(&numDescriptors), sizeof(numDescriptors));
            }
            file.close();

            return;
        }
    }
}