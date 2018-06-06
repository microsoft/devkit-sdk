test(memory_heap)
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