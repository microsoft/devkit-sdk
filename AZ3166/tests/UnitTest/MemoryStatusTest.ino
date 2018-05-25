test(memory_heap)
{
  Thread heap;

  heap.start(memory_heap_test);
}

test(memory_stack)
{
  Thread stack;

  stack.start(memory_stack_test);
}

void memory_heap_test()
{
  mbed_stats_heap_t heap_info;
  int c_size;
  int max_size;
  void *allocation = malloc(1000);

  mbed_stats_heap_get(&heap_info);
  c_size = heap_info.current_size;
  max_size = heap_info.max_size;
  free(allocation);
  mbed_stats_heap_get(&heap_info);

  assertEqual(c_size - heap_info.current_size, 1000);
  assertEqual(max_size, heap_info.max_size);

  delay(LOOP_DELAY);
}

void memory_stack_test()
{
  mbed_stats_stack_t stack_info[2];
  int cnt = 2;
  int retValue;

  retValue = mbed_stats_stack_get_each(stack_info, cnt);
  assertEqual(cnt, retValue);

  for (int i = 0; i < cnt; i++)
  {
    assertNotEqual(stack_info[i].reserved_size, 0);
    assertLessOrEqual(stack_info[i].max_size, stack_info[i].reserved_size);
  }
  delay(LOOP_DELAY);
}
