#include <node_api.h>

// Helper macro - check status and throw on error
#define NAPI_CALL(env, call)                                      \
  do {                                                            \
    napi_status status = (call);                                  \
    if (status != napi_ok) {                                      \
      const napi_extended_error_info* error_info = NULL;          \
      napi_get_last_error_info((env), &error_info);               \
      const char* msg = error_info->error_message;                \
      napi_throw_error((env), NULL, msg ? msg : "N-API error");   \
      return NULL;                                                \
    }                                                             \
  } while (0)
