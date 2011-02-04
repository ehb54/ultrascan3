#include <stdio.h>
#include <string.h>
#include "../include/us_ga.h"
#include "../include/us_ga_stacks.h"

value stack[MAX_STACKS][MAX_STACK_DEPTH];
int sp[MAX_STACKS];

void stacks_init() {
  int i;
  for(i = 0; i < MAX_STACKS; i++) {
    sp[i] = 0;
    memset(stack[i], 0, sizeof(value) * MAX_STACK_DEPTH);
  }
}

void push_stack(int s, value arg) {
  if(sp[s] > MAX_STACK_DEPTH) {
    //    fprintf(stderr, "stack overflow\r\n");
  } else {
    stack[s][sp[s]] = arg;
    sp[s]++;
  }
}

value pop_stack(int s) {
  if(sp[s] == 0) {
    return 0;
  } else {
    sp[s]--;
    return stack[s][sp[s]];
  }
}

int nonempty_stack(int s) {
  if(sp[s] == 0) {
    return 0;
  } else {
    return 1;
  }
}

void print_stack(int s) {
  int i;
  for(i = sp[s] - 1; i >= 0; i--) {
    printf("\t%d %d %g\n", s, i, stack[s][i]);
  }
}
