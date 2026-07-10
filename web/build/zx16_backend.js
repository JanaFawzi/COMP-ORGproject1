// This code implements the `-sMODULARIZE` settings by taking the generated
// JS program code (INNER_JS_CODE) and wrapping it in a factory function.

// Single threaded MINIMAL_RUNTIME programs do not need access to
// document.currentScript, so a simple export declaration is enough.
var createZx16Backend = (() => {
  // When MODULARIZE this JS may be executed later,
  // after document.currentScript is gone, so we save it.
  // In EXPORT_ES6 mode we can just use 'import.meta.url'.
  var _scriptName = globalThis.document?.currentScript?.src;
  return async function(moduleArg = {}) {
    var Module = moduleArg;
// include: shell.js
// include: minimum_runtime_check.js
(function() {
  // "30.0.0" -> 300000
  function humanReadableVersionToPacked(str) {
    str = str.split('-')[0]; // Remove any trailing part from e.g. "12.53.3-alpha"
    var vers = str.split('.').slice(0, 3);
    while(vers.length < 3) vers.push('00');
    vers = vers.map((n, i, arr) => n.padStart(2, '0'));
    return vers.join('');
  }
  // 300000 -> "30.0.0"
  var packedVersionToHumanReadable = n => [n / 10000 | 0, (n / 100 | 0) % 100, n % 100].join('.');

  var TARGET_NOT_SUPPORTED = 2147483647;

  // Note: We use a typeof check here instead of optional chaining using
  // globalThis because older browsers might not have globalThis defined.
  var currentNodeVersion = typeof process !== 'undefined' && process.versions?.node ? humanReadableVersionToPacked(process.versions.node) : TARGET_NOT_SUPPORTED;
  if (currentNodeVersion < 180300) {
    throw new Error(`This emscripten-generated code requires node v${ packedVersionToHumanReadable(180300) } (detected v${packedVersionToHumanReadable(currentNodeVersion)})`);
  }

  var userAgent = typeof navigator !== 'undefined' && navigator.userAgent;
  if (!userAgent) {
    return;
  }

  var currentSafariVersion = userAgent.includes("Safari/") && !userAgent.includes("Chrome/") && userAgent.match(/Version\/(\d+\.?\d*\.?\d*)/) ? humanReadableVersionToPacked(userAgent.match(/Version\/(\d+\.?\d*\.?\d*)/)[1]) : TARGET_NOT_SUPPORTED;
  if (currentSafariVersion < 150000) {
    throw new Error(`This emscripten-generated code requires Safari v${ packedVersionToHumanReadable(150000) } (detected v${currentSafariVersion})`);
  }

  var currentFirefoxVersion = userAgent.match(/Firefox\/(\d+(?:\.\d+)?)/) ? parseFloat(userAgent.match(/Firefox\/(\d+(?:\.\d+)?)/)[1]) : TARGET_NOT_SUPPORTED;
  if (currentFirefoxVersion < 79) {
    throw new Error(`This emscripten-generated code requires Firefox v79 (detected v${currentFirefoxVersion})`);
  }

  var currentChromeVersion = userAgent.match(/Chrome\/(\d+(?:\.\d+)?)/) ? parseFloat(userAgent.match(/Chrome\/(\d+(?:\.\d+)?)/)[1]) : TARGET_NOT_SUPPORTED;
  if (currentChromeVersion < 85) {
    throw new Error(`This emscripten-generated code requires Chrome v85 (detected v${currentChromeVersion})`);
  }
})();

// end include: minimum_runtime_check.js
// The Module object: Our interface to the outside world. We import
// and export values on it. There are various ways Module can be used:
// 1. Not defined. We create it here
// 2. A function parameter, function(moduleArg) => Promise<Module>
// 3. pre-run appended it, var Module = {}; ..generated code..
// 4. External script tag defines var Module.
// We need to check if Module already exists (e.g. case 3 above).
// Substitution will be replaced with actual code on later stage of the build,
// this way Closure Compiler will not mangle it (e.g. case 4. above).
// Note that if you want to run closure, and also to use Module
// after the generated code, you will need to define   var Module = {};
// before the code. Then that object will be used in the code, and you
// can continue to use Module afterwards as well.

// Determine the runtime environment we are in. You can customize this by
// setting the ENVIRONMENT setting at compile time (see settings.js).

// Attempt to auto-detect the environment
var ENVIRONMENT_IS_WEB = !!globalThis.window;
var ENVIRONMENT_IS_WORKER = !!globalThis.WorkerGlobalScope;
// N.b. Electron.js environment is simultaneously a NODE-environment, but
// also a web environment.
var ENVIRONMENT_IS_NODE = globalThis.process?.versions?.node && globalThis.process?.type != 'renderer';
var ENVIRONMENT_IS_SHELL = !ENVIRONMENT_IS_WEB && !ENVIRONMENT_IS_NODE && !ENVIRONMENT_IS_WORKER;

// --pre-jses are emitted after the Module integration code, so that they can
// refer to Module (if they choose; they can also define Module)


var programArgs = [];
var thisProgram = './this.program';
var quit_ = (status, toThrow) => {
  throw toThrow;
};

if (typeof __filename != 'undefined') { // Node
  _scriptName = __filename;
} else
  /*no-op*/{}

// `/` should be present at the end if `scriptDirectory` is not empty
var scriptDirectory = '';
function locateFile(path) {
  if (Module['locateFile']) {
    return Module['locateFile'](path, scriptDirectory);
  }
  return scriptDirectory + path;
}

// Hooks that are implemented differently in different runtime environments.
var readAsync, readBinary;

if (ENVIRONMENT_IS_NODE) {
  const isNode = globalThis.process?.versions?.node && globalThis.process?.type != 'renderer';
  if (!isNode) throw new Error('not compiled for this environment (did you build to HTML and try to run it not on the web, or set ENVIRONMENT to something - like node - and run it someplace else - like on the web?)');

  // These modules will usually be used on Node.js. Load them eagerly to avoid
  // the complexity of lazy-loading.
  var fs = require('node:fs');

  scriptDirectory = __dirname + '/';

// include: node_shell_read.js
readBinary = (filename) => {
  // We need to re-wrap `file://` strings to URLs.
  filename = isFileURI(filename) ? new URL(filename) : filename;
  var ret = fs.readFileSync(filename);
  assert(Buffer.isBuffer(ret));
  return ret;
};

readAsync = async (filename, binary = true) => {
  // See the comment in the `readBinary` function.
  filename = isFileURI(filename) ? new URL(filename) : filename;
  var ret = fs.readFileSync(filename, binary ? undefined : 'utf8');
  assert(binary ? Buffer.isBuffer(ret) : typeof ret == 'string');
  return ret;
};
// end include: node_shell_read.js
  if (process.argv.length > 1) {
    thisProgram = process.argv[1].replace(/\\/g, '/');
  }

  programArgs = process.argv.slice(2);

  quit_ = (status, toThrow) => {
    process.exitCode = status;
    throw toThrow;
  };

} else
if (ENVIRONMENT_IS_SHELL) {

} else

// Note that this includes Node.js workers when relevant (pthreads is enabled).
// Node.js workers are detected as a combination of ENVIRONMENT_IS_WORKER and
// ENVIRONMENT_IS_NODE.
if (ENVIRONMENT_IS_WEB || ENVIRONMENT_IS_WORKER) {
  try {
    scriptDirectory = new URL('.', _scriptName).href; // includes trailing slash
  } catch {
    // Must be a `blob:` or `data:` URL (e.g. `blob:http://site.com/etc/etc`), we cannot
    // infer anything from them.
  }

  if (!(globalThis.window || globalThis.WorkerGlobalScope)) throw new Error('not compiled for this environment (did you build to HTML and try to run it not on the web, or set ENVIRONMENT to something - like node - and run it someplace else - like on the web?)');

  {
// include: web_or_worker_shell_read.js
readAsync = async (url) => {
    assert(!isFileURI(url), "readAsync does not work with file:// URLs");
    var response = await fetch(url, { credentials: 'same-origin' });
    if (response.ok) {
      return response.arrayBuffer();
    }
    throw new Error(response.status + ' : ' + response.url);
  };
// end include: web_or_worker_shell_read.js
  }
} else
{
  throw new Error('environment detection error');
}

var out = console.log.bind(console);
var err = console.error.bind(console);

var IDBFS = 'IDBFS is no longer included by default; build with -lidbfs.js';
var PROXYFS = 'PROXYFS is no longer included by default; build with -lproxyfs.js';
var WORKERFS = 'WORKERFS is no longer included by default; build with -lworkerfs.js';
var FETCHFS = 'FETCHFS is no longer included by default; build with -lfetchfs.js';
var ICASEFS = 'ICASEFS is no longer included by default; build with -licasefs.js';
var JSFILEFS = 'JSFILEFS is no longer included by default; build with -ljsfilefs.js';
var OPFS = 'OPFS is no longer included by default; build with -lopfs.js';

var NODEFS = 'NODEFS is no longer included by default; build with -lnodefs.js';

// perform assertions in shell.js after we set up out() and err(), as otherwise
// if an assertion fails it cannot print the message

assert(!ENVIRONMENT_IS_WORKER, 'worker environment detected but not enabled at build time (add `worker` to `-sENVIRONMENT` to enable)');

assert(!ENVIRONMENT_IS_SHELL, 'shell environment detected but not enabled at build time (add `shell` to `-sENVIRONMENT` to enable)');

// end include: shell.js

// include: preamble.js
// === Preamble library stuff ===

// Documentation for the public APIs defined in this file must be updated in:
//    site/source/docs/api_reference/preamble.js.rst
// A prebuilt local version of the documentation is available at:
//    site/build/text/docs/api_reference/preamble.js.txt
// You can also build docs locally as HTML or other formats in site/
// An online HTML version (which may be of a different version of Emscripten)
//    is up at http://kripken.github.io/emscripten-site/docs/api_reference/preamble.js.html

var wasmBinary;

if (!globalThis.WebAssembly) {
  err('no native wasm support detected');
}

// Wasm globals

//========================================
// Runtime essentials
//========================================

// whether we are quitting the application. no code should run after this.
// set in exit() and abort()
var ABORT = false;

// set by exit() and abort().  Passed to 'onExit' handler.
// NOTE: This is also used as the process return code in shell environments
// but only when noExitRuntime is false.
var EXITSTATUS;

// In STRICT mode, we only define assert() when ASSERTIONS is set.  i.e. we
// don't define it at all in release modes.  This matches the behaviour of
// MINIMAL_RUNTIME.
// TODO(sbc): Make this the default even without STRICT enabled.
/** @type {function(*, string=)} */
function assert(condition, text) {
  if (!condition) {
    abort('Assertion failed' + (text ? ': ' + text : ''));
  }
}

// We used to include malloc/free by default in the past. Show a helpful error in
// builds with assertions.
function _malloc() {
  abort('malloc() called but not included in the build - add `_malloc` to EXPORTED_FUNCTIONS');
}
function _free() {
  // Show a helpful error since we used to include free by default in the past.
  abort('free() called but not included in the build - add `_free` to EXPORTED_FUNCTIONS');
}

/**
 * Indicates whether filename is delivered via file protocol (as opposed to http/https)
 * @noinline
 */
var isFileURI = (filename) => filename.startsWith('file://');

// include: runtime_common.js
// include: runtime_stack_check.js
// Initializes the stack cookie. Called at the startup of main and at the startup of each thread in pthreads mode.
function writeStackCookie() {
  var max = _emscripten_stack_get_end();
  assert((max & 3) == 0);
  // If the stack ends at address zero we write our cookies 4 bytes into the
  // stack.  This prevents interference with SAFE_HEAP and ASAN which also
  // monitor writes to address zero.
  if (max == 0) {
    max += 4;
  }
  // The stack grow downwards towards _emscripten_stack_get_end.
  // We write cookies to the final two words in the stack and detect if they are
  // ever overwritten.
  HEAPU32[((max)>>2)] = 0x02135467;
  HEAPU32[(((max)+(4))>>2)] = 0x89BACDFE;
  // Also test the global address 0 for integrity.
  HEAPU32[((0)>>2)] = 1668509029;
}

function checkStackCookie() {
  if (ABORT) return;
  var max = _emscripten_stack_get_end();
  // See writeStackCookie().
  if (max == 0) {
    max += 4;
  }
  var cookie1 = HEAPU32[((max)>>2)];
  var cookie2 = HEAPU32[(((max)+(4))>>2)];
  if (cookie1 != 0x02135467 || cookie2 != 0x89BACDFE) {
    abort(`Stack overflow! Stack cookie has been overwritten at ${ptrToString(max)}, expected hex dwords 0x89BACDFE and 0x2135467, but received ${ptrToString(cookie2)} ${ptrToString(cookie1)}`);
  }
  // Also test the global address 0 for integrity.
  if (HEAPU32[((0)>>2)] != 0x63736d65 /* 'emsc' */) {
    abort('Runtime error: The application has corrupted its heap memory area (address zero)!');
  }
}
// end include: runtime_stack_check.js
// include: runtime_exceptions.js
// Base Emscripten EH error class
class EmscriptenEH {}

class EmscriptenSjLj extends EmscriptenEH {}

// end include: runtime_exceptions.js
// include: runtime_debug.js
var runtimeDebug = true; // Switch to false at runtime to disable logging at the right times

// Used by XXXXX_DEBUG settings to output debug messages.
function dbg(...args) {
  if (!runtimeDebug && typeof runtimeDebug != 'undefined') return;
  // TODO(sbc): Make this configurable somehow.  Its not always convenient for
  // logging to show up as warnings.
  console.warn(...args);
}

// Endianness check
(() => {
  var h16 = new Int16Array(1);
  var h8 = new Int8Array(h16.buffer);
  h16[0] = 0x6373;
  if (h8[0] !== 0x73 || h8[1] !== 0x63) abort('Runtime error: expected the system to be little-endian! (Run with -sSUPPORT_BIG_ENDIAN to bypass)');
})();

function consumedModuleProp(prop) {
  var value = Module[prop];
  var msg = `Attempt to modify \`Module.${prop}\` after it has already been processed.  This can happen, for example, when code is injected via '--post-js' rather than '--pre-js'`;
  if (Array.isArray(value)) {
    value = new Proxy(value, {
      set(target, key, val) {
        abort(msg);
        return false;
      },
      defineProperty(target, key, descriptor) {
        abort(msg);
        return false;
      },
      deleteProperty(target, key) {
        abort(msg);
        return false;
      }
    });
  }
  Object.defineProperty(Module, prop, {
    configurable: true,
    get() { return value; },
    set() {
      abort(msg);
    }
  });
}

function makeInvalidEarlyAccess(name) {
  return () => assert(false, `call to '${name}' via reference taken before Wasm module initialization`);

}

function ignoredModuleProp(prop) {
  if (Object.getOwnPropertyDescriptor(Module, prop)) {
    abort(`\`Module.${prop}\` was supplied but \`${prop}\` not included in INCOMING_MODULE_JS_API`);
  }
}

// forcing the filesystem exports a few things by default
function isExportedByForceFilesystem(name) {
  return name === 'FS_createPath' ||
         name === 'FS_createDataFile' ||
         name === 'FS_createPreloadedFile' ||
         name === 'FS_preloadFile' ||
         name === 'FS_unlink' ||
         name === 'addRunDependency' ||
         // The old FS has some functionality that WasmFS lacks.
         name === 'FS_createLazyFile' ||
         name === 'FS_createDevice' ||
         name === 'removeRunDependency';
}

function missingLibrarySymbol(sym) {

  // Any symbol that is not included from the JS library is also (by definition)
  // not exported on the Module object.
  unexportedRuntimeSymbol(sym);
}

function unexportedRuntimeSymbol(sym) {
  if (!Object.getOwnPropertyDescriptor(Module, sym)) {
    Object.defineProperty(Module, sym, {
      configurable: true,
      get() {
        var msg = `'${sym}' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the Emscripten FAQ)`;
        if (isExportedByForceFilesystem(sym)) {
          msg += '. Alternatively, forcing filesystem support (-sFORCE_FILESYSTEM) can export this for you';
        }
        abort(msg);
      },
    });
  }
}

// end include: runtime_debug.js
// include: binaryDecode.js
// Prevent Closure from minifying the binaryDecode() function, or otherwise
// Closure may analyze through the WASM_BINARY_DATA placeholder string into this
// function, leading into incorrect results.
/** @noinline */
function binaryDecode(bin) {
  for (var i = 0, l = bin.length, o = new Uint8Array(l), c; i < l; ++i) {
    c = bin.charCodeAt(i);
    o[i] = ~c >> 8 & c; // Recover the null byte in a manner that is compatible with https://crbug.com/453961758
  }
  return o;
}
// end include: binaryDecode.js
// Memory management

var runtimeInitialized = false;



function updateMemoryViews() {
  // When memory growth is disabled this function should be called exactly once.
  assert(!HEAP8, 'updateMemoryViews should only be called once when ALLOW_MEMORY_GROWTH=0');
  var b = wasmMemory.buffer;
  HEAP8 = new Int8Array(b);
  HEAP16 = new Int16Array(b);
  HEAPU8 = new Uint8Array(b);
  HEAPU16 = new Uint16Array(b);
  HEAP32 = new Int32Array(b);
  HEAPU32 = new Uint32Array(b);
  HEAPF32 = new Float32Array(b);
  HEAPF64 = new Float64Array(b);
  HEAP64 = new BigInt64Array(b);
  HEAPU64 = new BigUint64Array(b);
}

// include: memoryprofiler.js
// end include: memoryprofiler.js
// end include: runtime_common.js
assert(globalThis.Int32Array && globalThis.Float64Array && Int32Array.prototype.subarray && Int32Array.prototype.set,
       'JS engine does not provide full typed array support');

function preRun() {
  var preRun = Module['preRun'];
  if (preRun) {
    if (typeof preRun == 'function') preRun = [preRun];
    onPreRuns.push(...preRun);
  }
  consumedModuleProp('preRun');
  // Begin ATPRERUNS hooks
  callRuntimeCallbacks(onPreRuns);
  // End ATPRERUNS hooks
}

function initRuntime() {
  assert(!runtimeInitialized);
  runtimeInitialized = true;

  checkStackCookie();

  // No ATINITS hooks

  wasmExports['__wasm_call_ctors']();

  // No ATPOSTCTORS hooks

  checkStackCookie();
}

function postRun() {
  checkStackCookie();

  var postRun = Module['postRun'];
  if (postRun) {
    if (typeof postRun == 'function') postRun = [postRun];
    onPostRuns.push(...postRun);
  }
  consumedModuleProp('postRun');

  // Begin ATPOSTRUNS hooks
  callRuntimeCallbacks(onPostRuns);
  // End ATPOSTRUNS hooks
}

/**
 * @param {string|number=} what
 */
function abort(what) {
  Module['onAbort']?.(what);

  what = `Aborted(${what})`;
  // TODO(sbc): Should we remove printing and leave it up to whoever
  // catches the exception?
  err(what);

  ABORT = true;

  // Use a wasm runtime error, because a JS error might be seen as a foreign
  // exception, which means we'd run destructors on it. We need the error to
  // simply make the program stop.
  // FIXME This approach does not work in Wasm EH because it currently does not assume
  // all RuntimeErrors are from traps; it decides whether a RuntimeError is from
  // a trap or not based on a hidden field within the object. So at the moment
  // we don't have a way of throwing a wasm trap from JS. TODO Make a JS API that
  // allows this in the wasm spec.

  // Suppress closure compiler warning here. Closure compiler's builtin extern
  // definition for WebAssembly.RuntimeError claims it takes no arguments even
  // though it can.
  // TODO(https://github.com/google/closure-compiler/pull/3913): Remove if/when upstream closure gets fixed.
  /** @suppress {checkTypes} */
  var e = new WebAssembly.RuntimeError(what);

  // Throw the error whether or not MODULARIZE is set because abort is used
  // in code paths apart from instantiation where an exception is expected
  // to be thrown when abort is called.
  throw e;
}

// show errors on likely calls to FS when it was not included
function fsMissing() {
  abort('Filesystem support (FS) was not included. The problem is that you are using files from JS, but files were not used from C/C++, so filesystem support was not auto-included. You can force-include filesystem support with -sFORCE_FILESYSTEM');
}
var FS = {
  init: fsMissing,
  createDataFile: fsMissing,
  createPreloadedFile: fsMissing,
  createLazyFile: fsMissing,
  open: fsMissing,
  mkdev: fsMissing,
  registerDevice:  fsMissing,
  analyzePath: fsMissing,
  ErrnoError: fsMissing,
};


function createExportWrapper(name, func, nargs) {
  assert(func);
  return (...args) => {
    assert(runtimeInitialized, `native function \`${name}\` called before runtime initialization`);
    // Only assert for too many arguments. Too few can be valid since the missing arguments will be zero filled.
    assert(args.length <= nargs, `native function \`${name}\` called with ${args.length} args but expects ${nargs}`);
    return func(...args);
  };
}

var wasmBinaryFile;

function findWasmBinary() {
  return binaryDecode(' asm   ``|` `~~``  ` ``` ` `||``` `~`~` `|~`~~ `~~|#wasi_snapshot_preview1fd_write ¿½																	\n\n\n  \n\n   		     \r\n    				pAA A Ð3memory __wasm_call_ctors \nzx16_reset \rzx16_seed_rng zx16_load_byte \nzx16_read8 zx16_write8 zx16_read16 zx16_write16 	zx16_set_pc \nzx16_get_pc zx16_set_sp zx16_get_sp \rzx16_get_register zx16_set_register 	zx16_step zx16_step_with_breakpoints zx16_step_over zx16_run_to_cursor zx16_run zx16_is_halted zx16_get_last_instruction zx16_get_output zx16_clear_output zx16_toggle_breakpoint zx16_has_breakpoint zx16_get_breakpoint_count zx16_clear_breakpoints zx16_has_breakpoint_hit zx16_get_breakpoint_hit_address zx16_clear_breakpoint_hit zx16_set_keyboard_key  zx16_get_keyboard_key !zx16_clear_keyboard_key "zx16_has_pending_tone #zx16_get_tone_frequency $zx16_get_tone_duration_ms %zx16_clear_tone_request &zx16_has_pending_stop_audio \'zx16_clear_stop_audio_request (zx16_get_volume_percent )__indirect_function_table fflush »strerror ½emscripten_stack_get_end ´emscripten_stack_get_base ³emscripten_stack_init ±emscripten_stack_get_free ²_emscripten_stack_restore ¸_emscripten_stack_alloc ¹emscripten_stack_get_current º	 A¨©¬\nö½ ±* A µ  A ¸ I# Ak! $    6 (Aÿÿq!A  Aÿÿqç  Aj$ ¥# Ak! $    6  6@@@ (A HAq\r  (ANAqE\r A 6A á ! (! (Aÿq!  Aÿÿq Aÿq®  A6 (! Aj$  # Ak! $    6@@@ (A HAq\r  (ANAqE\r A 6 A á  (Aÿÿq­ Aÿq6 (! Aj$  E# Ak! $    6  6 ( ( ! Aj$  # Ak! $    6@@@ (A HAq\r  (ANAqE\r A 6 A á  (Aÿÿq¯ Aÿÿq6 (! Aj$  §# Ak! $    6  6@@@ (A HAq\r  (ANAqE\r A 6A á ! (! (Aÿÿq!  Aÿÿq Aÿÿq°  A6 (! Aj$  I# Ak! $    6 (Aÿÿq!A  AÿÿqÀ  Aj$  A ¿ AÿÿqI# Ak! $    6 (Aÿÿq!A  AÿÿqÂ  Aj$  A Á AÿÿqJ# Ak! $    6A â  (³ Aÿÿq! Aj$  W# Ak! $    6  6A â  ( (AÿÿqAÿÿq´  Aj$  A Ä AÿÿqA Û ! AA   AqA Ý ! AA   AqY# Ak! $    6 (Aÿÿq!A  Aÿÿqß !AA  Aq! Aj$  # Ak! $    6 A 6@A ø !A ! Aq! !@ \r  ( (H!@ AqE\r A Ä   (Aj6 (! Aj$  A ø ! AA   Aq A à Aÿÿq A ã  A º Y# Ak! $    6 (Aþÿq!A  AÿÿqÒ !AA  Aq! Aj$  Y# Ak! $    6 (Aþÿq!A  AÿÿqÔ !AA  Aq! Aj$   A Ö  A Õ A Ù ! AA   Aq A Ú Aÿÿq A ¾ J# Ak! $    6 (Aÿÿq!A  Aÿÿqë  Aj$  A ê Aÿÿq A ¼ A î ! AA   Aq A ï Aÿÿq A ð Aÿÿq A ñ A ó ! AA   Aq A ô  A ö Aÿÿq	  ;# Ak! $    6 (! ¬  Aj$  W# Ak!   6 (! A 6@@ (AHAqE\r  (jA :    (Aj6 =# Ak!   6  ;\n (!Aÿÿ  /\nj!Aÿ -  E# Ak!   6  ;\n  : 	 (! - 	!Aÿÿ  /\nj :  # Ak!   6  ;\n (!Aÿÿ  /\nAj;Aÿÿ   /\nj-  : Aÿÿ   /j-  : Aÿ - !Aÿ  - AtrAÿÿq# Ak!   6  ;\n  ; (!Aÿÿ  /\nAj;Aÿÿ /Aÿq!Aÿÿ  /\nj :  Aÿÿ /AuAÿq!Aÿÿ  /j :  ;# Ak! $    6 (! ²  Aj$  a# Ak!   6 (! A 6@@ (AHAqE\r  (AtjA ;   (Aj6  Aþß;o# Ak!   6  6 (!@@@ (A HAq\r  (ANAqE\r A ;   (Atj/ ;Aÿÿ /i# Ak!   6  6  ; (!@@@ (A HAq\r  (ANAqE\r /!  (Atj ; h# Ak! $    6 (! «  Aj±  Aj¶  ·  ¸  Aj$  ;# Ak! $    6 (! ¹  Aj$  9# Ak!   6 (! A 6¨ A 6¬ A : ¨õ# Ak! $    6 (! ¬  Aj²  A ; AjAAþßAÿÿq´  A ; A6 º  ·  »  ¼  A ;´ A ;¶ A : ¸ A 6¼ A : À A 6Ä ½  Aj¹  ¾  A : Ê Aj$ a# Ak!   6 (! A 6 A 6@@ (AÀ HAqE\r  (AtjA ;   (Aj6 0# Ak!   6 (! A 6¤ A : ¤%# Ak!   6 (AáÙ;°## Ak!   6 (A ;²$# Ak!   6 (Aä ;È0# Ak!   6 (! A :   A ;¢*# Ak!   6 (!Aÿÿ /-# Ak!   6  ;\n ( /\n;E# Ak! $    6 (AjA³ Aÿÿq! Aj$  U# Ak! $    6  ;\n (Aj!A!Aÿÿ   /\n´  Aj$ n# Ak! $    6 (!Aÿÿ   /¯ ;\nAÿÿ  /Aj;Aÿÿ /\n! Aj$  û# AÐ k! $    6H (H!A !Aÿ@@ - Ê AÿqGAqE\r   /;N  Ã ; Aj! A4j!Aÿÿ   /   (D60  )<7(  )47   (06  )(7  ) 7  AjÅ   /;NAÿÿ /N! AÐ j$  # Ak! $    6ü (ü! - ! AK@@@@@@@@@@    (6ø  )7ð  ) 7è  (ø6  )ð7  )è7   Æ   (6à  )7Ø  ) 7Ð  (à6(  )Ø7   )Ð7  AjÇ   (6È  )7À  ) 7¸  (È6@  )À78  )¸70  A0jÈ   (6°  )7¨  ) 7   (°6X  )¨7P  ) 7H  AÈ jÉ   (6  )7  ) 7  (6p  )7h  )7`  Aà jÊ   (6  )7ø  ) 7ð  (6  )ø7  )ð7x  Aø jË   (6è  )7à  ) 7Ø  (è6   )à7  )Ø7  AjÌ   (6Ð  )7È  ) 7À  (Ð6¸  )È7°  )À7¨  A¨j  A6 Aj$ Ê\n&# A0k! $    6, (,! A 6 Aj!Aÿ   - ³ ;* Aj!Aÿ   - ³ ;( A ;&Aÿ@ - \r Aÿÿ /*!Aÿÿ   /(j;& Aj!Aÿ - !Aÿÿ   /&´ Aÿ@ - AFAqE\r Aÿÿ /*!	Aÿÿ  	 /(k;& Aj!\nAÿ - !Aÿÿ \n  /&´ Aÿ@ - AFAqE\r   /*;$  /(;" .$ ."H! AA  Aq;& Aj!\rAÿ - !Aÿÿ \r  /&´ Aÿ@ - AFAqE\r Aÿÿ /*!Aÿÿ  /(H! AA  Aq;& Aj!Aÿ - !Aÿÿ   /&´ Aÿ@ - AFAqE\r Aÿÿ  /(Aq6Aÿÿ  /* (t;& Aj!Aÿ - !Aÿÿ   /&´ Aÿ@ - AFAqE\r Aÿÿ  /(Aq6Aÿÿ  /* (u;& Aj!Aÿ - !Aÿÿ   /&´ Aÿ@ - AFAqE\r Aÿÿ  /(Aq6@@ (\r   /*;&Aÿÿ  /* (u6Aÿÿ@ /*AqE\r  (!A k! Aÿÿ t6  ( (r6  (;& Aj!Aÿ - !Aÿÿ   /&´ Aÿ@ - AFAqE\r Aÿÿ /*!Aÿÿ   /(r;& Aj!Aÿ - !Aÿÿ   /&´ Aÿ@ - AFAqE\r Aÿÿ /*!Aÿÿ   /(q;& Aj!Aÿ - ! Aÿÿ    /&´ Aÿ@ - A	FAqE\r Aÿÿ /*!!Aÿÿ  ! /(s;& Aj!"Aÿ - !#Aÿÿ " # /&´ Aÿ@ - A\nFAqE\r  Aj!$Aÿ - !%Aÿÿ $ % /(´ Aÿ@ - AFAqE\r   /*;Aÿ@ - AFAqE\r  Aj!&Aÿ - !\'Aÿÿ & \' /´   /(; A0j$ ¾# A k! $    6 (! A6 Aj!Aÿ   - ³ ; A ;Aÿÿ  / A	uAÿ q;Aÿ@ - \r Aÿÿ  / (j; Aj!Aÿ - !Aÿÿ   /´ Aÿ@ - AFAqE\r  . (ÁH! AA  Aq; Aj!Aÿ - !	Aÿÿ  	 /´ Aÿ@ - AFAqE\r Aÿÿ / (AÿÿqH!\n AA  \nAq; Aj!Aÿ - !Aÿÿ   /´ Aÿ@ - AFAqE\r Aÿÿ  /AuAq6Aÿÿ  /Aq6@ (AFAqE\r Aÿÿ  / (t; Aj!\rAÿ - !Aÿÿ \r  /´ @ (AFAqE\r Aÿÿ  / (u; Aj!Aÿ - !Aÿÿ   /´ @ (AFAqE\r @@ (\r   /;Aÿÿ  / (u6Aÿÿ@ /AqE\r  (!A k! Aÿÿ t (r6  (; Aj!Aÿ - !Aÿÿ   /´ Aÿ@ - AFAqE\r Aÿÿ /!Aÿÿ   /r; Aj!Aÿ - !Aÿÿ   /´ Aÿ@ - AFAqE\r Aÿÿ  / (Aÿÿqq; Aj!Aÿ - !Aÿÿ   /´ Aÿ@ - AFAqE\r Aÿÿ  / (Aÿÿqs; Aj!Aÿ - !Aÿÿ   /´ Aÿ@ - AFAqE\r  Aj!Aÿ  -  (Aÿÿq´  A j$  # Ak! $    6 (! A6 Aj!Aÿ   - ³ ;\n Aj!Aÿ   - ³ ;Aÿÿ  /\n6Aÿÿ  /6 Aÿÿ@ /\nAqE\r Aÿÿ  /\nAk6Aÿÿ@ /AqE\r Aÿÿ  /Ak6 Aÿ@ - \r Aÿÿ /\n!Aÿÿ@  /FAqE\r Aÿÿ  / (j;Aÿ@ - AFAqE\r Aÿÿ /\n!Aÿÿ@  /GAqE\r Aÿÿ  / (j;Aÿ@ - AFAqE\r Aÿÿ@ /\n\r Aÿÿ  / (j;Aÿ@ - AFAqE\r Aÿÿ@ /\nE\r Aÿÿ  / (j;Aÿ@ - AFAqE\r @ ( ( HAqE\r Aÿÿ  / (j;Aÿ@ - AFAqE\r @ ( ( NAqE\r Aÿÿ  / (j;Aÿ@ - AFAqE\r Aÿÿ /\n!Aÿÿ@  /HAqE\r Aÿÿ  / (j;Aÿ@ - AFAqE\r Aÿÿ /\n!	Aÿÿ@ 	 /NAqE\r Aÿÿ  / (j; Aj$ ü# Ak! $    6 (! A6 Aj!Aÿ   - ³ ;\nAÿÿ  /\n (j; Aj!Aÿ   - ³ ;Aÿ@ - \r  /!Aÿÿ /Aÿq!  Aÿÿq Aÿq® Aÿ@ - AFAqE\r  /Aÿÿq!Aÿÿ   /°  Aj$ §	# Ak! $    6 (! A6 Aj!Aÿ   - ³ ;\nAÿÿ  /\n (j; A ;Aÿ@ - \r Aÿÿ   /­ : Aÿ@@ - AqE\r Aÿ  - Aþr;Aÿ  - ; Aj!Aÿ - !Aÿÿ   /´ Aÿ@ - AFAqE\r Aÿÿ   /¯ ; Aj!Aÿ - !Aÿÿ   /´ Aÿ@ - AFAqE\r Aÿÿ   /­ Aÿq; Aj!	Aÿ - !\nAÿÿ 	 \n /´  Aj$ # Ak! $    6 (! A6Aÿ@ - AFAqE\r  Aj!Aÿ - !Aÿÿ   /´ Aÿÿ  / (j; Aj$ ³# Ak! $    6 (! A6  (;\nAÿ@ - AFAqE\r Aÿÿ  /Ak;Aÿÿ /!Aÿÿ   /\nj;\n Aj!Aÿ - !Aÿÿ   /\n´  Aj$ K# Ak!   ;Aÿÿ@@ /AqE\r  A Aq:  AAq:  - Aqâ# Ak! $    6  ; (!Aÿÿ@@ /Í Aq\r  A Aq: Aÿÿ@  /Ï AqE\r  AAq: @ (AÀ NAqE\r  A Aq:  /!  (Atj ;   (Aj6 AAq:  - Aq! Aj$  # Ak! $    6  ; (!Aÿÿ@@ /Í Aq\r  A Aq: Aÿÿ@  /Ñ A NAqE\r  AAq:  A Aq:  - Aq! Aj$  ÿ# A k! $    6  ; (!Aÿÿ   /Ñ 6@@ (A HAqE\r  A Aq:   (6@@ ( (AkHAqE\r  (AjAtj/ !  (Atj ;   (Aj6   (AkAtjA ;   (Aj6 AAq:  - Aq! A j$  # Ak!   6  ; (! A 6 @@@ (  (HAqE\r  ( Atj!Aÿÿ / !Aÿÿ@  /FAqE\r   ( 6  ( Aj6   A6 (V# Ak! $    6  ;\n (Aj!Aÿÿ  /\nÓ Aq! Aj$  # Ak! $    6  ; (!Aÿÿ@@  /Ï AqE\r Aÿÿ   /Ð Aq: Aÿÿ   /Î Aq:  - Aq! Aj$  V# Ak! $    6  ;\n (Aj!Aÿÿ  /\nÏ Aq! Aj$  F# Ak! $    6 (! Aj¹  ¾  Aj$ ># Ak! $    6 (Aj× ! Aj$   # Ak!   6 ((U# Ak! $    6 (! Aj!Aÿÿ  /Ï Aq! Aj$  7# Ak!   6 (!A !Aÿ -   AÿqGAq*# Ak!   6 (!Aÿÿ /¢# Ak! $    6 (!A !Aÿ@@ - Ê AÿqGAqE\r  A Aq: A !Aÿ@ -   AÿqGAqE\r Aÿÿ /¢!Aÿÿ  /FAqE\r  ¾  Ä  AAq: @ Ø AqE\r  A:    /;¢ A Aq:  ¾  Ä  AAq:  - Aq! Aj$  Ä# A k! $    ; Aj! Aj!Aÿÿ   / Aÿ@@ - AFAqE\r Aÿ - AFAqE\r  AAq: Aÿ@ - \r Aÿ - AFAqE\r  AAq:  A Aq:  - Aq! A j$  # Ak! $    6 (!A !Aÿ@@ - Ê AÿqGAqE\r  A Aq: Aÿÿ   /¯ ;Aÿÿ@ /Ü Aq\r  ¾  Ä  AAq: Aÿÿ  /Aj; ¾  Ä  A 6 @A !Aÿ - Ê AÿqG!A ! Aq! !@ \r Aÿÿ /!	Aÿÿ 	 /G!\nA ! \nAq! ! E\r  ( AèH!@ AqE\r @ Ø AqE\r  A:    /;¢ A Aq:  Ä   ( Aj6 Aÿÿ /!\rAÿÿ@ \r /FAqE\r  AAq: A !Aÿ@ - Ê AÿqGAqE\r  AAq:  A Aq:  - Aq! Aj$  K# Ak!   ;Aÿÿ@@ /AqE\r  A Aq:  AAq:  - AqÔ# Ak! $    6  ; (!A !Aÿ@@ - Ê AÿqGAqE\r  A Aq: Aÿÿ@ /Þ Aq\r  A Aq:  ¾ Aÿÿ /!Aÿÿ@  /FAqE\r  AAq:  A 6 @A !Aÿ - Ê AÿqG!A ! Aq!	 !\n@ 	\r Aÿÿ /!Aÿÿ  /G!A !\r Aq! \r!\n E\r  ( AÎ H!\n@ \nAqE\r @ Ø AqE\r  A:    /;¢ A Aq:  Ä   ( Aj6 Aÿÿ /!Aÿÿ@  /FAqE\r  AAq:  A Aq:  - Aq! Aj$  *# Ak!   6 (!Aÿÿ /# Ak!   6 (!# Ak!   6 (Aj!# Ak!   6 (A¤j­# Aà k! $    6\\ (\\! A ú  A0j!Aÿÿ  /6 A  Aj   A0jú  A0j!  Á Aÿÿq6  Aõ  A j   A0jú  A 6,@@ (,AHAqE\r A0j! (,!  Aj (,³ Aÿÿq6  6  AÛ     A0jú   (,Aj6,  Aà j$ O# Ak!   ;Aÿÿ@@ /AÀ JAqE\r  A Aq:  AAq:  - AqÑ# Ak! $    6x  ;v  ;t (x!Aÿÿ@@ /tå Aq\r  A Aq:  A ú  AÀ j!Aÿÿ  /v6  Aç  A j   AÀ jú  AÀ j!Aÿÿ  /t60 A  A0j   AÀ jú  A ;>@@Aÿÿ />!Aÿÿ  /tHAqE\rAÿÿ /v!Aÿÿ   />j;< AÀ j!	Aÿÿ  /<6 	AÊ  Aj   AÀ jú  A 68@ (8AH!\nA ! \nAq! !\r@ E\r Aÿÿ />!Aÿÿ  /tH!\r@ \rAqE\r   /v />j;6   /6­ : 5  - 56 A¼ ! AÀ j     AÀ jú   />Aj;>  (8Aj68 A  ú   AAq:  - Aq! Aj$  -# Ak!   6  ;\n ( /\n;°·# Ak!   6 (!Aÿÿ  /°6  ( (AtAÿÿqs6  (Aÿÿq6  ( (A	vs6  (Aÿÿq6  ( (AtAÿÿqs6  (Aÿÿq6  (;°Aÿÿ /°N# Ak!   ;Aÿÿ@@ /ALAqE\r  AAq:  A Aq:  - Aq*# Ak!   6 (!Aÿÿ /²# Ak! $    6  ; (!Aÿÿ@@ /é Aq\r  A Aq:   /;² AAq:  - Aq! Aj$  ·# Ak!   ;  ;\nAÿÿ@@ /AHAqE\r  A Aq: Aÿÿ@ /A JAqE\r  A Aq: Aÿÿ@ /\n\r  A Aq: Aÿÿ@ /\nA\'JAqE\r  A Aq:  AAq:  - Aq¼# Ak! $    6  ;  ; (! /Aÿÿq!Aÿÿ@@  /ì Aq\r  A Aq:   /;´  /;¶ A: ¸  (¼Aj6¼ AAq:  - Aq! Aj$  7# Ak!   6 (!A !Aÿ - ¸ AÿqGAq*# Ak!   6 (!Aÿÿ /´*# Ak!   6 (!Aÿÿ /¶9# Ak!   6 (! A ;´ A ;¶ A : ¸\\# Ak! $    6 (! ñ  A: À  (ÄAj6ÄAAq! Aj$  7# Ak!   6 (!A !Aÿ - À AÿqGAq## Ak!   6 (A : ÀO# Ak!   ;Aÿÿ@@ /Aä JAqE\r  A Aq:  AAq:  - Aq*# Ak!   6 (!Aÿÿ /È# Ak! $    6  ; (!Aÿÿ@@ /õ Aq\r  A Aq:   /;È AAq:  - Aq! Aj$  7# Ak!   6 (!A !Aÿ - Ê AÿqGAqy# Ak!   6  :  (!@ (¤AÿHAqE\r  - ! A¤j (¤j :    (¤Aj6¤ A¤j (¤jA :  # Ak! $    6  6 (!@@ (A FAqE\r  A 6@ ( (j,  E\r  ( (j,  ù   (Aj6  Aj$ `# Ak!   6  ;Aÿÿ@@ /AqE\r Aÿÿ  /Ak6Aÿÿ  /6 (# AÀ k! $    6<  ;: (<!Aÿÿ   /:û 6 A j!  (6  A¶     A jú   A j6A  Aj  AÀ j$ W# Ak!   6 (!@@ (¬ (¨NAqE\r  AAq:  A Aq:  - Aqn# Ak! $    6 (!@@ ý AqE\r  A :   A¨j (¬j-  :  , ! Aj$  h# Ak! $    6 (!  þ : @ ý Aq\r   (¬Aj6¬ , ! Aj$  z# Ak!   6  : @@@ , A FAq\r  , A\nFAq\r  , A\rFAq\r  , A	FAqE\r AAq:  A Aq:  - Aq¼# A k! $    6  ; (! A 6@@ (AHAqE\rAÿÿ   / (jAÿÿq­ : Aÿ@ - \r   , ù   , 6 A¹     (Aj6  A j$ Ë	# Ak! $    6 (! A6 A 6 A : @ ý !A ! Aq! !@ \r   þ À !@ AqE\r  ÿ @@ ý Aq\r  þ ÀA-FAqE\r  A6 ÿ @ ý Aq\r  þ ÀA+FAqE\r  ÿ @@ ý AsAqE\r  þ : @@ , A0HAq\r  , A9JAqE\r A:   (A\nl , A0kj6 ÿ  A !Aÿ@@ -  AÿqGAq\r  AjAA Aÿÿq´   ( (l6 Aj! (!	 A 	Aÿÿq´  Aj$ Ý\n# Ak! $    6  ;\n  ; (! A ;Aÿÿ@@ /\r  AjAA Aÿÿq´ @@ ý AsAqE\r  þ : @@ , A\nFAq\r  , A\rFAqE\r ÿ @ ý Aq\r   þ : @@@ , A\rFAqE\r  , A\nFAq\r , A\nFAqE\r , A\rFAqE\r ÿ Aÿÿ /!Aÿÿ@  /AkHAqE\r  /\n /j! - !  Aÿÿq ®   /Aj; ÿ  Aÿÿ /\n!Aÿÿ  /j!	A !\n  	Aÿÿq \nAÿq®  Aj!A!Aÿÿ   /´  Aj$  	# A0k! $    6, (,! A6Aÿÿ@ /\r   AjA³ ;*Aÿÿ  /*ü Aÿÿ@ /AFAqE\r   AjA³ ;(Aÿÿ  /(Aÿq: \'  , \'ù   , \'6 A¹   Aÿÿ@ /AFAqE\r   AjA³ ;$  AjA³ ;" /$Aÿÿq!Aÿÿ   /" Aÿÿ@ /AFAqE\r   Aÿÿ@ /AFAqE\r   AjA³ ; Aÿÿ  /  Aÿÿ@ /A FAqE\r   AjA³ ;Aÿÿ  /ç Aÿÿ@ /A!FAqE\r   è ; Aj!A!Aÿÿ   /´ Aÿÿ@ /A0FAqE\r  Aj!A!Aÿÿ   /²´ Aÿÿ@ /AÀ FAqE\r   AjA³ ;  AjA³ ; /Aÿÿq!	Aÿÿ  	 /í Aÿÿ@ /AÁ FAqE\r   AjA³ ;Aÿÿ  /÷ Aÿÿ@ /AÂ FAqE\r  ò Aÿÿ@ /AÐ FAqE\r  ä Aÿÿ@ /AÑ FAqE\r   AjA³ ;  AjA³ ; /Aÿÿq!\nAÿÿ  \n /æ Aÿÿ@ /AÿFAqE\r  A: Ê A0j$ ë# A k! $   6  ; (!   /; Aÿÿ    / : Aÿÿ    / : Aÿÿ    / :   A :   A :   A :   A 6  A ;  A :   A : Aÿ     -  6@  (\r Aÿÿ   /AuAq:     - : Aÿÿ   /A	uAq: @  (AFAqE\r Aÿÿ   /AuAq: Aÿÿ  /A	uAÿ q6    (A 6@  (AFAqE\r Aÿÿ   /AuAq: Aÿÿ   /A	uAq: Aÿÿ  /AuAqAt6    (A 6@  (AFAqE\r Aÿÿ   /AuAq: Aÿÿ   /A	uAq: Aÿÿ  /AuAq6    (A 6@  (AFAqE\r Aÿÿ   /AuAq: Aÿÿ   /A	uAq: Aÿÿ  /AuAq6    (A 6@  (AFAqE\r Aÿÿ   /AuAq: Aÿÿ   /AuAq:  A 6 (!Aÿÿ   /AuAqAtr6 (!Aÿÿ   /A	uA?qAtr6    (A\n 6@  (AFAqE\r Aÿÿ   /AuAq: Aÿÿ   /AuAq:  A 6  ( !Aÿÿ   /AuAqr6  ( !Aÿÿ   /A	uA?qAtr6    ( At6@  (AFAqE\r Aÿÿ   /AuAÿq;Aÿÿ    /6 A j$ /# Ak!   6  ;\nAÿÿ /\nAqAÿq2# Ak!   6  ;\nAÿÿ /\nAuAqAÿq2# Ak!   6  ;\nAÿÿ /\nAuAqAÿqø# Ak!   6  : Aÿ@@ - \r  A 6Aÿ@ - AFAqE\r  A6Aÿ@ - AFAqE\r  A6Aÿ@ - AFAqE\r  A6Aÿ@ - AFAqE\r  A6Aÿ@ - AFAqE\r  A6Aÿ@ - AFAqE\r  A6 A6 (# A k!   6  6  6 (Ak! A t6 (! A tAk6  ( (q6@ ( (qE\r   ( (Asr6 (;# Ak"$   6Aà    § ! Aj$  7# Ak"$   6    ­ ! Aj$  # A k"$    ("6  (!  6  6   k"6  j!@@@@@  (< AjAr Aj  F""AA " Aj ® E\r  !@  ("F\r@ AJ\r  ! AA   ("K"	j" (   A  	k"j6  AA 	j" (  k6   k! !  (<   	k" Aj ® E\r  AG\r    (,"6   6     (0j6 !A !  A 6  B 7    ( A r6  AF\r   (k! A j$   A  B  AØ±    AÜ±  Aà±  AÜ±  \\    (H"Aj r6H@  ( "AqE\r    A r6 A  B 7    (,"6   6     (0j6A é A G!@@@  AqE\r  E\r  Aÿq!@  -   F\r Aj"A G!  Aj" AqE\r \r  E\r@  -   AÿqF\r  AI\r  AÿqAl!@A  (  s"k rAxqAxG\r  Aj!  A|j"AK\r  E\r Aÿq!@@  -   G\r     Aj!  Aj"\r A   A   "  k  ~@  ½"B4§Aÿq"AÿF\r @ \r @@  D        b\r A !  D      ðC¢  !  ( A@j!  6     Axj6  BÿÿÿÿÿÿÿBð?¿!   ® @@ AH\r   D      à¢! @ AÿO\r  Axj!  D      à¢!  Aý AýIApj! AxJ\r   D      `¢! @ A¸pM\r  AÉj!  D      `¢!  Aðh AðhKAj!   Aÿj­B4¿¢  @    ü\n    @ AI\r         j!@@   sAq\r @@  Aq\r   !@ \r   !  !@  -  :   Aj! Aj"AqE\r  I\r  A|q!@ AÀ I\r   A@j"K\r @  ( 6   (6  (6  (6  (6  (6  (6  (6  ( 6   ($6$  ((6(  (,6,  (060  (464  (868  (<6< AÀ j! AÀ j" M\r   O\r@  ( 6  Aj! Aj" I\r @ AO\r   !@ AO\r   ! A|j!  !@  -  :    - :   - :   - :  Aj! Aj" M\r @  O\r @  -  :   Aj! Aj" G\r   æ@@ ("\r A !  \r (!@   ("kM\r      ($  @@ (PA H\r  E\r  !@@   j"Aj-  A\nF\r Aj"E\r      ($  " I\r  k! (!  !A !      ( j6  j! ò~@ E\r    :     j"Aj :   AI\r    :    :  A}j :   A~j :   AI\r    :  A|j :   A	I\r   A   kAq"j" AÿqAl"6    kA|q"j"A|j 6  A	I\r   6  6 Axj 6  Atj 6  AI\r   6  6  6  6 Apj 6  Alj 6  Ahj 6  Adj 6   AqAr"k"A I\r  ­B~!  j!@  7  7  7  7  A j! A`j"AK\r   æ# AÐk"$   6Ì A jA A(ü   (Ì6È@@A   AÈj AÐ j A j   A N\r A!     ( "A_q6 @@@@  (0\r   AÐ 60  A 6  B 7  (,!   6,A !  (\rA!   \r    AÈj AÐ j A j   ! A q!@ E\r   A A   ($    A 60   6,  A 6  (!  B 7 A !    ( " r6 A  A q!  AÐj$   ~# AÀ k"$   6< A)j! A\'j!	 A(j!\nA !A !@@@@@A !\r@ ! \r AÿÿÿÿsJ\r \r j! !\r@@@@@@ -  "E\r @@@@ Aÿq"\r  \r! A%G\r \r!@@ - A%F\r  ! \rAj!\r - ! Aj"! A%F\r  \r k"\r Aÿÿÿÿs"J\r\n@  E\r     \r   \r\r  6< Aj!\rA!@ , APj"A	K\r  - A$G\r  Aj!\rA! !  \r6<A !@@ \r,  "A`j"AM\r  \r!A ! \r!A t"AÑqE\r @  \rAj"6<  r! \r, "A`j"A O\r !\rA t"AÑq\r @@ A*G\r @@ , APj"\rA	K\r  - A$G\r @@  \r   \rAtjA\n6 A !  \rAtj( ! Aj!A! \r Aj!@  \r   6<A !A !  ( "\rAj6  \r( !A !  6< AJ\rA  k! AÀ r! A<j¡ "A H\r (<!A !\rA!@@ -  A.F\r A !@ - A*G\r @@ , APj"A	K\r  - A$G\r @@  \r   AtjA\n6 A !  Atj( ! Aj! \r Aj!@  \r A !  ( "Aj6  ( !  6< AJ!  Aj6<A! A<j¡ ! (<!@ \r!A! ",  "\rAjAFI\r Aj! A:l \rjAï j-  "\rAjAÿqAI\r   6<@@ \rAF\r  \rE\r\r@ A H\r @  \r   Atj \r6 \r   Atj) 70  E\r	 A0j \r  ¢  AJ\rA !\r  E\r	  -  A q\r Aÿÿ{q"  AÀ q!A !A ! \n!@@@@@@@@@@@@@@@@@ -  "À"\rASq \r AqAF \r "\rA¨j!	\n  \n!@ \rA¿j  \rAÓ F\rA !A ! )0!A !\r@@@@@@@   (0 6  (0 6  (0 ¬7  (0 ;  (0 :   (0 6  (0 ¬7  A AK! Ar!Aø !\rA !A ! )0" \n \rA q£ ! P\r AqE\r \rAvA j!A!A !A ! )0" \n¤ ! AqE\r   k"\r  \rJ!@ )0"BU\r  B  }"70A!A !@ AqE\r A!A !A A  Aq"!  \n¥ !  A Hq\r Aÿÿ{q  !@ B R\r  \r  \n! \n!A !  \n k Pj"\r  \rJ!\r - 0!\r (0"\rAÔ  \r!   Aÿÿÿÿ AÿÿÿÿI "\rj!@ AL\r  ! \r!\r ! \r! -  \r )0"PE\rA !\r	@ E\r  (0!A !\r  A  A  ¦  A 6  >  Aj60 Aj!A!A !\r@@ ( "E\r Aj ° "A H\r   \rkK\r Aj!  \rj"\r I\r A=! \rA H\r\r  A   \r ¦ @ \r\r A !\rA ! (0!@ ( "E\r Aj ° " j" \rK\r   Aj    Aj!  \rI\r   A   \r AÀ s¦   \r  \rJ!\r	  A Hq\r\nA=!   +0    \r    "\rA N\r \r- ! \rAj!\r   \r\n E\rA!\r@@  \rAtj( "E\r  \rAtj   ¢ A! \rAj"\rA\nG\r @ \rA\nI\r A!@  \rAtj( \rA! \rAj"\rA\nF\r A!  \r: \'A! 	! \n! ! \n!   k"  J" AÿÿÿÿsJ\rA=!   j"  J"\r K\r  A  \r  ¦         A0 \r  As¦   A0  A ¦         A  \r  AÀ s¦  (<!A !A=!  6 A! AÀ j$   @  -  A q\r      {A !@  ( ",  APj"A	M\r A @A!@ AÌ³æ K\r A  A\nl"j  AÿÿÿÿsK!   Aj"6  , ! ! ! APj"A\nI\r  ¾ @@@@@@@@@@@@@@@@@@@ Awj 	\n\r  ( "Aj6    ( 6   ( "Aj6    4 7   ( "Aj6    5 7   ( "Aj6    4 7   ( "Aj6    5 7   ( AjAxq"Aj6    ) 7   ( "Aj6    2 7   ( "Aj6    3 7   ( "Aj6    0  7   ( "Aj6    1  7   ( AjAxq"Aj6    ) 7   ( "Aj6    5 7   ( AjAxq"Aj6    ) 7   ( AjAxq"Aj6    ) 7   ( "Aj6    4 7   ( "Aj6    5 7   ( AjAxq"Aj6    + 9       5 @  P\r @ Aj"  §Aq-   r:    B" B R\r  . @  P\r @ Aj"  §AqA0r:    B" B R\r  ~@  BT\r @ Aj"  " B\n" B\n~}§A0r:   BÿÿÿÿV\r   §!@  B\nT\r @ Aj" " A\nn"A\nlkA0r:   Aã K\r @ E\r  Aj" A0r:   # Ak"$ @  L\r  AÀq\r     k"A AI" @ \r @   A   A~j"AÿK\r        Aj$      A A  È~~|# A°k"$ A ! A 6¬@@ ª "	BU\r A!\nA ! "ª !	@ AqE\r A!\nA !A A  Aq"\n! \nE!@@ 	Bøÿ Bøÿ R\r   A   \nAj" Aÿÿ{q¦     \n    A® AÂ  A q"\rA² AÆ  \r  bA    A    AÀ s¦     J! Aj!@@@@  A¬j "  "D        a\r   (¬"Aj6¬ A r"Aá G\r A r"Aá F\rA  A H! (¬!  Acj"6¬A  A H! D      °A¢! A Aè A Hj"!\r@ \r ü"6  \rAj!\r  ¸¡D    eÍÍA¢"D        b\r @@ AN\r  ! \r! ! ! !@ A AI!@ \rA|j" I\r  ­!B !	@  5   	|" BëÜ"	BëÜ~}>  A|j" O\r  BëÜT\r  A|j" 	> @@ \r" M\r A|j"\r( E\r   (¬ k"6¬ !\r A J\r @ AJ\r  AjA	nAj! Aæ F!@A  k"\rA	 \rA	I!@@  I\r A A ( !\rAëÜ v!A tAs!A ! !\r@ \r \r( " v j6   q l! \rAj"\r I\r A A ( !\r E\r   6  Aj!  (¬ j"6¬   \rj" "\r Atj   \rkAu J! A H\r A !@  O\r   kAuA	l!A\n!\r ( "A\nI\r @ Aj!  \rA\nl"\rO\r @ A   Aæ Fk A G Aç Fqk"\r  kAuA	lAwjN\r  A`Aìc A Hj \rAÈ j"A	m"Atj!A\n!\r@  A	lk"AJ\r @ \rA\nl!\r Aj"AG\r  Aj!@@ ( "  \rn" \rlk"\r   F\r@@ Aq\r D      @C! \rAëÜG\r  M\r A|j-  AqE\rD     @C!D      à?D      ð?D      ø?  FD      ø?  \rAv"F  I!@ \r  -  A-G\r  ! !   k"6     a\r    \rj"\r6 @ \rAëÜI\r @ A 6 @ A|j" O\r  A|j"A 6   ( Aj"\r6  \rAÿëÜK\r   kAuA	l!A\n!\r ( "A\nI\r @ Aj!  \rA\nl"\rO\r  Aj"\r   \rK!@@ "\r M"\r \rA|j"( E\r @@ Aç F\r  Aq! AsA A " J A{Jq" j!AA~  j! Aq"\r Aw!@ \r  \rA|j( "E\r A\n!A ! A\np\r @ "Aj!  A\nl"pE\r  As! \r kAuA	l!@ A_qAÆ G\r A !   jAwj"A  A J"  H!A !   j jAwj"A  A J"  H!A! AýÿÿÿAþÿÿÿ  r"J\r  A GjAj!@@ A_q"AÆ G\r   AÿÿÿÿsJ\r A  A J!@   Au"s k­ ¥ "kAJ\r @ Aj"A0:    kAH\r  A~j" :  A! AjA-A+ A H:    k" AÿÿÿÿsJ\rA!  j" \nAÿÿÿÿsJ\r  A    \nj" ¦     \n    A0   As¦ @@@@ AÆ G\r  AjA	r!    K"!@ 5  ¥ !@@  F\r   AjM\r@ Aj"A0:    AjK\r   G\r  Aj"A0:       k   Aj" M\r @ E\r   AÒ A    \rO\r AH\r@@ 5  ¥ " AjM\r @ Aj"A0:    AjK\r     A	 A	H   Awj! Aj" \rO\r A	J! ! \r @ A H\r  \r Aj \r K! AjA	r! !\r@@ \r5  ¥ " G\r  Aj"A0:  @@ \r F\r   AjM\r@ Aj"A0:    AjK\r    A   Aj!  rE\r   AÒ A       k"   J    k! \rAj"\r O\r AJ\r   A0 AjAA ¦      k   !  A0 A	jA	A ¦   A    AÀ s¦     J!  AtAuA	qj!@ AK\r  -  !D      ð?A4 Atk !@ A-G\r    ¡ !    ¡!@ (¬"\r \rAu"s k­ ¥ " G\r  Aj"A0:   (¬!\r \nAr! A q! A~j" Aj:   AjA-A+ \rA H:   AH AqEq! Aj!\r@ \r" ü"\rA j-   r:    \r·¡D      0@¢!@ Aj"\r AjkAG\r  D        a q\r  A.:  Aj!\r D        b\r A! Aûÿÿÿ \n  k"jkJ\r   A    j Aj \r Ajk" A~j H  "j"\r ¦         A0  \r As¦    Aj     A0  kA A ¦         A   \r AÀ s¦   \r  \rJ! A°j$  .  ( AjAxq"Aj6    )  )· 9    ½# A k"$     Aj " 6   A Gk6 A Aü  A6L A 6$ A6P  Aj6,  Aj6T  A :     § ! A j$  ¶  (T"( !@ ("  (  ("k"  I"E\r       (  j"6   ( k"6@    I"E\r       (  j"6   ( k6 A :      (,"6   6    Aÿÿÿÿ  «  @  \r A    6 A¬A!@@  E\r  Aÿ M\r@@A (ô ( \r  AqA¿F\r A6 @ AÿK\r    A?qAr:    AvAÀr:  A@@ A°I\r  A@qAÀG\r   A?qAr:    AvAàr:     AvA?qAr: A@ A|jAÿÿ?K\r    A?qAr:    AvAðr:     AvA?qAr:    AvA?qAr: A A6 A!    :  A @  \r A    A ¯   A $ A AjApq$  # # k #  # S~@@ AÀ qE\r   A@j­!B ! E\r  AÀ  k­  ­"!  !   7    7S~@@ AÀ qE\r   A@j­!B ! E\r  AÀ  k­  ­"!  !   7    7©~# A k"$  Bÿÿÿÿÿÿ?!@@ B0Bÿÿ"§"AÿjAýK\r   B< B! Aj­!@@  Bÿÿÿÿÿÿÿÿ" BT\r  B|!  BR\r  B |!B   BÿÿÿÿÿÿÿV"!  ­ |!@   P\r  BÿÿR\r   B< BB! Bÿ!@ AþM\r Bÿ!B ! @Aø Aø  P"" k"Að L\r B ! B !  BÀ  !A !@  F\r  Aj   A kµ  ) )B R!     ¶  ) "B< )B! @@ Bÿÿÿÿÿÿÿÿ ­"BT\r   B|!  BR\r   B  |!   B    BÿÿÿÿÿÿÿV"!  ­! A j$  B4 B  ¿\n   $ #   kApq"$   # @  \r A !@A (ð E\r A (ð » !@A (ä± E\r A (ä± »  r!@ ( " E\r @@  (  (F\r   »  r!  (8" \r   @  (  (F\r   A A   ($    (\r A@  ("  ("F\r     k¬A  ((    A 6  B 7  B 7A EA  !@  AK\r @@  \r A !   At/ " E\r  AÄ j!      ¼  AÞ-+   0X0x -0X+0X 0X-0x+0x 0x %s Unknown error nan inf %d %c  %02X NAN INF 0x%04X: . (null) x%d=0x%04X\n START=0x%04X\n SP=0x%04X\n LEN=0x%04X\n PC=0x%04X\n REGS\n MEM\n                           	             \n\n\n  	  	                               \r \r   	   	                                               	                                                  	                                                   	                                              	                                                      	                                                   	         0123456789ABCDEF   N ë§~ uú ¹,ý·z¼ ú¢ =I×  *_·úXÙ+Ê½áÍÜ@x }gaì å\nÔ Ì>Ov¯  D ® ®` úw!ë+ `A ©£nN                                                        *                    \'9H                                  8R`S  Ê»  Ò  é	>Yi~Success Illegal byte sequence Domain error Result not representable Not a tty Permission denied Operation not permitted No such file or directory No such process File exists Value too large for defined data type No space left on device Out of memory Resource busy Interrupted system call Resource temporarily unavailable Invalid seek Cross-device link Read-only file system Directory not empty Connection reset by peer Operation timed out Connection refused Host is down Host is unreachable Address in use Broken pipe I/O error No such device or address Block device required No such device Not a directory Is a directory Text file busy Exec format error Invalid argument Argument list too long Symbolic link loop Filename too long Too many open files in system No file descriptors available Bad file descriptor No child process Bad address File too large Too many links No locks available Resource deadlock would occur State not recoverable Owner died Operation canceled Function not implemented No message of desired type Identifier removed Device not a stream No data available Device timeout Out of streams resources Link has been severed Protocol error Bad message File descriptor in bad state Not a socket Destination address required Message too large Protocol wrong type for socket Protocol not available Protocol not supported Socket type not supported Not supported Protocol family not supported Address family not supported by protocol Address not available Network is down Network unreachable Connection reset by network Connection aborted No buffer space available Socket is connected Socket not connected Cannot send after socket shutdown Operation already in progress Operation in progress Stale file handle Data consistency error Resource not available Remote I/O error Quota exceeded No medium found Wrong medium type Multihop attempted Required key not available Key has expired Key has been revoked Key was rejected by service  Aà                                        Ø                           ÿÿÿÿ\n                                                               `   target_features+bulk-memory+bulk-memory-opt+call-indirect-overlong+\nmultivalue+mutable-globals+nontrapping-fptoint+reference-types+sign-ext');
}

function getBinarySync(file) {
  return file;
}

async function getWasmBinary(binaryFile) {

  // Otherwise, getBinarySync should be able to get it synchronously
  return getBinarySync(binaryFile);
}

async function instantiateArrayBuffer(binaryFile, imports) {
  try {
    var binary = await getWasmBinary(binaryFile);
    var instance = await WebAssembly.instantiate(binary, imports);
    return instance;
  } catch (reason) {
    err(`failed to asynchronously prepare wasm: ${reason}`);

    // Warn on some common problems.
    if (isFileURI(binaryFile)) {
      err(`warning: Loading from a file URI (${binaryFile}) is not supported in most browsers. See https://emscripten.org/docs/getting_started/FAQ.html#how-do-i-run-a-local-webserver-for-testing-why-does-my-program-stall-in-downloading-or-preparing`);
    }
    abort(reason);
  }
}

async function instantiateAsync(binary, binaryFile, imports) {
  return instantiateArrayBuffer(binaryFile, imports);
}

function getWasmImports() {
  // prepare imports
  var imports = {
    'env': wasmImports,
    'wasi_snapshot_preview1': wasmImports,
  };
  return imports;
}

// Create the wasm instance.
// Receives the wasm imports, returns the exports.
async function createWasm() {
  // Load the wasm module and create an instance of using native support in the JS engine.
  // handle a generated wasm instance, receiving its exports and
  // performing other necessary setup
  function receiveInstance(instance) {
    wasmExports = instance.exports;

    assignWasmExports(wasmExports);

    updateMemoryViews();

    return wasmExports;
  }

  // Prefer streaming instantiation if available.
  // Async compilation can be confusing when an error on the page overwrites Module
  // (for example, if the order of elements is wrong, and the one defining Module is
  // later), so we save Module and check it later.
  var trueModule = Module;
  function receiveInstantiationResult(result) {
    // 'result' is a ResultObject object which has both the module and instance.
    // receiveInstance() will swap in the exports (to Module.asm) so they can be called
    assert(Module === trueModule, 'the Module object should not be replaced during async compilation - perhaps the order of HTML elements is wrong?');
    trueModule = null;
    // TODO: Due to Closure regression https://github.com/google/closure-compiler/issues/3193, the above line no longer optimizes out down to the following line.
    // When the regression is fixed, can restore the above PTHREADS-enabled path.
    return receiveInstance(result['instance']);
  }

  var info = getWasmImports();

  // User shell pages can write their own Module.instantiateWasm = function(imports, successCallback) callback
  // to manually instantiate the Wasm module themselves. This allows pages to
  // run the instantiation parallel to any other async startup actions they are
  // performing.
  // Also pthreads and wasm workers initialize the wasm instance through this
  // path.
  var instantiateWasm = Module['instantiateWasm'];
  if (instantiateWasm) {
    return new Promise((resolve) => {
      try {
        instantiateWasm(info, (inst) => resolve(receiveInstance(inst)));
      } catch(e) {
        err(`Module.instantiateWasm callback failed with error: ${e}`);
        throw e;
      }
    });
  }

  wasmBinaryFile ??= findWasmBinary();
  var result = await instantiateAsync(wasmBinary, wasmBinaryFile, info);
  var exports = receiveInstantiationResult(result);
  return exports;
}

// end include: preamble.js

// Begin JS library code


  class ExitStatus {
      name = 'ExitStatus';
      constructor(status) {
        this.message = `Program terminated with exit(${status})`;
        this.status = status;
      }
    }

  /** @type {!Int16Array} */
  var HEAP16;

  /** @type {!Int32Array} */
  var HEAP32;

  /** not-@type {!BigInt64Array} */
  var HEAP64;

  /** @type {!Int8Array} */
  var HEAP8;

  /** @type {!Float32Array} */
  var HEAPF32;

  /** @type {!Float64Array} */
  var HEAPF64;

  /** @type {!Uint16Array} */
  var HEAPU16;

  /** @type {!Uint32Array} */
  var HEAPU32;

  /** not-@type {!BigUint64Array} */
  var HEAPU64;

  /** @type {!Uint8Array} */
  var HEAPU8;

  var callRuntimeCallbacks = (callbacks) => {
      while (callbacks.length > 0) {
        // Pass the module as the first argument.
        callbacks.shift()(Module);
      }
    };
  var onPostRuns = [];
  var addOnPostRun = (cb) => onPostRuns.push(cb);

  var onPreRuns = [];
  var addOnPreRun = (cb) => onPreRuns.push(cb);


  
    /**
   * @param {number} ptr
   * @param {string} type
   */
  function getValue(ptr, type = 'i8') {
    if (type.endsWith('*')) type = '*';
    switch (type) {
      case 'i1': return HEAP8[ptr];
      case 'i8': return HEAP8[ptr];
      case 'i16': return HEAP16[((ptr)>>1)];
      case 'i32': return HEAP32[((ptr)>>2)];
      case 'i64': return HEAP64[((ptr)>>3)];
      case 'float': return HEAPF32[((ptr)>>2)];
      case 'double': return HEAPF64[((ptr)>>3)];
      case '*': return HEAPU32[((ptr)>>2)];
      default: abort(`invalid type for getValue: ${type}`);
    }
  }

  var noExitRuntime = true;

  function ptrToString(ptr) {
      assert(typeof ptr === 'number', `ptrToString expects a number, got ${typeof ptr}`);
      // Convert to 32-bit unsigned value
      ptr >>>= 0;
      return '0x' + ptr.toString(16).padStart(8, '0');
    }

  
    /**
   * @param {number} ptr
   * @param {number} value
   * @param {string} type
   */
  function setValue(ptr, value, type = 'i8') {
    if (type.endsWith('*')) type = '*';
    switch (type) {
      case 'i1': HEAP8[ptr] = value; break;
      case 'i8': HEAP8[ptr] = value; break;
      case 'i16': HEAP16[((ptr)>>1)] = value; break;
      case 'i32': HEAP32[((ptr)>>2)] = value; break;
      case 'i64': HEAP64[((ptr)>>3)] = BigInt(value); break;
      case 'float': HEAPF32[((ptr)>>2)] = value; break;
      case 'double': HEAPF64[((ptr)>>3)] = value; break;
      case '*': HEAPU32[((ptr)>>2)] = value; break;
      default: abort(`invalid type for setValue: ${type}`);
    }
  }

  var stackRestore = (val) => __emscripten_stack_restore(val);

  var stackSave = () => _emscripten_stack_get_current();

  var warnOnce = (text) => {
      warnOnce.shown ||= {};
      if (!warnOnce.shown[text]) {
        warnOnce.shown[text] = 1;
        if (ENVIRONMENT_IS_NODE) text = 'warning: ' + text;
        err(text);
      }
    };

  

  var printCharBuffers = [null,[],[]];
  
  var UTF8Decoder = globalThis.TextDecoder && new TextDecoder();
  
  
    /**
   * heapOrArray is either a regular array, or a JavaScript typed array view.
   * @param {number} idx
   * @param {number=} maxBytesToRead
   * @param {boolean=} ignoreNul
   * @return {number}
   */
  var findStringEnd = (heapOrArray, idx, maxBytesToRead, ignoreNul) => {
      var maxIdx = idx + maxBytesToRead;
      if (ignoreNul) return maxIdx;
      // TextDecoder needs to know the byte length in advance, it doesn't stop on
      // null terminator by itself.
      // As a tiny code save trick, compare idx against maxIdx using a negation,
      // so that maxBytesToRead=undefined/NaN means Infinity.
      while (heapOrArray[idx] && !(idx >= maxIdx)) ++idx;
      return idx;
    };
  
  
    /**
   * Given a pointer 'idx' to a null-terminated UTF8-encoded string in the given
   * array that contains uint8 values, returns a copy of that string as a
   * Javascript String object.
   * heapOrArray is either a regular array, or a JavaScript typed array view.
   * @param {number=} idx
   * @param {number=} maxBytesToRead
   * @param {boolean=} ignoreNul - If true, the function will not stop on a NUL character.
   * @return {string}
   */
  var UTF8ArrayToString = (heapOrArray, idx = 0, maxBytesToRead, ignoreNul) => {
  
      var endPtr = findStringEnd(heapOrArray, idx, maxBytesToRead, ignoreNul);
  
      // When using conditional TextDecoder, skip it for short strings as the overhead of the native call is not worth it.
      if (endPtr - idx > 16 && heapOrArray.buffer && UTF8Decoder) {
        return UTF8Decoder.decode(heapOrArray.subarray(idx, endPtr));
      }
      var str = '';
      while (idx < endPtr) {
        // For UTF8 byte structure, see:
        // http://en.wikipedia.org/wiki/UTF-8#Description
        // https://www.ietf.org/rfc/rfc2279.txt
        // https://tools.ietf.org/html/rfc3629
        var u0 = heapOrArray[idx++];
        if (!(u0 & 0x80)) { str += String.fromCharCode(u0); continue; }
        var u1 = heapOrArray[idx++] & 63;
        if ((u0 & 0xE0) == 0xC0) { str += String.fromCharCode(((u0 & 31) << 6) | u1); continue; }
        var u2 = heapOrArray[idx++] & 63;
        if ((u0 & 0xF0) == 0xE0) {
          u0 = ((u0 & 15) << 12) | (u1 << 6) | u2;
        } else {
          if ((u0 & 0xF8) != 0xF0) warnOnce(`Invalid UTF-8 leading byte ${ptrToString(u0)} encountered when deserializing a UTF-8 string in wasm memory to a JS string!`);
          u0 = ((u0 & 7) << 18) | (u1 << 12) | (u2 << 6) | (heapOrArray[idx++] & 63);
        }
  
        if (u0 < 0x10000) {
          str += String.fromCharCode(u0);
        } else {
          var ch = u0 - 0x10000;
          str += String.fromCharCode(0xD800 | (ch >> 10), 0xDC00 | (ch & 0x3FF));
        }
      }
      return str;
    };
  var printChar = (stream, curr) => {
      var buffer = printCharBuffers[stream];
      assert(buffer);
      if (curr === 0 || curr === 10) {
        (stream === 1 ? out : err)(UTF8ArrayToString(buffer));
        buffer.length = 0;
      } else {
        buffer.push(curr);
      }
    };
  
  var flush_NO_FILESYSTEM = () => {
      // flush anything remaining in the buffers during shutdown
      _fflush(0);
      if (printCharBuffers[1].length) printChar(1, 10);
      if (printCharBuffers[2].length) printChar(2, 10);
    };
  
  
  
    /**
   * Given a pointer 'ptr' to a null-terminated UTF8-encoded string in the
   * emscripten HEAP, returns a copy of that string as a Javascript String object.
   *
   * @param {number} ptr
   * @param {number=} maxBytesToRead - An optional length that specifies the
   *   maximum number of bytes to read. You can omit this parameter to scan the
   *   string until the first 0 byte. If maxBytesToRead is passed, and the string
   *   at [ptr, ptr+maxBytesToReadr[ contains a null byte in the middle, then the
   *   string will cut short at that byte index.
   * @param {boolean=} ignoreNul - If true, the function will not stop on a NUL character.
   * @return {string}
   */
  var UTF8ToString = (ptr, maxBytesToRead, ignoreNul) => {
      assert(typeof ptr == 'number', `UTF8ToString expects a number (got ${typeof ptr})`);
      return ptr ? UTF8ArrayToString(HEAPU8, ptr, maxBytesToRead, ignoreNul) : '';
    };
  var SYSCALLS = {
  varargs:undefined,
  getStr(ptr) {
        var ret = UTF8ToString(ptr);
        return ret;
      },
  };
  var _fd_write = (fd, iov, iovcnt, pnum) => {
      // hack to support printf in SYSCALLS_REQUIRE_FILESYSTEM=0
      var num = 0;
      for (var i = 0; i < iovcnt; i++) {
        var ptr = HEAPU32[((iov)>>2)];
        var len = HEAPU32[(((iov)+(4))>>2)];
        iov += 8;
        for (var j = 0; j < len; j++) {
          printChar(fd, HEAPU8[ptr+j]);
        }
        num += len;
      }
      HEAPU32[((pnum)>>2)] = num;
      return 0;
    };

  var getCFunc = (ident) => {
      var func = Module['_' + ident]; // closure exported function
      assert(func, `Cannot call unknown function ${ident}, make sure it is exported`);
      return func;
    };
  
  var writeArrayToMemory = (array, buffer) => {
      assert(array.length >= 0, 'writeArrayToMemory array must have a length (should be an array or typed array)')
      HEAP8.set(array, buffer);
    };
  
  var lengthBytesUTF8 = (str) => {
      var len = 0;
      for (var i = 0; i < str.length; ++i) {
        // Gotcha: charCodeAt returns a 16-bit word that is a UTF-16 encoded code
        // unit, not a Unicode code point of the character! So decode
        // UTF16->UTF32->UTF8.
        // See http://unicode.org/faq/utf_bom.html#utf16-3
        var c = str.charCodeAt(i); // possibly a lead surrogate
        if (c <= 0x7F) {
          len++;
        } else if (c <= 0x7FF) {
          len += 2;
        } else if (c >= 0xD800 && c <= 0xDFFF) {
          len += 4; ++i;
        } else {
          len += 3;
        }
      }
      return len;
    };
  
  var stringToUTF8Array = (str, heap, outIdx, maxBytesToWrite) => {
      assert(typeof str === 'string', `stringToUTF8Array expects a string (got ${typeof str})`);
      // Parameter maxBytesToWrite is not optional. Negative values, 0, null,
      // undefined and false each don't write out any bytes.
      if (!(maxBytesToWrite > 0))
        return 0;
  
      var startIdx = outIdx;
      var endIdx = outIdx + maxBytesToWrite - 1; // -1 for string null terminator.
      for (var i = 0; i < str.length; ++i) {
        // For UTF8 byte structure, see http://en.wikipedia.org/wiki/UTF-8#Description
        // and https://www.ietf.org/rfc/rfc2279.txt
        // and https://tools.ietf.org/html/rfc3629
        var u = str.codePointAt(i);
        if (u <= 0x7F) {
          if (outIdx >= endIdx) break;
          heap[outIdx++] = u;
        } else if (u <= 0x7FF) {
          if (outIdx + 1 >= endIdx) break;
          heap[outIdx++] = 0xC0 | (u >> 6);
          heap[outIdx++] = 0x80 | (u & 63);
        } else if (u <= 0xFFFF) {
          if (outIdx + 2 >= endIdx) break;
          heap[outIdx++] = 0xE0 | (u >> 12);
          heap[outIdx++] = 0x80 | ((u >> 6) & 63);
          heap[outIdx++] = 0x80 | (u & 63);
        } else {
          if (outIdx + 3 >= endIdx) break;
          if (u > 0x10FFFF) warnOnce(`Invalid Unicode code point ${ptrToString(u)} encountered when serializing a JS string to a UTF-8 string in wasm memory! (Valid unicode code points should be in range 0-0x10FFFF).`);
          heap[outIdx++] = 0xF0 | (u >> 18);
          heap[outIdx++] = 0x80 | ((u >> 12) & 63);
          heap[outIdx++] = 0x80 | ((u >> 6) & 63);
          heap[outIdx++] = 0x80 | (u & 63);
          // Gotcha: if codePoint is over 0xFFFF, it is represented as a surrogate pair in UTF-16.
          // We need to manually skip over the second code unit for correct iteration.
          i++;
        }
      }
      // Null-terminate the pointer to the buffer.
      heap[outIdx] = 0;
      return outIdx - startIdx;
    };
  var stringToUTF8 = (str, outPtr, maxBytesToWrite) => {
      assert(typeof maxBytesToWrite == 'number', 'stringToUTF8 requires a third parameter that specifies the length of the output buffer');
      return stringToUTF8Array(str, HEAPU8, outPtr, maxBytesToWrite);
    };
  
  var stackAlloc = (sz) => __emscripten_stack_alloc(sz);
  var stringToUTF8OnStack = (str) => {
      var size = lengthBytesUTF8(str) + 1;
      var ret = stackAlloc(size);
      stringToUTF8(str, ret, size);
      return ret;
    };
  
  
  
  
  
    /**
   * @param {string|null=} returnType
   * @param {Array=} argTypes
   * @param {Array=} args
   * @param {Object=} opts
   */
  var ccall = (ident, returnType, argTypes, args, opts) => {
      // For fast lookup of conversion functions
      var toC = {
        'string': (str) => {
          var ret = 0;
          if (str !== null && str !== undefined && str !== 0) { // null string
            ret = stringToUTF8OnStack(str);
          }
          return ret;
        },
        'array': (arr) => {
          var ret = stackAlloc(arr.length);
          writeArrayToMemory(arr, ret);
          return ret;
        }
      };
  
      function convertReturnValue(ret) {
        if (returnType === 'string') {
          return UTF8ToString(ret);
        }
        if (returnType === 'boolean') return Boolean(ret);
        return ret;
      }
  
      var func = getCFunc(ident);
      var cArgs = [];
      var stack = 0;
      assert(returnType !== 'array', 'return type should not be "array"');
      if (args) {
        for (var i = 0; i < args.length; i++) {
          var converter = toC[argTypes[i]];
          if (converter) {
            if (stack === 0) stack = stackSave();
            cArgs[i] = converter(args[i]);
          } else {
            cArgs[i] = args[i];
          }
        }
      }
      var ret = func(...cArgs);
      function onDone(ret) {
        if (stack !== 0) stackRestore(stack);
        return convertReturnValue(ret);
      }
  
      ret = onDone(ret);
      return ret;
    };
  
    /**
   * @param {string=} returnType
   * @param {Array=} argTypes
   * @param {Object=} opts
   */
  var cwrap = (ident, returnType, argTypes, opts) => {
      return (...args) => ccall(ident, returnType, argTypes, args, opts);
    };


// End JS library code

// include: postlibrary.js
// This file is included after the automatically-generated JS library code
// but before the wasm module is created.

{

  // Begin ATMODULES hooks
  if (Module['noExitRuntime']) noExitRuntime = Module['noExitRuntime'];
if (Module['print']) out = Module['print'];
if (Module['printErr']) err = Module['printErr'];

Module['FS_createDataFile'] = FS.createDataFile;
Module['FS_createPreloadedFile'] = FS.createPreloadedFile;

  // End ATMODULES hooks

  checkIncomingModuleAPI();

  if (Module['arguments']) programArgs = Module['arguments'];
  if (Module['thisProgram']) thisProgram = Module['thisProgram'];

  // Assertions on removed incoming Module JS APIs.
  assert(typeof Module['memoryInitializerPrefixURL'] == 'undefined', 'Module.memoryInitializerPrefixURL option was removed, use Module.locateFile instead');
  assert(typeof Module['pthreadMainPrefixURL'] == 'undefined', 'Module.pthreadMainPrefixURL option was removed, use Module.locateFile instead');
  assert(typeof Module['cdInitializerPrefixURL'] == 'undefined', 'Module.cdInitializerPrefixURL option was removed, use Module.locateFile instead');
  assert(typeof Module['filePackagePrefixURL'] == 'undefined', 'Module.filePackagePrefixURL option was removed, use Module.locateFile instead');
  assert(typeof Module['read'] == 'undefined', 'Module.read option was removed');
  assert(typeof Module['readAsync'] == 'undefined', 'Module.readAsync option was removed (modify readAsync in JS)');
  assert(typeof Module['readBinary'] == 'undefined', 'Module.readBinary option was removed (modify readBinary in JS)');
  assert(typeof Module['setWindowTitle'] == 'undefined', 'Module.setWindowTitle option was removed (modify emscripten_set_window_title in JS)');
  assert(typeof Module['TOTAL_MEMORY'] == 'undefined', 'Module.TOTAL_MEMORY has been renamed Module.INITIAL_MEMORY');
  assert(typeof Module['ENVIRONMENT'] == 'undefined', 'Module.ENVIRONMENT has been deprecated. To force the environment, use the ENVIRONMENT compile-time option (for example, -sENVIRONMENT=web or -sENVIRONMENT=node)');
  assert(typeof Module['STACK_SIZE'] == 'undefined', 'STACK_SIZE can no longer be set at runtime.  Use -sSTACK_SIZE at link time')
  // If memory is defined in wasm, the user can't provide it, or set INITIAL_MEMORY
  assert(typeof Module['wasmMemory'] == 'undefined', 'Use of `wasmMemory` detected.  Use -sIMPORTED_MEMORY to define wasmMemory externally');
  assert(typeof Module['INITIAL_MEMORY'] == 'undefined', 'Detected runtime INITIAL_MEMORY setting.  Use -sIMPORTED_MEMORY to define wasmMemory dynamically');

  var preInit = Module['preInit'];
  if (preInit) {
    if (typeof preInit == 'function') Module['preInit'] = preInit = [preInit];
    // Written as a loop so that preInit functions that themselves add more
    // preInit functions.  Is this actually needed?
    while (preInit.length > 0) {
      preInit.shift()();
    }
  }
  consumedModuleProp('preInit');
}

// Begin runtime exports
  Module['ccall'] = ccall;
  Module['cwrap'] = cwrap;
  Module['UTF8ToString'] = UTF8ToString;
  var missingLibrarySymbols = [
  'writeI53ToI64',
  'writeI53ToI64Clamped',
  'writeI53ToI64Signaling',
  'writeI53ToU64Clamped',
  'writeI53ToU64Signaling',
  'readI53FromI64',
  'readI53FromU64',
  'convertI32PairToI53',
  'convertI32PairToI53Checked',
  'convertU32PairToI53',
  'bigintToI53Checked',
  'getTempRet0',
  'setTempRet0',
  'createNamedFunction',
  'zeroMemory',
  'exitJS',
  'getHeapMax',
  'abortOnCannotGrowMemory',
  'growMemory',
  'withStackSave',
  'strError',
  'inetPton4',
  'inetNtop4',
  'inetPton6',
  'inetNtop6',
  'readSockaddr',
  'writeSockaddr',
  'readEmAsmArgs',
  'jstoi_q',
  'getExecutableName',
  'autoResumeAudioContext',
  'getDynCaller',
  'dynCall',
  'handleException',
  'keepRuntimeAlive',
  'runtimeKeepalivePush',
  'runtimeKeepalivePop',
  'callUserCallback',
  'maybeExit',
  'asyncLoad',
  'asmjsMangle',
  'alignMemory',
  'mmapAlloc',
  'HandleAllocator',
  'getUniqueRunDependency',
  'addRunDependency',
  'removeRunDependency',
  'addOnInit',
  'addOnPostCtor',
  'addOnPreMain',
  'addOnExit',
  'STACK_SIZE',
  'STACK_ALIGN',
  'POINTER_SIZE',
  'ASSERTIONS',
  'convertJsFunctionToWasm',
  'getEmptyTableSlot',
  'updateTableMap',
  'getFunctionAddress',
  'addFunction',
  'removeFunction',
  'intArrayFromString',
  'intArrayToString',
  'AsciiToString',
  'stringToAscii',
  'UTF16ToString',
  'stringToUTF16',
  'lengthBytesUTF16',
  'UTF32ToString',
  'stringToUTF32',
  'lengthBytesUTF32',
  'stringToNewUTF8',
  'registerKeyEventCallback',
  'maybeCStringToJsString',
  'findEventTarget',
  'getBoundingClientRect',
  'fillMouseEventData',
  'registerMouseEventCallback',
  'registerWheelEventCallback',
  'registerUiEventCallback',
  'registerFocusEventCallback',
  'fillDeviceOrientationEventData',
  'registerDeviceOrientationEventCallback',
  'fillDeviceMotionEventData',
  'registerDeviceMotionEventCallback',
  'screenOrientation',
  'fillOrientationChangeEventData',
  'registerOrientationChangeEventCallback',
  'fillFullscreenChangeEventData',
  'registerFullscreenChangeEventCallback',
  'JSEvents_requestFullscreen',
  'JSEvents_resizeCanvasForFullscreen',
  'registerRestoreOldStyle',
  'hideEverythingExceptGivenElement',
  'restoreHiddenElements',
  'setLetterbox',
  'softFullscreenResizeWebGLRenderTarget',
  'doRequestFullscreen',
  'fillPointerlockChangeEventData',
  'registerPointerlockChangeEventCallback',
  'registerPointerlockErrorEventCallback',
  'requestPointerLock',
  'fillVisibilityChangeEventData',
  'registerVisibilityChangeEventCallback',
  'registerTouchEventCallback',
  'fillGamepadEventData',
  'registerGamepadEventCallback',
  'registerBeforeUnloadEventCallback',
  'fillBatteryEventData',
  'registerBatteryEventCallback',
  'setCanvasElementSize',
  'getCanvasElementSize',
  'jsStackTrace',
  'getCallstack',
  'convertPCtoSourceLocation',
  'getEnvStrings',
  'checkWasiClock',
  'wasiRightsToMuslOFlags',
  'wasiOFlagsToMuslOFlags',
  'initRandomFill',
  'randomFill',
  'safeSetTimeout',
  'setImmediateWrapped',
  'safeRequestAnimationFrame',
  'clearImmediateWrapped',
  'registerPostMainLoop',
  'registerPreMainLoop',
  'getPromise',
  'makePromise',
  'addPromise',
  'idsToPromises',
  'makePromiseCallback',
  'ExceptionInfo',
  'findMatchingCatch',
  'incrementUncaughtExceptionCount',
  'decrementUncaughtExceptionCount',
  'Browser_asyncPrepareDataCounter',
  'isLeapYear',
  'ydayFromDate',
  'arraySum',
  'addDays',
  'getSocketFromFD',
  'getSocketAddress',
  'FS_createPreloadedFile',
  'FS_preloadFile',
  'FS_modeStringToFlags',
  'FS_getMode',
  'FS_fileDataToTypedArray',
  'FS_stdin_getChar',
  'FS_mkdirTree',
  '_setNetworkCallback',
  'heapObjectForWebGLType',
  'toTypedArrayIndex',
  'webgl_enable_ANGLE_instanced_arrays',
  'webgl_enable_OES_vertex_array_object',
  'webgl_enable_WEBGL_draw_buffers',
  'webgl_enable_WEBGL_multi_draw',
  'webgl_enable_EXT_polygon_offset_clamp',
  'webgl_enable_EXT_clip_control',
  'webgl_enable_WEBGL_polygon_mode',
  'emscriptenWebGLGet',
  'computeUnpackAlignedImageSize',
  'colorChannelsInGlTextureFormat',
  'emscriptenWebGLGetTexPixelData',
  'emscriptenWebGLGetUniform',
  'webglGetProgramUniformLocation',
  'webglGetUniformLocation',
  'webglPrepareUniformLocationsBeforeFirstUse',
  'webglGetLeftBracePos',
  'emscriptenWebGLGetVertexAttrib',
  '__glGetActiveAttribOrUniform',
  'writeGLArray',
  'registerWebGlEventCallback',
  'runAndAbortIfError',
  'ALLOC_NORMAL',
  'ALLOC_STACK',
  'allocate',
  'writeStringToMemory',
  'writeAsciiToMemory',
  'allocateUTF8',
  'allocateUTF8OnStack',
  'demangle',
  'stackTrace',
  'getNativeTypeSize',
];
missingLibrarySymbols.forEach(missingLibrarySymbol)

  var unexportedSymbols = [
  'run',
  'out',
  'err',
  'callMain',
  'abort',
  'wasmExports',
  'writeStackCookie',
  'checkStackCookie',
  'INT53_MAX',
  'INT53_MIN',
  'HEAP8',
  'HEAPU8',
  'HEAP16',
  'HEAPU16',
  'HEAP32',
  'HEAPU32',
  'HEAPF32',
  'HEAPF64',
  'HEAP64',
  'HEAPU64',
  'stackSave',
  'stackRestore',
  'stackAlloc',
  'ptrToString',
  'ENV',
  'ERRNO_CODES',
  'DNS',
  'Protocols',
  'Sockets',
  'timers',
  'warnOnce',
  'readEmAsmArgsArray',
  'wasmTable',
  'wasmMemory',
  'noExitRuntime',
  'addOnPreRun',
  'addOnPostRun',
  'freeTableIndexes',
  'functionsInTableMap',
  'setValue',
  'getValue',
  'PATH',
  'PATH_FS',
  'UTF8Decoder',
  'UTF8ArrayToString',
  'stringToUTF8Array',
  'stringToUTF8',
  'lengthBytesUTF8',
  'UTF16Decoder',
  'stringToUTF8OnStack',
  'writeArrayToMemory',
  'JSEvents',
  'specialHTMLTargets',
  'findCanvasEventTarget',
  'currentFullscreenStrategy',
  'restoreOldWindowedStyle',
  'UNWIND_CACHE',
  'ExitStatus',
  'flush_NO_FILESYSTEM',
  'emSetImmediate',
  'emClearImmediate_deps',
  'emClearImmediate',
  'promiseMap',
  'uncaughtExceptionCount',
  'exceptionCaught',
  'Browser',
  'requestFullscreen',
  'requestFullScreen',
  'setCanvasSize',
  'getUserMedia',
  'createContext',
  'getPreloadedImageData__data',
  'wget',
  'MONTH_DAYS_REGULAR',
  'MONTH_DAYS_LEAP',
  'MONTH_DAYS_REGULAR_CUMULATIVE',
  'MONTH_DAYS_LEAP_CUMULATIVE',
  'SYSCALLS',
  'preloadPlugins',
  'FS_stdin_getChar_buffer',
  'FS_unlink',
  'FS_createPath',
  'FS_createDevice',
  'FS_readFile',
  'FS',
  'FS_root',
  'FS_mounts',
  'FS_devices',
  'FS_streams',
  'FS_nextInode',
  'FS_nameTable',
  'FS_currentPath',
  'FS_initialized',
  'FS_ignorePermissions',
  'FS_filesystems',
  'FS_syncFSRequests',
  'FS_lookupPath',
  'FS_getPath',
  'FS_hashName',
  'FS_hashAddNode',
  'FS_hashRemoveNode',
  'FS_lookupNode',
  'FS_createNode',
  'FS_destroyNode',
  'FS_isRoot',
  'FS_isMountpoint',
  'FS_isFile',
  'FS_isDir',
  'FS_isLink',
  'FS_isChrdev',
  'FS_isBlkdev',
  'FS_isFIFO',
  'FS_isSocket',
  'FS_flagsToPermissionString',
  'FS_nodePermissions',
  'FS_mayLookup',
  'FS_mayCreate',
  'FS_mayDelete',
  'FS_mayOpen',
  'FS_checkOpExists',
  'FS_nextfd',
  'FS_getStreamChecked',
  'FS_getStream',
  'FS_createStream',
  'FS_closeStream',
  'FS_dupStream',
  'FS_doSetAttr',
  'FS_chrdev_stream_ops',
  'FS_major',
  'FS_minor',
  'FS_makedev',
  'FS_registerDevice',
  'FS_getDevice',
  'FS_getMounts',
  'FS_syncfs',
  'FS_mount',
  'FS_unmount',
  'FS_lookup',
  'FS_mknod',
  'FS_statfs',
  'FS_statfsStream',
  'FS_statfsNode',
  'FS_create',
  'FS_mkdir',
  'FS_mkdev',
  'FS_symlink',
  'FS_rename',
  'FS_rmdir',
  'FS_readdir',
  'FS_readlink',
  'FS_stat',
  'FS_fstat',
  'FS_lstat',
  'FS_doChmod',
  'FS_chmod',
  'FS_lchmod',
  'FS_fchmod',
  'FS_doChown',
  'FS_chown',
  'FS_lchown',
  'FS_fchown',
  'FS_doTruncate',
  'FS_truncate',
  'FS_ftruncate',
  'FS_utime',
  'FS_open',
  'FS_close',
  'FS_isClosed',
  'FS_llseek',
  'FS_read',
  'FS_write',
  'FS_mmap',
  'FS_msync',
  'FS_ioctl',
  'FS_writeFile',
  'FS_cwd',
  'FS_chdir',
  'FS_createDefaultDirectories',
  'FS_createDefaultDevices',
  'FS_createSpecialDirectories',
  'FS_createStandardStreams',
  'FS_staticInit',
  'FS_init',
  'FS_quit',
  'FS_findObject',
  'FS_analyzePath',
  'FS_createFile',
  'FS_createDataFile',
  'FS_forceLoadFile',
  'FS_createLazyFile',
  'MEMFS',
  'TTY',
  'PIPEFS',
  'SOCKFS',
  'tempFixedLengthArray',
  'miniTempWebGLFloatBuffers',
  'miniTempWebGLIntBuffers',
  'GL',
  'AL',
  'GLUT',
  'EGL',
  'GLEW',
  'IDBStore',
  'SDL',
  'SDL_gfx',
  'print',
  'printErr',
  'jstoi_s',
];
unexportedSymbols.forEach(unexportedRuntimeSymbol);

  // End runtime exports
  // Begin JS library exports
  // End JS library exports

// end include: postlibrary.js

function checkIncomingModuleAPI() {
  ignoredModuleProp('fetchSettings');
  ignoredModuleProp('logReadFiles');
  ignoredModuleProp('loadSplitModule');
  ignoredModuleProp('onMalloc');
  ignoredModuleProp('onRealloc');
  ignoredModuleProp('onFree');
  ignoredModuleProp('onSbrkGrow');
  ignoredModuleProp('onCOSCacheHit');
  ignoredModuleProp('onCOSCacheMiss');
  ignoredModuleProp('onCOSStore');
  ignoredModuleProp('GL_MAX_TEXTURE_IMAGE_UNITS');
  ignoredModuleProp('SDL_canPlayWithWebAudio');
  ignoredModuleProp('SDL_numSimultaneouslyQueuedBuffers');
  ignoredModuleProp('freePreloadedMediaOnUse');
  ignoredModuleProp('preinitializedWebGLContext');
  ignoredModuleProp('keyboardListeningElement');
  ignoredModuleProp('doNotCaptureKeyboard');
  ignoredModuleProp('extraStackTrace');
  ignoredModuleProp('preloadPlugins');
  ignoredModuleProp('preMainLoop');
  ignoredModuleProp('postMainLoop');
  ignoredModuleProp('forcedAspectRatio');
  ignoredModuleProp('mainScriptUrlOrBlob');
  ignoredModuleProp('onFullScreen');
  ignoredModuleProp('INITIAL_MEMORY');
  ignoredModuleProp('wasmMemory');
  ignoredModuleProp('wasmBinary');
}

// Imports from the Wasm binary.
var _zx16_reset = Module['_zx16_reset'] = makeInvalidEarlyAccess('_zx16_reset');
var _zx16_seed_rng = Module['_zx16_seed_rng'] = makeInvalidEarlyAccess('_zx16_seed_rng');
var _zx16_load_byte = Module['_zx16_load_byte'] = makeInvalidEarlyAccess('_zx16_load_byte');
var _zx16_read8 = Module['_zx16_read8'] = makeInvalidEarlyAccess('_zx16_read8');
var _zx16_write8 = Module['_zx16_write8'] = makeInvalidEarlyAccess('_zx16_write8');
var _zx16_read16 = Module['_zx16_read16'] = makeInvalidEarlyAccess('_zx16_read16');
var _zx16_write16 = Module['_zx16_write16'] = makeInvalidEarlyAccess('_zx16_write16');
var _zx16_set_pc = Module['_zx16_set_pc'] = makeInvalidEarlyAccess('_zx16_set_pc');
var _zx16_get_pc = Module['_zx16_get_pc'] = makeInvalidEarlyAccess('_zx16_get_pc');
var _zx16_set_sp = Module['_zx16_set_sp'] = makeInvalidEarlyAccess('_zx16_set_sp');
var _zx16_get_sp = Module['_zx16_get_sp'] = makeInvalidEarlyAccess('_zx16_get_sp');
var _zx16_get_register = Module['_zx16_get_register'] = makeInvalidEarlyAccess('_zx16_get_register');
var _zx16_set_register = Module['_zx16_set_register'] = makeInvalidEarlyAccess('_zx16_set_register');
var _zx16_step = Module['_zx16_step'] = makeInvalidEarlyAccess('_zx16_step');
var _zx16_step_with_breakpoints = Module['_zx16_step_with_breakpoints'] = makeInvalidEarlyAccess('_zx16_step_with_breakpoints');
var _zx16_step_over = Module['_zx16_step_over'] = makeInvalidEarlyAccess('_zx16_step_over');
var _zx16_run_to_cursor = Module['_zx16_run_to_cursor'] = makeInvalidEarlyAccess('_zx16_run_to_cursor');
var _zx16_run = Module['_zx16_run'] = makeInvalidEarlyAccess('_zx16_run');
var _zx16_is_halted = Module['_zx16_is_halted'] = makeInvalidEarlyAccess('_zx16_is_halted');
var _zx16_get_last_instruction = Module['_zx16_get_last_instruction'] = makeInvalidEarlyAccess('_zx16_get_last_instruction');
var _zx16_get_output = Module['_zx16_get_output'] = makeInvalidEarlyAccess('_zx16_get_output');
var _zx16_clear_output = Module['_zx16_clear_output'] = makeInvalidEarlyAccess('_zx16_clear_output');
var _zx16_toggle_breakpoint = Module['_zx16_toggle_breakpoint'] = makeInvalidEarlyAccess('_zx16_toggle_breakpoint');
var _zx16_has_breakpoint = Module['_zx16_has_breakpoint'] = makeInvalidEarlyAccess('_zx16_has_breakpoint');
var _zx16_get_breakpoint_count = Module['_zx16_get_breakpoint_count'] = makeInvalidEarlyAccess('_zx16_get_breakpoint_count');
var _zx16_clear_breakpoints = Module['_zx16_clear_breakpoints'] = makeInvalidEarlyAccess('_zx16_clear_breakpoints');
var _zx16_has_breakpoint_hit = Module['_zx16_has_breakpoint_hit'] = makeInvalidEarlyAccess('_zx16_has_breakpoint_hit');
var _zx16_get_breakpoint_hit_address = Module['_zx16_get_breakpoint_hit_address'] = makeInvalidEarlyAccess('_zx16_get_breakpoint_hit_address');
var _zx16_clear_breakpoint_hit = Module['_zx16_clear_breakpoint_hit'] = makeInvalidEarlyAccess('_zx16_clear_breakpoint_hit');
var _zx16_set_keyboard_key = Module['_zx16_set_keyboard_key'] = makeInvalidEarlyAccess('_zx16_set_keyboard_key');
var _zx16_get_keyboard_key = Module['_zx16_get_keyboard_key'] = makeInvalidEarlyAccess('_zx16_get_keyboard_key');
var _zx16_clear_keyboard_key = Module['_zx16_clear_keyboard_key'] = makeInvalidEarlyAccess('_zx16_clear_keyboard_key');
var _zx16_has_pending_tone = Module['_zx16_has_pending_tone'] = makeInvalidEarlyAccess('_zx16_has_pending_tone');
var _zx16_get_tone_frequency = Module['_zx16_get_tone_frequency'] = makeInvalidEarlyAccess('_zx16_get_tone_frequency');
var _zx16_get_tone_duration_ms = Module['_zx16_get_tone_duration_ms'] = makeInvalidEarlyAccess('_zx16_get_tone_duration_ms');
var _zx16_clear_tone_request = Module['_zx16_clear_tone_request'] = makeInvalidEarlyAccess('_zx16_clear_tone_request');
var _zx16_has_pending_stop_audio = Module['_zx16_has_pending_stop_audio'] = makeInvalidEarlyAccess('_zx16_has_pending_stop_audio');
var _zx16_clear_stop_audio_request = Module['_zx16_clear_stop_audio_request'] = makeInvalidEarlyAccess('_zx16_clear_stop_audio_request');
var _zx16_get_volume_percent = Module['_zx16_get_volume_percent'] = makeInvalidEarlyAccess('_zx16_get_volume_percent');
var _fflush = makeInvalidEarlyAccess('_fflush');
var _strerror = makeInvalidEarlyAccess('_strerror');
var _emscripten_stack_get_end = makeInvalidEarlyAccess('_emscripten_stack_get_end');
var _emscripten_stack_get_base = makeInvalidEarlyAccess('_emscripten_stack_get_base');
var _emscripten_stack_init = makeInvalidEarlyAccess('_emscripten_stack_init');
var _emscripten_stack_get_free = makeInvalidEarlyAccess('_emscripten_stack_get_free');
var __emscripten_stack_restore = makeInvalidEarlyAccess('__emscripten_stack_restore');
var __emscripten_stack_alloc = makeInvalidEarlyAccess('__emscripten_stack_alloc');
var _emscripten_stack_get_current = makeInvalidEarlyAccess('_emscripten_stack_get_current');
var memory = makeInvalidEarlyAccess('memory');
var __indirect_function_table = makeInvalidEarlyAccess('__indirect_function_table');
var wasmMemory = makeInvalidEarlyAccess('wasmMemory');

function assignWasmExports(wasmExports) {
  assert(typeof wasmExports['zx16_reset'] != 'undefined', 'missing Wasm export: zx16_reset');
  assert(typeof wasmExports['zx16_seed_rng'] != 'undefined', 'missing Wasm export: zx16_seed_rng');
  assert(typeof wasmExports['zx16_load_byte'] != 'undefined', 'missing Wasm export: zx16_load_byte');
  assert(typeof wasmExports['zx16_read8'] != 'undefined', 'missing Wasm export: zx16_read8');
  assert(typeof wasmExports['zx16_write8'] != 'undefined', 'missing Wasm export: zx16_write8');
  assert(typeof wasmExports['zx16_read16'] != 'undefined', 'missing Wasm export: zx16_read16');
  assert(typeof wasmExports['zx16_write16'] != 'undefined', 'missing Wasm export: zx16_write16');
  assert(typeof wasmExports['zx16_set_pc'] != 'undefined', 'missing Wasm export: zx16_set_pc');
  assert(typeof wasmExports['zx16_get_pc'] != 'undefined', 'missing Wasm export: zx16_get_pc');
  assert(typeof wasmExports['zx16_set_sp'] != 'undefined', 'missing Wasm export: zx16_set_sp');
  assert(typeof wasmExports['zx16_get_sp'] != 'undefined', 'missing Wasm export: zx16_get_sp');
  assert(typeof wasmExports['zx16_get_register'] != 'undefined', 'missing Wasm export: zx16_get_register');
  assert(typeof wasmExports['zx16_set_register'] != 'undefined', 'missing Wasm export: zx16_set_register');
  assert(typeof wasmExports['zx16_step'] != 'undefined', 'missing Wasm export: zx16_step');
  assert(typeof wasmExports['zx16_step_with_breakpoints'] != 'undefined', 'missing Wasm export: zx16_step_with_breakpoints');
  assert(typeof wasmExports['zx16_step_over'] != 'undefined', 'missing Wasm export: zx16_step_over');
  assert(typeof wasmExports['zx16_run_to_cursor'] != 'undefined', 'missing Wasm export: zx16_run_to_cursor');
  assert(typeof wasmExports['zx16_run'] != 'undefined', 'missing Wasm export: zx16_run');
  assert(typeof wasmExports['zx16_is_halted'] != 'undefined', 'missing Wasm export: zx16_is_halted');
  assert(typeof wasmExports['zx16_get_last_instruction'] != 'undefined', 'missing Wasm export: zx16_get_last_instruction');
  assert(typeof wasmExports['zx16_get_output'] != 'undefined', 'missing Wasm export: zx16_get_output');
  assert(typeof wasmExports['zx16_clear_output'] != 'undefined', 'missing Wasm export: zx16_clear_output');
  assert(typeof wasmExports['zx16_toggle_breakpoint'] != 'undefined', 'missing Wasm export: zx16_toggle_breakpoint');
  assert(typeof wasmExports['zx16_has_breakpoint'] != 'undefined', 'missing Wasm export: zx16_has_breakpoint');
  assert(typeof wasmExports['zx16_get_breakpoint_count'] != 'undefined', 'missing Wasm export: zx16_get_breakpoint_count');
  assert(typeof wasmExports['zx16_clear_breakpoints'] != 'undefined', 'missing Wasm export: zx16_clear_breakpoints');
  assert(typeof wasmExports['zx16_has_breakpoint_hit'] != 'undefined', 'missing Wasm export: zx16_has_breakpoint_hit');
  assert(typeof wasmExports['zx16_get_breakpoint_hit_address'] != 'undefined', 'missing Wasm export: zx16_get_breakpoint_hit_address');
  assert(typeof wasmExports['zx16_clear_breakpoint_hit'] != 'undefined', 'missing Wasm export: zx16_clear_breakpoint_hit');
  assert(typeof wasmExports['zx16_set_keyboard_key'] != 'undefined', 'missing Wasm export: zx16_set_keyboard_key');
  assert(typeof wasmExports['zx16_get_keyboard_key'] != 'undefined', 'missing Wasm export: zx16_get_keyboard_key');
  assert(typeof wasmExports['zx16_clear_keyboard_key'] != 'undefined', 'missing Wasm export: zx16_clear_keyboard_key');
  assert(typeof wasmExports['zx16_has_pending_tone'] != 'undefined', 'missing Wasm export: zx16_has_pending_tone');
  assert(typeof wasmExports['zx16_get_tone_frequency'] != 'undefined', 'missing Wasm export: zx16_get_tone_frequency');
  assert(typeof wasmExports['zx16_get_tone_duration_ms'] != 'undefined', 'missing Wasm export: zx16_get_tone_duration_ms');
  assert(typeof wasmExports['zx16_clear_tone_request'] != 'undefined', 'missing Wasm export: zx16_clear_tone_request');
  assert(typeof wasmExports['zx16_has_pending_stop_audio'] != 'undefined', 'missing Wasm export: zx16_has_pending_stop_audio');
  assert(typeof wasmExports['zx16_clear_stop_audio_request'] != 'undefined', 'missing Wasm export: zx16_clear_stop_audio_request');
  assert(typeof wasmExports['zx16_get_volume_percent'] != 'undefined', 'missing Wasm export: zx16_get_volume_percent');
  assert(typeof wasmExports['fflush'] != 'undefined', 'missing Wasm export: fflush');
  assert(typeof wasmExports['strerror'] != 'undefined', 'missing Wasm export: strerror');
  assert(typeof wasmExports['emscripten_stack_get_end'] != 'undefined', 'missing Wasm export: emscripten_stack_get_end');
  assert(typeof wasmExports['emscripten_stack_get_base'] != 'undefined', 'missing Wasm export: emscripten_stack_get_base');
  assert(typeof wasmExports['emscripten_stack_init'] != 'undefined', 'missing Wasm export: emscripten_stack_init');
  assert(typeof wasmExports['emscripten_stack_get_free'] != 'undefined', 'missing Wasm export: emscripten_stack_get_free');
  assert(typeof wasmExports['_emscripten_stack_restore'] != 'undefined', 'missing Wasm export: _emscripten_stack_restore');
  assert(typeof wasmExports['_emscripten_stack_alloc'] != 'undefined', 'missing Wasm export: _emscripten_stack_alloc');
  assert(typeof wasmExports['emscripten_stack_get_current'] != 'undefined', 'missing Wasm export: emscripten_stack_get_current');
  assert(typeof wasmExports['memory'] != 'undefined', 'missing Wasm export: memory');
  assert(typeof wasmExports['__indirect_function_table'] != 'undefined', 'missing Wasm export: __indirect_function_table');
  _zx16_reset = Module['_zx16_reset'] = createExportWrapper('zx16_reset', wasmExports['zx16_reset'], 0);
  _zx16_seed_rng = Module['_zx16_seed_rng'] = createExportWrapper('zx16_seed_rng', wasmExports['zx16_seed_rng'], 1);
  _zx16_load_byte = Module['_zx16_load_byte'] = createExportWrapper('zx16_load_byte', wasmExports['zx16_load_byte'], 2);
  _zx16_read8 = Module['_zx16_read8'] = createExportWrapper('zx16_read8', wasmExports['zx16_read8'], 1);
  _zx16_write8 = Module['_zx16_write8'] = createExportWrapper('zx16_write8', wasmExports['zx16_write8'], 2);
  _zx16_read16 = Module['_zx16_read16'] = createExportWrapper('zx16_read16', wasmExports['zx16_read16'], 1);
  _zx16_write16 = Module['_zx16_write16'] = createExportWrapper('zx16_write16', wasmExports['zx16_write16'], 2);
  _zx16_set_pc = Module['_zx16_set_pc'] = createExportWrapper('zx16_set_pc', wasmExports['zx16_set_pc'], 1);
  _zx16_get_pc = Module['_zx16_get_pc'] = createExportWrapper('zx16_get_pc', wasmExports['zx16_get_pc'], 0);
  _zx16_set_sp = Module['_zx16_set_sp'] = createExportWrapper('zx16_set_sp', wasmExports['zx16_set_sp'], 1);
  _zx16_get_sp = Module['_zx16_get_sp'] = createExportWrapper('zx16_get_sp', wasmExports['zx16_get_sp'], 0);
  _zx16_get_register = Module['_zx16_get_register'] = createExportWrapper('zx16_get_register', wasmExports['zx16_get_register'], 1);
  _zx16_set_register = Module['_zx16_set_register'] = createExportWrapper('zx16_set_register', wasmExports['zx16_set_register'], 2);
  _zx16_step = Module['_zx16_step'] = createExportWrapper('zx16_step', wasmExports['zx16_step'], 0);
  _zx16_step_with_breakpoints = Module['_zx16_step_with_breakpoints'] = createExportWrapper('zx16_step_with_breakpoints', wasmExports['zx16_step_with_breakpoints'], 0);
  _zx16_step_over = Module['_zx16_step_over'] = createExportWrapper('zx16_step_over', wasmExports['zx16_step_over'], 0);
  _zx16_run_to_cursor = Module['_zx16_run_to_cursor'] = createExportWrapper('zx16_run_to_cursor', wasmExports['zx16_run_to_cursor'], 1);
  _zx16_run = Module['_zx16_run'] = createExportWrapper('zx16_run', wasmExports['zx16_run'], 1);
  _zx16_is_halted = Module['_zx16_is_halted'] = createExportWrapper('zx16_is_halted', wasmExports['zx16_is_halted'], 0);
  _zx16_get_last_instruction = Module['_zx16_get_last_instruction'] = createExportWrapper('zx16_get_last_instruction', wasmExports['zx16_get_last_instruction'], 0);
  _zx16_get_output = Module['_zx16_get_output'] = createExportWrapper('zx16_get_output', wasmExports['zx16_get_output'], 0);
  _zx16_clear_output = Module['_zx16_clear_output'] = createExportWrapper('zx16_clear_output', wasmExports['zx16_clear_output'], 0);
  _zx16_toggle_breakpoint = Module['_zx16_toggle_breakpoint'] = createExportWrapper('zx16_toggle_breakpoint', wasmExports['zx16_toggle_breakpoint'], 1);
  _zx16_has_breakpoint = Module['_zx16_has_breakpoint'] = createExportWrapper('zx16_has_breakpoint', wasmExports['zx16_has_breakpoint'], 1);
  _zx16_get_breakpoint_count = Module['_zx16_get_breakpoint_count'] = createExportWrapper('zx16_get_breakpoint_count', wasmExports['zx16_get_breakpoint_count'], 0);
  _zx16_clear_breakpoints = Module['_zx16_clear_breakpoints'] = createExportWrapper('zx16_clear_breakpoints', wasmExports['zx16_clear_breakpoints'], 0);
  _zx16_has_breakpoint_hit = Module['_zx16_has_breakpoint_hit'] = createExportWrapper('zx16_has_breakpoint_hit', wasmExports['zx16_has_breakpoint_hit'], 0);
  _zx16_get_breakpoint_hit_address = Module['_zx16_get_breakpoint_hit_address'] = createExportWrapper('zx16_get_breakpoint_hit_address', wasmExports['zx16_get_breakpoint_hit_address'], 0);
  _zx16_clear_breakpoint_hit = Module['_zx16_clear_breakpoint_hit'] = createExportWrapper('zx16_clear_breakpoint_hit', wasmExports['zx16_clear_breakpoint_hit'], 0);
  _zx16_set_keyboard_key = Module['_zx16_set_keyboard_key'] = createExportWrapper('zx16_set_keyboard_key', wasmExports['zx16_set_keyboard_key'], 1);
  _zx16_get_keyboard_key = Module['_zx16_get_keyboard_key'] = createExportWrapper('zx16_get_keyboard_key', wasmExports['zx16_get_keyboard_key'], 0);
  _zx16_clear_keyboard_key = Module['_zx16_clear_keyboard_key'] = createExportWrapper('zx16_clear_keyboard_key', wasmExports['zx16_clear_keyboard_key'], 0);
  _zx16_has_pending_tone = Module['_zx16_has_pending_tone'] = createExportWrapper('zx16_has_pending_tone', wasmExports['zx16_has_pending_tone'], 0);
  _zx16_get_tone_frequency = Module['_zx16_get_tone_frequency'] = createExportWrapper('zx16_get_tone_frequency', wasmExports['zx16_get_tone_frequency'], 0);
  _zx16_get_tone_duration_ms = Module['_zx16_get_tone_duration_ms'] = createExportWrapper('zx16_get_tone_duration_ms', wasmExports['zx16_get_tone_duration_ms'], 0);
  _zx16_clear_tone_request = Module['_zx16_clear_tone_request'] = createExportWrapper('zx16_clear_tone_request', wasmExports['zx16_clear_tone_request'], 0);
  _zx16_has_pending_stop_audio = Module['_zx16_has_pending_stop_audio'] = createExportWrapper('zx16_has_pending_stop_audio', wasmExports['zx16_has_pending_stop_audio'], 0);
  _zx16_clear_stop_audio_request = Module['_zx16_clear_stop_audio_request'] = createExportWrapper('zx16_clear_stop_audio_request', wasmExports['zx16_clear_stop_audio_request'], 0);
  _zx16_get_volume_percent = Module['_zx16_get_volume_percent'] = createExportWrapper('zx16_get_volume_percent', wasmExports['zx16_get_volume_percent'], 0);
  _fflush = createExportWrapper('fflush', wasmExports['fflush'], 1);
  _strerror = createExportWrapper('strerror', wasmExports['strerror'], 1);
  _emscripten_stack_get_end = wasmExports['emscripten_stack_get_end'];
  _emscripten_stack_get_base = wasmExports['emscripten_stack_get_base'];
  _emscripten_stack_init = wasmExports['emscripten_stack_init'];
  _emscripten_stack_get_free = wasmExports['emscripten_stack_get_free'];
  __emscripten_stack_restore = wasmExports['_emscripten_stack_restore'];
  __emscripten_stack_alloc = wasmExports['_emscripten_stack_alloc'];
  _emscripten_stack_get_current = wasmExports['emscripten_stack_get_current'];
  memory = wasmMemory = wasmExports['memory'];
  __indirect_function_table = wasmExports['__indirect_function_table'];
}

var wasmImports = {
  /** @export */
  fd_write: _fd_write
};


// include: postamble.js
// === Auto-generated postamble setup entry stuff ===

var calledRun;

function stackCheckInit() {
  // This is normally called automatically during __wasm_call_ctors but need to
  // get these values before even running any of the ctors so we call it redundantly
  // here.
  _emscripten_stack_init();
  // TODO(sbc): Move writeStackCookie to native to to avoid this.
  writeStackCookie();
}

async function run() {
  assert(!calledRun);
  calledRun = true;

  stackCheckInit();

  preRun();

  var setStatus = Module['setStatus'];
  if (setStatus) {
    setStatus('Running...');
    // Yield to the event loop to allow the browser to paint "Running..."
    await new Promise((resolve) => setTimeout(resolve, 1));
    // Then we want to clear the status text, but only after the rest of this function runs.
    setTimeout(setStatus, 1, '');
  }

  if (ABORT) return;

  initRuntime();

  Module['onRuntimeInitialized']?.();
  consumedModuleProp('onRuntimeInitialized');

  assert(!Module['_main'], 'compiled without a main, but one is present. if you added it from JS, use Module["onRuntimeInitialized"]');

  postRun();
}

function checkUnflushedContent() {
  // Compiler settings do not allow exiting the runtime, so flushing
  // the streams is not possible. but in ASSERTIONS mode we check
  // if there was something to flush, and if so tell the user they
  // should request that the runtime be exitable.
  // Normally we would not even include flush() at all, but in ASSERTIONS
  // builds we do so just for this check, and here we see if there is any
  // content to flush, that is, we check if there would have been
  // something a non-ASSERTIONS build would have not seen.
  // How we flush the streams depends on whether we are in SYSCALLS_REQUIRE_FILESYSTEM=0
  // mode (which has its own special function for this; otherwise, all
  // the code is inside libc)
  var oldOut = out;
  var oldErr = err;
  var has = false;
  out = err = (x) => {
    has = true;
  }
  try { // it doesn't matter if it fails
    flush_NO_FILESYSTEM();
  } catch(e) {}
  out = oldOut;
  err = oldErr;
  if (has) {
    warnOnce('stdio streams had content in them that was not flushed. you should set EXIT_RUNTIME to 1 (see the Emscripten FAQ), or make sure to emit a newline when you printf etc.');
    warnOnce('(this may also be due to not including full filesystem support - try building with -sFORCE_FILESYSTEM)');
  }
}

var wasmExports;

// In modularize mode the generated code is within a factory function so we
// can use await here (since it's not top-level-await).
wasmExports = await createWasm();
await run();

// end include: postamble.js

// include: postamble_modularize.js
// In MODULARIZE mode we wrap the generated code in a factory function
// and return either the Module itself, or a promise of the module.

// Assertion for attempting to access module properties on the incoming
// moduleArg.  In the past we used this object as the prototype of the module
// and assigned properties to it, but now we return a distinct object.  This
// keeps the instance private until it is ready (i.e the promise has been
// resolved).
for (const prop of Object.keys(Module)) {
  if (!(prop in moduleArg)) {
    Object.defineProperty(moduleArg, prop, {
      configurable: true,
      get() {
        abort(`Access to module property ('${prop}') is no longer possible via the module constructor argument; Instead, use the result of the module constructor.`)
      }
    });
  }
}
// end include: postamble_modularize.js



    return Module;
  };
})();

// Export using a UMD style export, or ES6 exports if selected
if (typeof exports === 'object' && typeof module === 'object') {
  module.exports = createZx16Backend;
  // This default export looks redundant, but it allows TS to import this
  // commonjs style module.
  module.exports.default = createZx16Backend;
} else if (typeof define === 'function' && define['amd'])
  define([], () => createZx16Backend);

