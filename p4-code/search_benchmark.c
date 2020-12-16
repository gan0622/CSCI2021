#include "search.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
// Complete this main to benchmark the search algorithms outlined in
// the project specification

int main(int argc, char *argv[]){
  if(argc < 3 || argc > 6){
    printf("usage: %s <minpow> <maxpow> <repeats> [which] \n",argv[0]);
    printf("which is a combination of: \n"
    "a : Linear Array Search \n"
    "l : Linked List Search \n"
    "b : Binary Array Search \n"
    "t : Binary Tree Search \n"
    "(default all)\n");
    return 1;
  }

  int minPow = atoi(argv[1]); 
  int maxPow = atoi(argv[2]);
  int repeat = atoi(argv[3]);
  
  int a = 0; //linear array search
  int b = 0; //binary search
  int c = 0; //linked list search
  int d = 0; //tree search
  
  


  if(argc == 5){
    char *arr = argv[4];
    
    // size_t size = sizeof(arr) / sizeof(arr[0]);
    for(int i = 0; arr[i] != '\0' ; i++){
      if(arr[i] == 'a'){
        a = 1;
      }else if(arr[i] == 'l'){
        c = 1;
      }else if(arr[i] == 'b'){
        b = 1;
      }else if(arr[i] == 't'){
        d = 1;
    }
    }
  }else{
    a = 1;
    c = 1;
    b = 1;
    d = 1;
  }
  

  printf("LENGTH  SEARCHES");
  if(a == 1){
    printf("%10s","array");
  }
  if(c == 1){
    printf("%10s","list");
  }
  if(b == 1){
    printf("%10s","binary");
  }
  if(d == 1){
    printf("%10s","tree");
  }
  printf("\n");

  for(int i = minPow; i <= maxPow ; i++){
      int length = 1 << i ; // double the minPow 
      int search = length * 2 * repeat; 
      printf("%6d ", length);
      printf("%6d ", search);
      if(a == 1){
        int *arr = make_evens_array(length);
        clock_t begin, end;
        begin = clock();
        for(int times = 0; times < repeat; times++){  
          for(int num = 0; num <= (2*length)-1; num++){ 
              linear_array_search(arr,length,num);
          }
        }
        end = clock();
        double time = (double) (end - begin) / CLOCKS_PER_SEC;// CLOCKS_PER_SEC
        free(arr);
        printf("%10.4e ", time);
      }
      if(b == 1){
        list_t *arr = make_evens_list(length);
        clock_t begin, end;
        begin = clock();
        for(int times = 0; times < repeat; times++){
          for(int num = 0; num <= (2*length)-1; num++){
              linkedlist_search(arr,length,num);
          }
        }
        end = clock();
        double time = (double) (end - begin) /CLOCKS_PER_SEC;// CLOCKS_PER_SEC
        list_free(arr);
        printf("%10.4e ", time);
      }
      if(c == 1){
        int *arr = make_evens_array(length);
        clock_t begin, end;
        begin = clock();
        for(int times = 0; times < repeat; times++){
          for(int num = 0; num <= (2*length)-1; num++){
              binary_array_search(arr,length,num);
          }
        }
        end = clock();
        double time = (double) (end - begin) / CLOCKS_PER_SEC;// CLOCKS_PER_SEC
        free(arr);
        printf("%10.4e ", time);
      }
      if(d == 1){
        bst_t *arr = make_evens_tree(length);
        clock_t begin, end;
        begin = clock();
        for(int times = 0; times < repeat; times++){
          for(int num = 0; num <= (2*length)-1; num++){
              binary_tree_search(arr,length,num);
          }
        }
        end = clock();
        double time = (double) (end - begin)/ CLOCKS_PER_SEC;// CLOCKS_PER_SEC
        bst_free(arr);
        printf("%10.4e ", time);
      }
      printf("\n");
  }
  return 0;
}
