#include<node_api.h>
#include<lite3-napi.h>
#include<lite3_context_api.h>

static napi_value Lite3Version(napi_env env, napi_callback_info info) {
  (void)info;  // unused

  napi_value result;
  NAPI_CALL(env, NULL, napi_create_string_utf8(env, LITE3_LIB_VERSION, NAPI_AUTO_LENGTH, &result), NULL);
  return result;
}

// Module initialization
static napi_value Init(napi_env env, napi_value exports) {
  // Register exported functions here
  napi_property_descriptor props[] = {
    { "lite3Version", NULL, Lite3Version, NULL, NULL, NULL, napi_enumerable, NULL },
    { "encode", NULL, encode, NULL, NULL, NULL, napi_enumerable, NULL },
    { "decode", NULL, decode, NULL, NULL, NULL, napi_enumerable, NULL },
    // Proxy support functions:
    { "getType", NULL, proxy_get_type, NULL, NULL, NULL, napi_enumerable, NULL },
    { "getArrayType", NULL, proxy_get_array_type, NULL, NULL, NULL, napi_enumerable, NULL },
    { "getValue", NULL, proxy_get_value, NULL, NULL, NULL, napi_enumerable, NULL },
    { "getArrayElement", NULL, proxy_get_array_element, NULL, NULL, NULL, napi_enumerable, NULL },
    { "getChildOffset", NULL, proxy_get_child_offset, NULL, NULL, NULL, napi_enumerable, NULL },
    { "getArrayChildOffset", NULL, proxy_get_array_child_offset, NULL, NULL, NULL, napi_enumerable, NULL },
    { "getKeys", NULL, proxy_get_keys, NULL, NULL, NULL, napi_enumerable, NULL },
    { "getLength", NULL, proxy_get_length, NULL, NULL, NULL, napi_enumerable, NULL },
    { "hasKey", NULL, proxy_has_key, NULL, NULL, NULL, napi_enumerable, NULL },
    { "getRootType", NULL, proxy_get_root_type, NULL, NULL, NULL, napi_enumerable, NULL }
  };

  NAPI_CALL(env, NULL, napi_define_properties(env, exports, a_count(props), props), NULL);

  return exports;
}

// Register the module
NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)
