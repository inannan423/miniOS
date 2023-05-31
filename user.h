//
// Created by OrangeJuice on 2023/5/29.
//

#ifndef MINIOS_USER_H
#define MINIOS_USER_H

#endif //MINIOS_USER_H
#include <iostream>

using namespace std;

class user {
public:
    int isused; // 1位，是否使用
    string username; // 12位，用户名
    string password; //最多 8 位，由字母数字组成
    int root; // 4位，用户根目录 FCB
    user() {
        isused = 0;
        username = "000000000000";
        password = "00000000";
        root = 0;
    }
};