#pragma once

#ifdef GLF_STATIC_LIBRARY
  #define AFX_EXT_GLF_INTERFACE
#else
  #if defined WIN32 || defined __CYGWIN__
    #ifdef EXPORT_GLF_INTERFACE
      #ifdef __GNUC__
        #define AFX_EXT_GLF_INTERFACE __attribute__((dllexport))
      #else
        #define AFX_EXT_GLF_INTERFACE __declspec(dllexport)
      #endif
    #else
      #ifdef __GNUC__
        #define AFX_EXT_GLF_INTERFACE __attribute__((dllimport))
      #else
        #define AFX_EXT_GLF_INTERFACE __declspec(dllimport)
      #endif
    #endif
  #else
    #if __GNUC__ >= 4
      #define AFX_EXT_GLF_INTERFACE __attribute__((visibility("default")))
    #else
      #define AFX_EXT_GLF_INTERFACE
    #endif
  #endif
#endif

