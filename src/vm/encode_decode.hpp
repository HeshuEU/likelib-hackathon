/* Generated by Cython 0.29.20 */

#ifndef __PYX_HAVE__encode_decode
#define __PYX_HAVE__encode_decode

#include <python3.7/Python.h>
#include <string>

#ifndef __PYX_HAVE_API__encode_decode

#ifndef __PYX_EXTERN_C
  #ifdef __cplusplus
    #define __PYX_EXTERN_C extern "C"
  #else
    #define __PYX_EXTERN_C extern
  #endif
#endif

#ifndef DL_IMPORT
  #define DL_IMPORT(_T) _T
#endif

__PYX_EXTERN_C std::string getMethodsByArguments(char const *, char const *);
__PYX_EXTERN_C std::string encodeMessageFunction(char const *, char const *, char const *, char const *);
__PYX_EXTERN_C std::string encodeMessageConstructor(char const *, char const *, char const *);
__PYX_EXTERN_C std::string decodeMessage(char const *, char const *);

#endif /* !__PYX_HAVE_API__encode_decode */

/* WARNING: the interface of the module init function changed in CPython 3.5. */
/* It now returns a PyModuleDef instance instead of a PyModule instance. */

#if PY_MAJOR_VERSION < 3
PyMODINIT_FUNC initencode_decode(void);
#else
PyMODINIT_FUNC PyInit_encode_decode(void);
#endif

#endif /* !__PYX_HAVE__encode_decode */
