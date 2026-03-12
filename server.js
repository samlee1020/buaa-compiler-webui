const express = require('express');
const cors = require('cors');
const fs = require('fs');
const { execSync } = require('child_process');

const app = express();
app.use(cors());
app.use(express.json());

app.post('/api/compile', (req, res) => {
  try {
    // 写入临时文件
    fs.writeFileSync('./temp_testfile.txt', req.body.code);
    
    // 执行编译器
    execSync('./build/Compiler');
    
    // 读取输出文件
    const llvmIR = fs.readFileSync('./llvm_ir.txt', 'utf8');
    const mips = fs.readFileSync('./mips.txt', 'utf8');
    const symbol = fs.readFileSync('./symbol.txt', 'utf8');
    const error = fs.readFileSync('./error.txt', 'utf8');
    
    res.json({
      llvmIR,
      mips,
      symbol,
      error,
      success: error === ''
    });
  } catch (err) {
    res.status(500).json({ error: err.message });
  }
});

app.listen(3001, () => {
  console.log('Server running on port 3001');
});