#include <iostream>
#include <vector>
#include <cstdlib>
#include <cassert>
#include <ctime>
#include <omp.h>

using namespace std;

vector<vector<int>> create_random_matrix(int rows, int cols) {
    vector<vector<int>> matrix;
    for(int i = 0; i < rows; i++) {
        vector<int> row_vector;
        for(int j = 0; j < cols; j++) {
            row_vector.push_back(rand() % 100);
        }
        matrix.push_back(row_vector);
    }
    return matrix;
}

vector<vector<int>> create_zero_matrix(int rows, int cols) {
    vector<vector<int>> matrix;
    for(int i = 0; i < rows; i++) {
        vector<int> row_vector;
        for(int j = 0; j < cols; j++) {
            row_vector.push_back(0);
        }
        matrix.push_back(row_vector);
    }
    return matrix;
}

void print_matrix(vector<vector<int>> m) {
    int n_rows = m.size();
    int n_cols = m[0].size();

    for(int i = 0; i < n_rows; i++) {
        for(int j = 0; j < n_cols; j++) {
            cout << m[i][j] << " ";
        }
        cout << endl;
    }
    cout << "-*-*-" << endl;
}

vector<vector<int>> matrix_multiply(vector<vector<int>> A, vector<vector<int>> B) {
    int n_rows_a = A.size();
    int n_cols_a = A[0].size();
    int n_rows_b = B.size();
    int n_cols_b = B[0].size();

    assert((n_cols_a == n_rows_b) && "input matrices can't be multiplied!");

    vector<vector<int>> C = create_zero_matrix(n_rows_a, n_cols_b);
    for(int i = 0; i < n_rows_a; i++) {
        for(int j = 0; j < n_cols_b; j++) {
            for(int k = 0; k < n_cols_a; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
    return C;
}

vector<vector<int>> matrix_multiply_parallel(vector<vector<int>> A, vector<vector<int>> B) {
    int n_rows_a = A.size();
    int n_cols_a = A[0].size();
    int n_rows_b = B.size();
    int n_cols_b = B[0].size();

    assert((n_cols_a == n_rows_b) && "input matrices can't be multiplied!");

    vector<vector<int>> C = create_zero_matrix(n_rows_a, n_cols_b);
    #pragma omp parallel for collapse(2)
    for(int i = 0; i < n_rows_a; i++) {
        for(int j = 0; j < n_cols_b; j++) {
            for(int k = 0; k < n_cols_a; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
    return C;
}

int main(int argc, char* argv[]) {
    int dim = 1000;
    vector<vector<int>> A = create_random_matrix(dim, dim);
    cout << "Generated random matrix A with dimensions " << dim << "x" << dim << endl;
    //print_matrix(A);
    vector<vector<int>> B = create_random_matrix(dim, dim);
    cout << "Generated random matrix B with dimensions " << dim << "x" << dim << endl;
    //print_matrix(B);
    cout << "Calculating product of A, B -> C" << endl;
    time_t start_seq, end_seq, start_pl, end_pl;
    time(&start_seq);
    vector<vector<int>> C = matrix_multiply(A, B);
    time(&end_seq);
    //print_matrix(C);
    time(&start_pl);
    matrix_multiply_parallel(A, B);
    time(&end_pl);
    cout << "Sequential algorithm took " << difftime(end_seq, start_seq) << " seconds." << endl;
    cout << "Parallel algorithm with 4 threads took " << difftime(end_pl, start_pl) << " seconds." << endl;
}
