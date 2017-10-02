{
    "targets": [
        {
            "target_name": "tsk-js",
            "libraries": ["-Wl,-Bsymbolic,/usr/local/lib/libtsk.a"],
            "sources": [
                "src/module.cc",
                "src/tsk_js.cc",
                "src/tsk_js_utl.cc"
            ]
        }
    ]
}
