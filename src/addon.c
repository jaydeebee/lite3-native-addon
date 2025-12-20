#include <node_api.h>
#include <lite3-napi.h>

// Example function: lite3.version()
static napi_value Version(napi_env env, napi_callback_info info) {
  (void)info;  // unused

  napi_value result;
  NAPI_CALL(env, napi_create_string_utf8(env, "0.1.0", NAPI_AUTO_LENGTH, &result));
  return result;
}

// Module initialization - this is the entry point
static napi_value Init(napi_env env, napi_value exports) {
  // Register exported functions here
  napi_property_descriptor props[] = {
    { "version", NULL, Version, NULL, NULL, NULL, napi_default, NULL },
  };

  NAPI_CALL(env, napi_define_properties(env, exports, sizeof(props) / sizeof(props[0]), props));

  return exports;
}

// Register the module
NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)
