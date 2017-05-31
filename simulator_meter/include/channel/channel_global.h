#pragma once

#include <deque>

#ifdef WIN32
  #define CHANNEL_EXPORT __declspec(dllexport)
#else
  #define CHANNEL_EXPORT
#endif
