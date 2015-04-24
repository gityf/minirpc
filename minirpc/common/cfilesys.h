/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The header file of class CFileSys.
*/

#ifndef _COMMON_CFILESYS_H_
#define _COMMON_CFILESYS_H_

#include <cstdio>
#include <string>
#include <vector>
#include <fcntl.h>
#include <sys/stat.h>

using namespace std;

namespace wyf {
    class CFileSys {
    public:
        // file or directory exist or not.
        static bool IsFileExist(const string &file);

        // file is syblink exist or not.
        static bool IsLink(const string &file);
        // is the directory
        static bool IsDir(const string &file);

        // create file soft link.
        static bool MkLink(const string &srcfile, const string &destfile);

        // get file size.
        static size_t FileSize(const string &file);

        // get file modify time.
        static time_t FileTime(const string &file);
        // get file path with-out file name.
        static string FilePath(const string &file);
        // get file name.
        static string FileName(const string &file);
        // get only lib name, such libm.so, m is return.
        static string OnlyLibName(const string& libName);
        // rename file or directory
        static bool RenameFile(const string &oldname, const string &newname);
        // copy file
        static bool CopyFile(const string &srcfile, const string &destfile);
        // delete file.
        static bool DelFile(const string &file);
        // move file
        static bool MoveFile(const string &srcfile, const string &destfile);

        // list files.
        static void ListFiles(const string &dir, vector<string> *files);
        // create directory
        static bool CreateDir(const string &dir,
            const mode_t mode = S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
        // copy directory
        static bool CopyDir(const string &srcdir, const string &destdir);
        // delete directory
        static bool DelDir(const string &dir);
        // move directory
        static bool MoveDir(const string &srcdir, const string &destdir);

        // lock file by file descriptor.
        static void LockFile(int fd, const int type);
        // lock file.
        static inline void LockFile(FILE *fp, const int type) {
            LockFile(fileno(fp), type);
        }

        // is file of fd locked.
        static bool IsLocked(int fd);
        // is file locked.
        static bool IsLocked(FILE *fp) {
            return IsLocked(fileno(fp));
        }
        // open lock file.
        static FILE* LockFileOpen(const string &file, const char* mode, const int type);
    };

} // namespace wyf

#endif //_COMMON_CFILESYS_H_

