# Samlee's BUAA Compiler Web UI

这是一个轻量化网页编辑器，可以在线把一个简化C语言程序编译成MIPS汇编代码。由React, Vite, TypeScript, Monaco Editor, Tailwind CSS构建。

下面是以及部署好的demo（国内访问可能需要代理）

- [Vercel Demo](https://samlee-buaa-compiler-webui-p27wfqimz-samlee1020s-projects.vercel.app/)
- [Cloudflare Demo](https://buaa-compiler-webui.samlee1020.workers.dev)

## 开发说明

简要介绍项目各部分内容和开发过程

* 编译器：源于2025年的北航编译技术实验，使用C++从零开发一个编译器，**无AI手工**实现了词法分析、语法分析、语义分析、LLVM IR中间代码生成、MIPS代码生成、简单的优化。
* 编译器WASM模块：使用Codex把原编译器实现代码修改成可以构建出Web Assembly的形式，导出成WASM模块函数供JS调用。
* 前端：使用Codex开发的基于React, Vite, TypeScript, Tailwind CSS的前端界面，使用Monaco Editor实现网页代码编辑器，使用Web Worker调用WASM编译线程，避免编译时阻塞主线程。

编译器代码及WASM构建流程见以下仓库:

[buaa-compiler-wasm](https://github.com/samlee1020/buaa-compiler-wasm)

## 部署和构建

部署：

```bash
npm install
npm run dev
```

构建：

```bash
npm run build
```
