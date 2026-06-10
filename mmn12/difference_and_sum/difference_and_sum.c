#include <stdio.h>
#include <stdlib.h>

#define DIFF(a, b) (((a - b) > 0) ? (a - b) : (-(a - b)))                          
#define SUMM(a, b) (a + b)                                                         

void findDifferenceAndSum(int a, int b, int *sum, int *diff){                      
    *sum = a + b;                                                              
    *diff = ((a - b) >= 0) ? (a - b) : (-(a - b));                             
}                    


void* saveArray(int *arr, int *length, int *capacity, int num1, int num2){
    if (*length + 2 >= *capacity){
        int* temp = realloc(arr, 2 * (*capacity) * sizeof(int));
        if (temp == NULL){
            return arr;
        }
        *capacity *= 2;
        arr = temp;
    }
    arr[(*length)++] = num1;
    arr[(*length)++] = num2;

    return arr;
}


int main(){                                                                        

    int capacity = 10, length = 0;                                             
    int *arr = malloc(capacity * sizeof(int));
    int i;
    int flag = 1;
    int res;

    int num1, num2;
    int temp_sum, temp_diff;

    while (flag){
        printf("Enter two numbers separeted by a space: ");
        res = scanf("%d %d", &num1, &num2);
        if (res == EOF)
            flag = 0;
        else{
            printf("\n");
            printf("The original numbers received are: %d, %d\n", num1, num2);

            arr = saveArray(arr, &length, &capacity, num1, num2);

            temp_sum = num1;
            temp_diff = num2;

            printf("Calling function findDifferenceAndSum:\n"); 
            findDifferenceAndSum(temp_sum, temp_diff, &temp_sum, &temp_diff);                                        
            printf("The difference is: %d\n", temp_diff); 
            printf("The sum is: %d\n", temp_sum); 

            printf("Calling macro DIFF: ");
            printf("The difference is: %d\n", DIFF(num1, num2));
            
            printf("Calling macro SUM: ");
            printf("The sum is: %d\n", SUMM(num1, num2)); 
            printf("\n");
        }
    }

    printf("\nThe array is: ");
    for (i = 0; i < length; i++){
        printf("%d ", arr[i]);
    }                       
    printf("\n");                

    free (arr);
    return 0;                                                                  
}

