import Editor from "@monaco-editor/react";
import { useEffect, useRef, useState } from "react";
import { compileSource, getCompilerModule, type CompilerResult } from "./lib/compiler";

const examples = [
  {
    id: "basic-function",
    name: "Basic Function",
    description: "函数定义、返回值与 printf 输出。",
    code: `const int N = 10;

int add(int a, int b) {
  return a + b;
}

int main() {
  int x = 3;
  int y = 4;
  int z = add(x, y);
  printf("%d\\n", z);
  return 0;
}`,
  },
  {
    id: "array-for",
    name: "Array + For",
    description: "一维数组、for 循环与数组传参。",
    code: `int sum(int arr[]) {
  int i = 0;
  int total = 0;
  for (i = 0; i < 5; i = i + 1) {
    total = total + arr[i];
  }
  return total;
}

int main() {
  int nums[5] = {1, 2, 3, 4, 5};
  printf("%d\\n", sum(nums));
  return 0;
}`,
  },
  {
    id: "if-short-circuit",
    name: "If + Short Circuit",
    description: "if / else、逻辑与或与短路求值。",
    code: `int counter = 0;

int touch() {
  counter = counter + 1;
  return 1;
}

int main() {
  if (0 && touch()) {
    printf("never\\n");
  } else {
    printf("%d\\n", counter);
  }

  if (1 || touch()) {
    printf("%d\\n", counter);
  }

  return 0;
}`,
  },
  {
    id: "static-local",
    name: "Static Local",
    description: "静态局部变量与多次函数调用。",
    code: `int next_value() {
  static int seed = 2;
  seed = seed + 3;
  return seed;
}

int main() {
  printf("%d\\n", next_value());
  printf("%d\\n", next_value());
  return 0;
}`,
  },
  {
    id: "missing-semicolon",
    name: "Missing Semicolon",
    description: "缺少分号，属于语法错误，会报错，但不影响编译结果输出。",
    code: `int main() {
  int a = 1
  printf("%d\\n", a);
  return 0;
}`,
  },
  {
    id: "undefined-variable",
    name: "Undefined Variable",
    description: "使用未定义变量，属于语义错误，会报错，不能输出编译结果。",
    code: `int main() {
  int a = 1;
  int b = a + c;
  printf("%d\\n", b);
  return 0;
}`,
  },
] as const;

const tabs = [
  { key: "error", label: "Error" },
  { key: "symbol", label: "Symbol" },
  { key: "llvm_ir", label: "LLVM IR" },
  { key: "mips", label: "MIPS" },
] as const;

type TabKey = (typeof tabs)[number]["key"];
type Status = "initializing" | "ready" | "compiling" | "success" | "failure";

const statusMeta: Record<
  Status,
  { label: string; badgeClassName: string; dotClassName: string }
> = {
  initializing: {
    label: "Initializing compiler",
    badgeClassName: "border-amber-200 bg-amber-50 text-amber-900",
    dotClassName: "bg-amber-500",
  },
  ready: {
    label: "Compiler ready",
    badgeClassName: "border-emerald-200 bg-emerald-50 text-emerald-900",
    dotClassName: "bg-emerald-500",
  },
  compiling: {
    label: "Compiling",
    badgeClassName: "border-sky-200 bg-sky-50 text-sky-900",
    dotClassName: "bg-sky-500",
  },
  success: {
    label: "Compile complete",
    badgeClassName: "border-teal-200 bg-teal-50 text-teal-900",
    dotClassName: "bg-teal-600",
  },
  failure: {
    label: "Compile failed",
    badgeClassName: "border-rose-200 bg-rose-50 text-rose-900",
    dotClassName: "bg-rose-500",
  },
};

function normalizeCompileErrorMessage(error: unknown) {
  const message = error instanceof Error ? error.message : "编译失败。";

  if (
    message.includes("Aborted(") ||
    message.includes("Build with -sASSERTIONS for more info")
  ) {
    return "编译超时，请检查源代码是否符合文法";
  }

  return message;
}

function getPanelContent(result: CompilerResult | null, activeTab: TabKey) {
  if (!result) {
    return "还没有编译结果，先点击 Compile 试试。";
  }

  const value = result[activeTab];
  return value && value.trim().length > 0 ? value : "该面板当前没有内容。";
}

export default function App() {
  const grammarPdfUrl = "/docs/grammar-introduction.pdf";
  const [selectedExampleId, setSelectedExampleId] = useState<(typeof examples)[number]["id"]>(
    "basic-function",
  );
  const [sourceCode, setSourceCode] = useState<string>(examples[0].code);
  const [optimize, setOptimize] = useState(true);
  const [status, setStatus] = useState<Status>("initializing");
  const [activeTab, setActiveTab] = useState<TabKey>("error");
  const [result, setResult] = useState<CompilerResult | null>(null);
  const [runtimeError, setRuntimeError] = useState<string>("");
  const [copyFeedback, setCopyFeedback] = useState<string>("");
  const [isGrammarOpen, setIsGrammarOpen] = useState(false);
  const statusRef = useRef<Status>("initializing");
  const compileActionRef = useRef<() => void>(() => undefined);

  useEffect(() => {
    let mounted = true;

    void getCompilerModule()
      .then(() => {
        if (!mounted) {
          return;
        }
        setStatus("ready");
      })
      .catch((error: unknown) => {
        if (!mounted) {
          return;
        }
        setStatus("failure");
        setRuntimeError(error instanceof Error ? error.message : "编译器初始化失败。");
      });

    return () => {
      mounted = false;
    };
  }, []);

  useEffect(() => {
    statusRef.current = status;
  }, [status]);

  useEffect(() => {
    function handleKeydown(event: KeyboardEvent) {
      if ((event.metaKey || event.ctrlKey) && event.key === "Enter") {
        event.preventDefault();

        if (status === "initializing" || status === "compiling") {
          return;
        }

        void handleCompile();
      }
    }

    window.addEventListener("keydown", handleKeydown);
    return () => {
      window.removeEventListener("keydown", handleKeydown);
    };
  }, [status, sourceCode, optimize]);

  async function handleCompile() {
    setStatus("compiling");
    setRuntimeError("");
    setCopyFeedback("");

    try {
      const nextResult = await compileSource(sourceCode, optimize);
      setResult(nextResult);

      if (nextResult.error && nextResult.error.trim()) {
        setActiveTab("error");
      } else {
        setActiveTab("mips");
      }

      setStatus("success");
    } catch (error: unknown) {
      setStatus("failure");
      setActiveTab("error");
      setRuntimeError(normalizeCompileErrorMessage(error));
    }
  }

  useEffect(() => {
    compileActionRef.current = () => {
      if (statusRef.current === "initializing" || statusRef.current === "compiling") {
        return;
      }

      void handleCompile();
    };
  });

  async function handleCopy() {
    const content = runtimeError
      ? runtimeError
      : getPanelContent(result, activeTab);

    try {
      await navigator.clipboard.writeText(content);
      setCopyFeedback("Copied");
      window.setTimeout(() => setCopyFeedback(""), 1500);
    } catch {
      setCopyFeedback("Copy failed");
      window.setTimeout(() => setCopyFeedback(""), 1500);
    }
  }

  function handleClear() {
    setResult(null);
    setRuntimeError("");
    setCopyFeedback("");
    setActiveTab("error");
    setStatus("ready");
  }

  const currentStatus = statusMeta[status];
  const currentPanelContent = runtimeError
    ? runtimeError
    : getPanelContent(result, activeTab);
  const selectedExample = examples.find((example) => example.id === selectedExampleId) ?? examples[0];

  function handleExampleChange(nextExampleId: (typeof examples)[number]["id"]) {
    const nextExample = examples.find((example) => example.id === nextExampleId);

    if (!nextExample) {
      return;
    }

    setSelectedExampleId(nextExample.id);
    setSourceCode(nextExample.code);
    setCopyFeedback("");
  }

  return (
    <div className="min-h-screen bg-[linear-gradient(180deg,#f5f7ef_0%,#edf1f7_40%,#dde4ee_100%)] text-slate-900">
      <div className="mx-auto flex min-h-screen max-w-[1500px] flex-col px-4 py-5 lg:px-6">
        <header className="mb-5 rounded-[32px] border border-white/65 bg-white/75 p-5 shadow-[0_28px_80px_rgba(15,23,42,0.08)] backdrop-blur">
          <div className="flex flex-col gap-4 xl:flex-row xl:items-end xl:justify-between">
            <div className="space-y-4">
              <div className="flex flex-wrap items-center gap-3">
                <span className="rounded-full bg-teal-100 px-3 py-1 text-xs font-semibold uppercase tracking-[0.24em] text-teal-800">
                  Compiler Workbench
                </span>
                <span
                  className={`inline-flex items-center gap-2 rounded-full border px-3 py-1.5 text-sm font-medium ${currentStatus.badgeClassName}`}
                >
                  <span className={`h-2.5 w-2.5 rounded-full ${currentStatus.dotClassName}`} />
                  {currentStatus.label}
                </span>
              </div>

              <div className="space-y-2">
                <h1 className="text-3xl font-semibold tracking-tight text-slate-950 md:text-4xl">
                  Lightweight Web Compiler
                </h1>
                <p className="max-w-3xl text-sm leading-7 text-slate-600 md:text-[15px]">
                  浏览器内直接完成编译，支持源码编辑、文法查看和 LLVM IR / MIPS 输出展示。
                </p>
                <p className="max-w-3xl text-sm leading-7 text-slate-500">
                  项目编译器基于仓库{" "}
                  <a
                    className="font-medium text-teal-700 transition hover:text-teal-900 hover:underline"
                    href="https://github.com/samlee1020/buaa-compiler-wasm"
                    rel="noreferrer"
                    target="_blank"
                  >
                    buaa-compiler-wasm
                  </a>
                  。
                </p>
              </div>
            </div>

            <div className="flex flex-wrap gap-3 xl:justify-end">
              <button
                className="rounded-full bg-slate-950 px-5 py-2.5 text-sm font-semibold text-white transition hover:bg-slate-800"
                disabled={status === "initializing" || status === "compiling"}
                onClick={() => void handleCompile()}
                type="button"
              >
                Compile
              </button>
              <button
                className="rounded-full border border-slate-200 bg-white px-5 py-2.5 text-sm font-semibold text-slate-700 transition hover:border-slate-300 hover:text-slate-950"
                onClick={() => handleExampleChange("basic-function")}
                type="button"
              >
                Load Sample
              </button>
              <button
                className="rounded-full border border-slate-200 bg-white px-5 py-2.5 text-sm font-semibold text-slate-700 transition hover:border-slate-300 hover:text-slate-950"
                onClick={() => setIsGrammarOpen(true)}
                type="button"
              >
                Grammar Guide
              </button>
            </div>
          </div>
        </header>

        <main className="grid flex-1 gap-5 xl:grid-cols-[minmax(0,1.15fr)_420px]">
          <section className="flex min-h-[640px] flex-col rounded-[32px] border border-slate-900/10 bg-[#07111f] p-4 shadow-[0_30px_90px_rgba(15,23,42,0.18)]">
            <div className="mb-4 flex flex-wrap items-center justify-between gap-3 rounded-[24px] border border-slate-800 bg-slate-950/60 px-4 py-3">
              <div>
                <h2 className="text-sm font-semibold uppercase tracking-[0.22em] text-teal-300">
                  Source Editor
                </h2>
                <p className="mt-1 text-sm text-slate-400">Monaco Editor</p>
              </div>

              <div className="flex flex-wrap items-center gap-2">
                <label className="inline-flex items-center gap-3 rounded-full border border-slate-700 bg-slate-900 px-4 py-2 text-sm font-medium text-slate-200">
                  <span className="text-slate-400">Example</span>
                  <select
                    className="min-w-[160px] bg-transparent text-slate-100 outline-none"
                    onChange={(event) =>
                      handleExampleChange(event.target.value as (typeof examples)[number]["id"])}
                    value={selectedExampleId}
                  >
                    {examples.map((example) => (
                      <option className="bg-slate-900 text-slate-100" key={example.id} value={example.id}>
                        {example.name}
                      </option>
                    ))}
                  </select>
                </label>
                <label className="inline-flex items-center gap-3 rounded-full border border-slate-700 bg-slate-900 px-4 py-2 text-sm font-medium text-slate-200">
                  <input
                    checked={optimize}
                    className="h-4 w-4 accent-teal-500"
                    onChange={(event) => setOptimize(event.target.checked)}
                    type="checkbox"
                  />
                  Optimize
                </label>
              </div>
            </div>

            <div className="mb-4 rounded-[20px] border border-slate-800 bg-slate-950/40 px-4 py-3">
              <p className="text-xs font-semibold uppercase tracking-[0.18em] text-slate-500">
                Current Example
              </p>
              <p className="mt-2 text-sm font-semibold text-slate-100">{selectedExample.name}</p>
              <p className="mt-1 text-sm leading-6 text-slate-400">{selectedExample.description}</p>
            </div>

            <div className="min-h-[520px] flex-1 overflow-hidden rounded-[24px] border border-slate-800 bg-[#0b1220]">
              <Editor
                defaultLanguage="cpp"
                height="100%"
                onChange={(value) => setSourceCode(value ?? "")}
                onMount={(editor, monaco) => {
                  editor.addCommand(monaco.KeyMod.CtrlCmd | monaco.KeyCode.Enter, () => {
                    compileActionRef.current();
                  });
                }}
                options={{
                  automaticLayout: true,
                  cursorBlinking: "smooth",
                  fontFamily: "JetBrains Mono, Menlo, Monaco, Consolas, monospace",
                  fontLigatures: true,
                  fontSize: 14,
                  lineHeight: 24,
                  minimap: { enabled: false },
                  padding: { top: 18, bottom: 18 },
                  roundedSelection: true,
                  scrollBeyondLastLine: false,
                  smoothScrolling: true,
                  tabSize: 2,
                }}
                theme="vs-dark"
                value={sourceCode}
              />
            </div>
          </section>

          <section className="grid min-h-[640px] gap-5">
            <div className="flex min-h-[420px] flex-col rounded-[32px] border border-white/70 bg-white/85 p-4 shadow-[0_28px_80px_rgba(15,23,42,0.08)] backdrop-blur">
              <div className="mb-4 flex items-start justify-between gap-3 rounded-[24px] bg-slate-50 px-4 py-3">
                <div>
                  <h2 className="text-sm font-semibold uppercase tracking-[0.22em] text-teal-700">
                    Output Panel
                  </h2>
                  <p className="mt-1 text-sm leading-6 text-slate-500">Error / Symbol / LLVM IR / MIPS</p>
                </div>
                <div className="flex items-center gap-2">
                  {copyFeedback ? (
                    <span className="rounded-full bg-teal-100 px-3 py-1 text-xs font-semibold uppercase tracking-[0.18em] text-teal-800">
                      {copyFeedback}
                    </span>
                  ) : null}
                  <button
                    className="rounded-full border border-slate-200 bg-white px-4 py-2 text-sm font-medium text-slate-700 transition hover:border-slate-300 hover:text-slate-950"
                    onClick={() => void handleCopy()}
                    type="button"
                  >
                    Copy
                  </button>
                  <button
                    className="rounded-full border border-slate-200 bg-white px-4 py-2 text-sm font-medium text-slate-700 transition hover:border-slate-300 hover:text-slate-950"
                    onClick={handleClear}
                    type="button"
                  >
                    Clear
                  </button>
                </div>
              </div>

              <div className="mb-4 flex flex-wrap gap-2">
                {tabs.map((tab) => {
                  const active = activeTab === tab.key;
                  return (
                    <button
                      className={`rounded-full px-4 py-2 text-sm font-medium transition ${
                        active
                          ? "bg-slate-950 text-white shadow-lg shadow-slate-950/10"
                          : "border border-slate-200 bg-white text-slate-600 hover:border-slate-300 hover:text-slate-900"
                      }`}
                      key={tab.key}
                      onClick={() => setActiveTab(tab.key)}
                      type="button"
                    >
                      {tab.label}
                    </button>
                  );
                })}
              </div>

              <div className="flex-1 overflow-hidden rounded-[24px] border border-slate-200 bg-slate-50">
                {runtimeError ? (
                  <div className="h-full overflow-auto p-5">
                    <p className="mb-3 text-sm font-semibold uppercase tracking-[0.18em] text-rose-700">
                      Runtime Error
                    </p>
                    <pre className="whitespace-pre-wrap break-words font-mono text-sm leading-6 text-rose-900">
                      {runtimeError}
                    </pre>
                  </div>
                ) : (
                  <pre className="h-full overflow-auto whitespace-pre-wrap break-words p-5 font-mono text-sm leading-6 text-slate-800">
                    {currentPanelContent}
                  </pre>
                )}
              </div>
            </div>
          </section>
        </main>

        <footer className="mt-5 rounded-[28px] border border-white/70 bg-white/70 px-5 py-4 shadow-[0_16px_40px_rgba(15,23,42,0.04)] backdrop-blur">
          <div className="flex flex-col gap-3 text-sm text-slate-600 md:flex-row md:items-center md:justify-end">
            <p className="text-slate-500">编译超时时，请优先检查源代码是否符合支持文法。</p>
          </div>
        </footer>
      </div>

      {isGrammarOpen ? (
        <div className="fixed inset-0 z-50 flex items-center justify-center bg-slate-950/50 p-4 backdrop-blur-sm">
          <div className="flex h-[88vh] w-full max-w-6xl flex-col overflow-hidden rounded-[28px] border border-white/70 bg-white shadow-[0_24px_80px_rgba(15,23,42,0.25)]">
            <div className="flex items-center justify-between border-b border-slate-200 px-5 py-4">
              <div>
                <h2 className="text-lg font-semibold text-slate-950">源代码支持文法介绍</h2>
                <p className="mt-1 text-sm text-slate-500">
                  当前内容来自 `grammar-introduction.pdf`
                </p>
              </div>
              <div className="flex items-center gap-2">
                <a
                  className="rounded-full border border-slate-200 bg-white px-4 py-2 text-sm font-medium text-slate-700 transition hover:border-slate-300 hover:text-slate-950"
                  href={grammarPdfUrl}
                  rel="noreferrer"
                  target="_blank"
                >
                  新标签页打开
                </a>
                <button
                  className="rounded-full bg-slate-950 px-4 py-2 text-sm font-medium text-white transition hover:bg-slate-800"
                  onClick={() => setIsGrammarOpen(false)}
                  type="button"
                >
                  Close
                </button>
              </div>
            </div>

            <div className="flex-1 bg-slate-100">
              <iframe
                className="h-full w-full"
                src={grammarPdfUrl}
                title="源代码支持文法介绍"
              />
            </div>
          </div>
        </div>
      ) : null}
    </div>
  );
}
