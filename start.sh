#!/bin/bash

echo "Starting C Compiler Web Service..."

# 启动后端服务
echo "Starting backend server..."
node server.js &
BACKEND_PID=$!
echo "Backend server started with PID: $BACKEND_PID"

# 等待后端服务启动
sleep 2

# 启动前端服务
echo "Starting frontend server..."
cd frontend && npm run dev &
FRONTEND_PID=$!
echo "Frontend server started with PID: $FRONTEND_PID"

# 保存进程ID到文件
echo $BACKEND_PID > backend.pid
echo $FRONTEND_PID > frontend.pid

echo "All services started successfully!"
echo "Backend server: http://localhost:3001"
echo "Frontend server: http://localhost:5173"
echo "To stop services, run: ./stop.sh"
