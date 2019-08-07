const Rx = require("rxjs/Rx");
const path = require("path");
const fs = require("fs");
const extractZip = require("extract-zip");
const rootPackageJson = require("../package.json");
const request = require("request");
const libUrl = `${process.platform}/${process.arch}/libtsk.a`;


const unzip$ = Rx.Observable.of({
    tskJsFolder: path.join(__dirname, ".."),
    vendorDir: path.join(__dirname, "../vendor"),
    vendorZip: path.join(__dirname, "../vendor.zip")
})
.filter((session) => fs.existsSync(session.vendorZip))
// Unzip
.mergeMap(session => Rx.Observable.create(observer => {
    extractZip(session.vendorZip, { dir: session.vendorDir }, (err) => {
        if(err) {
            console.log("")
        } else {
            observer.next(session);
        }
    });
}))
.mapTo("Extracted vendor.zip")


const download$ = Rx.Observable.of({
    baseUrl: "http://docs-tsk-js.54.37.131.126.xip.io/download",
    libFile: path.join(__dirname, "../lib/libtsk.a")
})
.filter((session) => !fs.existsSync(session.libFile))
// Download file
.mergeMap(session => Rx.Observable.create(observer => {
    const url = `${session.baseUrl}/${libUrl}`;
    request(url, {encoding: 'binary'}, function (error, response, body) {
        if (error) { observer.error(); }
        else { 
            session.response = response;
            session.body = body;
            observer.next(session); 
        }
        observer.complete();
    });
}))
.filter(session => session.response.statusCode === 200)
.map(session => {
    fs.writeFile(session.libFile, session.body, 'binary', function (err) {});
})
.mapTo(`Downloaded ${libUrl}`)


const preinstall = [
    unzip$,
    download$
];


const main = () => {
    preinstall.forEach(obs$ => obs$.subscribe(
        x => console.log(x),
        err => console.log(err),
        () => {}
    ));
}

main();
