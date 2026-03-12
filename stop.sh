#!/bin/bash

echo "Stopping C Compiler Web Service..."

# 停止后端服务
if [ -f backend.pid ]; then
    BACKEND_PID=$(cat backend.pid)
    echo "Stopping backend server with PID: $BACKEND_PID"
    kill $BACKEND_PID 2>/dev/null
    rm backend.pid
    echo "Backend server stopped"
else
    echo "No backend.pid file found, backend server may not be running"
fi

# 停止前端服务
if [ -f frontend.pid ]; then
    FRONTEND_PID=$(cat frontend.pid)
    echo "Stopping frontend server with PID: $FRONTEND_PID"
    kill $FRONTEND_PID 2>/dev/null
    rm frontend.pid
    echo "Frontend server stopped"
else
    echo "No frontend.pid file found, frontend server may not be running"
fi

echo "All services stopped successfully!"
