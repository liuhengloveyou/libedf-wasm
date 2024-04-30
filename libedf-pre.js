///////////////////////////////////////////////////////////////////////////////
// wasm Module
///////////////////////////////////////////////////////////////////////////////
Module.MKDIRED = false;
Module.MOUNT_DIR = "/working";
Module.current_edf_file = null;
// 主worker的SharedArrayBuffer
Module.sharedDataBuffer = null;
// 主worker的SharedArrayBuffer中波形数据的指针
Module.sharedDataBufferPtr = 0;


Module.onRuntimeInitialized = () => {
    console.log("libedf Module.onRuntimeInitialized", Module)

    if (!Module.MKDIRED) {
        FS.mkdir(Module.MOUNT_DIR);
        Module.MKDIRED = true;
    }

    postMessage({
        method: 'loaded'
    });
    // var myUint8Array = Module.getBytes()
    // console.log("myUint8Array>>>>>>>>>>>>>>>>>>", myUint8Array.byteOffset, myUint8Array.byteLength, myUint8Array);
    // Module.canvas = document.getElementById('canvas');
    // Module.canvas.addEventListener("webglcontextlost", function(e) { alert('FIXME: WebGL context lost, please reload the page'); e.preventDefault(); }, false);
    // console.log("Module.onRuntimeInitialized", Module.canvas)
}

// 加载文件系统
Module.mountFile = (file) => {
    console.log(">>>", Module.MKDIRED, Module.MOUNT_DIR, typeof (file))

    let name = file.name;
    FS.mount(WORKERFS, { files: [file] }, Module.MOUNT_DIR);

    return Module.MOUNT_DIR + "/" + name;
}

///////////////////////////////////////////////////////////////////////////////
// webworker message handler
///////////////////////////////////////////////////////////////////////////////
onmessage = function (event) {
    const data = event.data;
    console.log("wasm.worker.onmessage::", data.method, data);
    switch (data.method) {
        case "open": {
            doOpenEdfFile(data.file, data.buff, data.ptr);
            break;
        }
        case "read": {
            Module.sharedDataBuffer = data.buffer;
            Module.sharedDataBufferPtr = data.ptr;
            console.log("wasm.worker.shareBuffer::", Module.sharedDataBufferPtr, Module.sharedDataBuffer);
            doReadOneDataRecord(data.rid);
            break;
        }
        case "close": {
            doCloseEdfFile();
            break;
        }
        case "labels": {
            doReadLabel();
            break;
        }
        default:
            break;
    }
};

function doOpenEdfFile(file) {
    const fn = Module.mountFile(file)
    const mate = Module.edf_open(fn);
    console.log("doOpenEdfFile fn::", fn, mate, file)

    self.postMessage({
        method: 'opened',
        param: mate,
    });
}

function doCloseEdfFile() {
    Module.edf_close();
}

function doReadOneDataRecord(rid) {
    Module.edf_read_datarecord(rid)
}

function doReadLabel() {
    var labels = Module.get_labels()
    console.log("labels>>>>>>>>>>>>>>>>>>", labels.byteOffset, labels.byteLength, labels);
}

function callFromC(myUint8Array) {
    var myUint8Array = Module.getBytes()
    console.log("myUint8Array>>>>>>>>>>>>>>>>>>", myUint8Array.byteOffset, myUint8Array.byteLength, myUint8Array);

    let numberOfFloats = myUint8Array.byteLength / 4;
    // let dataView = new DataView(myUint8Array.buffer);
    // sometimes your Uint8Array is part of larger buffer, then you will want to do this instead of line above:
    let dataView = new DataView(myUint8Array.buffer, myUint8Array.byteOffset, myUint8Array.byteLength)

    const range = (start, stop, step = 1) => Array(Math.ceil((stop - start) / step)).fill(start).map((x, y) => x + y * step)

    //let arrayOfNumbers = range(0, numberOfFloats).map(idx => dataView.getFloat32(idx * 4, false));  
    // be careful with endianness, you may want to do:
    let arrayOfNumbers = range(0, numberOfFloats).map(idx => dataView.getFloat32(idx * 4, true))


    // const floatArray = Float32Array.from(myUint8Array, octet => octet / 0xFF)
    console.log("myUint8Array>>>>>>>>>>>>>>>>>>", arrayOfNumbers);
    // let dv = new DataView(myUint8Array.buffer);
    // // let offset = 0;
    // for (let i = 0; i < 40; i += 4 ) {
    //     console.log(">>>", dv.getFloat32(i))
    // }
}
