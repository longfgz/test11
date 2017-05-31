#pragma once

#include <iostream>


#ifdef WIN32
  #define LOGGER_EXPORT __declspec(dllexport)
#else
  #define LOGGER_EXPORT
#endif

#define DESCRB_NAME
