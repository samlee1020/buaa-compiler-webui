import React from 'react';
import { useDispatch, useSelector } from 'react-redux';
import type { RootState, AppDispatch } from '../store';
import { setOutput } from '../store/slices/outputSlice';
import { setIsCompiling } from '../store/slices/uiSlice';

const ControlBar: React.FC = () => {
  const dispatch = useDispatch<AppDispatch>();
  const code = useSelector((state: RootState) => state.code.value);
  const isCompiling = useSelector((state: RootState) => state.ui.isCompiling);

  const handleCompile = async () => {
    dispatch(setIsCompiling(true));
    try {
      const response = await fetch('http://localhost:3001/api/compile', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify({ code }),
      });
      const data = await response.json();
      dispatch(setOutput(data));
    } catch (error) {
      console.error('编译失败:', error);
      dispatch(setOutput({
        llvmIR: '',
        mips: '',
        symbol: '',
        error: '编译服务连接失败，请检查后端服务是否运行',
        success: false,
      }));
    } finally {
      dispatch(setIsCompiling(false));
    }
  };

  return (
    <div className="flex items-center justify-between p-4 border-b border-gray-200 bg-white shadow-sm">
      <div className="flex items-center gap-2">
        <div className="w-8 h-8 rounded-lg bg-primary-500 flex items-center justify-center">
          <span className="text-white font-bold text-lg">C</span>
        </div>
        <div className="text-xl font-bold text-secondary-800">C语言编译器</div>
      </div>
      <button
        onClick={handleCompile}
        disabled={isCompiling}
        className="px-6 py-2 bg-primary-500 text-white rounded-lg hover:bg-primary-600 disabled:bg-gray-400 transition-all duration-300 transform hover:scale-105 flex items-center gap-2 shadow-md hover:shadow-lg disabled:cursor-not-allowed"
      >
        {isCompiling ? (
          <>
            <div className="w-4 h-4 border-2 border-white border-t-transparent rounded-full animate-spin"></div>
            <span>编译中...</span>
          </>
        ) : (
          <>
            <span>编译</span>
          </>
        )}
      </button>
    </div>
  );
};

export default ControlBar;