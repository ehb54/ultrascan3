#ifndef US_GA_STACKS_H
#define US_GA_STACKS_H

#define MAX_STACK_DEPTH  250000
#define MAX_STACKS       5
#define RESULT_STACK     0
#define ARGUMENT_STACK   1
#define CONCENTRATION_STACK   2
#define CONSTANTS_STACK   3
#define TMP_STACK   4

extern value stack[MAX_STACKS][MAX_STACK_DEPTH];
extern int sp[MAX_STACKS];

void stacks_init();
void push_stack(int , value );
value pop_stack(int );
int nonempty_stack(int );
void print_stack(int ); 

#endif
