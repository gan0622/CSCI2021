// optimized versions of matrix diagonal summing
#include "matvec.h"

int matsquare_VER1(matrix_t mat, matrix_t matsq)
{
  // YOUR CODE HERE
  int rowCount = mat.rows;
  int colCount = mat.cols;

  for (int i = 0; i < rowCount; i++)
  {
    for (int j = 0; j < colCount; j++)
    {
      MSET(matsq, i, j, 0);
    }
  }
  // loop unrolling: taking a loop for each iteration divide into 4 seperate
  // component
  for (int i = 0; i < rowCount; i++)
  {
    for (int j = 0; j < colCount; j++)
    {
      int k;
      int lead = MGET(mat, i, j);
      for (k = 0; k < rowCount-4; k += 4)
      {
        int mik1 = MGET(mat, j, k); // 0 0 
        int cur1 = MGET(matsq, i, k); // 0 0 
        MSET(matsq, i, k, cur1 + mik1 * lead); // 0 0
        if (k + 3 >= rowCount) {
          if (k + 1 < rowCount) {
            int mik2 = MGET(mat, j, k + 1); 
            int cur2 = MGET(matsq, i, k + 1);
            MSET(matsq, i, k + 1, cur2 + mik2 * lead);
            if (k + 2 < rowCount)
            {
              int mik3 = MGET(mat, j, k + 2);
              int cur3 = MGET(matsq, i, k + 2);
              MSET(matsq, i, k + 2, cur3 + mik3 * lead);
            }
          }
          continue;
        }
        else
        {
          int mik2 = MGET(mat, j, k + 1); 
          int cur2 = MGET(matsq, i, k + 1); 
          MSET(matsq, i, k + 1, cur2 + mik2 * lead);  
          int mik3 = MGET(mat, j, k + 2);
          int cur3 = MGET(matsq, i, k + 2);
          MSET(matsq, i, k + 2, cur3 + mik3 * lead);
          int mik4 = MGET(mat, j, k + 3);
          int cur4 = MGET(matsq, i, k + 3);
          MSET(matsq, i, k + 3, cur4 + mik4 * lead);
        }
      }
      for(;k < rowCount; k++){
          int mik2 = MGET(mat, j, k); 
          int cur2 = MGET(matsq, i, k); 
          MSET(matsq, i, k , cur2 + mik2 * lead);
        }
    }
  }
  return 0;
}

int matsquare_OPTM(matrix_t mat, matrix_t matsq)
{
  if (mat.rows != mat.cols || // must be a square matrix to square it
      mat.rows != matsq.rows || mat.cols != matsq.cols)
  {
    printf("matsquare_OPTM: dimension mismatch\n");
    return 1;
  }
  // Call to some version of optimized code
  return matsquare_VER1(mat, matsq);
}
