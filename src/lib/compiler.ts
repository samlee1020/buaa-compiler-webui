export type CompilerResult = {
  error?: string;
  symbol?: string;
  llvm_ir?: string;
  mips?: string;
};

type WorkerRequest =
  | { id: number; type: "init" }
  | { id: number; type: "compile"; payload: { optimize: boolean; sourceCode: string } };

type WorkerResponse =
  | { id: number; type: "init:success" }
  | { id: number; type: "compile:success"; payload: CompilerResult }
  | { id: number; type: "error"; error: string };

type PendingRequest = {
  resolve: (value: CompilerResult | void) => void;
  reject: (reason?: unknown) => void;
};

let workerInstance: Worker | null = null;
let requestId = 0;
const pendingRequests = new Map<number, PendingRequest>();

function getWorker(): Worker {
  if (!workerInstance) {
    workerInstance = new Worker(
      new URL("../workers/compiler.worker.ts", import.meta.url),
      { type: "module" },
    );

    workerInstance.onmessage = (event: MessageEvent<WorkerResponse>) => {
      const message = event.data;
      const pending = pendingRequests.get(message.id);

      if (!pending) {
        return;
      }

      pendingRequests.delete(message.id);

      if (message.type === "error") {
        pending.reject(new Error(message.error));
        return;
      }

      if (message.type === "compile:success") {
        pending.resolve(message.payload);
        return;
      }

      pending.resolve();
    };

    workerInstance.onerror = (event) => {
      const error = event.error instanceof Error
        ? event.error
        : new Error(event.message || "Worker 运行失败。");

      for (const pending of pendingRequests.values()) {
        pending.reject(error);
      }

      pendingRequests.clear();
    };
  }

  return workerInstance;
}

function sendMessage(message: WorkerRequest): Promise<CompilerResult | void> {
  const worker = getWorker();

  return new Promise((resolve, reject) => {
    pendingRequests.set(message.id, { resolve, reject });
    worker.postMessage(message);
  });
}

let initPromise: Promise<void> | null = null;

export async function getCompilerModule(): Promise<void> {
  if (!initPromise) {
    initPromise = sendMessage({
      id: ++requestId,
      type: "init",
    }).then(() => undefined);
  }

  return initPromise;
}

export async function compileSource(
  sourceCode: string,
  optimize: boolean,
): Promise<CompilerResult> {
  await getCompilerModule();

  return sendMessage({
    id: ++requestId,
    type: "compile",
    payload: {
      optimize,
      sourceCode,
    },
  }) as Promise<CompilerResult>;
}
