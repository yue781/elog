#pragma once
#include <memory>
#include <string>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4244)
#endif
#ifdef __APPLE__
#include <thread>
#endif

namespace elog {
// The context used to pass each output
struct context
{
   // 用于传递每次输出内容的上下文
   int                 level;
   unsigned int        tid;
   int                 line{};
   int                 err{};
   const char*         short_filename{};
   const char*         long_filename{};
   const char*         func_name{};
   std::string         text;
   // 计算中间经常可变的位置信息长度，可用于通过memset优化清零
   static unsigned int GetNoTextAndLevelLength(context& ctx)
   {
      static const unsigned int s_ctx_len = (char*)&ctx.text - (char*)&ctx.line;
      return s_ctx_len;
   }

   static std::shared_ptr<context> New()
   {
#if __cplusplus >= 201403L || (_MSC_VER && _MSVC_LANG >= 201403L)
      return std::make_shared<context>();
#else
      return SharedContext{new context};
#endif
   }
};
using SharedContext = std::shared_ptr<context>;
}   // namespace elog

#ifdef _MSC_VER
#pragma warning(pop)
#endif