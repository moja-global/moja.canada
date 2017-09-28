#ifndef sawtooth_matrix_h
#define sawtooth_matrix_h

struct Sawtooth_Matrix {
	//number of rows in the matrix
	size_t rows;
	//number of columns in the matrix
	size_t cols;
	//matrix storage
	double* values;

	size_t size() { return rows*cols; }
	double GetValue(size_t row, size_t col){
		return values[row * cols + col]; 
	}
	void SetValue(size_t row, size_t col, double value) {
		values[row * cols + col] = value;
	}
};
 
struct Sawtooth_Matrix_Int {
	//number of rows in the matrix
	size_t rows;
	//number of columns in the matrix
	size_t cols;
	//matrix storage
	int* values;

	size_t size() { return rows*cols; }
	int GetValue(size_t row, size_t col) {
		return values[row * cols + col];
	}
	void SetValue(size_t row, size_t col, int value) {
		values[row * cols + col] = value;
	}
};

#endif

