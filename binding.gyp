{
    "targets": [
        {
            "target_name": "tsk-js",
            "include_dirs": [
                "./lib/libtsk/include"
            ],
            "sources": [
                "src/utl/tsk_js_debug.cc",
                "src/module.cc",
                "src/tsk_js.cc",

                "src/utl/tsk_js_c_opts.cc",
                "src/utl/tsk_js_file_content.cc",
                "src/utl/tsk_js_file.cc",
                "src/utl/tsk_js_opts.cc",
                "src/utl/tsk_js_part.cc",
                "src/utl/tsk_js_vs.cc",
                "src/utl/tsk_js_fs.cc",

                "src/analyze/tsk_js_analyze.cc",
                "src/list/tsk_js_list.cc",
                "src/get/tsk_js_get.cc",
                "src/timeline/tsk_js_timeline.cc",
                "src/search/tsk_js_search.cc"
            ],

            "conditions": [
                ["OS=='linux'", {
                    # Path relative to source files (./src)
                    "libraries": ["../lib/libtsk/libtsk.a"],
                }],
                ["OS=='mac'", {
                    # Path relative to source files (./src)
                    "libraries": ["../lib/libtsk/libtsk.a"],
                }],
                ["OS=='win'", {
                    "defines": [
                        "USE_WCHAR_TSK_CHAR",
                    ],
                    "copies":[{
                        'destination': './build/Release/',
                        'files': [
                            "./lib/dlls/api-ms-win-core-console-l1-1-0.dll",
                            "./lib/dlls/api-ms-win-core-datetime-l1-1-0.dll",
                            "./lib/dlls/api-ms-win-core-debug-l1-1-0.dll",
                            "./lib/dlls/api-ms-win-core-errorhandling-l1-1-0.dll",
                            "./lib/dlls/api-ms-win-core-file-l1-1-0.dll",
                            "./lib/dlls/api-ms-win-core-file-l1-2-0.dll",
                            "./lib/dlls/api-ms-win-core-file-l2-1-0.dll",
                            "./lib/dlls/api-ms-win-core-handle-l1-1-0.dll",
                            "./lib/dlls/api-ms-win-core-heap-l1-1-0.dll",
                            "./lib/dlls/api-ms-win-core-interlocked-l1-1-0.dll",
                            "./lib/dlls/api-ms-win-core-libraryloader-l1-1-0.dll",
                            "./lib/dlls/api-ms-win-core-localization-l1-2-0.dll",
                            "./lib/dlls/api-ms-win-core-memory-l1-1-0.dll",
                            "./lib/dlls/api-ms-win-core-namedpipe-l1-1-0.dll",
                            "./lib/dlls/api-ms-win-core-processenvironment-l1-1-0.dll",
                            "./lib/dlls/api-ms-win-core-processthreads-l1-1-0.dll",
                            "./lib/dlls/api-ms-win-core-processthreads-l1-1-1.dll",
                            "./lib/dlls/api-ms-win-core-profile-l1-1-0.dll",
                            "./lib/dlls/api-ms-win-core-rtlsupport-l1-1-0.dll",
                            "./lib/dlls/api-ms-win-core-string-l1-1-0.dll",
                            "./lib/dlls/api-ms-win-core-synch-l1-1-0.dll",
                            "./lib/dlls/api-ms-win-core-synch-l1-2-0.dll",
                            "./lib/dlls/api-ms-win-core-sysinfo-l1-1-0.dll",
                            "./lib/dlls/api-ms-win-core-timezone-l1-1-0.dll",
                            "./lib/dlls/api-ms-win-core-util-l1-1-0.dll",
                            "./lib/dlls/api-ms-win-crt-conio-l1-1-0.dll",
                            "./lib/dlls/api-ms-win-crt-convert-l1-1-0.dll",
                            "./lib/dlls/api-ms-win-crt-environment-l1-1-0.dll",
                            "./lib/dlls/api-ms-win-crt-filesystem-l1-1-0.dll",
                            "./lib/dlls/api-ms-win-crt-heap-l1-1-0.dll",
                            "./lib/dlls/api-ms-win-crt-locale-l1-1-0.dll",
                            "./lib/dlls/api-ms-win-crt-math-l1-1-0.dll",
                            "./lib/dlls/api-ms-win-crt-multibyte-l1-1-0.dll",
                            "./lib/dlls/api-ms-win-crt-private-l1-1-0.dll",
                            "./lib/dlls/api-ms-win-crt-process-l1-1-0.dll",
                            "./lib/dlls/api-ms-win-crt-runtime-l1-1-0.dll",
                            "./lib/dlls/api-ms-win-crt-stdio-l1-1-0.dll",
                            "./lib/dlls/api-ms-win-crt-string-l1-1-0.dll",
                            "./lib/dlls/api-ms-win-crt-time-l1-1-0.dll",
                            "./lib/dlls/api-ms-win-crt-utility-l1-1-0.dll",
                            "./lib/dlls/concrt140.dll",
                            "./lib/dlls/libewf.dll",
                            "./lib/dlls/libtsk_jni.dll",
                            "./lib/dlls/libvhdi.dll",
                            "./lib/dlls/libvmdk.dll",
                            "./lib/dlls/msvcp140.dll",
                            "./lib/dlls/ucrtbase.dll",
                            "./lib/dlls/vccorlib140.dll",
                            "./lib/dlls/vcruntime140.dll",
                            "./lib/dlls/zlib.dll",
                        ]
                    }],
                    # Path relative to source files (./src)
                    "libraries": [
                        "../lib/libtsk/libtsk.lib",
                        "../lib/libewf/libewf.lib",
                        "../lib/libvmdk/libvmdk.lib",
                        "../lib/libvhdi/libvhdi.lib",
                    ]
                }]
            ]
        }
    ]
}
