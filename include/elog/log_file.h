//
// Created by Alone on 2022-9-21.
//
#pragma once
#include <ctime>
#include <memory>
#include <mutex>

#include "file_appender.h"

LBLOG_NAMESPACE_BEGIN
namespace detail {
class LogFile : noncopyable
{
public:
   // 默认是线程安全，默认3s刷新，默认1024次执行检查
   explicit LogFile(const char* basename, int rollSize, bool threadSafe = true,
                    int flushInterval = 3, int checkEveryN = 1024);
   ~LogFile() = default;

   void append(const char* line, int len);
   void flush();
   void rollFile(const time_t* now = nullptr);

private:
   void append_unlocked(const char* line, int len);

private:
   enum { kRollPerSeconds = 60 * 60 * 24 };
   const char*                 m_basename;
   std::unique_ptr<std::mutex> m_mtx;
   std::unique_ptr<FileAppender> m_file;   // 真正写入文件系统的appender
   time_t m_lastPeriod{};   // 以天为单位的time，用取整计算表示
   time_t m_lastRoll{};   // 上一次roll日志的时间（精确到秒的时间）
   time_t m_lastFlush{};   // 上一次flush的时间

   const int m_rollSize;
   const int m_flushInterval;
   const int m_checkEveryN;
   int       m_count{};   // 计算Log的次数
};
}   // namespace detail
LBLOG_NAMESPACE_END
