{
  "targets": [
    {
      "target_name": "lite3",
      "sources": [
        "src/addon.c"
      ],
      "include_dirs": [
        "include",
        "deps/lite3/include"
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
      ]
    }
  ]
}
