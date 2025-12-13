#ifndef QUIK_API_H
#define QUIK_API_H

// 静态库：空宏
// 动态库：取消注释下面的定义
// #define QUIK_BUILD_DLL

#ifdef QUIK_BUILD_DLL
    #ifdef QUIK_LIBRARY
        #define QUIK_API __declspec(dllexport)
    #else
        #define QUIK_API __declspec(dllimport)
    #endif
#else
    #define QUIK_API
#endif

#endif // QUIK_API_H
