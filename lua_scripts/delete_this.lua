local abc = thread_pool.create_table()
abc.ret = 4

-- thread_pool.submit(function(abc)
--   print(abc.ret)
-- end, abc)
--
-- thread_pool.submit(function(abc)
--   print(abc.ret)
-- end, abc)
--
-- thread_pool.submit(function(abc)
--   print(abc.ret)
-- end, abc)

--print(thread_pool.free_threads())
--assert(thread_pool.free_threads() == 7)

-- for i = 0, 8-1 do
--   print(i)
-- end

thread_pool.submit_works(8, function(start, count, abc)
  for i = start, start+count-1 do
    --abc[i] = assign_physical_properties()
    local id = effil.thread_id()
    --print(abc.ret, id)
    abc[i] = id
  end
end, abc)

thread_pool.submit_works(16, function(start, count, abc)
  for i = start, start+count-1 do
    --abc[i] = assign_physical_properties()
    local id = effil.thread_id()
    --print(abc.ret, id)
    --abc.ret = abc.ret + 1
    abc[i] = id
  end
end, abc)

--print("works_count ", thread_pool.works_count())
--print("free_threads ", thread_pool.free_threads())

thread_pool.compute()
--thread_pool.wait()

effil.sleep(2)

--print(abc.ret)
for i = 0, 16-1 do
  print(abc[i])
end

-- короче thread_pool.wait() не работает, сколько бы я не пытался его реанимировать
-- нам нужно убедиться, что у нас шаг последовательно идет по функциям
-- это не гарантируется если мы просто закидываем данные в effil.channel
-- с другой стороны есть гарантии что мы последовательно берем функции из канала
-- нам в любом случае нужен какой то способ подождать потоки
-- можно как костыль складывать в канал какие то данные, и по количеству потоков выходить

print("main thread id ", effil.thread_id())

local a = 1
print(a)

--print(abc.ret)

-- thread_pool.submit_works(8,
--   function(start, job_count, atomic_val)
--     --local a = atomic_val:fetch_add(1)
--     local a = 1
--     print(a)
--   end,
--   atomic_val
-- )
