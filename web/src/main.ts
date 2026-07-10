declare const createZx16Backend: any;
declare const zx16Programs: any;

const canvas = document.getElementById("graphicsCanvas") as HTMLCanvasElement;
const internalsButton = document.getElementById("internalsButton") as HTMLButtonElement;
const internalsPanel = document.getElementById("internalsPanel") as HTMLElement;
const registersText = document.getElementById("registersText") as HTMLElement;
const registerEditText = document.getElementById("registerEditText") as HTMLElement;
const ramText = document.getElementById("ramText") as HTMLElement;
const cursorText = document.getElementById("cursorText") as HTMLElement;
const memoryEditText = document.getElementById("memoryEditText") as HTMLElement;
const breakpointText = document.getElementById("breakpointText") as HTMLElement;
const consoleText = document.getElementById("consoleText") as HTMLElement;

const runButton = document.getElementById("runButton") as HTMLButtonElement;
const speedButtons = document.getElementById("speedButtons") as HTMLElement;
const slugButton = document.getElementById("slugButton") as HTMLButtonElement;
const wormButton = document.getElementById("wormButton") as HTMLButtonElement;
const pythonButton = document.getElementById("pythonButton") as HTMLButtonElement;
const pauseButton = document.getElementById("pauseButton") as HTMLButtonElement;
const stepButton = document.getElementById("stepButton") as HTMLButtonElement;
const runCursorButton = document.getElementById("runCursorButton") as HTMLButtonElement;
const resetButton = document.getElementById("resetButton") as HTMLButtonElement;
const clearBreakpointsButton = document.getElementById("clearBreakpointsButton") as HTMLButtonElement;
const internalRunButton = document.getElementById("internalRunButton") as HTMLButtonElement;
const internalResetButton = document.getElementById("internalResetButton") as HTMLButtonElement;
const scoreText = document.getElementById("scoreText") as HTMLElement;
const statusText = document.getElementById("statusText") as HTMLElement;

const context = canvas.getContext("2d") as CanvasRenderingContext2D;

const screenWidth = 320;
const screenHeight = 240;
const tileSize = 16;
const tileColumns = 20;
const tileRows = 15;
const tileMapBase = 0xF000;
const tileDefinitionBase = 0xF200;
const tileBytes = 128;
const paletteBase = 0xFA00;
const scoreAddress = 0x8008;
const zx16KeyNone = 0;
const zx16KeyUp = 1;
const zx16KeyDown = 2;
const zx16KeyLeft = 3;
const zx16KeyRight = 4;
const zx16KeySpace = 5;
const zx16KeyEnter = 6;
const zx16KeyEscape = 7;
const slugStepsPerFrame = 550;
const wormStepsPerFrame = 850;
const pythonStepsPerFrame = 1150;
const debugStepsPerFrame = 60;
const snakeInternalsRefreshFrames = 5;

let backend: any = null;
let read8: any = null;
let write8: any = null;
let read16: any = null;
let write16: any = null;
let resetCpu: any = null;
let seedRng: any = null;
let getPc: any = null;
let setPc: any = null;
let getSp: any = null;
let setSp: any = null;
let getRegister: any = null;
let setRegister: any = null;
let stepCpu: any = null;
let stepWithBreakpoints: any = null;
let isHalted: any = null;
let getOutput: any = null;
let setKeyboardKey: any = null;
let getKeyboardKey: any = null;
let clearKeyboardKey: any = null;
let toggleBreakpoint: any = null;
let hasBreakpoint: any = null;
let getBreakpointCount: any = null;
let clearBreakpoints: any = null;
let hasPendingTone: any = null;
let getToneFrequency: any = null;
let getToneDurationMs: any = null;
let clearToneRequest: any = null;
let hasPendingStopAudio: any = null;
let clearStopAudioRequest: any = null;
let getVolumePercent: any = null;
let audioContext: any = null;
let activeOscillator: any = null;
let activeGain: any = null;
let audioStatus = "Audio ready";
let running = false;
let runToCursorActive = false;
let runFrameScheduled = false;
let frameNumber = 0;
let selectedRegisterIndex = -1;
let registerEditValue = "";
let selectedMemoryAddress = -1;
let memoryEditActive = false;
let memoryEditValue = "";
let cursorAddress = -1;
let loadedProgramName = "Demo";
let screenImage: any = null;
let drawFrameCount = 0;
let snakeStepsPerFrame = wormStepsPerFrame;
let selectedSpeedName = "";

function toHex16(value: number): string {
    return (value & 0xffff).toString(16).toUpperCase().padStart(4, "0");
}

function toHex8(value: number): string {
    return (value & 0xff).toString(16).toUpperCase().padStart(2, "0");
}

function isHexKey(key: string): boolean {
    if (key.length !== 1) {
        return false;
    }

    const upper = key.toUpperCase();

    if (upper >= "0" && upper <= "9") {
        return true;
    }

    if (upper >= "A" && upper <= "F") {
        return true;
    }

    return false;
}

function parseHexValue(text: string): number {
    if (text.length === 0) {
        return -1;
    }

    return parseInt(text, 16);
}

function isInstructionByte(address: number, instructionAddress: number): boolean {
    const firstByte = instructionAddress & 0xFFFF;
    const secondByte = (instructionAddress + 1) & 0xFFFF;

    return address === firstByte || address === secondByte;
}

function makeRgb332(red: number, green: number, blue: number): number {
    return ((red & 7) << 5) | ((green & 7) << 2) | (blue & 3);
}

function rgb332ToCss(value: number): string {
    const red3 = (value >> 5) & 7;
    const green3 = (value >> 2) & 7;
    const blue2 = value & 3;

    const red8 = (red3 << 5) | (red3 << 2) | (red3 >> 1);
    const green8 = (green3 << 5) | (green3 << 2) | (green3 >> 1);
    const blue8 = (blue2 << 6) | (blue2 << 4) | (blue2 << 2) | blue2;

    return "rgb(" + red8 + "," + green8 + "," + blue8 + ")";
}

function rgb332ToRed(value: number): number {
    const red3 = (value >> 5) & 7;

    return (red3 << 5) | (red3 << 2) | (red3 >> 1);
}

function rgb332ToGreen(value: number): number {
    const green3 = (value >> 2) & 7;

    return (green3 << 5) | (green3 << 2) | (green3 >> 1);
}

function rgb332ToBlue(value: number): number {
    const blue2 = value & 3;

    return (blue2 << 6) | (blue2 << 4) | (blue2 << 2) | blue2;
}

function getScreenImage(): any {
    if (screenImage !== null) {
        return screenImage;
    }

    if ((context as any).createImageData) {
        screenImage = context.createImageData(screenWidth, screenHeight);
    }
    else {
        screenImage = {
            width: screenWidth,
            height: screenHeight,
            data: new Uint8ClampedArray(screenWidth * screenHeight * 4)
        };
    }

    return screenImage;
}

function writeImagePixel(data: any, pixelIndex: number, paletteIndex: number, red: any, green: any, blue: any): void {
    const dataIndex = pixelIndex * 4;

    data[dataIndex] = red[paletteIndex];
    data[dataIndex + 1] = green[paletteIndex];
    data[dataIndex + 2] = blue[paletteIndex];
    data[dataIndex + 3] = 255;
}

function writeTilePixel(tileIndex: number, x: number, y: number, paletteIndex: number): void {
    const byteAddress = tileDefinitionBase + tileIndex * tileBytes + Math.floor((y * tileSize + x) / 2);
    const oldValue = read8(byteAddress);
    let newValue = oldValue;

    if ((x & 1) === 0) {
        newValue = (oldValue & 0xF0) | (paletteIndex & 0x0F);
    }
    else {
        newValue = (oldValue & 0x0F) | ((paletteIndex & 0x0F) << 4);
    }

    write8(byteAddress, newValue);
}

function readTilePixel(tileIndex: number, x: number, y: number): number {
    const byteAddress = tileDefinitionBase + tileIndex * tileBytes + Math.floor((y * tileSize + x) / 2);
    const value = read8(byteAddress);

    if ((x & 1) === 0) {
        return value & 0x0F;
    }

    return (value >> 4) & 0x0F;
}

function fillTile(tileIndex: number, paletteIndex: number): void {
    for (let y = 0; y < tileSize; y++) {
        for (let x = 0; x < tileSize; x++) {
            writeTilePixel(tileIndex, x, y, paletteIndex);
        }
    }
}

function makeCheckerTile(tileIndex: number, firstColor: number, secondColor: number): void {
    for (let y = 0; y < tileSize; y++) {
        for (let x = 0; x < tileSize; x++) {
            const useFirst = (Math.floor(x / 4) + Math.floor(y / 4)) % 2 === 0;
            writeTilePixel(tileIndex, x, y, useFirst ? firstColor : secondColor);
        }
    }
}

function makeStripeTile(tileIndex: number, firstColor: number, secondColor: number): void {
    for (let y = 0; y < tileSize; y++) {
        for (let x = 0; x < tileSize; x++) {
            writeTilePixel(tileIndex, x, y, y < 8 ? firstColor : secondColor);
        }
    }
}

function loadProgramBytes(bytes: any): boolean {
    if (bytes.length === 0) {
        return false;
    }

    resetCpu();

    const loadAddress = bytes.length === 65536 ? 0 : 0x0020;

    for (let i = 0; i < bytes.length; i++) {
        if (!write8(loadAddress + i, bytes[i])) {
            return false;
        }
    }

    return true;
}

function makeSnakeSeed(): number {
    const clockSeed = Date.now() & 0xFFFF;
    const randomSeed = Math.floor(Math.random() * 0x10000) & 0xFFFF;
    let fastSeed = 0;

    if (typeof performance !== "undefined") {
        fastSeed = Math.floor(performance.now() * 1000) & 0xFFFF;
    }

    let seed = (clockSeed ^ randomSeed ^ fastSeed) & 0xFFFF;

    if (seed === 0) {
        seed = 1;
    }

    return seed;
}

function seedSnakeRng(): void {
    if (seedRng !== null) {
        seedRng(makeSnakeSeed());
    }
}

function loadStaticVramDemo(): void {
    resetCpu();
    loadedProgramName = "Demo";
    clearKeyboardKey();
    drawFrameCount = 0;

    write8(paletteBase + 0, makeRgb332(0, 0, 0));
    write8(paletteBase + 1, makeRgb332(0, 2, 0));
    write8(paletteBase + 2, makeRgb332(0, 6, 0));
    write8(paletteBase + 3, makeRgb332(7, 7, 1));
    write8(paletteBase + 4, makeRgb332(2, 2, 2));
    write8(paletteBase + 5, makeRgb332(7, 7, 7));

    fillTile(0, 0);
    fillTile(1, 1);
    makeCheckerTile(2, 1, 2);
    makeStripeTile(3, 3, 1);
    makeCheckerTile(4, 4, 0);

    for (let row = 0; row < tileRows; row++) {
        for (let col = 0; col < tileColumns; col++) {
            let tileIndex = 1;

            if ((row + col) % 7 === 0) {
                tileIndex = 3;
            }
            else if ((row + col) % 5 === 0) {
                tileIndex = 4;
            }
            else if ((row + col) % 2 === 0) {
                tileIndex = 2;
            }

            write8(tileMapBase + row * tileColumns + col, tileIndex);
        }
    }
}

function loadSnakeProgram(): void {
    if (typeof zx16Programs === "undefined" || !zx16Programs.snake) {
        audioStatus = "Snake program was not found in web/build/programs.js.";
        updateInternals();
        return;
    }

    stopCurrentTone();
    running = false;
    runToCursorActive = false;
    frameNumber = 0;
    drawFrameCount = 0;
    resetDebugSelections();

    if (!loadProgramBytes(zx16Programs.snake)) {
        audioStatus = "Could not load Snake program bytes.";
        drawPage();
        return;
    }

    loadedProgramName = "Snake";
    seedSnakeRng();
    clearKeyboardKey();
    audioStatus = "Snake loaded. Press Start, then use arrow keys.";
    drawPage();
}

function drawVramScreen(): void {
    const image = getScreenImage();
    const data = image.data;
    const red = new Array(16);
    const green = new Array(16);
    const blue = new Array(16);

    for (let i = 0; i < 16; i++) {
        const rgb332 = read8(paletteBase + i);

        red[i] = rgb332ToRed(rgb332);
        green[i] = rgb332ToGreen(rgb332);
        blue[i] = rgb332ToBlue(rgb332);
    }

    for (let row = 0; row < tileRows; row++) {
        for (let col = 0; col < tileColumns; col++) {
            const tileIndex = read8(tileMapBase + row * tileColumns + col);
            const tileAddress = tileDefinitionBase + tileIndex * tileBytes;

            for (let y = 0; y < tileSize; y++) {
                const screenY = row * tileSize + y;
                const screenX = col * tileSize;
                const rowPixelIndex = screenY * screenWidth + screenX;
                const tileRowAddress = tileAddress + y * 8;

                for (let pair = 0; pair < 8; pair++) {
                    const packedPixels = read8(tileRowAddress + pair);
                    const leftPaletteIndex = packedPixels & 0x0F;
                    const rightPaletteIndex = (packedPixels >> 4) & 0x0F;
                    const pixelIndex = rowPixelIndex + pair * 2;

                    writeImagePixel(data, pixelIndex, leftPaletteIndex, red, green, blue);
                    writeImagePixel(data, pixelIndex + 1, rightPaletteIndex, red, green, blue);
                }
            }
        }
    }

    if ((context as any).putImageData) {
        context.putImageData(image, 0, 0);
    }
}

function mapBrowserKeyToZx16(key: string): number {
    if (key === "ArrowUp" || key === "w" || key === "W") {
        return zx16KeyUp;
    }

    if (key === "ArrowDown" || key === "s" || key === "S") {
        return zx16KeyDown;
    }

    if (key === "ArrowLeft" || key === "a" || key === "A") {
        return zx16KeyLeft;
    }

    if (key === "ArrowRight" || key === "d" || key === "D") {
        return zx16KeyRight;
    }

    if (key === " ") {
        return zx16KeySpace;
    }

    if (key === "Enter") {
        return zx16KeyEnter;
    }

    if (key === "Escape") {
        return zx16KeyEscape;
    }

    return zx16KeyNone;
}

function getKeyName(keyCode: number): string {
    if (keyCode === zx16KeyUp) {
        return "UP";
    }

    if (keyCode === zx16KeyDown) {
        return "DOWN";
    }

    if (keyCode === zx16KeyLeft) {
        return "LEFT";
    }

    if (keyCode === zx16KeyRight) {
        return "RIGHT";
    }

    if (keyCode === zx16KeySpace) {
        return "SPACE";
    }

    if (keyCode === zx16KeyEnter) {
        return "ENTER";
    }

    if (keyCode === zx16KeyEscape) {
        return "ESCAPE";
    }

    return "NONE";
}

function getAudioContext(): any {
    const AudioContextClass = window.AudioContext || (window as any).webkitAudioContext;

    if (AudioContextClass === undefined) {
        audioStatus = "Web Audio is not supported in this browser.";
        return null;
    }

    if (audioContext === null) {
        audioContext = new AudioContextClass();
    }

    return audioContext;
}

function prepareAudioFromUserEvent(): void {
    const contextValue = getAudioContext();

    if (contextValue === null) {
        return;
    }

    if (contextValue.state === "suspended") {
        contextValue.resume();
    }
}

function stopCurrentTone(): void {
    if (activeOscillator !== null) {
        try {
            activeOscillator.stop();
        }
        catch {
        }

        activeOscillator.disconnect();
        activeOscillator = null;
    }

    if (activeGain !== null) {
        activeGain.disconnect();
        activeGain = null;
    }
}

function playToneFromEcall(frequency: number, durationMs: number): void {
    const contextValue = getAudioContext();

    if (contextValue === null) {
        return;
    }

    if (frequency <= 0 || durationMs <= 0) {
        return;
    }

    stopCurrentTone();

    const oscillator = contextValue.createOscillator();
    const gain = contextValue.createGain();
    const volume = getVolumePercent === null ? 50 : getVolumePercent();
    const safeVolume = Math.max(0, Math.min(volume, 100)) / 100;
    const startTime = contextValue.currentTime;
    const endTime = startTime + durationMs / 1000;

    oscillator.type = "square";
    oscillator.frequency.setValueAtTime(frequency, startTime);
    gain.gain.setValueAtTime(safeVolume * 0.25, startTime);

    oscillator.connect(gain);
    gain.connect(contextValue.destination);

    oscillator.start(startTime);
    oscillator.stop(endTime);

    activeOscillator = oscillator;
    activeGain = gain;
    audioStatus = "Playing " + frequency + " Hz for " + durationMs + " ms at " + volume + "% volume.";

    oscillator.onended = (): void => {
        if (activeOscillator === oscillator) {
            activeOscillator = null;
        }

        if (activeGain === gain) {
            gain.disconnect();
            activeGain = null;
        }
    };
}

function processAudioRequests(): void {
    if (hasPendingStopAudio !== null && hasPendingStopAudio()) {
        stopCurrentTone();
        clearStopAudioRequest();
        audioStatus = "Audio stopped by ECALL.";
    }

    if (hasPendingTone !== null && hasPendingTone()) {
        const frequency = getToneFrequency();
        const durationMs = getToneDurationMs();

        playToneFromEcall(frequency, durationMs);
        clearToneRequest();
    }
}

function clearRegisterSelection(): void {
    selectedRegisterIndex = -1;
    registerEditValue = "";
}

function clearMemorySelection(): void {
    selectedMemoryAddress = -1;
    memoryEditActive = false;
    memoryEditValue = "";
}

function resetDebugSelections(): void {
    clearRegisterSelection();
    clearMemorySelection();
    cursorAddress = -1;
    runToCursorActive = false;
}

function getDataValue(target: any, container: any, name: string): string {
    let current = target;

    while (current !== null && current !== container) {
        if (current.dataset && current.dataset[name] !== undefined) {
            return current.dataset[name];
        }

        current = current.parentElement;
    }

    return "";
}

function startRegisterEdit(registerIndex: number): void {
    if (running) {
        return;
    }

    if (registerIndex < 0 || registerIndex >= 8) {
        return;
    }

    selectedRegisterIndex = registerIndex;
    registerEditValue = "";
    clearMemorySelection();
    updateInternals();
}

function startMemoryEdit(address: number): void {
    if (running) {
        return;
    }

    selectedMemoryAddress = address & 0xFFFF;
    memoryEditActive = true;
    memoryEditValue = "";
    clearRegisterSelection();
    updateInternals();
}

function setRunCursor(address: number): void {
    cursorAddress = address & 0xFFFE;
    updateInternals();
}

function applyRegisterEdit(): void {
    if (selectedRegisterIndex < 0 || registerEditValue.length === 0) {
        clearRegisterSelection();
        updateInternals();
        return;
    }

    const value = parseHexValue(registerEditValue);

    if (value >= 0) {
        setRegister(selectedRegisterIndex, value);
    }

    clearRegisterSelection();
    updateInternals();
}

function applyMemoryEdit(): void {
    if (!memoryEditActive || selectedMemoryAddress < 0) {
        clearMemorySelection();
        updateInternals();
        return;
    }

    const value = parseHexValue(memoryEditValue);

    if (memoryEditValue.length === 2 && value >= 0) {
        write8(selectedMemoryAddress, value);
        memoryEditActive = false;
        memoryEditValue = "";
    }
    else if (memoryEditValue.length === 4 && value >= 0) {
        if ((selectedMemoryAddress & 1) === 0 && selectedMemoryAddress !== 0xFFFF) {
            write16(selectedMemoryAddress, value);
            memoryEditActive = false;
            memoryEditValue = "";
        }
    }

    updateInternals();
    drawVramScreen();
}

function handleDebuggerKeyInput(event: KeyboardEvent): boolean {
    const editingRegister = selectedRegisterIndex >= 0;
    const editingMemory = memoryEditActive;

    if (!editingRegister && !editingMemory) {
        return false;
    }

    if (event.key === "Escape") {
        clearRegisterSelection();
        clearMemorySelection();
        event.preventDefault();
        updateInternals();
        return true;
    }

    if (event.key === "Backspace") {
        if (editingRegister && registerEditValue.length > 0) {
            registerEditValue = registerEditValue.slice(0, registerEditValue.length - 1);
        }

        if (editingMemory && memoryEditValue.length > 0) {
            memoryEditValue = memoryEditValue.slice(0, memoryEditValue.length - 1);
        }

        event.preventDefault();
        updateInternals();
        return true;
    }

    if (event.key === "Enter") {
        if (editingRegister) {
            applyRegisterEdit();
        }
        else {
            applyMemoryEdit();
        }

        event.preventDefault();
        return true;
    }

    if (event.key.length === 1) {
        if (isHexKey(event.key)) {
            const value = event.key.toUpperCase();

            if (editingRegister && registerEditValue.length < 4) {
                registerEditValue += value;
            }

            if (editingMemory && memoryEditValue.length < 4) {
                memoryEditValue += value;
            }

            updateInternals();
        }

        event.preventDefault();
        return true;
    }

    event.preventDefault();
    return true;
}

function handleRegisterClick(event: any): void {
    const registerText = getDataValue(event.target, registersText, "register");

    if (registerText.length === 0) {
        return;
    }

    startRegisterEdit(parseInt(registerText, 10));
}

function handleMemoryClick(event: any): void {
    const addressText = getDataValue(event.target, ramText, "address");

    if (addressText.length === 0) {
        return;
    }

    setRunCursor(parseInt(addressText, 10));
}

function handleMemoryContextMenu(event: any): void {
    const addressText = getDataValue(event.target, ramText, "address");

    if (addressText.length === 0) {
        return;
    }

    event.preventDefault();

    const address = parseInt(addressText, 10);

    if (event.shiftKey) {
        toggleBreakpoint(address & 0xFFFE);
        updateInternals();
        return;
    }

    startMemoryEdit(address);
}

function handleKeyDown(event: KeyboardEvent): void {
    if (handleDebuggerKeyInput(event)) {
        return;
    }

    const keyCode = mapBrowserKeyToZx16(event.key);

    if (keyCode === zx16KeyNone || setKeyboardKey === null) {
        return;
    }

    event.preventDefault();
    prepareAudioFromUserEvent();
    setKeyboardKey(keyCode);
    processAudioRequests();
    updateInternals();
}

function handleKeyUp(event: KeyboardEvent): void {
    if (selectedRegisterIndex >= 0 || memoryEditActive) {
        return;
    }

    const keyCode = mapBrowserKeyToZx16(event.key);

    if (keyCode === zx16KeyNone || clearKeyboardKey === null) {
        return;
    }

    event.preventDefault();

    if (loadedProgramName === "Snake" && running) {
        return;
    }

    if (getKeyboardKey !== null && getKeyboardKey() !== keyCode) {
        return;
    }

    clearKeyboardKey();
    updateInternals();
}

function renderRegisters(): void {
    let html =
        "<div class=\"register-row\">PC&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;: 0x" + toHex16(getPc()) + "</div>" +
        "<div class=\"register-row\">SP/x2&nbsp;&nbsp;: 0x" + toHex16(getSp()) + "</div>";

    for (let i = 0; i < 8; i++) {
        let label = "x" + i + "&nbsp;&nbsp;&nbsp;&nbsp;: 0x";

        if (i === 2) {
            label = "x2/sp : 0x";
        }

        let rowClass = "register-row";

        if (selectedRegisterIndex === i) {
            rowClass += " selected";
        }

        html +=
            "<div class=\"" + rowClass + "\" data-register=\"" + i + "\">" +
            label + toHex16(getRegister(i)) +
            "</div>";
    }

    registersText.innerHTML = html;

    if (selectedRegisterIndex >= 0) {
        if (registerEditValue.length > 0) {
            registerEditText.textContent = "x" + selectedRegisterIndex + " = 0x" + registerEditValue;
        }
        else {
            registerEditText.textContent = "x" + selectedRegisterIndex + " = 0x____";
        }
    }
    else {
        registerEditText.textContent = "no register selected";
    }
}

function getMemoryBaseAddress(): number {
    const pc = getPc();

    if (pc >= 30) {
        return (pc - 30) & 0xFFFF;
    }

    return 0;
}

function getMemoryByteClass(address: number): string {
    const pc = getPc();
    const instructionAddress = address & 0xFFFE;
    let className = "memory-byte";

    if (hasBreakpoint !== null && hasBreakpoint(instructionAddress)) {
        className += " breakpoint";
    }
    else if (selectedMemoryAddress === address) {
        className += " selected";
    }
    else if (cursorAddress >= 0 && isInstructionByte(address, cursorAddress)) {
        className += " cursor";
    }
    else if (isInstructionByte(address, pc)) {
        className += " pc";
    }

    return className;
}

function renderMemory(): void {
    const baseAddress = getMemoryBaseAddress();
    let html = "";

    for (let row = 0; row < 10; row++) {
        const rowAddress = (baseAddress + row * 6) & 0xFFFF;

        html += "<div class=\"memory-row\">";
        html += "<span class=\"memory-address\">0x" + toHex16(rowAddress) + ":</span>";

        for (let col = 0; col < 6; col++) {
            const address = (rowAddress + col) & 0xFFFF;
            const byteClass = getMemoryByteClass(address);

            html +=
                "<span class=\"" + byteClass + "\" data-address=\"" + address + "\">" +
                toHex8(read8(address)) +
                "</span>";
        }

        html += "</div>";
    }

    ramText.innerHTML = html;

    const breakpointCount = getBreakpointCount === null ? 0 : getBreakpointCount();
    breakpointText.textContent = "BP:" + breakpointCount;

    if (cursorAddress >= 0) {
        cursorText.textContent = "Run cursor: 0x" + toHex16(cursorAddress);
    }
    else {
        cursorText.textContent = "Run cursor: ----";
    }

    if (selectedMemoryAddress >= 0) {
        if (memoryEditActive) {
            memoryEditText.textContent = "Edit 0x" + toHex16(selectedMemoryAddress) + " = " + memoryEditValue;
        }
        else {
            memoryEditText.textContent =
                "Selected: 0x" + toHex16(selectedMemoryAddress) + " = " + toHex8(read8(selectedMemoryAddress));
        }
    }
    else {
        memoryEditText.textContent = "no memory byte selected";
    }
}

function renderConsole(keyCode: number): void {
    const output = getOutput === null ? "" : getOutput();
    let text = "";

    if (output.length > 0) {
        text += output;

        if (!output.endsWith("\n")) {
            text += "\n";
        }
    }
    else if (loadedProgramName === "Snake") {
        text += "Snake loaded from asm/bin/snake.bin.\n";
    }
    else {
        text += "Static VRAM tile map rendered from WASM backend memory.\n";
    }

    text += "Keyboard ECALL key: " + getKeyName(keyCode) + "\n";
    text += audioStatus;

    consoleText.textContent = text;
}

function getSnakeScore(): number {
    if (read16 === null || loadedProgramName !== "Snake") {
        return 0;
    }

    return read16(scoreAddress);
}

function refreshControls(): void {
    const halted = isHalted !== null && isHalted();
    const programText = loadedProgramName + " ";

    runButton.disabled = running || halted;
    internalRunButton.disabled = running || halted;
    pauseButton.disabled = !running;
    stepButton.disabled = running || halted;
    runCursorButton.disabled = running || halted || cursorAddress < 0;
    slugButton.disabled = running || halted;
    wormButton.disabled = running || halted;
    pythonButton.disabled = running || halted;
    scoreText.textContent = "Score: " + getSnakeScore();

    if (halted && loadedProgramName === "Snake" && getOutput().indexOf("GAME OVER") >= 0) {
        statusText.textContent = "Snake GAME OVER";
    }
    else if (halted) {
        statusText.textContent = programText + "HALTED";
    }
    else if (running && runToCursorActive) {
        statusText.textContent = programText + "RUN TO CURSOR";
    }
    else if (running) {
        statusText.textContent = programText + selectedSpeedName + " RUNNING";
    }
    else {
        statusText.textContent = programText + "PAUSED";
    }
}

function updateInternals(): void {
    const keyCode = getKeyboardKey === null ? zx16KeyNone : getKeyboardKey();

    renderRegisters();
    renderMemory();
    renderConsole(keyCode);
    refreshControls();
}

function drawPage(): void {
    if (backend === null) {
        return;
    }

    drawVramScreen();
    drawFrameCount++;

    if (!running || loadedProgramName !== "Snake" || drawFrameCount % snakeInternalsRefreshFrames === 0) {
        updateInternals();
    }
}

function runCpuSlice(): void {
    if (isHalted === null || stepWithBreakpoints === null) {
        running = false;
        return;
    }

    const stepsThisFrame = loadedProgramName === "Snake" ? snakeStepsPerFrame : debugStepsPerFrame;

    for (let i = 0; i < stepsThisFrame; i++) {
        if (isHalted()) {
            running = false;
            runToCursorActive = false;
            return;
        }

        if (runToCursorActive && cursorAddress >= 0 && getPc() === cursorAddress) {
            running = false;
            runToCursorActive = false;
            return;
        }

        const executed = stepWithBreakpoints();

        processAudioRequests();

        if (!executed) {
            running = false;
            runToCursorActive = false;
            return;
        }

        if (runToCursorActive && cursorAddress >= 0 && getPc() === cursorAddress) {
            running = false;
            runToCursorActive = false;
            return;
        }
    }
}

function runLoop(): void {
    runFrameScheduled = false;

    if (!running) {
        updateInternals();
        return;
    }

    runCpuSlice();
    drawPage();

    if (running) {
        scheduleRunLoop();
    }
}

function scheduleRunLoop(): void {
    if (runFrameScheduled) {
        return;
    }

    runFrameScheduled = true;

    const requestFrame = (window as any).requestAnimationFrame;

    if (requestFrame) {
        requestFrame(runLoop);
    }
    else {
        setTimeout(runLoop, 16);
    }
}

function setRunning(value: boolean): void {
    if (backend === null) {
        return;
    }

    running = value;
    if (!running) {
        runToCursorActive = false;
    }

    processAudioRequests();
    drawPage();

    if (running) {
        scheduleRunLoop();
    }
}

function stepOnce(): void {
    if (backend === null || stepCpu === null || isHalted === null) {
        return;
    }

    if (running || isHalted()) {
        return;
    }

    stepCpu();
    processAudioRequests();
    drawPage();
}

function runToCursor(): void {
    if (backend === null || isHalted === null) {
        return;
    }

    if (cursorAddress < 0 || isHalted()) {
        return;
    }

    runToCursorActive = getPc() !== cursorAddress;
    setRunning(runToCursorActive);
}

async function start(): Promise<void> {
    backend = await createZx16Backend({
        locateFile: (fileName: string): string => "build/" + fileName
    });

    read8 = backend.cwrap("zx16_read8", "number", ["number"]);
    write8 = backend.cwrap("zx16_write8", "number", ["number", "number"]);
    read16 = backend.cwrap("zx16_read16", "number", ["number"]);
    write16 = backend.cwrap("zx16_write16", "number", ["number", "number"]);
    resetCpu = backend.cwrap("zx16_reset", null, []);
    seedRng = backend.cwrap("zx16_seed_rng", null, ["number"]);
    getPc = backend.cwrap("zx16_get_pc", "number", []);
    setPc = backend.cwrap("zx16_set_pc", null, ["number"]);
    getSp = backend.cwrap("zx16_get_sp", "number", []);
    setSp = backend.cwrap("zx16_set_sp", null, ["number"]);
    getRegister = backend.cwrap("zx16_get_register", "number", ["number"]);
    setRegister = backend.cwrap("zx16_set_register", null, ["number", "number"]);
    stepCpu = backend.cwrap("zx16_step", "number", []);
    stepWithBreakpoints = backend.cwrap("zx16_step_with_breakpoints", "number", []);
    isHalted = backend.cwrap("zx16_is_halted", "number", []);
    getOutput = backend.cwrap("zx16_get_output", "string", []);
    setKeyboardKey = backend.cwrap("zx16_set_keyboard_key", null, ["number"]);
    getKeyboardKey = backend.cwrap("zx16_get_keyboard_key", "number", []);
    clearKeyboardKey = backend.cwrap("zx16_clear_keyboard_key", null, []);
    toggleBreakpoint = backend.cwrap("zx16_toggle_breakpoint", "number", ["number"]);
    hasBreakpoint = backend.cwrap("zx16_has_breakpoint", "number", ["number"]);
    getBreakpointCount = backend.cwrap("zx16_get_breakpoint_count", "number", []);
    clearBreakpoints = backend.cwrap("zx16_clear_breakpoints", null, []);
    hasPendingTone = backend.cwrap("zx16_has_pending_tone", "number", []);
    getToneFrequency = backend.cwrap("zx16_get_tone_frequency", "number", []);
    getToneDurationMs = backend.cwrap("zx16_get_tone_duration_ms", "number", []);
    clearToneRequest = backend.cwrap("zx16_clear_tone_request", null, []);
    hasPendingStopAudio = backend.cwrap("zx16_has_pending_stop_audio", "number", []);
    clearStopAudioRequest = backend.cwrap("zx16_clear_stop_audio_request", null, []);
    getVolumePercent = backend.cwrap("zx16_get_volume_percent", "number", []);

    loadSnakeProgram();
}

window.addEventListener("keydown", handleKeyDown);
window.addEventListener("keyup", handleKeyUp);
registersText.addEventListener("click", handleRegisterClick);
ramText.addEventListener("click", handleMemoryClick);
ramText.addEventListener("contextmenu", handleMemoryContextMenu);

internalsButton.addEventListener("click", (): void => {
    internalsPanel.classList.toggle("hidden");

    if (internalsPanel.classList.contains("hidden")) {
        internalsButton.textContent = "View Internals";
    }
    else {
        internalsButton.textContent = "Hide Internals";
    }
});

function startRunningFromButton(): void {
    prepareAudioFromUserEvent();
    frameNumber++;
    speedButtons.classList.add("hidden");
    setRunning(true);
}

function showSpeedButtons(): void {
    speedButtons.classList.remove("hidden");
    statusText.textContent = "Choose speed";
}

function startSnakeWithSpeed(speedName: string, stepsPerFrame: number): void {
    snakeStepsPerFrame = stepsPerFrame;
    selectedSpeedName = speedName;
    startRunningFromButton();
}

function restartSnakeFromButton(): void {
    stopCurrentTone();
    audioStatus = "Audio ready";
    frameNumber = 0;
    running = false;
    runToCursorActive = false;
    selectedSpeedName = "";
    loadSnakeProgram();
    showSpeedButtons();
}

runButton.addEventListener("click", (): void => {
    showSpeedButtons();
});

internalRunButton.addEventListener("click", (): void => {
    startSnakeWithSpeed("Worm", wormStepsPerFrame);
});

slugButton.addEventListener("click", (): void => {
    startSnakeWithSpeed("Slug", slugStepsPerFrame);
});

wormButton.addEventListener("click", (): void => {
    startSnakeWithSpeed("Worm", wormStepsPerFrame);
});

pythonButton.addEventListener("click", (): void => {
    startSnakeWithSpeed("Python", pythonStepsPerFrame);
});

pauseButton.addEventListener("click", (): void => {
    setRunning(false);
});

stepButton.addEventListener("click", (): void => {
    prepareAudioFromUserEvent();
    frameNumber++;
    stepOnce();
});

runCursorButton.addEventListener("click", (): void => {
    prepareAudioFromUserEvent();
    runToCursor();
});

resetButton.addEventListener("click", (): void => {
    restartSnakeFromButton();
});

internalResetButton.addEventListener("click", (): void => {
    restartSnakeFromButton();
});

clearBreakpointsButton.addEventListener("click", (): void => {
    if (clearBreakpoints !== null) {
        clearBreakpoints();
    }

    updateInternals();
});

start().catch((error: Error): void => {
    consoleText.textContent = "Could not start web simulator: " + error.message;
});
