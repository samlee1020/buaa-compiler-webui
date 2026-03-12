import React from 'react';
import { Editor as MonacoEditor } from '@monaco-editor/react';
import { useDispatch, useSelector } from 'react-redux';
import type { RootState } from '../store';
import { setCode } from '../store/slices/codeSlice';

const Editor: React.FC = () => {
  const dispatch = useDispatch();
  const code = useSelector((state: RootState) => state.code.value);

  const handleEditorChange = (value: string | undefined) => {
    if (value) {
      dispatch(setCode(value));
    }
  };

  return (
    <div className="h-full w-full relative overflow-hidden">
      {/* 装饰性背景元素 */}
      <div className="absolute top-0 right-0 w-40 h-40 bg-primary-100 rounded-full -mr-20 -mt-20 opacity-50"></div>
      <div className="absolute bottom-0 left-0 w-60 h-60 bg-secondary-100 rounded-full -ml-30 -mb-30 opacity-50"></div>
      
      <MonacoEditor
        height="100%"
        language="c"
        value={code}
        onChange={handleEditorChange}
        options={{
          minimap: { enabled: false },
          fontSize: 14,
          tabSize: 2,
          theme: 'vs-light',
          lineNumbers: 'on',
          scrollBeyondLastLine: false,
          automaticLayout: true,
          scrollbar: {
            vertical: 'auto',
            horizontal: 'auto',
            verticalScrollbarSize: 8,
            horizontalScrollbarSize: 8
          }
        }}
      />
    </div>
  );
};

export default Editor;