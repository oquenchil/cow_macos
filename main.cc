#include <iostream>
#include <copyfile.h>
#include <unistd.h>
#include <sys/stat.h>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <functional>

std::string TEXT(30000, 'A');
int MAX_DEPTH = 5;
int MAX_FILES = 7;

std::string ORIGINAL_PATH("/tmp/originals");
std::string ORIGINAL_COPY_PATH(ORIGINAL_PATH + "2");
std::string SYMLINKS_PATH("/tmp/symlinks");

void CreateFileOp(std::string path) {
    std::ofstream ofs(path);
    ofs << TEXT;
    ofs.close();
}

void ApplyOpFoldersAndFilesRec(std::string path, int i, int depth, std::function<void(std::string)> op) {
    if (depth == MAX_DEPTH) {
        return;
    }

    std::string new_dir_path = path + "/path" + std::to_string(i);
    mkdir(new_dir_path.c_str(), 0777);
    for (int j = 0; j < MAX_FILES; j++) {
        std::string filepath = new_dir_path + "/file" + std::to_string(j);
        op(filepath);
    } 

    for (int j = 0; j < MAX_FILES; j++) {
        ApplyOpFoldersAndFilesRec(new_dir_path, j, depth + 1, op);
    }

} 

void CreateFoldersAndFiles() {
    std::filesystem::remove_all(ORIGINAL_PATH);
    std::filesystem::remove_all(ORIGINAL_COPY_PATH);
    std::filesystem::remove_all(SYMLINKS_PATH);
    mkdir(ORIGINAL_PATH.c_str(), 0777);
    mkdir(SYMLINKS_PATH.c_str(), 0777);
    ApplyOpFoldersAndFilesRec(ORIGINAL_PATH, 0, 0, CreateFileOp);
}

void SymlinkFileOp(std::string link) {
    std::string target(link);
    target.replace(0, SYMLINKS_PATH.size(), ORIGINAL_PATH);
    symlink(target.c_str(), link.c_str());
}

void SymlinkFiles() {
    ApplyOpFoldersAndFilesRec(SYMLINKS_PATH, 0, 0, SymlinkFileOp);
}

void PrintDuration(std::chrono::milliseconds ms, std::string message) {
    std::cout << message << " = " << std::chrono::duration_cast<std::chrono::milliseconds>(ms).count() << "[ms]" << std::endl;
}

void MeasureCopy() {
    CreateFoldersAndFiles();
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    copyfile(ORIGINAL_PATH.c_str(), ORIGINAL_COPY_PATH.c_str(), NULL, COPYFILE_ALL | COPYFILE_RECURSIVE);
    PrintDuration(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - begin), "Copy");
}

void MeasureCopyClone() {
    CreateFoldersAndFiles();
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    copyfile(ORIGINAL_PATH.c_str(), ORIGINAL_COPY_PATH.c_str(), NULL, COPYFILE_CLONE | COPYFILE_RECURSIVE);
    PrintDuration(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - begin), "CopyClone");
}

void MeasureSymlink() {
    CreateFoldersAndFiles();
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    SymlinkFiles();
    PrintDuration(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - begin), "Symlink");
}


int main() {
    std::cout << "Program meant for macOS only" << std::endl;
    MeasureCopyClone();
    MeasureCopy();
    MeasureSymlink();
    return 0;
}
