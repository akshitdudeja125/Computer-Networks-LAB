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

double evaluatePostfix(const char *expression)
{
    Stack stack;
    initialize(&stack, strlen(expression));

    double operand = 0;
    double fractionalPart = 0;
    bool isFractional = false;
    double decimalPlace = 0.1; // To handle decimal places

    for (int i = 0; expression[i] != '\0'; i++)
    {
        if (isdigit(expression[i]))
        {
            if (isFractional)
            {
                fractionalPart += (expression[i] - '0') * decimalPlace;
                decimalPlace /= 10;
            }
            else
                operand = operand * 10 + (expression[i] - '0');
        }
        else if (expression[i] == '.')
            isFractional = true;
        else if (expression[i] == ' ')
        {
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
                printf("Invalid expression: too many operators\n");
                exit(EXIT_FAILURE);
            }
            double operand2 = pop(&stack);
            if (isEmpty(&stack))
            {
                printf("Invalid expression: not enough operands\n");
                exit(EXIT_FAILURE);
            }
            double operand1 = pop(&stack);
            switch (expression[i])
            {
            case '+':
                push(&stack, operand1 + operand2);
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
                    printf("Division by zero error!\n");
                    exit(EXIT_FAILURE);
                }
                push(&stack, operand1 / operand2);
                break;
            case '^':
                push(&stack, pow(operand1, operand2));
                break;
            default:
                printf("Invalid operator!\n");
                exit(EXIT_FAILURE);
            }
            i++;
        }
    }

    if (isEmpty(&stack))
    {
        printf("Empty expression!\n");
        exit(EXIT_FAILURE);
    }
    double result = pop(&stack);
    if (!isEmpty(&stack))
    {
        printf("Invalid expression: too many operands\n");
        exit(EXIT_FAILURE);
    }
    return result;
}

int main()
{
    char expression[] = "100 200 + 2 / 5 * 7 +"; // Changed to array
    double result = evaluatePostfix(expression);
    printf("Result: %lf\n", result);

    return 0;
}
