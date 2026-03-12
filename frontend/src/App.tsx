
import { Provider } from 'react-redux';
import { store } from './store';
import ControlBar from './components/ControlBar';
import Editor from './components/Editor';
import OutputPanel from './components/OutputPanel';
import ErrorPanel from './components/ErrorPanel';
import './App.css';

function App() {
  return (
    <Provider store={store}>
      <div className="h-screen flex flex-col bg-secondary-50">
        <ControlBar />
        <div className="flex flex-1 overflow-hidden p-4 gap-6">
          <div className="w-1/2 h-full bg-white rounded-xl shadow-lg flex flex-col transition-all duration-300 hover:shadow-xl">
            <div className="p-3 font-medium text-secondary-700 bg-gradient-to-r from-primary-50 to-secondary-50 rounded-t-xl">
              代码编辑
            </div>
            <div className="flex-1 overflow-auto">
              <Editor />
            </div>
          </div>
          <div className="w-1/2 h-full bg-white rounded-xl shadow-lg flex flex-col transition-all duration-300 hover:shadow-xl">
            <div className="p-3 font-medium text-secondary-700 bg-gradient-to-r from-primary-50 to-secondary-50 rounded-t-xl">
              编译输出
            </div>
            <div className="flex-1 overflow-auto">
              <OutputPanel />
            </div>
            <div className="h-1/4 border-t border-gray-100 rounded-b-xl">
              <ErrorPanel />
            </div>
          </div>
        </div>
      </div>
    </Provider>
  );
}

export default App
