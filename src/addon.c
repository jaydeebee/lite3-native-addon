#include<node_api.h>
#include<lite3-napi.h>
#include<lite3_context_api.h>

static napi_value Version(napi_env env, napi_callback_info info) {
  (void)info;  // unused

  napi_value result;
  NAPI_CALL(env, NULL, napi_create_string_utf8(env, "0.1.0", NAPI_AUTO_LENGTH, &result), NULL);
  return result;
}

// Module initialization
static napi_value Init(napi_env env, napi_value exports) {
  // Register exported functions here
  napi_property_descriptor props[] = {
    { "version", NULL, Version, NULL, NULL, NULL, napi_enumerable, NULL },
    { "encode", NULL, encode, NULL, NULL, NULL, napi_enumerable, NULL },
    { "decode", NULL, decode, NULL, NULL, NULL, napi_enumerable, NULL }
  };

  NAPI_CALL(env, NULL, napi_define_properties(env, exports, a_count(props), props), NULL);

  return exports;
}

// Register the module
NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)
