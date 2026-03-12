import React from 'react';
import { useDispatch, useSelector } from 'react-redux';
import type { RootState } from '../store';
import { setActiveTab } from '../store/slices/uiSlice';

const OutputPanel: React.FC = () => {
  const dispatch = useDispatch();
  const output = useSelector((state: RootState) => state.output);
  const activeTab = useSelector((state: RootState) => state.ui.activeTab);

  const renderContent = () => {
    switch (activeTab) {
      case 'llvm':
        return <pre className="whitespace-pre-wrap font-mono text-sm text-secondary-800">{output.llvmIR}</pre>;
      case 'mips':
        return <pre className="whitespace-pre-wrap font-mono text-sm text-secondary-800">{output.mips}</pre>;
      case 'symbol':
        return <pre className="whitespace-pre-wrap font-mono text-sm text-secondary-800">{output.symbol}</pre>;
      default:
        return null;
    }
  };

  return (
    <div className="h-full relative">
      {/* 装饰性背景元素 */}
      <div className="absolute top-10 left-0 w-32 h-32 bg-primary-50 rounded-full -ml-16 opacity-50"></div>
      <div className="absolute bottom-10 right-0 w-48 h-48 bg-secondary-50 rounded-full -mr-24 opacity-50"></div>
      
      <div className="flex bg-secondary-50/50 backdrop-blur-sm relative z-10 rounded-t-lg p-1">
        <button 
          className={`px-4 py-2 transition-all duration-300 rounded-lg ${activeTab === 'llvm' ? 'bg-white text-primary-700 shadow-sm' : 'text-secondary-600 hover:bg-secondary-100'}`}
          onClick={() => dispatch(setActiveTab('llvm'))}
        >
          LLVM IR
        </button>
        <button 
          className={`px-4 py-2 transition-all duration-300 rounded-lg ${activeTab === 'mips' ? 'bg-white text-primary-700 shadow-sm' : 'text-secondary-600 hover:bg-secondary-100'}`}
          onClick={() => dispatch(setActiveTab('mips'))}
        >
          MIPS 汇编
        </button>
        <button 
          className={`px-4 py-2 transition-all duration-300 rounded-lg ${activeTab === 'symbol' ? 'bg-white text-primary-700 shadow-sm' : 'text-secondary-600 hover:bg-secondary-100'}`}
          onClick={() => dispatch(setActiveTab('symbol'))}
        >
          符号表
        </button>
      </div>
      <div className="p-4 h-[calc(100%-56px)] overflow-auto bg-white relative z-10">
        <div className="animate-fade-in bg-white/80 backdrop-blur-sm p-4 rounded-lg border border-gray-100 shadow-sm">
          {renderContent()}
        </div>
      </div>
    </div>
  );
};

export default OutputPanel;