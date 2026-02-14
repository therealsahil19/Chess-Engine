import '@testing-library/jest-dom';

// Mock Worker for stockfish.js
class Worker {
    constructor(stringUrl) {
        this.url = stringUrl;
        this.onmessage = () => { };
    }

    postMessage(msg) {
        // console.log('Worker received:', msg);
    }

    terminate() { }
}

window.Worker = Worker;
