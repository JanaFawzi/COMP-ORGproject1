const elements = {};
const drawCalls = [];
const memory = new Uint8Array(65536);
const windowEvents = {};
let keyboardKey = 0;
let pc = 0x0020;
let halted = false;
const registers = new Uint16Array(8);
const breakpoints = new Set();
let pendingTone = false;
let pendingStopAudio = false;
let audioStarted = false;
let audioStopped = false;
let audioConnected = false;
let audioVolume = 50;
let rngSeed = 0xACE1;

function makeElement(id) {
    const element = {
        id,
        textContent: "",
        innerHTML: "",
        disabled: false,
        classList: {
            values: new Set(["hidden"]),
            add(name) {
                this.values.add(name);
            },
            remove(name) {
                this.values.delete(name);
            },
            toggle(name) {
                if (this.values.has(name)) {
                    this.values.delete(name);
                }
                else {
                    this.values.add(name);
                }
            },
            contains(name) {
                return this.values.has(name);
            }
        },
        addEventListener(name, callback) {
            this[name] = callback;
        }
    };

    elements[id] = element;
    return element;
}

global.document = {
    getElementById(id) {
        return elements[id] || makeElement(id);
    }
};

global.window = {
    addEventListener(name, callback) {
        windowEvents[name] = callback;
    },
    requestAnimationFrame(callback) {
        this.lastAnimationFrame = callback;
        return 1;
    }
};

class MockAudioContext {
    constructor() {
        this.currentTime = 1;
        this.destination = {};
        this.state = "suspended";
    }

    resume() {
        this.state = "running";
    }

    createOscillator() {
        return {
            type: "",
            frequency: {
                setValueAtTime(value) {
                    if (value !== 440) {
                        throw new Error("Audio frequency was not read from the backend.");
                    }
                }
            },
            connect() {
                audioConnected = true;
            },
            disconnect() {},
            start() {
                audioStarted = true;
            },
            stop() {
                audioStopped = true;
            },
            onended: null
        };
    }

    createGain() {
        return {
            gain: {
                setValueAtTime(value) {
                    if (value <= 0) {
                        throw new Error("Audio gain was not set.");
                    }
                }
            },
            connect() {},
            disconnect() {}
        };
    }
}

global.window.AudioContext = MockAudioContext;
global.AudioContext = MockAudioContext;

const canvas = makeElement("graphicsCanvas");
canvas.width = 320;
canvas.height = 240;
canvas.getContext = function () {
    return {
        fillStyle: "",
        strokeStyle: "",
        lineWidth: 1,
        fillRect(x, y, w, h) {
            drawCalls.push(["fillRect", x, y, w, h]);
        },
        createImageData(w, h) {
            return {
                width: w,
                height: h,
                data: new Uint8ClampedArray(w * h * 4)
            };
        },
        putImageData(image, x, y) {
            drawCalls.push(["putImageData", x, y, image.data.length]);
        },
        beginPath() {
            drawCalls.push(["beginPath"]);
        },
        moveTo(x, y) {
            drawCalls.push(["moveTo", x, y]);
        },
        lineTo(x, y) {
            drawCalls.push(["lineTo", x, y]);
        },
        stroke() {
            drawCalls.push(["stroke"]);
        }
    };
};

makeElement("internalsButton");
makeElement("internalsPanel");
makeElement("registersText");
makeElement("registerEditText");
makeElement("ramText");
makeElement("cursorText");
makeElement("memoryEditText");
makeElement("breakpointText");
makeElement("consoleText");
makeElement("runButton");
makeElement("pauseButton");
makeElement("stepButton");
makeElement("runCursorButton");
makeElement("resetButton");
makeElement("clearBreakpointsButton");
makeElement("internalRunButton");
makeElement("internalResetButton");
makeElement("scoreText");
makeElement("statusText");

global.zx16Programs = {
    snake: [0xAA, 0xBB, 0xCC, 0xDD]
};

global.setInterval = function () {};

global.createZx16Backend = async function () {
    return {
        cwrap(name) {
            if (name === "zx16_reset") {
                return function () {
                    memory.fill(0);
                    registers.fill(0);
                    registers[2] = 0xEFFE;
                    pc = 0x0020;
                    halted = false;
                    breakpoints.clear();
                    rngSeed = 0xACE1;
                };
            }

            if (name === "zx16_seed_rng") {
                return function (seed) {
                    rngSeed = seed & 0xFFFF;
                };
            }

            if (name === "zx16_read8") {
                return function (address) {
                    return memory[address & 0xFFFF];
                };
            }

            if (name === "zx16_write8") {
                return function (address, value) {
                    memory[address & 0xFFFF] = value & 0xFF;
                    return 1;
                };
            }

            if (name === "zx16_read16") {
                return function (address) {
                    address = address & 0xFFFF;
                    return memory[address] | (memory[(address + 1) & 0xFFFF] << 8);
                };
            }

            if (name === "zx16_write16") {
                return function (address, value) {
                    address = address & 0xFFFF;
                    memory[address] = value & 0xFF;
                    memory[(address + 1) & 0xFFFF] = (value >> 8) & 0xFF;
                    return 1;
                };
            }

            if (name === "zx16_get_pc") {
                return function () {
                    return pc;
                };
            }

            if (name === "zx16_set_pc") {
                return function (value) {
                    pc = value & 0xFFFF;
                };
            }

            if (name === "zx16_get_sp") {
                return function () {
                    return registers[2];
                };
            }

            if (name === "zx16_set_sp") {
                return function (value) {
                    registers[2] = value & 0xFFFF;
                };
            }

            if (name === "zx16_get_register") {
                return function (index) {
                    return registers[index] || 0;
                };
            }

            if (name === "zx16_set_register") {
                return function (index, value) {
                    registers[index] = value & 0xFFFF;
                };
            }

            if (name === "zx16_step") {
                return function () {
                    pc = (pc + 2) & 0xFFFF;
                    return 0;
                };
            }

            if (name === "zx16_step_with_breakpoints") {
                return function () {
                    if (breakpoints.has(pc & 0xFFFE)) {
                        return 0;
                    }

                    pc = (pc + 2) & 0xFFFF;
                    return 1;
                };
            }

            if (name === "zx16_is_halted") {
                return function () {
                    return halted ? 1 : 0;
                };
            }

            if (name === "zx16_get_output") {
                return function () {
                    return "";
                };
            }

            if (name === "zx16_set_keyboard_key") {
                return function (value) {
                    keyboardKey = value;
                };
            }

            if (name === "zx16_get_keyboard_key") {
                return function () {
                    return keyboardKey;
                };
            }

            if (name === "zx16_clear_keyboard_key") {
                return function () {
                    keyboardKey = 0;
                };
            }

            if (name === "zx16_toggle_breakpoint") {
                return function (address) {
                    address = address & 0xFFFE;

                    if (breakpoints.has(address)) {
                        breakpoints.delete(address);
                    }
                    else {
                        breakpoints.add(address);
                    }

                    return 1;
                };
            }

            if (name === "zx16_has_breakpoint") {
                return function (address) {
                    return breakpoints.has(address & 0xFFFE) ? 1 : 0;
                };
            }

            if (name === "zx16_get_breakpoint_count") {
                return function () {
                    return breakpoints.size;
                };
            }

            if (name === "zx16_clear_breakpoints") {
                return function () {
                    breakpoints.clear();
                };
            }

            if (name === "zx16_has_pending_tone") {
                return function () {
                    return pendingTone ? 1 : 0;
                };
            }

            if (name === "zx16_get_tone_frequency") {
                return function () {
                    return 440;
                };
            }

            if (name === "zx16_get_tone_duration_ms") {
                return function () {
                    return 120;
                };
            }

            if (name === "zx16_clear_tone_request") {
                return function () {
                    pendingTone = false;
                };
            }

            if (name === "zx16_has_pending_stop_audio") {
                return function () {
                    return pendingStopAudio ? 1 : 0;
                };
            }

            if (name === "zx16_clear_stop_audio_request") {
                return function () {
                    pendingStopAudio = false;
                };
            }

            if (name === "zx16_get_volume_percent") {
                return function () {
                    return audioVolume;
                };
            }

            throw new Error("Unexpected cwrap function: " + name);
        }
    };
};

require("./build/main.js");

setTimeout(function () {
    if (drawCalls.length === 0) {
        throw new Error("Canvas was not drawn.");
    }

    if (memory[0x0020] !== 0xAA || memory[0x0023] !== 0xDD) {
        throw new Error("Snake program was not loaded automatically into backend memory.");
    }

    if (!elements.consoleText.textContent.includes("Snake")) {
        throw new Error("Snake status was not shown in the console.");
    }

    if (!elements.registersText.innerHTML.includes("data-register=\"3\"")) {
        throw new Error("Register debugger rows were not rendered.");
    }

    elements.registersText.click({
        target: {
            dataset: { register: "3" },
            parentElement: null
        }
    });

    windowEvents.keydown({ key: "A", preventDefault() {} });
    windowEvents.keydown({ key: "B", preventDefault() {} });
    windowEvents.keydown({ key: "Enter", preventDefault() {} });

    if (registers[3] !== 0x00AB) {
        throw new Error("Register edit did not update the backend register.");
    }

    elements.ramText.contextmenu({
        target: {
            dataset: { address: String(0xF000) },
            parentElement: null
        },
        shiftKey: false,
        preventDefault() {}
    });

    windowEvents.keydown({ key: "5", preventDefault() {} });
    windowEvents.keydown({ key: "A", preventDefault() {} });
    windowEvents.keydown({ key: "Enter", preventDefault() {} });

    if (memory[0xF000] !== 0x5A) {
        throw new Error("Memory byte edit did not update backend RAM.");
    }

    elements.ramText.click({
        target: {
            dataset: { address: String(0x0020) },
            parentElement: null
        }
    });

    if (!elements.cursorText.textContent.includes("0x0020")) {
        throw new Error("Memory left-click did not set the pink run cursor.");
    }

    elements.ramText.contextmenu({
        target: {
            dataset: { address: String(0x0020) },
            parentElement: null
        },
        shiftKey: true,
        preventDefault() {}
    });

    elements.ramText.contextmenu({
        target: {
            dataset: { address: String(0x0024) },
            parentElement: null
        },
        shiftKey: true,
        preventDefault() {}
    });

    if (breakpoints.size !== 2 || !elements.breakpointText.textContent.includes("BP:2")) {
        throw new Error("Multiple breakpoints were not preserved.");
    }

    if (!elements.ramText.innerHTML.includes("breakpoint")) {
        throw new Error("Breakpoint color class was not rendered in the memory viewer.");
    }

    const drawCallsBeforeKey = drawCalls.length;

    windowEvents.keydown({
        key: "ArrowRight",
        preventDefault() {}
    });

    if (keyboardKey !== 4) {
        throw new Error("ArrowRight did not reach the simulator keyboard interface.");
    }

    if (!elements.consoleText.textContent.includes("RIGHT")) {
        throw new Error("Keyboard state was not shown in the page internals.");
    }

    if (drawCalls.length !== drawCallsBeforeKey) {
        throw new Error("Keyboard update redrew the full canvas instead of only updating internals.");
    }

    pendingTone = true;

    windowEvents.keydown({
        key: "ArrowUp",
        preventDefault() {}
    });

    if (pendingTone) {
        throw new Error("Tone request was not cleared after Web Audio playback.");
    }

    if (!audioStarted || !audioStopped || !audioConnected) {
        throw new Error("Web Audio did not play from a browser event.");
    }

    if (!elements.consoleText.textContent.includes("Playing 440 Hz")) {
        throw new Error("Audio status was not shown in the console.");
    }

    if (!elements.scoreText.textContent.includes("Score:")) {
        throw new Error("Score was not displayed on the main screen.");
    }

    pendingStopAudio = true;

    elements.runButton.click();

    if (pendingStopAudio) {
        throw new Error("Stop-audio request was not cleared.");
    }

    windowEvents.keyup({
        key: "ArrowUp",
        preventDefault() {}
    });

    if (keyboardKey !== 1) {
        throw new Error("Snake key release should keep the last direction while the game is running.");
    }

    elements.pauseButton.click();

    windowEvents.keyup({
        key: "ArrowUp",
        preventDefault() {}
    });

    if (keyboardKey !== 0) {
        throw new Error("Key release did not clear the simulator keyboard interface after pausing.");
    }

    elements.internalRunButton.click();
    elements.internalResetButton.click();
    elements.stepButton.click();
    elements.runCursorButton.click();
    elements.clearBreakpointsButton.click();

    console.log("ZX16 web page load test passed");
}, 0);
