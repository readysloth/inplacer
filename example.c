#include <stdio.h>
#include "inplacer.h"


void changer(char *str, size_t size){
  for(int i = 0; i < size; i++){
    str[i] += 1;
  }
}


void main(){
  char input[] = "012345678";
  inplacer_set_changer_function(changer);
  inplacer_inplace(input, sizeof(input), stdout);
}
