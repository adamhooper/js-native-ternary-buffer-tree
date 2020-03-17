{
  "targets": [
    {
      "target_name": "ternary_buffer_tree",
      "include_dirs": ["<!@(node -p \"require('node-addon-api').include\")"],
      "defines": [ "NAPI_DISABLE_CPP_EXCEPTIONS" ],
      "sources": [ "src/TernarySearchTree.cc", "src/TernaryBufferTree.cc", "src/addon.cc" ],
      "cflags_cc": [ "-std=c++17", "-Wall", "-fvisibility=hidden" ],
      "xcode_settings": {
        "GCC_SYMBOLS_PRIVATE_EXTERN": "YES", # -fvisibility=hidden
      },
    }
  ]
}
