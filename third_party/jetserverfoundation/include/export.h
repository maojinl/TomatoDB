#ifndef JSF_INCLUDE_EXPORT_H_
#define JSF_INCLUDE_EXPORT_H_

#if !defined(JSF_EXPORT)

#if defined(JSF_SHARED_LIBRARY)
#if defined(_WIN32)

#if defined(JSF_COMPILE_LIBRARY)
#define JSF_EXPORT __declspec(dllexport)
#else
#define JSF_EXPORT __declspec(dllimport)
#endif  // defined(JSF_COMPILE_LIBRARY)

#else  // defined(_WIN32)
#if defined(JSF_COMPILE_LIBRARY)
#define JSF_EXPORT __attribute__((visibility("default")))
#else
#define JSF_EXPORT
#endif
#endif  // defined(_WIN32)

#else  // defined(JSF_SHARED_LIBRARY)
#define JSF_EXPORT
#endif

#endif  // !defined(JSF_EXPORT)

#endif  // JSF_INCLUDE_EXPORT_H_