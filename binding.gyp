{
    "targets": [
        {
            "target_name": "tsk-js",
            # Path relative to source files (./src)
            "libraries": ["-Wl,-Bsymbolic,../vendor/sleuthkit/tsk/.libs/libtsk.a"],
            "include_dirs": [
                "./vendor/sleuthkit/"
            ],
            "sources": [
                "src/module.cc",
                "src/tsk_js.cc",
                "src/tsk_js_utl.cc",
                "src/tsk_js_file.cc",

                "src/tsk_js_analyze.cc",
                "src/tsk_js_list.cc",
                "src/tsk_js_get.cc",
                "src/tsk_js_timeline.cc",
                "src/tsk_js_search.cc"
            ]
        }
    ]
}
