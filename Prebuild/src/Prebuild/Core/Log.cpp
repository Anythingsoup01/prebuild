#include "pbpch.h"
#include "Log.h"

#include <spdlog/sinks/stdout_color_sinks.h>

Ref<spdlog::logger> Log::s_APILogger;

void Log::Init()
{
	spdlog::set_pattern("%^[%T] %n: %v%$");
	s_APILogger = spdlog::stdout_color_mt("CEAPI");
	s_APILogger->set_level(spdlog::level::trace);
}
