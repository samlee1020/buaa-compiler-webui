import { configureStore } from '@reduxjs/toolkit';
import codeReducer from './slices/codeSlice';
import outputReducer from './slices/outputSlice';
import uiReducer from './slices/uiSlice';

export const store = configureStore({
  reducer: {
    code: codeReducer,
    output: outputReducer,
    ui: uiReducer,
  },
});

export type RootState = ReturnType<typeof store.getState>;
export type AppDispatch = typeof store.dispatch;