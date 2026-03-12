import React from 'react';
import { useSelector } from 'react-redux';
import type { RootState } from '../store';

const ErrorPanel: React.FC = () => {
  const error = useSelector((state: RootState) => state.output.error);
  const success = useSelector((state: RootState) => state.output.success);

  if (!error && success) {
    return (
      <div className="p-4 h-full bg-gradient-to-r from-success-50 to-secondary-50 rounded-b-xl">
        <div className="flex items-center gap-2 text-success-600 font-medium">
          <div className="w-6 h-6 rounded-full bg-success-100 flex items-center justify-center shadow-sm">
            <span className="text-success-600 font-bold">✓</span>
          </div>
          <span>编译成功！</span>
        </div>
      </div>
    );
  }

  return (
    <div className="p-4 h-full bg-gradient-to-r from-accent-50 to-secondary-50 rounded-b-xl overflow-auto">
      {error ? (
        <div className="flex flex-col gap-3">
          <div className="flex items-center gap-2 text-accent-600 font-medium">
            <div className="w-6 h-6 rounded-full bg-accent-100 flex items-center justify-center shadow-sm">
              <span className="text-accent-600 font-bold">✗</span>
            </div>
            <span>编译错误</span>
          </div>
          <pre className="whitespace-pre-wrap text-accent-600 font-mono text-sm bg-white p-4 rounded-lg border border-accent-100 shadow-sm">{error}</pre>
        </div>
      ) : (
        <div className="flex items-center gap-2 text-secondary-500">
          <div className="w-6 h-6 rounded-full bg-secondary-100 flex items-center justify-center shadow-sm">
            <span className="text-secondary-500">i</span>
          </div>
          <span>无错误信息</span>
        </div>
      )}
    </div>
  );
};

export default ErrorPanel;