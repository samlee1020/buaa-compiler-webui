import { createSlice } from '@reduxjs/toolkit';
import type { PayloadAction } from '@reduxjs/toolkit';

type TabType = 'llvm' | 'mips' | 'symbol';

interface UiState {
  activeTab: TabType;
  isCompiling: boolean;
}

const initialState: UiState = {
  activeTab: 'llvm',
  isCompiling: false,
};

export const uiSlice = createSlice({
  name: 'ui',
  initialState,
  reducers: {
    setActiveTab: (state, action: PayloadAction<TabType>) => {
      state.activeTab = action.payload;
    },
    setIsCompiling: (state, action: PayloadAction<boolean>) => {
      state.isCompiling = action.payload;
    },
  },
});

export const { setActiveTab, setIsCompiling } = uiSlice.actions;

export default uiSlice.reducer;