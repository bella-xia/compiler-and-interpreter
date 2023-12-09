// This program multiplies 10x10 square matrices,
// represented as 100 element 1-D arrays
int main(void) {
  long a[100], b[100], c[100];
  int M;
  int i, j, k;
  int idx;
  long r, val;

  M = 10;
  // do the matrix multiplication: note that the
  // loops are structured to avoid iterating over a column
  // of elements and incurring excessive cache misses
  // as a result
  idx = i*M + j;
  val = c[i*M + j];



  return 0;
}
