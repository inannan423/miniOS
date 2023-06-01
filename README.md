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

| 命令     | 说明             | 示例                          | 完成情况 |
|--------|----------------|-----------------------------|------|
| cd     | 改变当前目录         | `cd ..`                     | ✔️   |
| dir    | 显示当前目录下的文件和子目录 | `dir`                       | ✔️   |
| mkdir  | 创建子目录          | `mkdir test`                | ✔️   |
| rmdir  | 删除子目录          | `rmdir test`                | ✔️   |
| create | 创建文件           | `create test.txt`           | ✔️   |
| open   | 打开文件           | `open test.txt`             | ❌    |
| read   | 读取文件           | `read test.txt`             | ❌    |
| write  | 写入文件           | `write test.txt`            | ❌    |
| close  | 关闭文件           | `close test.txt`            | ❌    |
| lseek  | 移动文件指针         | `lseek test.txt 10`         | ❌    |
| help   | 显示帮助信息         | `help`                      | ✔️   |
| time   | 显示当前时间         | `time`                      | ✔️   |
| ver    | 显示版本信息         | `ver`                       | ✔️   |
| rename | 重命名文件          | `rename test.txt test2.txt` | ✔️   |
| import | 导入文件           | `import c:\test.txt .`      | ✔️   |
| export | 导出文件           | `export test.txt c:\`       | ✔️   |
| exit   | 退出程序           | `exit`                      | ✔️   |

- lseek 用于移动文件指针，第二个参数为偏移量，第三个参数为移动方式，0为从文件开头，1为从当前位置，2为从文件末尾。