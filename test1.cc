#include "compact_3d_lookup.h"
#include <iostream>

int
main(int argc, char**argv)
{
  // 16x16x16 space, 8 values max 
  compact_3d_lookup<int, 4, 4, 4, 3> lookup_table;
  lookup_table.push_back(1, 0, 9, 9);
  lookup_table.push_back(10, 0, 8, 0);
  lookup_table.do_index();
  assert(lookup_table(0, 9, 9) == 1);
  assert(lookup_table(0, 8, 0) == 10);
  assert(lookup_table.find(0, 8, 0) != lookup_table.end());
  assert(lookup_table.find(0, 9, 0) == lookup_table.end());
}
