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

    my_js_struct: function(p, byteSize) {
        const dataView = new DataView(Module.HEAPU8.buffer, p, byteSize)
       
        console.log(">>>", Module)
        console.log("my_js_struct>>>>>>>>>>>>>>>>>>", p, dataView.getInt32(0, true), dataView.getInt32(4, true), Module.HEAP32[p << 2]);
        //  for (let i = 1; i <= 100; i ++ ) {
        //     console.log("my_js_struct>>>>>>>>>>>>>>>>>>", i, dataView.getFloat32(4+i*4, true));
        // }
    },

    transpost_frame_to_js: function(rid, yptr, uptr, vptr, width, height) {
        console.log("transpost_frame_to_js>>>", rid, width, height)
        // const data = Module.getImageInfo(ptr / 4);

        self.postMessage({
            type: 'play/frame',
            id: rid,
            data: Module.HEAPU8,
            yptr: yptr,
            uptr: uptr,
            vptr: vptr, 
            width: width,
            height: height,
        });
        // // console.log('transpostFrame==>', id, imageCapture.captureInfo);
        // if (imageCapture.imageList[id].length >= imageCapture.captureInfo[id]) {
        //     // 说明已经到了数目 可以postonfinish事件
        //     self.postMessage({
        //         type: 'receiveImageOnSuccess',
        //         id,
        //         meta: metaDataMap[id] || {},
        //         // ...imageCapture.imageList[id], //TODO: 这个是否post未确定
        //     });
        // }
    }
});