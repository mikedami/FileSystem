#define FUSE_USE_VERSION 26
#include <fuse.h>
#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <unistd.h>
#include "../libWad/Wad.h"
using namespace std;

// FUSE callbacks
static int wadfs_getattr(const char *path, struct stat *stbuf) {
    Wad* instance = (Wad*)(fuse_get_context()->private_data);

    vector<string> list;

    //Sets stbuf struct for if it is a directory
    if(instance->isDirectory(path)){
        stbuf->st_mode = S_IFDIR | 0777;
        stbuf->st_size = 0;
        stbuf->st_blocks = 1;
        stbuf->st_nlink = 2;
    
    //If content, does the same thing and grabs attributes
    } else if(instance->isContent(path)){
        stbuf->st_mode = S_IFREG | 0666;
        size_t fileSize =  instance->getSize(path);
        stbuf->st_size = static_cast<size_t>(fileSize);
        stbuf->st_blocks = 1;
        stbuf->st_nlink = 1;
    } else{
        return -ENOENT;
    }
    return 0;
}

static int wadfs_mknod(const char *path, mode_t mode, dev_t dev) {
    Wad* instance = (Wad*)(fuse_get_context()->private_data);

    //If specified file doesn't exist yet, initialize it
    if(!instance->isContent(path)){
        instance->createFile(path);
        return 0;
    } else {
        return -EEXIST;
    }
}

static int wadfs_mkdir(const char *path, mode_t mode) {
    Wad* instance = (Wad*)(fuse_get_context()->private_data);

    //If new directory doesn't exist, create it
    if(!instance->isDirectory(path)){
        instance->createDirectory(path);
        return 0;
    } else {
        return -EEXIST;
    }
}

static int wadfs_read(const char *path, char *buffer, size_t length, off_t offset, struct fuse_file_info *fi) {
    Wad* instance = (Wad*)(fuse_get_context()->private_data);

    //Grabs content of file and prints it out
    int ans = instance->getContents(path, buffer, length, offset);
    if(ans < 0){
        return -EIO;
    }
    return ans;
}

static int wadfs_write(const char *path, const char *buffer, size_t length, off_t offset, struct fuse_file_info *fi) {
    Wad* instance = (Wad*)(fuse_get_context()->private_data);

    //Sets new data if file has no contents now
    int ans = instance->writeToFile(path, buffer, length, offset);
    if(ans < 0){
        return -EIO;
    }
    return ans;
}

static int wadfs_readdir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    Wad* instance = (Wad*)(fuse_get_context()->private_data);

    // Check if the path is a directory
    if (!instance->isDirectory(path)) {
        return -ENOENT;  
    }

    vector<string> elements;
    int ans = instance->getDirectory(path, &elements);
    if (ans < 0) {
        return -EIO;
    }

    //Fill buffer with all elements in that directory
    for (const string& element : elements) {
        filler(buffer, element.c_str(), nullptr, 0);
    }

    return 0;
}

static struct fuse_operations operations = {
    .getattr    = wadfs_getattr,
    .mknod      = wadfs_mknod,
    .mkdir      = wadfs_mkdir,
    .read       = wadfs_read,
    .write      = wadfs_write,
    .readdir    = wadfs_readdir,
};

int main(int argc, char* argv[]){
    
    string wadPath = argv[argc-2];

    if(wadPath.at(0) != '/'){
        wadPath = std::string(get_current_dir_name()) + "/" + wadPath;
    }

    Wad* wadFile = Wad::loadWad(wadPath);
    argv[argc - 2] = argv[argc - 1];
    argc--;

    return fuse_main(argc, argv, &operations, wadFile);
}