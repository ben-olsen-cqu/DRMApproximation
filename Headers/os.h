//#if defined(__APPLE__)
//#define MAC
//#endif

#if defined(WIN64) || defined(WIN32)
#define WINDOWS
#endif

#if defined(unix) || defined(__unix__) || defined(__unix)
#define LINUX
#endif

//#if defined(MAC) || defined(LINUX)
//#endif 
//#if defined(WINDOWS)
//#endif