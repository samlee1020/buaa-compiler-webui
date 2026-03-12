# C语言编译器Web界面

一个基于C++的编译器，带有现代化的Web前端界面，支持编译一种简化的C代码并生成LLVM IR和MIPS汇编。
具体的语法见[2025北航编译技术实验文法定义及相关说明](./2025编译技术实验文法定义及相关说明.pdf)

## 技术栈

### 后端
- **C++**：核心编译逻辑
- **Node.js** + **Express.js**：Web服务
- **CMake**：构建工具

### 前端
- **React** + **TypeScript**：前端框架
- **Tailwind CSS**：样式框架
- **Monaco Editor**：代码编辑器
- **Redux Toolkit**：状态管理
- **Vite**：构建工具

## 安装步骤

### 1. 克隆项目

```bash
git clone https://github.com/samlee1020/buaa-compiler-webui.git
cd buaa-compiler-webui
```

### 2. 安装依赖

#### 后端依赖
```bash
npm install
```

#### 前端依赖
```bash
cd frontend
npm install
cd ..
```

### 3. 编译C++编译器

```bash
mkdir -p build
cd build
cmake ..
make
cd ..
```

## 使用方法

### 1. 启动服务

```bash
./start.sh
```

服务启动后，会在终端显示服务地址：
- 后端服务：http://localhost:3001
- 前端服务：http://localhost:5173

### 2. 访问界面

打开浏览器，访问 http://localhost:5173/，即可看到C语言编译器的Web界面。

### 3. 编写和编译代码

- 在左侧代码编辑器中输入C代码
- 点击右上角的"编译"按钮
- 在右侧查看编译输出，包括LLVM IR、MIPS汇编和符号表
- 底部会显示编译错误信息（如果有）

### 4. 停止服务

```bash
./stop.sh
```

## 项目结构

```
├── build/            # 编译产物（忽略）
├── frontend/         # 前端代码
│   ├── src/          # React源代码
│   ├── package.json  # 前端依赖配置
│   └── ...           # 其他前端配置文件
├── front-end/        # C++前端（词法分析器、语法分析器等）
├── llvm/             # LLVM IR生成相关代码
├── mips/             # MIPS汇编生成相关代码
├── start.sh          # 启动服务脚本
├── stop.sh           # 停止服务脚本
├── main.cpp          # 主入口文件
├── CMakeLists.txt    # CMake配置文件
└── README.md         # 项目说明
```

## 核心功能

### 1. 词法分析
- 将C代码转换为词法单元（tokens）
- 支持关键字、标识符、常量、运算符等

### 2. 语法分析
- 构建抽象语法树（AST）
- 支持C语言的基本语法结构

### 3. 语义分析
- 类型检查
- 变量作用域分析
- 错误检测

### 4. 代码生成
- 生成LLVM IR中间代码
- 生成MIPS汇编代码

### 5. Web界面
- 代码编辑：支持语法高亮和自动缩进
- 编译控制：一键编译，显示编译状态
- 结果展示：多标签页展示不同格式的输出
- 错误定位：显示详细的错误信息

## 示例代码

```c
int main() {
    int a = 7, b = 13;
    int c = a * b;
    printf("%d\n", c);
    return 0;
}
```

编译后会生成对应的LLVM IR和MIPS汇编代码。

