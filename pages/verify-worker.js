"use strict";

importScripts("./sha256.js", "./supported-disc.js");

const blockSize = 1024 * 1024;

self.onmessage = async ({data: file}) => {
  if (file.size !== supportedDisc.bytes) {
    self.postMessage({state: "sizeMismatch", bytes: supportedDisc.bytes});
    return;
  }

  try {
    const hash = await self.hashwasm.createSHA256();
    let previousProgress = 0;
    for (let offset = 0; offset < file.size; offset += blockSize) {
      const block = await file.slice(offset, offset + blockSize).arrayBuffer();
      hash.update(new Uint8Array(block));
      const progress = Math.floor(100 * (offset + block.byteLength) / file.size);
      if (progress > previousProgress) {
        self.postMessage({state: "progress", progress});
        previousProgress = progress;
      }
    }
    const sha256 = hash.digest();
    self.postMessage({
      state: sha256 === supportedDisc.sha256 ? "valid" : "hashMismatch",
      sha256,
    });
  } catch {
    self.postMessage({state: "error"});
  }
};
