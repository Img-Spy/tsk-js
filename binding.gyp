{
    "targets": [
        {
            "target_name": "tsk-js",
            "libraries": ["-Wl,-Bsymbolic,/usr/local/lib/libtsk.a"],
            "sources": [
                "src/module.cc",
                "src/tsk_js.cc",
                "src/tsk_js_utl.cc",

                "src/tsk_js_analyze.cc",
                "src/tsk_js_list.cc",
                "src/tsk_js_get.cc",
                "src/tsk_js_timeline.cc"
            ]
        }
    ]
}
