{
    "targets": [
        {
            "target_name": "tsk-js",
            # Path relative to source files (./src)
            "libraries": ["-Wl,-Bsymbolic,../lib/libtsk.a"],
            "include_dirs": [
                "./vendor/sleuthkit/"
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

            ]
        }
    ]
}
