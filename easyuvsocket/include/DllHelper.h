#ifdef BUILD_EASYUVSOCKET
#define EASYUVSOCKET_API EXPORT_EASYUVSOCKET
#else
#define EASYUVSOCKET_API IMPORT_EASYUVSOCKET
#endif

#if defined(_MSC_VER)
    //  Microsoft 
    #define EXPORT_EASYUVSOCKET __declspec(dllexport)
    #define IMPORT_EASYUVSOCKET __declspec(dllimport)
#elif defined(__GNUC__)
    //  GCC
    #define EXPORT_EASYUVSOCKET __attribute__((visibility("default")))
    #define IMPORT_EASYUVSOCKET
#else
    //  do nothing and hope for the best?
    #define EXPORT_EASYUVSOCKET
    #define IMPORT_EASYUVSOCKET
    #pragma warning Unknown dynamic link import/export semantics.
#endif
