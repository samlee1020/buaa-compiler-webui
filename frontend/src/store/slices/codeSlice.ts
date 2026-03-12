import { createSlice } from '@reduxjs/toolkit';
import type { PayloadAction } from '@reduxjs/toolkit';

interface CodeState {
  value: string;
}

const initialState: CodeState = {
  value: `int main() {
    int a = 7, b = 13;
    int c = a * b;
    printf("%d\n", c);
    return 0;
}`,
};

export const codeSlice = createSlice({
  name: 'code',
  initialState,
  reducers: {
    setCode: (state, action: PayloadAction<string>) => {
      state.value = action.payload;
    },
  },
});

export const { setCode } = codeSlice.actions;

export default codeSlice.reducer;