{
  "variables": {
    "openssl_fips": ""
  },
  "targets": [
    {
      "target_name": "lite3",
      "sources": [
        "src/addon.c",
        "src/addon_encode.c",
        "src/addon_decode.c",
        "src/addon_proxy.c",
        "deps/lite3/src/lite3.c",
        "deps/lite3/src/json_enc.c",
        "deps/lite3/src/ctx_api.c",
        "deps/lite3/lib/yyjson/yyjson.c",
        "deps/lite3/lib/nibble_base64/base64.c"
      ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")",
        "include",
        "deps/lite3/include",
        "deps/lite3/lib"
      ],
      "cflags": [
        "-std=gnu11",
        "-Wall",
        "-Wextra"
      ],
      "xcode_settings": {
        "GCC_C_LANGUAGE_STANDARD": "gnu11",
        "WARNING_CFLAGS": ["-Wall", "-Wextra"]
      },
      "defines": [
        "NAPI_VERSION=9"
      ],
      "configurations": {
        "Debug": {
          "cflags": ["-g", "-O0"],
          "xcode_settings": {
            "GCC_OPTIMIZATION_LEVEL": "0",
            "GCC_GENERATE_DEBUGGING_SYMBOLS": "YES"
          },
          "defines": [
            "DEBUG",
            "LITE3_ERROR_MESSAGES",
            "LITE3_DEBUG",
            "LITE3_JSON"
          ]
        },
        "Release": {
          "cflags": ["-O3"],
          "xcode_settings": {
            "GCC_OPTIMIZATION_LEVEL": "3"
          },
          "defines": ["NDEBUG"]
        }
      }
    }
  ]
}
