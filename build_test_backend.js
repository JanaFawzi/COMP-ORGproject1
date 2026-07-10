const fs = require("fs");
const path = require("path");

const root = path.join(__dirname, "..");
const inputPath = path.join(__dirname, "src", "test_backend.ts");
const outputDir = path.join(root, "web-build", "ts");
const outputPath = path.join(outputDir, "test_backend.js");

let code = fs.readFileSync(inputPath, "utf8");

code = code.replace(/^declare .*;\r?\n/gm, "");
code = code.replace(/type\s+\w+\s*=\s*\([^;]*;\r?\n/g, "");
code = code.replace(/type\s+\w+\s*=\s*\{[\s\S]*?\};\r?\n/g, "");
code = code.replace(/: CreateZx16Backend/g, "");
code = code.replace(/: string/g, "");
code = code.replace(/: number/g, "");
code = code.replace(/: Promise<void>/g, "");
code = code.replace(/: void/g, "");
code = code.replace(/\(error: Error\)/g, "(error)");

fs.mkdirSync(outputDir, { recursive: true });
fs.writeFileSync(outputPath, code);

console.log("Built " + path.relative(root, outputPath));
