//
// Created by OrangeJuice on 2023/5/29.
//

#ifndef MINIOS_FCB_H
#define MINIOS_FCB_H
#include "os.h"
#include <iostream>
#include <thread>
#include <string>
#include <utility>

using namespace std;


class fcb {
public:
    int isused; // 1位，是否使用
    int isHide; // 1位，是否隐藏
    string name; // 20位，文件名
    int type;	// 1位，类别 0：文件 1：目录
    int user;	// 1位，用户 0：root 1：user
    int size;	// 7位，文件大小
    int address; // 4位，物理地址
    string modifyTime; // 12位，修改时间

    fcb() {
        isused = 0;
        isHide = 0;
        name = "00000000000000000000";
        type = 0;
        user = 0;
        size = 0;
        address = 0;
        modifyTime = "202305291112";
    }

    fcb(string n, int t, int u, int s, int a) {
        isused = 1;
        isHide = 0;
        name = std::move(n);
        type = t;
        user = u;
        size = s;
        address = a;
        modifyTime = "202305291112";
    }

    void reset() {
        isused = 0;
        isHide = 0;
        name = "00000000000000000000";
        type = 0;
        user = 0;
        size = 0;
        address = 0;
        modifyTime = "202305291112";
    }
};


#endif //MINIOS_FCB_H

// 一个 FCB 占用 1+20+1+1+7+4+14 = 48 个字节