addToLibrary({
    my_js: function (p, byteSize) {
        const myUint8Array = Module.HEAPU8
        const numberOfFloats = byteSize / 4;
        const dataView = new DataView(myUint8Array.buffer, p, byteSize)

        const range = (start, stop, step = 1) => Array(Math.ceil((stop - start) / step)).fill(start).map((x, y) => x + y * step)

        //let arrayOfNumbers = range(0, numberOfFloats).map(idx => dataView.getFloat32(idx * 4, false));  
        // be careful with endianness, you may want to do:
        const arrayOfNumbers = range(0, numberOfFloats).map(idx => dataView.getFloat32(idx * 4, true))


        // const floatArray = Float32Array.from(myUint8Array, octet => octet / 0xFF)
        console.log("myUint8Array>>>>>>>>>>>>>>>>>>", arrayOfNumbers);
    },

    transpost_data_to_js: function (rid, ptr, size) {
        console.log("transpost_data_to_js>>>", rid, ptr, size)
        const srcArr = new Uint8Array(Module.HEAPU8.buffer, ptr, size);
        const sharedArray = new Uint8Array(Module.sharedDataBuffer, Module.sharedDataBufferPtr, size)
        sharedArray.set(srcArr)

        self.postMessage({
            method: 'data',
            rid: rid
        });
    }
});