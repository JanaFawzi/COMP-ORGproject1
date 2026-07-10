const fs = require("fs");
const path = require("path");

const inputPath = path.join(__dirname, "src", "main.ts");
const outputDir = path.join(__dirname, "build");
const outputPath = path.join(outputDir, "main.js");
const snakeBinPath = path.join(__dirname, "..", "asm", "bin", "snake.bin");
const snakeListPath = path.join(__dirname, "..", "asm", "bin", "snake.lst");
const programsPath = path.join(outputDir, "programs.js");

let code = fs.readFileSync(inputPath, "utf8");

code = code.replace(/ as [A-Za-z0-9_]+/g, "");
code = code.replace(/^declare .*;\r?\n/gm, "");
code = code.replace(/: any/g, "");
code = code.replace(/: string/g, "");
code = code.replace(/: number/g, "");
code = code.replace(/: boolean/g, "");
code = code.replace(/: KeyboardEvent/g, "");
code = code.replace(/: Promise<void>/g, "");
code = code.replace(/: void/g, "");
code = code.replace(/\(error: Error\)/g, "(error)");

fs.mkdirSync(outputDir, { recursive: true });
fs.writeFileSync(outputPath, code);

console.log("Built web\\build\\main.js");

if (fs.existsSync(snakeBinPath)) {
    const snakeBytes = Array.from(fs.readFileSync(snakeBinPath));
    let snakeListing = "";

    if (fs.existsSync(snakeListPath)) {
        snakeListing = fs.readFileSync(snakeListPath, "utf8").replace(/\r\n/g, "\n");
    }

    const programsCode =
        "window.zx16Programs = window.zx16Programs || {};\n" +
        "window.zx16Programs.snake = [" + snakeBytes.join(",") + "];\n" +
        "window.zx16Programs.snakeListing = " + JSON.stringify(snakeListing) + ";\n";

    fs.writeFileSync(programsPath, programsCode);
    console.log("Built web\\build\\programs.js");
}
else {
    fs.writeFileSync(programsPath, "window.zx16Programs = window.zx16Programs || {};\n");
    console.log("Built empty web\\build\\programs.js");
}
