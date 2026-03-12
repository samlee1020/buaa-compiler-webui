import { createSlice } from '@reduxjs/toolkit';
import type { PayloadAction } from '@reduxjs/toolkit';

interface OutputState {
  llvmIR: string;
  mips: string;
  symbol: string;
  error: string;
  success: boolean;
}

const initialState: OutputState = {
  llvmIR: '',
  mips: '',
  symbol: '',
  error: '',
  success: false,
};

export const outputSlice = createSlice({
  name: 'output',
  initialState,
  reducers: {
    setOutput: (_state, action: PayloadAction<OutputState>) => {
      return action.payload;
    },
    clearOutput: () => {
      return initialState;
    },
  },
});

export const { setOutput, clearOutput } = outputSlice.actions;

export default outputSlice.reducer;