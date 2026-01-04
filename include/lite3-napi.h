#ifndef LITE3_NAPI_H
# define LITE3_NAPI_H
# include <node_api.h>

// Helper macro - check status and throw on error
# define NAPI_CALL(_env, _ctx, call, failure)                     \
  do {                                                            \
    napi_status status = (call);                                  \
    if (status != napi_ok) {                                      \
      const napi_extended_error_info* error_info = NULL;          \
      napi_get_last_error_info((_env), &error_info);              \
      const char* msg = error_info->error_message;                \
      napi_throw_error((_env), NULL, msg ? msg : "N-API error");  \
      if (_ctx) {                                                 \
        lite3_ctx_destroy(_ctx);                                  \
      }                                                           \
      return failure;                                             \
    }                                                             \
  } while (0)

# define LITE3_CALL(_env, _ctx, call, failure)     \
  do {                                             \
    int rc = (call);                               \
    if (rc != 0) {                                 \
      napi_throw_error(_env, NULL, "Lite3 error"); \
      if (_ctx) {                                  \
        lite3_ctx_destroy(_ctx);                   \
      }                                            \
      return failure;                              \
    }                                              \
  } while (0)

# define a_count(x)  (sizeof(x) / sizeof(*x))

// Declarations for project functions:
extern napi_value encode(napi_env, napi_callback_info);
extern napi_value decode(napi_env, napi_callback_info);

#endif // LITE3_NAPI_H
