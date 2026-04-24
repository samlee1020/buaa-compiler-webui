type CompilerResult = {
  error?: string;
  symbol?: string;
  llvm_ir?: string;
  mips?: string;
};

type CompilerModule = {
  compileToJson: (optimize: boolean, sourceCode: string) => string;
};

type CompilerFactory = (options?: {
  locateFile?: (path: string) => string;
}) => Promise<CompilerModule>;

type WorkerRequest =
  | { id: number; type: "init" }
  | { id: number; type: "compile"; payload: { optimize: boolean; sourceCode: string } };

type WorkerResponse =
  | { id: number; type: "init:success" }
  | { id: number; type: "compile:success"; payload: CompilerResult }
  | { id: number; type: "error"; error: string };

let modulePromise: Promise<CompilerModule> | null = null;

async function loadCompilerFactory(): Promise<CompilerFactory> {
  const compilerEntryUrl = new URL(
    "/wasm-compiler/compiler.js",
    self.location.origin,
  ).href;
  const module = await import(/* @vite-ignore */ compilerEntryUrl);
  return module.default as CompilerFactory;
}

async function getCompilerModule(): Promise<CompilerModule> {
  if (!modulePromise) {
    modulePromise = loadCompilerFactory().then((createCompilerModule) =>
      createCompilerModule({
        locateFile(path) {
          return `/wasm-compiler/${path}`;
        },
      }),
    );
  }

  return modulePromise;
}

function postMessageToMain(message: WorkerResponse) {
  self.postMessage(message);
}

self.onmessage = async (event: MessageEvent<WorkerRequest>) => {
  const message = event.data;

  try {
    if (message.type === "init") {
      await getCompilerModule();
      postMessageToMain({
        id: message.id,
        type: "init:success",
      });
      return;
    }

    const module = await getCompilerModule();
    const resultJson = module.compileToJson(
      message.payload.optimize,
      message.payload.sourceCode,
    );

    postMessageToMain({
      id: message.id,
      type: "compile:success",
      payload: JSON.parse(resultJson) as CompilerResult,
    });
  } catch (error: unknown) {
    postMessageToMain({
      id: message.id,
      type: "error",
      error: error instanceof Error ? error.message : "编译器 worker 执行失败。",
    });
  }
};
