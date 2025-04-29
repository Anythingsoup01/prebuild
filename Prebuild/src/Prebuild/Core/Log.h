#pragma once

#include <spdlog/spdlog.h>

class Log
{
public:
    static void Init();
    
    static Ref<spdlog::logger>& GetAPILogger() { return s_APILogger; }
private:
    static Ref<spdlog::logger> s_APILogger;
};

#define PB_CRITICAL(...)             ::Log::GetAPILogger()->critical(__VA_ARGS__)
#define PB_ERROR(...)                ::Log::GetAPILogger()->error(__VA_ARGS__)
#define PB_WARN(...)                 ::Log::GetAPILogger()->warn(__VA_ARGS__)
#define PB_INFO(...)                 ::Log::GetAPILogger()->info(__VA_ARGS__)
#define PB_TRACE(...)                ::Log::GetAPILogger()->trace(__VA_ARGS__)

