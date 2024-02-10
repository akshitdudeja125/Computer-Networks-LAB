#ifndef POSTFIX_EVALUATOR_H
#define POSTFIX_EVALUATOR_H

#include <stdbool.h>

typedef struct Stack
{
    double *items;
    int top;
    int size;
} Stack;

void initialize(Stack *s, unsigned capacity);
bool isEmpty(const Stack *s);
void push(Stack *s, double value);
double pop(Stack *s);
double evaluatePostfix(const char *expression, int *errorCode, char *errorMessage);

#endif /* POSTFIX_EVALUATOR_H */
