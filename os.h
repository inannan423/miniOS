//
// Created by OrangeJuice on 2023/5/29.
//

#ifndef MINIOS_OS_H
#define MINIOS_OS_H

#endif //MINIOS_OS_H
#include <mutex>
#include <condition_variable>
#include <vector>

using namespace std;

class os {
public:
    os();
    ~os();
    void run();

    [[noreturn]] void kernel();

    static void initFileSystem();

    static void initTemps();

private:
    mutex m;
    condition_variable cv;
    bool ready;

    static void createFileSys();


    static void reset();

    void createFile(const string& filename, int type);

    void deleteFile(const string &filename);


    void showFileList();


    void cd(const string &filename);

    bool saveUserToFile(int u);

    bool saveFcbToFile(int f);

    bool saveBitMapToFile();

    bool saveFatBlockToFile();

    bool saveFileSys(int f, string content);

    int getEmptyBlock();

    int getEmptyFcb();

    bool saveFileSys(int f, std::vector<int> content);

    void userRegister();

    int makeDirectory(int u);

    bool deleteFileSystemFile(int f);

    void userLogin();

    vector<int> getFcbs(int fcb);

    void displayFileInfo();
};