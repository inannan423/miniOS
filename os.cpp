//
// Created by OrangeJuice on 2023/5/29.
//
#include "user.h"
#include "fcb.h"
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <sstream>

#define userDataAddress 1048576 // 用户数据区起始地址
#define maxBlockCount 9216  // 最大块数
#define maxUserCount 10 // 最大用户数

#define modifedTimesLength 7
#define userLength (25+4*2)*10  // 4*2 用于换行符
#define fatLength (48+8*2)*9216
#define bitMapLength (1+2)*9216

// 存储目录结构
vector<int> dirStack;
vector<int> catalogStack;   //way
int currentCatalog = 0; // 当前目录的 FCB 号
vector<int> filesInCatalog; // 当前目录下的文件

using namespace std;

/*
 * user
 * int isused; // 1位，是否使用
    string username; // 12位，用户名
    string password; //最多 8 位，由字母数字组成
    int root; // 4位，用户根目录 FCB
 * */
user *user;
int nowUser = -1;    // 当前用户

// 线程 1：命令行线程
thread::id cmd;
// 线程 2：内核线程
thread::id kernels;

int modifedTimes = 0;   // 修改次数
int message = 0;    // 用于识别是哪个指令
string argument;   // 用于存储指令的参数
int *fatBlock;  // fatBlock
int *bitMap;    // bitMap
string currentPath = "czh";    // 当前路径

/*
 * fcb
 *  int isused; // 1位，是否使用
    int isHide; // 1位，是否隐藏
    string name; // 20位，文件名
    int type;	// 1位，类别 0：文件 1：目录
    int user;	// 1位，用户 0：root 1：user
    int size;	// 7位，文件大小
    int address; // 4位，物理地址
    string modifyTime; // 14位，修改时间
 * */
fcb *fcbs;  // fcb
bool isLogin = false;   // 是否登录

// 补齐位数存储，例如需要存为 4 位，但是只有 2 位，那么就在前面补 0
string fillFileStrins(string str, int len) {
    int length = str.length();
    if (str.length() < len) {
        for (int i = 0; i < len - length; i++) {
            str += "%";
        }
    }
    return str;
}

// int 转 string
string intToString(int num, int len) {
    string str = to_string(num);
    return fillFileStrins(str, len);
}

// string 转 int
int stringToInt(const string &str) {
    return strtol(str.c_str(), nullptr, 10);
}

string getTrueFileStrings(string s){
    string str = "";
    for (int i = 0; i < s.length(); i++) {
        if (s[i] != '%') {
            str += s[i];
        }
    }
    return str;
}

// 初始化数组信息
void os::initTemps() {
    fatBlock = new int[maxBlockCount];
    for (int i = 0; i < maxBlockCount; i++) {
        fatBlock[i] = 0;
    }
    bitMap = new int[maxBlockCount];
    for (int i = 0; i < maxBlockCount; i++) {
        bitMap[i] = 0;
    }
    fcbs = new fcb[maxBlockCount];
    user = new class user[maxUserCount];
    message = 0;
    argument = "";
    nowUser = -1;
}

// 获取当前时间，输出为 202305301454 的形式
string getCurrentTime() {
    time_t now = time(0);
    tm *ltm = localtime(&now);
    string year = intToString(1900 + ltm->tm_year, 4);
    string month = intToString(1 + ltm->tm_mon, 2);
    string day = intToString(ltm->tm_mday, 2);
    string hour = intToString(ltm->tm_hour, 2);
    string min = intToString(ltm->tm_min, 2);
    return year + month + day + hour + min;
}

// 保存用户信息
bool os::saveUserToFile(int u) {
    fstream file;
    string ss;
    file.open("disk.txt", ios::out | ios::in);
    if (!file.is_open()) {
        cout << "Error: Can't open file!" << endl;
        return false;
    }
    file.seekg(modifedTimesLength + u * 33, ios::beg);
    file << user[u].isused << endl;
    file << fillFileStrins(user[u].username, 12) << endl;
    file << fillFileStrins(user[u].password, 8) << endl;
    file << fillFileStrins(intToString(user[u].root, 4), 4) << endl;
    file.close();
    return true;
}

// 保存 fatBlock 信息
bool os::saveFatBlockToFile() {
    fstream file;
    string ss;
    file.open("disk.txt", ios::out | ios::in);
    if (!file.is_open()) {
        cout << "Error: Can't open file!" << endl;
        return false;
    }
    file.seekg(modifedTimesLength + userLength, ios::beg);
    for (int i = 0; i < maxBlockCount; i++) {
        file << intToString(fatBlock[i], 4) << endl;
    }
    file.close();
    return true;
}

// 保存 bitMap 信息
bool os::saveBitMapToFile() {
    fstream file;
    string ss;
    file.open("disk.txt", ios::out | ios::in);
    if (!file.is_open()) {
        cout << "Error: Can't open file!" << endl;
        return false;
    }
    file.seekg(modifedTimesLength + userLength + fatLength, ios::beg);
    for (int i = 0; i < maxBlockCount; i++) {
        file << bitMap[i] << endl;
    }
    file.close();
    return true;
}

bool os::saveFcbToFile(int f) {
    fstream file;
    string ss;
    file.open("disk.txt", ios::out | ios::in);
    if (!file.is_open()) {
        cout << "Error: Can't open file!" << endl;
        return false;
    }
    file.seekg(modifedTimesLength + userLength + fatLength + bitMapLength + 64 * f, ios::beg);
    file << fcbs[f].isused << endl;
    file << fcbs[f].isHide << endl;
    file << fillFileStrins(fcbs[f].name, 20) << endl;
    file << fcbs[f].type << endl;
    file << fcbs[f].user << endl;
    file << fcbs[f].size << endl;
    file << fcbs[f].address << endl;
    file << fillFileStrins(fcbs[f].modifyTime, 12) << endl;
    file.close();
    return true;
}

int os::getEmptyFcb() {
    for (int i = 0; i < maxBlockCount; i++) {
        if (fcbs[i].isused == 0) {
            return i;
        }
    }
    return -1;
}

// 获取空闲 Block
int os::getEmptyBlock() {
    for (int i = 0; i < maxBlockCount; i++) {
        if (bitMap[i] == 0) {
            bitMap[i] = 1;
            return i;
        }
    }
    return -1;
}

// 创建函数，用于保存修改过的文件系统
bool os::saveFileSys(int f, string content) {
    fstream file;
    string ss;
    file.open("disk.txt", ios::out | ios::in);
    if (!file.is_open()) {
        cout << "Error: Can't open file!" << endl;
        return false;
    }
    // 定位到 fcbs 开始的位置
    file.seekg(modifedTimesLength + userLength + fatLength + bitMapLength + 64 * f, ios::beg);
    file << fcbs[f].isused << endl;
    file << fcbs[f].isHide << endl;
    file << fillFileStrins(fcbs[f].name, 20) << endl;
    file << fcbs[f].type << endl;
    file << fcbs[f].user << endl;
    file << fcbs[f].size << endl;
    file << fcbs[f].address << endl;
    file << fillFileStrins(fcbs[f].modifyTime, 12) << endl;
    // 定位到用户数据块
    file.seekg(userDataAddress + fcbs[f].address * 1026, ios::beg);  // 1024 + 2（换行符）
    int blockNum = fcbs[f].size / 1024;   // 用于记录当前文件需要多少个块
    string temp;    // 用于记录当前块的内容
    int addressPointer = 0;  // 用于记录当前块的地址指针
    int nowAddress = fcbs[f].address;   // 用于记录当前块的地址
    while (true) {
        if (blockNum <= 0) {
            break;
        }
        int size = 0;   // 用于记录当前块的大小
        int big = 0;
        int i = addressPointer;   // 用于记录当前块的迭代次数
        while (true) {
            if (content[i] == '\n') {
                big += 2;
            } else {
                big += 1;
            }
            i++;
            if (i == addressPointer + 1024) {
                break;
            }
        }
        temp = content.substr(addressPointer, big);
        addressPointer += big;
        blockNum--;
        file << temp << endl;
        fatBlock[nowAddress] = getEmptyBlock();
        if (fatBlock[nowAddress] == -1) {
            fatBlock[nowAddress] = 0;
            return false;
        }
        nowAddress = fatBlock[nowAddress];
        file.seekg(userDataAddress + nowAddress * 1026, ios::beg);
    }
    temp = content.substr(addressPointer, content.size() - addressPointer);
    file << temp;

    int size = 0;
    int big = 0;
    int j = addressPointer;
    while (true) {
        if (content[j] == '\n') {
            big += 2;
        } else {
            big += 1;
        }
        j++;
        if (j == content.size()) {
            break;
        }
    }
    for (int i = 0; i < 1024 - big; i++) {
        file << "%";
    }
    file << endl;
    file.close();
    saveFatBlockToFile();
    saveBitMapToFile();
    return true;
}

bool os::saveFileSys(int f, vector<int> content) {
    fstream file;
    string ss;
    file.open("disk.txt", ios::out | ios::in);
    if (!file.is_open()) {
        cout << "Error: Can't open file!" << endl;
        return false;
    }
    // 定位到 fcbs 开始的位置
    file.seekg(modifedTimesLength + userLength + fatLength + bitMapLength + 64 * f, ios::beg);
    file << fcbs[f].isused << endl;
    file << fcbs[f].isHide << endl;
    file << fillFileStrins(fcbs[f].name, 20) << endl;
    file << fcbs[f].type << endl;
    file << fcbs[f].user << endl;
    file << fcbs[f].size << endl;
    file << fcbs[f].address << endl;
    file << fillFileStrins(fcbs[f].modifyTime, 12) << endl;
    // 定位到用户数据块
    file.seekg(userDataAddress + fcbs[f].address * 1026, ios::beg);  // 1024 + 2（换行符）
    int blockNum = fcbs[f].size / 1024;   // 用于记录当前文件需要多少个块
    string temp;    // 用于记录当前块的内容
    int addressPointer = 0;  // 用于记录当前块的地址指针
    int nowAddress = fcbs[f].address;   // 用于记录当前块的地址
    while (true) {
        if (blockNum <= 0) {
            break;
        }
        int max = addressPointer + 1024;
        for (int i = addressPointer; i < max; i++) {
            file << intToString(content[i], 4) << "%";
        }
        file << "     " << endl;
        blockNum--;
        fatBlock[nowAddress] = getEmptyBlock();
        if (fatBlock[nowAddress] == -1) {
            fatBlock[nowAddress] = 0;
            return false;
        }
        nowAddress = fatBlock[nowAddress];
        file.seekg(userDataAddress + nowAddress * 1026, ios::beg);
    }
    for (int i = addressPointer; i < content.size(); i++) {
        file << intToString(content[i], 4) << "%";
    }
    for (int i = 0; i < 1024 + (addressPointer - content.size()) * 5; i++) {
        file << "%";
    }
    file << endl;
    file.close();
    saveFatBlockToFile();
    saveBitMapToFile();
    return true;
}

// 删除文件中的信息
bool os::deleteFileSystemFile(int f) {
    int n = fcbs[f].address;
    vector<int> changes;
    while (true) {
        changes.push_back(n);
        bitMap[n] = 0;
        n = fatBlock[n];
        if (fatBlock[n] == 0) {
            break;
        }
    }
    for (int i = 0; i < changes.size(); i++) {
        fatBlock[changes[i]] = 0;
    }
    for (int i = 2; i < catalogStack.size(); i++) {
        fcbs[catalogStack[i]].modifyTime = getCurrentTime();
        saveFcbToFile(catalogStack[i]);
    }
    fcbs[f].reset();
    saveFcbToFile(f);
    saveFatBlockToFile();
    saveBitMapToFile();
    return true;
}

// 创建目录，参数为用户编号
int os::makeDirectory(int u) {
    if (u != -1) {
        int voidFcb = getEmptyFcb();
        if (voidFcb == -1) {
            cout << "Error: Can't create directory!" << endl;
            return -1;
        }
        fcbs[voidFcb].isused = 1;
        fcbs[voidFcb].isHide = 0;
        fcbs[voidFcb].type = 1;
        fcbs[voidFcb].user = u;
        fcbs[voidFcb].size = 0;
        fcbs[voidFcb].address = getEmptyBlock();
        fcbs[voidFcb].modifyTime = getCurrentTime();
        fcbs[voidFcb].name = "root";
        // 用户数据块不足
        if (fcbs[voidFcb].address == -1) {
            // 释放已分配的空间
            deleteFileSystemFile(voidFcb);
            cout << "Error: Can't create directory!" << endl;
            return -1;
        }
        user[nowUser].root = voidFcb;

        // 创建 README.txt
        int voidFcbFile = getEmptyFcb();
        if (voidFcbFile == -1) {
            cout << "Error: Can't create directory!" << endl;
            return -1;
        }
        fcbs[voidFcbFile].isused = 1;
        fcbs[voidFcbFile].isHide = 0;
        fcbs[voidFcbFile].type = 0;
        fcbs[voidFcbFile].user = u;
        fcbs[voidFcbFile].size = 0;
        fcbs[voidFcbFile].address = getEmptyBlock();
        fcbs[voidFcbFile].modifyTime = getCurrentTime();
        fcbs[voidFcbFile].name = "README.txt";
        // 用户数据块不足
        if (fcbs[voidFcbFile].address == -1) {
            // 释放已分配的空间
            deleteFileSystemFile(voidFcb);
            cout << "Error: Can't create directory!" << endl;
            return -1;
        }

        // 目录项
        vector<int> stack;
        stack.push_back(voidFcbFile);
        if (!saveFileSys(voidFcb, stack)) {
            cout << "Error: Can't create directory!" << endl;
            // 释放已分配的空间
            deleteFileSystemFile(voidFcb);
            deleteFileSystemFile(voidFcbFile);
            return -1;
        } else {
            cout << "Create directory successfully!" << endl;
        }
        string info = "This is temporary README.txt.";
        if (!saveFileSys(voidFcbFile, info)) {
            cout << "Error: Can't create directory!" << endl;
            // 释放已分配的空间
            deleteFileSystemFile(voidFcb);
            deleteFileSystemFile(voidFcbFile);
            return -1;
        } else {
            cout << "Create README.txt successfully!" << endl;
        }
        user[u].root = voidFcb;
        return voidFcb;
    } else {
//        cout << "Please input the name of the directory: ";
//        string dirName;
//        while (cin>>dirName){
//            if (dirName == "root"){
//                cout << "Error: Can't create root directory!" << endl;
//                cout << "Please input the name of the directory: ";
//                continue;
//            }
//            if (dirName.size() > 20) {
//                cout << "Error: Directory name is too long!" << endl;
//                cout << "Please input the name of the directory: ";
//                continue;
//            }
//            bool flag = false;
//            for (int i=0;i< dirStack.size();i++){
//                if (fcbs[dirStack[i]].name == dirName && fcbs[dirStack[i]].type == 1){
//                    flag = true;
//                }
//            }
//            if (flag){
//                cout << "Error: Directory name is already exist!" << endl;
//                cout << "Please input the name of the directory: ";
//                continue;
//            }
//            break;
//        }
//
//        int voidFcb = getEmptyFcb();
//        if (voidFcb == -1) {
//            cout << "Error: No space for new directory!" << endl;
//            return -1;
//        }
//        fcbs[voidFcb].isused = 1;
//        fcbs[voidFcb].isHide = 0;
//        fcbs[voidFcb].name = dirName;
//        fcbs[voidFcb].type = 1;
//        fcbs[voidFcb].user = u;
//        fcbs[voidFcb].size = 0;
//        fcbs[voidFcb].address = getEmptyBlock();
//        fcbs[voidFcb].modifyTime = getCurrentTime();
//        vector
    }
}

void os::displayFileInfo(){
    cout << "序号\t" << "文件名\t\t" << "类型" << "\t" << "可见性\t" <<"权限\t" << "大小\t" << "最后修改时间\t" << endl;
    for (int i = 0; i < filesInCatalog.size(); i++) {
        if (fcbs[filesInCatalog[i]].user == nowUser)
            cout << i << "\t" << fcbs[filesInCatalog[i]].name << "\t\t"
                 << ((fcbs[filesInCatalog[i]].type == 0) ? "file" : "dir")
                 << "\t" << ((fcbs[filesInCatalog[i]].isHide == 0) ? "可见" : "隐藏")
                 << "\t" << ((fcbs[filesInCatalog[i]].size == 0) ? fcbs[filesInCatalog[i]].size : -1)
                 << "\t" << fcbs[filesInCatalog[i]].modifyTime << "\t" << endl;
    }
    cout << endl;
}

// 从文件中读取指定 FCB 的内容
vector<int> os::getFcbs(int fcb) {
    fstream file;
    string ss;
    file.open("disk.txt", ios::in | ios::out);
    if (!file.is_open()) {
        cout << "Error: Can't open the disk!" << endl;
        vector<int> error;
        return error;
    }
    string data = "";
    string temp;
    int fcbAddress = fcbs[fcb].address;
    while(true){
        file.seekg(userDataAddress + fcbAddress * 1024, ios::beg);
        if(fatBlock[fcbAddress] == 0){
            break;
        }
        file >> temp;
        data += temp;
        fcbAddress = fatBlock[fcbAddress];  // 下一个数据块
    }
    file>>temp;
    data += temp;
    istringstream iss(data);
    string s;
    vector<int> res;
    while (getline(iss, s, '%')) {
        res.push_back(strtol(s.c_str(), nullptr, 10));
    }
    file.close();
    return res;
}

// 实现用户注册逻辑，将用户信息写入文件
void os::userRegister() {
    int u = -1;
    // 查找空白空间
    for (int i = 0; i < 10; i++) {
        if (user[i].isused == 0) {
            u = i;
            break;
        }
    }
    if (u == -1) {
        cout << "Error: No space for new user!" << endl;
        return;
    }
    cout<<"* Welcome to register!You should input your username and password.The username should be less than 20 characters and only contains letters and numbers.And the password should be less than 8 characters."<<endl;
    cout << "Please input your username: ";
    string username;
    while (cin >> username) {
        if (username.size() > 20) {
            cout << "Error: Username is too long!" << endl;
            cout << "Please input your username: ";
            continue;
        }
        // 判断用户是否已经存在
        for (int i = 0; i < 10; i++) {
            if (user[i].username == username) {
                cout << "Error: Username is already exist!" << endl;
                continue;
            }
        }
        bool flag = true;
        for (int i = 0; i < username.size(); i++) {
            if (username[i] < 48 || (username[i] > 57 && username[i] < 65) || (username[i] > 90 && username[i] < 97) ||
                username[i] > 122) {
                flag = false;
                break;
            }
        }
        if (flag) {
            break;
        } else {
            cout << "Error: Username is not valid!" << endl;
            cout << "Please input your username: ";
        }
    }

    user[u].username = username;
    cout << "Please input your password: ";
    string password;
    while (cin >> password) {
        if (password.size() > 8) {
            cout << "Error: Password is too long!" << endl;
            cout << "Please input your password: ";
            continue;
        }
        bool flag = true;
        for (int i = 0; i < password.size(); i++) {
            if (password[i] < 48 || (password[i] > 57 && password[i] < 65) || (password[i] > 90 && password[i] < 97) ||
                password[i] > 122) {
                flag = false;
                break;
            }
        }
        if (flag) {
            break;
        } else {
            cout << "Error: Password is not valid!" << endl;
            cout << "Please input your password: ";
        }
    }
    user[u].password = password;
    user[u].isused = 1;
    user[u].root = 0;
    cout << "*** User " << user[u].username << " regist successfully!" << endl;
    saveUserToFile(u);
    makeDirectory(u);
}

// 实现用户登录逻辑
void os::userLogin() {
    int u = -1;
    cout << "Input \"--register\" to register a new user." << endl;
    cout << "Please input your username: ";
    string username;
    cin >> username;
    if (username == "--register") {
        userRegister();
        return;
    }
    for (int i = 0; i < 10; i++) {
        if (user[i].username == username) {
            u = i;
        }
    }
    if (u == -1) {
        cout << "Error: Username is not exist!" << endl;
        cout << "You can register first!" << endl;
        return;
    }
    cout << "Please input your password: ";
    string password;
    cin >> password;
    if (user[u].password == password) {
        nowUser = u;
        dirStack.push_back(u);
        currentCatalog = user[u].root;
        catalogStack.push_back(currentCatalog);
        filesInCatalog = getFcbs(user[u].root);
        cout<<"* Welcome "<<user[u].username<<"!"<<endl;
        displayFileInfo();
        isLogin = true;
    } else {
        cout << "Error: Password is not correct!" << endl;
        return;
    }
}

void os::createFileSys() {
    thread::id id = this_thread::get_id();
    fstream file;   // 文件流
    // 文件在 cmake-build-debug/ 下
    file.open("disk.txt", ios::out);    // 打开文件
    if (!file) {
        cout << "Error: Can't open file!" << endl;
        exit(1);
    }
    cout << "*** Creating file system..." << endl;
    modifedTimes = 0;   // 修改次数
    file << "*****" << endl; // 修改次数
    // 写入初始化的用户信息
    for (int i = 0; i < 10; i++) {
        file << user[i].isused << endl;
        file << fillFileStrins(user[i].username, 12) << endl;
        file << fillFileStrins(user[i].password, 8) << endl;
        file << fillFileStrins(intToString(user[i].root, 4), 4) << endl;
    }
    // 写入初始化的 fat block 信息
    for (int i = 0; i < maxBlockCount; i++) {
        file << intToString(fatBlock[i], 4) << endl;
    }
    // 写入初始化的 bit map 信息
    for (int i = 0; i < maxBlockCount; i++) {
        file << bitMap[i] << endl;
    }
    // 写入初始化的 fcb 信息
    for (int i = 0; i < maxBlockCount; i++) {
        file << fcbs[i].isused << endl;
        file << fcbs[i].isHide << endl;
        file << fillFileStrins(fcbs[i].name, 20) << endl;
        file << fcbs[i].type << endl;
        file << fcbs[i].user << endl;
        file << fcbs[i].size << endl;
        file << intToString(fcbs[i].address, 4) << endl;
        file << fillFileStrins(fcbs[i].modifyTime, 12) << endl;
    }
    // 定位到用户数据
    file.seekg(userDataAddress, ios::beg);
    file << endl;
    // 写入初始化的用户数据
    for (int i = 0; i < maxBlockCount; i++) {
        // 1024 个 0
        for (int j = 0; j < 1024; j++) {
            file << "0";
        }
        file << endl;
    }
    file.close();
}

// 读出文件信息
void os::initFileSystem() {
    fstream file;
    string ss;
    cout << "*** Reading file system..." << endl;
    cout << ss << endl;
    file.open("disk.txt", ios::in | ios::out);  // 读写模式打开文件
    if (!file.is_open()) {
        createFileSys();
        return;
    }
    cout << "success" << endl;
    file >> ss;
    modifedTimes = stringToInt(ss);    // 修改次数
    // 填入用户信息
    for (int i = 0; i < 10; i++) {
        file >> ss;
        user[i].isused = stringToInt(ss);
        file >> ss;
        user[i].username = getTrueFileStrings(ss);
        file >> ss;
        user[i].password = getTrueFileStrings(ss);
        file >> ss;
        user[i].root = stringToInt(ss);
    }
    // 填入 fat block 信息
    for (int i = 0; i < maxBlockCount; i++) {
        file >> ss;
        fatBlock[i] = strtol(ss.c_str(), nullptr, 10);  // 三个参数分别为：字符串，指针，进制，意思是将字符串转换为 10 进制的整数
    }
    // 填入 bit map 信息
    for (int i = 0; i < maxBlockCount; i++) {
        file >> ss;
        bitMap[i] = stringToInt(ss);
    }
    // 填入 fcb 信息
    for (int i = 0; i < maxBlockCount; i++) {
        file >> ss;
        fcbs[i].isused = stringToInt(ss);
        file >> ss;
        fcbs[i].isHide = stringToInt(ss);
        file >> ss;
        fcbs[i].name = getTrueFileStrings(ss);
        file >> ss;
        fcbs[i].type = stringToInt(ss);
        file >> ss;
        fcbs[i].user = stringToInt(ss);
        file >> ss;
        fcbs[i].size = stringToInt(ss);
        file >> ss;
        fcbs[i].address = stringToInt(ss);
        file >> ss;
        fcbs[i].modifyTime = getTrueFileStrings(ss);
    }
    file.close();
}

// 条件变量，kernel 线程和 run 线程共享
os::os() : ready(false) {
    cout << "*** Preparing for the system..." << endl;
    fatBlock = new int[maxBlockCount];
    for (int i = 0; i < maxBlockCount; i++) {
        fatBlock[i] = 0;
    }
    bitMap = new int[maxBlockCount];
    for (int i = 0; i < maxBlockCount; i++) {
        bitMap[i] = 0;
    }
    fcbs = new fcb[maxBlockCount];
    user = new class user[10];  // class 用于区分 user 类型和 user 类
    message = 0;    // 用于识别是哪个指令, 0 代表无指令
}

void os::reset() {
    fatBlock = new int[maxBlockCount];
    for (int i = 0; i < maxBlockCount; i++) {
        fatBlock[i] = 0;
    }
    bitMap = new int[maxBlockCount];
    for (int i = 0; i < maxBlockCount; i++) {
        bitMap[i] = 0;
    }
    fcbs = new fcb[maxBlockCount];
    user = new class user[10];
}

os::~os() {
//    std::cout << "os destructor" << std::endl;
}


// create 命令，创建文件
void os::createFile(const string &filename, int type) {
    // 判断是否有重名文件
    for (int i = 0; i < maxBlockCount; i++) {
        if (fcbs[i].name == filename && fcbs[i].isused == 1) {
            cout << "Error: File already exists" << endl;
            return;
        }
    }
    // 判断是否有空闲 fcb
    int fcbIndex = -1;
    for (int i = 0; i < maxBlockCount; i++) {
        if (fcbs[i].isused == 0) {
            fcbIndex = i;
            break;
        }
    }
    if (fcbIndex == -1) {
        cout << "Error: No more space for new file" << endl;
        return;
    }
    // 判断是否有空闲 block
    int blockIndex = -1;
    for (int i = 0; i < maxBlockCount; i++) {
        if (bitMap[i] == 0) {
            blockIndex = i;
            break;
        }
    }
    if (blockIndex == -1) {
        cout << "Error: No more space for new file" << endl;
        return;
    }
    // 创建文件
    fcbs[fcbIndex].isused = 1;
    fcbs[fcbIndex].isHide = 0;
    fcbs[fcbIndex].name = filename;
    fcbs[fcbIndex].type = type;

    fcbs[fcbIndex].user = nowUser;
    fcbs[fcbIndex].size = 0;
    fcbs[fcbIndex].address = blockIndex;
    fcbs[fcbIndex].modifyTime = getCurrentTime();
    bitMap[blockIndex] = 1;

    cout << "File created successfully" << endl;
}

void os::deleteFile(const string &filename) {
    // 判断是否有该文件
    int fcbIndex = -1;
    for (int i = 0; i < maxBlockCount; i++) {
        if (fcbs[i].name == filename && fcbs[i].isused == 1) {
            fcbIndex = i;
            break;
        }
    }
    if (fcbIndex == -1) {
        cout << "Error: No such file" << endl;
        return;
    }
    // 判断是否有权限
    if (fcbs[fcbIndex].user != nowUser && nowUser != 0) {
        cout << "Error: Permission denied" << endl;
        return;
    }
    // 删除文件
    int blockIndex = fcbs[fcbIndex].address;
    bitMap[blockIndex] = 0;
    fcbs[fcbIndex].isused = 0;
    fcbs[fcbIndex].isHide = 0;
    fcbs[fcbIndex].name = "";
    fcbs[fcbIndex].type = 0;
    fcbs[fcbIndex].user = 0;
    fcbs[fcbIndex].size = 0;
    fcbs[fcbIndex].address = 0;
    fcbs[fcbIndex].modifyTime = "";


    cout << "File deleted successfully" << endl;
}

void os::showFileList() {
    cout << "File list:" << endl;
    cout << "Name\t\tType\t\tSize\t\tModify time" << endl;
    for (int i = 0; i < maxBlockCount; i++) {
        if (fcbs[i].isused == 1 && fcbs[i].isHide == 0) {
            cout << fcbs[i].name << "\t\t" << fcbs[i].type << "\t\t" << fcbs[i].size << "\t\t" << fcbs[i].modifyTime
                 << endl;
        }
    }
}

void os::cd(const string &filename) {
    if (filename == "..") {
        if (currentPath == "czh") {
            cout << "Error: Already in root directory" << endl;
            return;
        }
        currentPath = currentPath.substr(0, currentPath.length() - 1);
        currentPath = currentPath.substr(0, currentPath.find_last_of("/") + 1);
        return;
    }
    if (filename == "~") {
        currentPath = "czh";
        return;
    }
    if (filename == "") {
        cout << "Error: No such directory" << endl;
        return;
    }
    if (filename[0] == '/') {
        if (filename == "/") {
            currentPath = "czh";
            return;
        }
        string temp = filename.substr(1, filename.length() - 1);
        if (temp.find_first_of("/") != string::npos) {
            cout << "Error: No such directory" << endl;
            return;
        }
        for (int i = 0; i < maxBlockCount; i++) {
            if (fcbs[i].isused == 1 && fcbs[i].isHide == 0 && fcbs[i].type == 0 && fcbs[i].name == temp) {
                currentPath = "/" + temp + "/";
                return;
            }
        }
        cout << "Error: No such directory" << endl;
        return;
    }
    if (filename.find_first_of("/") != string::npos) {
        cout << "Error: No such directory" << endl;
        return;
    }
    for (int i = 0; i < maxBlockCount; i++) {
        if (fcbs[i].isused == 1 && fcbs[i].isHide == 0 && fcbs[i].type == 1 && fcbs[i].name == filename) {
            currentPath = currentPath + filename + "/";
            return;
        }
    }
    cout << "Error: No such directory" << endl;
    return;
}

// cmd 线程，用于接收用户输入
void os::run() {
    // 线程 1：命令行线程
    cmd = this_thread::get_id();
    std::cout << "* Welcome to MiniOS 23.5.29 LTS" << std::endl;
    std::cout << "\n"
                 "       ██╗███████╗████████╗ ██████╗ ███████╗\n"
                 "       ██║██╔════╝╚══██╔══╝██╔═══██╗██╔════╝\n"
                 "       ██║█████╗     ██║   ██║   ██║███████╗\n"
                 "  ██   ██║██╔══╝     ██║   ██║   ██║╚════██║\n"
                 "  ╚█████╔╝███████╗   ██║   ╚██████╔╝███████║\n"
                 "   ╚════╝ ╚══════╝   ╚═╝    ╚═════╝ ╚══════╝\n"
                 "                                            " << std::endl;
    cout << "* Type 'help' to get help." << endl << endl;

    string command;
    while (true) {
        // 检查登录
        if (nowUser == -1) {
            userLogin();
        } else {
            if (currentPath == "czh") {
                cout << "czh@MiniOS:/czh/:";
            } else {
                cout << "czh@MiniOS::/czh" << currentPath << ":";
            }
            cin >> command;
            if (command == "help") {
                cout << "* help: 获取帮助" << endl
                     << "* hello: 打印 Hello World!" << endl
                     << "* exit: 退出系统" << endl;
            } else if (command == "exit") {
                cout << "Bye!" << endl;
                system("pause");
                exit(0);
            } else if (command == "print") {
                // 获取 hello 后面的参数，可能带空格
                string arg;
                getline(cin, arg);
                // 传递参数给 cmd_handler 线程
                argument = arg;
                unique_lock<mutex> lock(m); // 加锁，防止多个线程同时访问
                message = 1;
                ready = true;
                cv.notify_all();    // 唤醒所有线程
                cv.wait(lock, [this] { return !ready; });   // 等待 ready 变为 false
            } else if (command == "create") {
                string arg;
                getline(cin, arg);
                argument = arg;
                unique_lock<mutex> lock(m);
                message = 2;
                ready = true;
                cv.notify_all();
                cv.wait(lock, [this] { return !ready; });
            } else if (command == "delete") {
                string arg;
                getline(cin, arg);
                argument = arg;
                unique_lock<mutex> lock(m);
                message = 3;
                ready = true;
                cv.notify_all();
                cv.wait(lock, [this] { return !ready; });
            } else if (command == "ls") {
                string arg;
                getline(cin, arg);
                argument = arg;
                unique_lock<mutex> lock(m);
                message = 4;
                ready = true;
                cv.notify_all();
                cv.wait(lock, [this] { return !ready; });
            } else if (command == "cd") {
                string arg;
                getline(cin, arg);
                argument = arg;
                unique_lock<mutex> lock(m);
                message = 5;
                ready = true;
                cv.notify_all();
                cv.wait(lock, [this] { return !ready; });
            } else if (command == "mkdir") {
                string arg;
                getline(cin, arg);
                argument = arg;
                unique_lock<mutex> lock(m);
                message = 6;
                ready = true;
                cv.notify_all();
                cv.wait(lock, [this] { return !ready; });
            } else if (command == "register") {
                string arg;
                getline(cin, arg);
                argument = arg;
                // 如果携带参数，则提示不合法
                if (arg.find_first_not_of(" ") != string::npos) {
                    cout << "Error: Invalid argument" << endl;
                    continue;
                }
                unique_lock<mutex> lock(m);
                message = 7;
                ready = true;
                cv.notify_all();
                cv.wait(lock, [this] { return !ready; });
            } else if (command == "open") {
                string arg;
                getline(cin, arg);
                argument = arg;
                unique_lock<mutex> lock(m);
                message = 8;
                ready = true;
                cv.notify_all();
                cv.wait(lock, [this] { return !ready; });
            } else {
                cout << command << ": command not found" << endl;
            }
        }
    }
}


// kernel，内核处理程序
[[noreturn]] void os::kernel() {
    kernels = this_thread::get_id();
    // 循环处理接受 message 变化
    while (true) {
        unique_lock<mutex> lock(m); // 加锁，防止多个线程同时访问
        cv.wait(lock, [this] { return ready; });    // 等待 ready 变为 true
        // *1 print 方法
        if (message == 1) {
            if (argument.empty()) {
                cout << "argument is empty!" << endl;
            } else {
                // 去除首个空格
                argument.erase(0, 1);
                cout << argument << endl;
            }
            message = 0;
            ready = false;  // ready 变为 false
            cv.notify_all();    // 唤醒所有线程
        }
            // *2 create 方法
        else if (message == 2) {
            // 接受两个参数 filename 和 type
            if (argument.empty()) {
                cout << "argument is empty!" << endl;
            } else if (argument.find(' ') == string::npos) {
                cout << "argument is not enough!" << endl;
            } else {
                // 去除首个空格
                argument.erase(0, 1);
                // 获取 filename
                string filename = argument.substr(0, argument.find(' '));
                // 获取 type
                string type = argument.substr(argument.find(' ') + 1);
                // type 只能为 dir 或 file
                if (type != "dir" && type != "file") {
                    cout << "type is wrong!" << endl;
                } else {
                    // 创建文件
                    createFile(filename, type == "dir" ? 0 : 1);
                }
            }
            message = 0;
            ready = false;  // ready 变为 false
            cv.notify_all();    // 唤醒所有线程
        }
            // *3 delete 方法
        else if (message == 3) {
            if (argument.empty()) {
                cout << "argument is empty!" << endl;
            } else {
                // 去除首个空格
                argument.erase(0, 1);
                // 删除文件
                deleteFile(argument);
            }
            message = 0;
            ready = false;  // ready 变为 false
            cv.notify_all();    // 唤醒所有线程
        }
            // *4 ls 方法
        else if (message == 4) {
            // 显示文件列表
            showFileList();

            message = 0;
            ready = false;  // ready 变为 false
            cv.notify_all();    // 唤醒所有线程
        } else if (message == 5) {
            if (argument.empty()) {
                cout << "argument is empty!" << endl;
            } else {
                // 去除首个空格
                argument.erase(0, 1);
                // 切换目录
                cd(argument);
            }
            message = 0;
            ready = false;  // ready 变为 false
            cv.notify_all();    // 唤醒所有线程
        }
            // *7 register 方法
        else if (message == 7) {
            // 注册用户
            userRegister();
            message = 0;
            ready = false;  // ready 变为 false
            cv.notify_all();    // 唤醒
        }
    }
}


