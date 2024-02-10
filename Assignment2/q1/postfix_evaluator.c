#include "postfix_evaluator.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

void initialize(Stack *s, unsigned capacity)
{
    s->top = -1;
    s->size = capacity; // Initial size
    s->items = (double *)malloc(s->size * sizeof(double));
    if (s->items == NULL)
    {
        printf("Memory allocation error!\n");
        exit(EXIT_FAILURE);
    }
}

bool isEmpty(const Stack *s)
{
    return s->top == -1;
}

void push(Stack *s, double value)
{
    if (s->top == s->size - 1)
    {
        s->size *= 2; // Double the size
        s->items = (double *)realloc(s->items, s->size * sizeof(double));
        if (s->items == NULL)
        {
            printf("Memory reallocation error!\n");
            exit(EXIT_FAILURE);
        }
    }
    s->items[++s->top] = value;
}

double pop(Stack *s)
{
    if (!isEmpty(s))
    {
        return s->items[s->top--];
    }
    else
    {
        printf("Stack underflow!\n");
        exit(EXIT_FAILURE);
    }
}

double evaluatePostfix(const char *expression, int *errorCode, char *errorMessage)
{
    *errorCode = 0;
    Stack stack;
    initialize(&stack, strlen(expression));

    double operand = 0;
    double fractionalPart = 0;
    bool isFractional = false;
    double decimalPlace = 0.1; // To handle decimal places
    bool isNegative = false;   // Flag to track negative numbers

    for (int i = 0; expression[i] != '\0'; i++)
    {
        // for(int i=0;i<=stack.top;i++)
        //     printf("%lf ",stack.items[i]);
        // printf("\n");
        if (isdigit(expression[i]))
        {
            if (expression[i + 1] == '\0')
            {
                operand = -1;
                break;
            }
            if (isFractional)
            {
                fractionalPart += (expression[i] - '0') * decimalPlace;
                decimalPlace /= 10;
            }
            else
                operand = operand * 10 + (expression[i] - '0');
        }
        else if (expression[i] == '-' && (i == 0 || expression[i - 1] == ' '))
        {
            // Check if the '-' sign represents a negative number
            isNegative = true;
        }
        else if (expression[i] == '.')
            isFractional = true;
        else if (expression[i] == ' ')
        {
            if (isNegative)
            {
                operand = -operand; // Make the operand negative
                isNegative = false; // Reset negative flag
            }
            if (isFractional)
            {
                operand += fractionalPart;
                fractionalPart = 0;
                decimalPlace = 0.1;
                isFractional = false;
            }
            push(&stack, operand);
            operand = 0; // Reset operand
        }
        else
        {
            if (isEmpty(&stack))
            {
                *errorCode = 1; // Too many operators error code
                strcpy(errorMessage, "Too many operators");
                return 0; // Return 0 to indicate error
            }
            double operand2 = pop(&stack);
            if (isEmpty(&stack))
            {
                *errorCode = 2; // Not enough operands error code
                strcpy(errorMessage, "Not enough operands");
                return 0; // Return 0 to indicate error
            }
            double operand1 = pop(&stack);
            switch (expression[i])
            {
            case '+':
                push(&stack, operand1 + operand2);
                // printf("%lf",stack.top);
                break;
            case '-':
                push(&stack, operand1 - operand2);
                break;
            case '*':
                push(&stack, operand1 * operand2);
                break;
            case '/':
                if (operand2 == 0)
                {
                    *errorCode = 3; // Division by zero error code
                    strcpy(errorMessage, "Division by zero");
                    return 0; // Return 0 to indicate error
                }
                push(&stack, operand1 / operand2);
                break;
            case '^':
                push(&stack, pow(operand1, operand2));
                break;
            default:
                *errorCode = 4; // Invalid operator error code
                strcpy(errorMessage, "Invalid operator");
                return 0; // Return 0 to indicate error
            }
            if (expression[i + 1] != '\0')
                i++;
        }
    }

    if (isEmpty(&stack))
    {
        *errorCode = 5; // Empty expression error code
        strcpy(errorMessage, "Empty expression");
        return 0; // Return 0 to indicate error
    }
    if (operand == -1)
    {
        // printf("%lf\n", stack.items[stack.top]);
        *errorCode = 6; // Too many operands error code
        strcpy(errorMessage, "Too many operands");
        return 0; // Return 0 to indicate error
    }
    double result = pop(&stack);
    // printf("Error Code %d\n", *errorCode);
    // printf("Error Message %s\n", errorMessage);
    // printf("Result %lf\n", result);
    return result;
}
// 100.55 200.77 + 2.13 / -5 * 7.34 +