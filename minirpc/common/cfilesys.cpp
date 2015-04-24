/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The source file of class CFileSys.
*/

#include <cstdio>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/file.h>
#include "common/cfilesys.h"

using namespace std;

namespace wyf {

// \param file 文件路径名
// \retval true 文件存在
// \retval false 不存在
bool CFileSys::IsFileExist(const string &file) {
    return (access(file.c_str(), F_OK) == 0) ? true : false;
}

// \param file 文件路径名
// \retval true 文件存在且为符号链接
// \retval false 不存在或者不是符号链接
bool CFileSys::IsLink(const string &file) {
    struct stat statbuf;

    if(lstat(file.c_str(), &statbuf) == 0 && S_ISLNK(statbuf.st_mode) != 0) {
        return true;
    }
    return false;
}

// \param dile 目录路径名
// \retval true 目录存在
// \retval false 不存在或者不是目录
bool CFileSys::IsDir(const string &file) {
    struct stat statbuf;

    if(stat(file.c_str(), &statbuf) == 0 && S_ISDIR(statbuf.st_mode) != 0) {
        return true;
    }
    return false;
}

// \param srcfile 原文件名
// \param destfile 新链接名
// \retval true 操作成功
// \retval false 不成功
bool CFileSys::MkLink(const string &srcfile, const string &destfile) {
    if (symlink(srcfile.c_str(), destfile.c_str()) == 0)
        return true;
    else
        return false;
}

// \param file 文件路径名
// \return 若文件存在则返回大小,否则返回-1
size_t CFileSys::FileSize(const string &file) {
    struct stat statbuf;

    if(stat(file.c_str(), &statbuf)==0)
        return statbuf.st_size;
    else
        return 0;
}

// \param file 文件路径名
// \return 若文件存在则返回其最后更改时间,否则返回-1
time_t CFileSys::FileTime(const string &file) {
    struct stat statbuf;

    if(stat(file.c_str(), &statbuf)==0)
        return statbuf.st_mtime;
    else
        return -1;
}

// \param file 文件路径名
// \return 若能取得文件路径则返回,否则返回空字符串
string CFileSys::FilePath(const string &file) {
    size_t p;
    if ((p=file.rfind("/")) != file.npos)
        return file.substr(0, p);
    else if ((p=file.rfind("\\")) != file.npos)
        return file.substr(0, p);
    return string("");
}

// \param file 文件路径名
// \return 若能取得文件名称则返回,否则返回原文件路径名称
string CFileSys::FileName(const string &file) {
    size_t p;
    if ((p=file.rfind("/")) != file.npos)
        return file.substr(p+1);
    else if ((p=file.rfind("\\")) != file.npos)
        return file.substr(p+1);
    return file;
}

// libm.so --> m   libmylib.so --> mylib
string CFileSys::OnlyLibName(const string& libName) {
    string fileName = FileName(libName);
    if (fileName.compare(0, 3, "lib") == 0) {
        fileName.erase(0, 3);
    }
    size_t p;
    if ((p = fileName.rfind(".so")) != fileName.npos) {
        fileName.erase(p, 3);
    }
    if ((p = fileName.rfind(".sl")) != fileName.npos) {
        fileName.erase(p, 3);
    }
    return fileName;
}

// \param oldname 原文件名
// \param newname 新文件名
// \retval true 操作成功
// \retval false 失败
bool CFileSys::RenameFile(const string &oldname, const string &newname) {
    if (rename(oldname.c_str(), newname.c_str()) != -1)
        return true;
    else
        return false;
}

// \param srcfile 原文件名
// \param destfile 目的文件名,文件属性为0666
// \retval true 操作成功
// \retval false 失败
bool CFileSys::CopyFile(const string &srcfile, const string &destfile) {
    FILE *src=NULL, *dest=NULL;
    if ((src=fopen(srcfile.c_str(),"rb")) == NULL) {
        return false;
    }
    if ((dest=fopen(destfile.c_str(),"wb+")) == NULL) {
        fclose(src);
        return false;
    }

    const static int bufsize = 8192;
    char buf[bufsize];
    size_t n;
    while ((n=fread(buf, 1, bufsize, src)) >= 1) {
        if (fwrite(buf, 1, n, dest) != n) {
            fclose(src);
            fclose(dest);
            return false;
        }
    }

    fclose(src);
    fclose(dest);

    //chmod to 0666
    mode_t mask = umask(0);
    chmod(destfile.c_str(), S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
    umask(mask);

    return true;
}

// \param file 文件路径名
// \retval true 删除成功
// \retval false 文件不存在或者删除失败
bool CFileSys::DelFile(const string &file) {
    if (remove(file.c_str()) == 0)
        return true;
    else
        return false;
}

// \param srcfile 原文件名
// \param destfile 新文件名
// \retval true 操作成功
// \retval false 失败
bool CFileSys::MoveFile(const string &srcfile, const string &destfile) {
    if (RenameFile(srcfile,destfile))
        return true;

    // rename fail, copy and delete file
    if (CopyFile(srcfile,destfile)) {
        if (DelFile(srcfile))
            return true;
    }

    return false;
}

// \param dir 参数为目录路径名
// \return 返回结果为文件及子目录列表,子目录的第一个字符为 '/',
// 返回结果中不包括代表当前及上一级目录的 "/.", "/.."
void CFileSys::ListFiles(const string &dir, vector<string> *files) {
    string file;
    DIR *pdir = NULL;
    dirent *pdirent = NULL;

    if ((pdir=opendir(dir.c_str())) != NULL) {
        while ((pdirent=readdir(pdir)) != NULL) {
            file = pdirent->d_name;
            if (file!="." && file!="..") {
                if (IsDir(dir+"/"+file))
                    file = "/"+file;
                files->push_back(file);
            }
        }
        closedir(pdir);
    }
}

// \param dir 要创建的目录,若上层目录不存在则自动创建
// \param mode 创建目录权限,默认为S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH(0755)
// 成功返回true, 否则返回false.
// \retval true 操作成功
// \retval false 失败
bool CFileSys::CreateDir(const string &dir, const mode_t mode) {
    // check
    size_t len = dir.length();
    if (len <= 0) return false;

    string tomake;
    char curchr;
    for(size_t i=0; i<len; ++i) {
        // append
        curchr = dir[i];
        tomake += curchr;
        if (curchr=='/' || i==(len-1)) {
            // need to mkdir
            if (!IsFileExist(tomake) && !IsDir(tomake)) {
                // able to mkdir
                mode_t mask = umask(0);
                if (mkdir(tomake.c_str(),mode) == -1) {
                    umask(mask);
                    return false;
                }
                umask(mask);
            }
        }
    }

    return true;
}

// \param srcdir 原目录
// \param destdir 目的目录
// \retval true 操作成功
// \retval false 失败
bool CFileSys::CopyDir(const string &srcdir, const string &destdir) {
    vector<string> files;
    ListFiles(srcdir, &files);
    string from;
    string to;

    // 创建目标目录
    if (!IsFileExist(destdir))
        CreateDir(destdir);

    for (size_t i=0; i<files.size(); ++i) {
        from = srcdir + "/" + files[i];
        to = destdir + "/" + files[i];

        // 子目录,递归调用
        if (files[i][0] == '/') {
             if (!CopyDir(from,to))
                return false;
        } else if (!CopyFile(from,to)) {
            return false;
        }
    }

    return true;
}

// \param dir 要删除的目录
// \retval true 操作成功
// \retval false 失败
bool CFileSys::DelDir(const string &dir) {
    vector<string> files;
    ListFiles(dir, &files);
    string todel;

    // 删除文件
    for (size_t i=0; i<files.size(); ++i) {
        todel = dir + "/" + files[i];

        // 子目录,递归调用
        if (files[i][0] == '/') {
             if (!DelDir(todel))
                return false;
        } else if (!DelFile(todel)) {
            return false;
        }
    }

    // 删除目录
    if (rmdir(dir.c_str()) == 0)
        return true;

    return false;
}

// \param srcdir 原目录
// \param destdir 目的目录
// \retval true 操作成功
// \retval false 失败
bool CFileSys::MoveDir(const string &srcdir, const string &destdir) {
    if (RenameFile(srcdir,destdir))
        return true;

    // rename fail, copy and delete dir
    if (CopyDir(srcdir,destdir)) {
        if (DelDir(srcdir))
            return true;
    }

    return false;
}

// \param fd 文件句柄
// \param type 锁模式，可选F_WRLCK、F_RDLCK、F_UNLCK
void CFileSys::LockFile(int fd, const int type) {
    struct flock lck;
    lck.l_start = 0;
    lck.l_len = 0;
    lck.l_whence = SEEK_SET;

    lck.l_type = type;
    if (type==F_WRLCK || type==F_RDLCK)
        lck.l_type = type;
    else
        lck.l_type = F_UNLCK;

    while (fcntl(fd,F_SETLKW,&lck)==-1 && errno==EINTR);
    return;
}

// \param fd 文件句柄
// \retval true 文件已被锁
// \retval false 文件未被锁
bool CFileSys::IsLocked(int fd) {
    struct flock lck;
    lck.l_start = 0;
    lck.l_len = 0;
    lck.l_whence = SEEK_SET;
    lck.l_type = F_WRLCK;

    while ((fcntl(fd, F_GETLK, &lck))==-1 && errno==EINTR);

    if (lck.l_type == F_UNLCK)
        return false;
    else
        return true;
}

// \param file 文件路径
// \param mode 文件打开模式，与fopen()同参数意义相同
// \param type 锁模式，可选F_WRLCK、F_RDLCK、F_UNLCK
// \return 文件句柄，失败返回NULL
FILE* CFileSys::LockFileOpen(const string &file, const char* mode, const int type) {
    FILE *fp;
    mode_t mask = umask(0);
    int fd = open(file.c_str(), O_CREAT|O_EXCL|O_RDWR,
                   S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
    if (fd >= 0) {
        // file not exist, create success
        LockFile(fd, type);
        fp = fdopen(fd, mode);
    } else {
        // file exist
        fp = fopen(file.c_str(), mode);
        if (fp == NULL)
            return NULL;
        LockFile(fileno(fp), type);
    }
    umask(mask);

    return fp;
}

} // namespace wyf
