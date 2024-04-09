
///////////////////////////////////////////////////////////////////////////////
// wasm Module
///////////////////////////////////////////////////////////////////////////////
Module.MKDIRED = false;
Module.MOUNT_DIR = "/working";

Module.onRuntimeInitialized = () => {
    console.log("Module.onRuntimeInitialized")
    
    postMessage({
        id: 'loaded',
    });

    // var myUint8Array = Module.getBytes()
    // console.log("myUint8Array>>>>>>>>>>>>>>>>>>", myUint8Array.byteOffset, myUint8Array.byteLength, myUint8Array);
    // Module.canvas = document.getElementById('canvas');
    // Module.canvas.addEventListener("webglcontextlost", function(e) { alert('FIXME: WebGL context lost, please reload the page'); e.preventDefault(); }, false);
    // console.log("Module.onRuntimeInitialized", Module.canvas)
}


// 加载文件
Module.mountFile = (file) => {
    console.log(">>>", Module.MKDIRED, Module.MOUNT_DIR, typeof (file))
    
    if (!Module.MKDIRED) {
        FS.mkdir(Module.MOUNT_DIR);
        Module.MKDIRED = true;
    }

    let name = file.name;
    FS.mount(WORKERFS, { files: [file] }, Module.MOUNT_DIR);

    return Module.MOUNT_DIR + "/" + name;
}

Module.singleImage = (imgDataPtr) => {
    const width = Module.HEAPU32[imgDataPtr];
    const height = Module.HEAPU32[imgDataPtr + 1];
    const duration = Module.HEAPU32[imgDataPtr + 2];
    const imageBufferPtr = Module.HEAPU32[imgDataPtr + 3];
    const imageBuffer = Module.HEAPU8.slice(imageBufferPtr, imageBufferPtr + width * height * 3);
    Module._free(imgDataPtr);
    Module._free(imageBufferPtr);

    const imageDataBuffer = new Uint8ClampedArray(width * height * 4);

    let j = 0;
    for (let i = 0; i < imageBuffer.length; i++) {
        if (i && i % 3 === 0) {
            imageDataBuffer[j] = 255;
            j += 1;
        }

        imageDataBuffer[j] = imageBuffer[i];
        j += 1;
    }
    return {
        width,
        height,
        duration,
        imageDataBuffer,
    };

}


///////////////////////////////////////////////////////////////////////////////
// web worker message handler
///////////////////////////////////////////////////////////////////////////////
onmessage = function (event) {
    const data = event.data;
    console.log("wasm.worker.onmessage data::", data);
        
    switch (data.type) {
        case "open": {
            doOpenEdfFile(data.file);
            break;
        }
        case "read": {
            doReadEdfFile(1, 100);
            break;
        }
        case "read_label": {
            doReadLabel();
            break;
        }
        default:
            break;
    }
};

function doOpenEdfFile(file)
{
    const fn = Module.mountFile(file)
    console.log("doOpenEdfFile::", fn)
    Module.edf_open(fn);
}

function doReadEdfFile(s, n)
{
    Module.edf_read_datarecord(s, n)
}

function doReadLabel()
{
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

const doCapture = (id, file) => {
    const fn = Module.mountFile(file)
    console.log("doCapture mountFile::", id, fn, file)

    Module.capture(fn)
    // Module.readFile(fn)
    //let point = Module.getPoint();
    //console.log("point>>>>>>>>>>>>>>>>>>", point)
}

const doPlayVideo = (id, file) => {
    const fn = Module.mountFile(file)
    console.log("doPlay mountFile::", id, fn, file)

    Module.playVideo(id, fn)
}

const doPlayVideoByWebsocket = (id, url) => {
    Module.playVideoByWebsocket(id, url);
}
