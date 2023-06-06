# 操作系统课程设计

## 任务书

### 第一部分 磁盘模拟

用一个文件来模拟一个磁盘驱动器，并在该磁盘驱动器中存放文件和目录。
在模拟的磁盘驱动器中，利用文件系统的基础理论，管理记录在磁盘驱动器中的内容。

### 第二部分 文件系统实现

书写一个命令行形式的驱动器内容访问接口，即仿照cmd.exe的形式完成磁盘驱动器中内容的查看。要求实现的命令包括：`cd`，`dir`，`mkdir`，`rmdir`，`create`，`open`，`read`, `write`, `close`, `lseek`，`help`，`time`，`ver`，`rename`，`import`，`export`。

另外，需要支持从本地磁盘复制内容到虚拟的磁盘驱动器中，也支持从虚拟的磁盘驱动器复制内容到本地磁盘。
比如：
- `import c:\a.txt .`  将本地C盘下的a.txt导入到当前目录。
- `export a.txt c:\`   将当前目录下的a.txt导出到本地C盘。

## 评分标准

### 50-70分档，仅实现一个命令解释执行的shell程序
实现要求：
1，	要求至少完成cd，dir，mkdir，rmdir，create，open，read, write, close, lseek，help，time，ver，rename。实现时，可以用windows或者linux系统，尤其是目录访问过程。例如，GetCurrentDirectory，FindFile，FindFileNext等函数
2，	命令的主要功能要实现，例如 “dir *.txt” “dir /s”等，实现过少，则认为没有实现该命令。

### 70-90分档，在上一档的shell基础上，实现了在一个物理文件中模拟文件系统的功能。
实现要求：
1，该物理文件为二进制文件。
2，实习报告中需要给出所模拟的文件系统格式。
3，能静态保存虚拟文件系统的内容，即上次程序运行保存的内容，下次程序运行能看到。
4，实现import, export功能；


### 90分以上档，又实现了多个shell可以同时运行操作同一个模拟文件系统的物理文件。
实现要求：
完整实现最好，能给出有效解决途径或者设计思路，写在实习报告中也给一定分数。

## 命令

| 命令     | 说明             | 完成情况 |
|--------|----------------|------|
| cd     | 改变当前目录         | ✔️   |
| dir    | 显示当前目录下的文件和子目录 | ✔️   |
| mkdir  | 创建子目录          | ✔️   |
| rmdir  | 删除子目录          | ✔️   |
| create | 创建文件           | ✔️   |
| open   | 打开文件           | ✔️   |
| read   | 读取文件           | ✔️   |
| write  | 写入文件           | ✔️   |
| close  | 关闭文件           | ✔️   |
| lseek  | 移动文件指针         | ✔️   |
| help   | 显示帮助信息         | ✔️   |
| time   | 显示当前时间         | ✔️   |
| ver    | 显示版本信息         | ✔️   |
| rename | 重命名文件          | ✔️   |
| import | 导入文件           | ✔️   |
| export | 导出文件           | ✔️   |
| exit   | 退出程序           | ✔️   |

- lseek 用于移动文件指针，第二个参数为偏移量，第三个参数为移动方式，0为从文件开头，1为从当前位置，2为从文件末尾。

## 实现

### `cd`

`cd`指令的实现过程可以概括如下：

1. 接收一个目录路径作为参数，例如`cd new`表示切换到名为"new"的子目录。

2. 解析目录路径，将其拆分为不同的部分。

3. 根据解析的结果进行不同的操作：
    - 如果路径以"."开头，表示相对于当前目录进行操作：
        - 如果路径为".."，表示返回上级目录。
        - 如果路径为"."，表示保持当前目录不变。
        - 否则，表示无效命令。
    - 如果路径以其他字符串开头，表示相对于根目录进行操作：
        - 清除目录栈。
        - 将根目录作为当前目录，并将其压入目录栈。
        - 依次进入路径中的每个子目录，更新当前目录和目录栈，并打开当前目录。

4. 更新当前目录的文件列表，以反映切换后的目录。

5. 完成`cd`指令的操作。

总结起来，`cd`指令的实现过程涉及解析路径、切换目录、更新文件列表等步骤，以实现在文件管理系统中切换当前目录的功能。

`cd /new/a` 的实现过程如下：

1. 检查根目录下是否存在名为 "new" 的子目录。

2. 如果存在 "new" 子目录，则进入该子目录，并更新当前目录和目录栈。

3. 在 "new" 子目录中检查是否存在名为 "a" 的子目录。

4. 如果存在 "a" 子目录，则进入该子目录，并更新当前目录和目录栈。

5. 更新当前目录的文件列表，以反映切换后的目录。

6. 完成 `cd /new/a` 指令的操作。

总结起来，`cd /new/a` 指令的实现过程是先切换到根目录，然后依次进入 "new" 和 "a" 两个子目录，以实现在文件管理系统中切换当前目录的功能。

```c++
// 删除子目录，递归删除子目录下的所有文件和子目录
int os::removeDirectory(string name) {
    // name 是空格隔开的字符串, 例如 "dir1 dir2 dir3" ，将其转换为 vector
    vector<string> dirNames;
    string temp;
    for (int i = 0; i < name.size(); i++) {
        if (name[i] == ' ') {
            dirNames.push_back(temp);
            temp = "";
        } else {
            temp += name[i];
        }
    }
    dirNames.push_back(temp);
    int n = -1;
    int deleteNumber;
    for (int i = 0; i < dirNames.size(); i++) {
        for (int j = 0; j < filesInCatalog.size(); j++) {
            if (fcbs[filesInCatalog[j]].name == dirNames[i] && fcbs[filesInCatalog[j]].type == 1) {
                n = filesInCatalog[j];
                deleteNumber = j;
                break;
            }
        }
        if (n == -1) {
            cout << "Error: Directory " << dirNames[i] << " is not exist!" << endl;
            return -1;
        }
        vector<int> stack;
        findAllFiles(stack, n);
        for (int j = 0; j < stack.size(); j++) {
            deleteFileSystemFile(stack[j]);
        }
        cout << "Delete directory " << dirNames[i] << " successfully!" << endl;
        filesInCatalog.erase(filesInCatalog.begin() + deleteNumber);
        saveFileSys(currentCatalog, filesInCatalog);
    }
    return 1;
}
```