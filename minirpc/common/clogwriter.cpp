/*
** Copyright (C) 2014 Wang Yaofu
** All rights reserved.
**
**Author:Wang Yaofu voipman@qq.com
**Description: The source file of class CLogWriter.
*/
#include <stdarg.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include "common/clogwriter.h"

// ---------------------------------------------------------------------------
// CLogWriter::CLogWriter()
//
// constructor
// ---------------------------------------------------------------------------
//
CLogWriter::CLogWriter() {
    iIndex = 0;
    iDay   = 0;
    iProcessId = 0;
    iProcessId = getpid();
    memset(iHostName, 0, sizeof(iHostName));
    gethostname(iHostName, sizeof(iHostName));
}

// ---------------------------------------------------------------------------
// CLogWriter::~CLogWriter()
//
// destructor.
// ---------------------------------------------------------------------------
//
CLogWriter::~CLogWriter() {
}

// ---------------------------------------------------------------------------
// void CLogWriter::GenLogFileName()
//
// local file name is generated.
// ---------------------------------------------------------------------------
//
void CLogWriter::GenLogFileName() {
    time_t cur_time;
    struct tm *tm;
    time(&cur_time);
    tm=localtime(&cur_time);
    if (iDay != tm->tm_mday) {
        iDay = tm->tm_mday;
        iIndex = 0;
    }
    snprintf(iLocalLogFileName, sizeof(iLocalLogFileName),
        "%s.%4d%02d%02d.%d.%d",
        iLogConfig.iLogFileName.c_str(),
        1900+tm->tm_year,tm->tm_mon+1,tm->tm_mday, getpid(), iIndex);
}

// ---------------------------------------------------------------------------
// bool CLogWriter::InitLogFile()
//
// init log file and open log file.
// ---------------------------------------------------------------------------
//
bool CLogWriter::InitLogFile() {
    GenLogFileName();
    iOutPutPtrace.close();
    iOutPutPtrace.clear();
    iOutPutPtrace.open(iLocalLogFileName, ios::out | ios::app);
    if(iOutPutPtrace.bad() || !iOutPutPtrace.good()) {
        fprintf(stderr, "CLogWriter::InitLogFile():open file %s failed.\n",
            iLocalLogFileName);
        fflush(stderr);
        return false;
    }
    return true;
}

// ---------------------------------------------------------------------------
// bool CLogWriter::CheckLogFile()
//
// check whether log file exists.
// ---------------------------------------------------------------------------
//
bool CLogWriter::CheckLogFile() {
    const char* funcName = "CLogWriter::CheckLogFile()";
    GenLogFileName();
    struct stat64 stinfo;

    memset(&stinfo, 0, sizeof(stinfo));
    int ret = stat64(iLocalLogFileName, &stinfo);
    if(0 == ret) {
        if (stinfo.st_size > MAX_LOG_SIZE) {
            // if log size if bigger than 1G, get a new log name.
            iIndex++;
            try {
                iOutPutPtrace.close();
                return InitLogFile();
            } catch (...) {
                fprintf(stderr, "%s:close file failed.errno:[%d]\n", funcName, errno);
            }
        }
        //file exist
        return true;
    } else {
        if (iOutPutPtrace.good()) {
            try {
                iOutPutPtrace.close();
                return InitLogFile();
            } catch (...) {
                fprintf(stderr, "%s:close file failed.errno:[%d]\n", funcName, errno);
            }
        }
        //other error occurred
        return false;
    }
}

// ---------------------------------------------------------------------------
// bool CLogWriter::CheckWritable()
//
// check whether writable of log file.
// ---------------------------------------------------------------------------
//
bool CLogWriter::CheckWritable() {
    if (iOutPutPtrace.good()) {
        return CheckLogFile();
    } else {
        return InitLogFile();
    }
}

// ---------------------------------------------------------------------------
// bool CLogWriter::Init(struct CConfig& aAiConfig)
//
// init data for writer.
// ---------------------------------------------------------------------------
//
bool CLogWriter::Init(const struct CLogConfig& aLogConfig) {
    iLogConfig = aLogConfig;
    return true;
}

// ---------------------------------------------------------------------------
// void SetLogLevel(int aLevel);
//
// log level is set.
// ---------------------------------------------------------------------------
//
void CLogWriter::SetLogLevel(const int aLevel) {
    iLogConfig.iLogLevel = static_cast<ELogLevel>(aLevel);
}

// ---------------------------------------------------------------------------
// void SetLogOption(unsigned int aOption);
//
// log option is set.
// ---------------------------------------------------------------------------
//
void CLogWriter::SetLogOption(const unsigned int aOption) {
    iLogConfig.iLogOption |= aOption;
}

// ---------------------------------------------------------------------------
// void SetLogOption(unsigned int aOption);
//
// log option is cleared.
// ---------------------------------------------------------------------------
//
void CLogWriter::ClearLogOption(const unsigned int aOption) {
    iLogConfig.iLogOption &= ~aOption;
}

// ---------------------------------------------------------------------------
// void SetLogFileName(string aLogFileName);
//
// log name is set.
// ---------------------------------------------------------------------------
//
void CLogWriter::SetLogFileName(const string& aLogFileName) {
    iLogConfig.iLogFileName = aLogFileName;
}

// ---------------------------------------------------------------------------
// bool CLogWriter::Log()
//
// log is wrote into a buffer.
// ---------------------------------------------------------------------------
//
bool CLogWriter::Log(ELogLevel aLogLevel, const char* aFormat, ...) {
    if (aLogLevel > iLogConfig.iLogLevel) {
        return true;
    }
    va_list mark;
    char buf[2049] = {0};
    int off = 0;
    if (iLogConfig.iLogOption & 1) {
        struct timeval tv;

        gettimeofday(&tv,NULL);
        off = strftime(buf, sizeof(buf),"%D %H:%M:%S.", localtime(&tv.tv_sec));
        sprintf(buf+off, "%03d:[%d]:[%d,%lu]-",
            (int)tv.tv_usec/1000, aLogLevel, iProcessId, pthread_self());
        off = strlen(buf);
    }
    va_start(mark, aFormat);
    vsnprintf(buf+off, 2048-off, aFormat, mark);
    va_end(mark);
    return LogRaw(aLogLevel, buf);
}


// ---------------------------------------------------------------------------
// bool CLogWriter::LogRaw()
//
// log is wrote into file.
// ---------------------------------------------------------------------------
//
bool CLogWriter::LogRaw(ELogLevel aLogLevel, const char* aMsgRaw) {
    if (aLogLevel > iLogConfig.iLogLevel) {
        return true;
    }
    iMutex.lock();
    //check whether writable
    if(CheckWritable()) {
        iOutPutPtrace.write(aMsgRaw, strlen(aMsgRaw));
        iOutPutPtrace << "\n";
        iOutPutPtrace.flush();
        //fprintf(stderr, "%s\n", aMsgRaw);
        //fflush(stderr);
    } else {
        iMutex.unlock();
        return false;
    }
    iMutex.unlock();
    return true;
}

// log buffer with len.
bool CLogWriter::LogHexWithLen(ELogLevel aLogLevel,
                               const char* aMsgRaw,
                               int aLen) {
    if (aLogLevel > iLogConfig.iLogLevel) {
        return true;
    }
    iMutex.lock();
    //check whether writable
    if(CheckWritable()) {
        char hexItem[4] = {0};
        for (int ii = 1; ii <= aLen; ii++) {
            sprintf(hexItem, "%02X ", (unsigned char)aMsgRaw[ii-1]);
            iOutPutPtrace.write(hexItem, 3);
            //fprintf(stderr, "%s", hexItem);
            //fflush(stderr);
            if (ii % 16 == 0)
            {
                iOutPutPtrace << "\n";
                //fprintf(stderr, "\n");
                //fflush(stderr);
            }
        }
        iOutPutPtrace << "\n";
    } else {
        iMutex.unlock();
        return false;
    }
    iMutex.unlock();
    return true;
}

void CLogWriter::LogHexAsc(ELogLevel aLogLevel, const void *aBuf, int aBufLen) {
    if (aLogLevel > iLogConfig.iLogLevel) {
        return;
    }

    // Check args
    if (aBuf == NULL) {
        // Null buffer address
        return;
    }

    char    out[100];         // Formatted output string
    // Dump the contents of the data buffer, one 16-byte line at a time
    const unsigned char* dat = (const unsigned char *) aBuf;
    LogRaw(aLogLevel,
        "OFFS  ----------------- HEXADECIMAL------------------  *------ASCII-----*");

    for (int off = 0x0000;  off < aBufLen;  off += 16) {
        int j;
        int o = 0;

        // Format and print the contents of this line

        // Format the address offset
        sprintf(out+o, "%04X:", off & 0xFFFF);
        o += 5;

        // Format the data as hex
        for (j = 0;  j < 16  &&  off+j < aBufLen;  j++) {
            sprintf(out+o, " %02X", dat[off+j]);
            o += 3;
        }

        for (;  j < 16;  j++) {
            sprintf(out+o, "   ");
            o += 3;
        }

        // Format the data as printable characters
        out[o++] = ' ';
        out[o++] = ' ';
        out[o++] = '(';

        for (j = 0;  j < 16  &&  off+j < aBufLen;  j++) {
            int ch = dat[off+j];
            out[o++] = isprint(ch) ? ch : '.';
        }

        out[o++] = ')';
        //out[o++] = '\n';
        out[o++] = '\0';

        // Print the formatted output line
        LogRaw(aLogLevel, out);
    }
}
