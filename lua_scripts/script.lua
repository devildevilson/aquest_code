local effil     = require("effil")
local llthreads = require("llthreads2")
local mtmsg     = require("mtmsg")
local mtstates  = require("mtstates")
local config_table = {with_timers = false, verbose_errors = true, demote_full_userdata = false}
local lanes     = require("lanes").configure(config_table)

local function thread_pool_create()
  local self = {
    thread_count = math.max(effil.hardware_threads()-1, 1),
    func_queue = effil.channel(nil),
    thread_main = function(channel, free_threads_count)
      while true do
        local func = channel:pop()
        free_threads_count:pop()
        func()
        free_threads_count:push(1)
      end
    end,

    thread_spawner = function(thread_count, thread_main, func_queue, free_threads_count)
      local threads = {}
      for i = 0, thread_count-1 do
        threads[i] = effil.thread(thread_main)(func_queue, free_threads_count)
        free_threads_count:push(1)
      end
      return threads
    end,
  }

  --print("thread_count ",self.thread_count)

  self.free_threads_count = effil.channel(self.thread_count)
  self.threads = self.thread_spawner(self.thread_count, self.thread_main, self.func_queue, self.free_threads_count)

  local submit = function(func, ...)
    assert(type(func) == "function", "expected function")
    local arg = {...}
    self.func_queue:push(
      function()
        local arg1 = arg
        local func1 = func
        func1(table.unpack(arg1))
      end
    )
  end

  local submit_works = function(count, func, ...)
    local arg = {...}
    local works_count = math.ceil(count / (self.thread_count+1))
    local start = 0
    for i = 0, self.thread_count do
      local job_count = math.min(works_count, count-start)
      if job_count == 0 then break end
      submit(func, start, job_count, table.unpack(arg))
      start = start + job_count
    end
  end

  local create_table = function()
    return effil.table()
  end

  local clear = function()
    while true do
      if func_queue:size() == 0 then break end
    end

    for i = 0, self.thread_count-1 do
      self.threads[i]:cancel()
    end
  end

  local free_threads = function()
    return self.free_threads_count:size()
  end

  local works_count = function()
    return self.func_queue:size()
  end

  -- local thread_id = function()
  --   for i = 0, self.thread_count-1 do
  --     self.threads[i]
  --   end
  -- end

  local wait = function()
    local t1 = os.time()
    repeat
      --print("free_threads()   ",free_threads())
      --print("func_queue:size()",self.func_queue:size())
      -- local counter = 0
      -- for i = 0, self.thread_count-1 do
      --   local status, err, trace = self.threads[i]:status()
      --   if status == "paused" then counter = counter + 1 end
      -- end
      local t2 = os.time()
      if os.difftime(t2, t1) > 2.0 then error("waiting threads takes too long") end
    until free_threads() == self.thread_count and self.func_queue:size() == 0
  end

  local compute = function()
    while true do
      if self.func_queue:size() == 0 then return end
      local func = self.func_queue:pop(0)
      if func == nil then return end
      func()
    end
  end

  return {
    submit = submit,
    submit_works = submit_works,
    create_table = create_table,
    clear = clear,
    free_threads = free_threads,
    works_count = works_count,
    compute = compute,
    wait = wait
  }
end

local function thread_pool_create2(threads_count)
  local thread_count = math.max(threads_count-1, 1)
  local linda = lanes.linda()
  -- local buffer = mtmsg.newbuffer()
  -- local state = mtstates.newstate(
  --   function(buffer_id)
  --     local list = {}
  --     function list.new()
  --       return {
  --         first = 0,
  --         last = -1,
  --         push_front = function(self, value)
  --           local first = self.first - 1
  --           self.first = first
  --           self[first] = value
  --         end,
  --
  --         push_back = function(self, value)
  --           local last = self.last + 1
  --           self.last = last
  --           self[last] = value
  --         end,
  --
  --         pop_front = function(self)
  --           local first = self.first
  --           if first > self.last then error("list is empty") end
  --           local value = self[first]
  --           self[first] = nil        -- to allow garbage collection
  --           self.first = first + 1
  --           return value
  --         end,
  --
  --         pop_back = function(self)
  --           local last = self.last
  --           if self.first > last then error("list is empty") end
  --           local value = self[last]
  --           self[last] = nil         -- to allow garbage collection
  --           self.last = last - 1
  --           return value
  --         end,
  --
  --         size = function(self)
  --           return self.last + 1 - self.first
  --         end
  --       }
  --     end
  --
  --     --local queue = list.new()
  --     local mtmsg = require("mtmsg")
  --     local queue = mtmsg.buffer(buffer_id)
  --     local free_threads = {}
  --     local cancel = false
  --     local cmds = {
  --       push = function(arg)
  --         -- добавляем функцию в очередь
  --         -- есть ли в луа очереди?
  --         --queue:push_back(arg)
  --         queue:addmsg(arg)
  --       end,
  --       pop = function(thread_id)
  --         local msg = queue:nextmsg(0)
  --         if cancel then return 1 end
  --         if msg == nil then
  --           free_threads[thread_id] = 1
  --           return 0
  --         end
  --         return msg
  --         -- if cancel then return 1 end
  --         -- if queue:size() == 0 then
  --         --   free_threads[thread_id] = 1
  --         --   return 0
  --         -- end
  --         -- free_threads[thread_id] = 0
  --         -- return queue:pop_front()
  --       end,
  --       int = function()
  --         cancel = true
  --       end,
  --       wait = function(thread_count)
  --         local count = 0
  --         for i = 0, thread_count-1 do
  --           count = count + free_threads[i]
  --         end
  --         --return count == thread_count and queue:size() == 0
  --         return count == thread_count and queue:nextmsg(0) == nil
  --       end
  --     }
  --
  --     return function(cmd, ...)
  --       return cmds[cmd](...)
  --     end
  --   end,
  --   buffer:id()
  -- )

  -- local thread_main1 = function(buffer_id)
  --   local mtmsg  = require("mtmsg")
  --   local buffer = mtmsg.buffer(buffer_id)
  --   --buffer:nonblock(false)
  --   while true do
  --     local func = buffer:nextmsg()
  --     func()
  --   end
  -- end

  local thread_main2 = function() -- state_id, thread_id
    -- local mtstates = require("mtstates")
    -- local mtmsg = require("mtmsg")
    -- local state = mtstates.state(stateId)
    -- local id = thread_id
    while true do
      -- local func = state:call("pop", id)
      local key, func = linda:receive(nil, "task")

      if func == 1 then return
      -- elseif func == 0 then mtmsg.sleep(0.01)
      elseif func == 0 then
        linda:send(nil, "ready", 1)
        linda:receive(nil, "sync")
      else
        assert(type(func) == "function")
        func()
      end
    end
  end

  local threads = {}
  for i = 0, thread_count-1 do
    --threads[i] = llthreads.new(thread_main2, state:id(), i)
    -- threads[i] = llthreads.new(
    -- [[
    --   local state_id, thread_id = ...
    --   local mtstates = require("mtstates")
    --   local mtmsg = require("mtmsg")
    --   local state = mtstates.state(state_id)
    --   local id = thread_id
    --   while true do
    --     local func = state:call("pop", id)
    --
    --     if func == 1 then return
    --     elseif func == 0 then mtmsg.sleep(0.01)
    --     else func() end
    --   end
    -- ]], state:id(), i)
    -- threads[i]:start()
    threads[i] = lanes.gen("base", "table", {clonable = clonable,}, thread_main2)()
  end

  local submit = function(func, ...)
    assert(type(func) == "function", "expected function")
    local arg = {...}
    local func2 = func
    -- buffer:addmsg(
    --   function()
    --     local arg1 = arg
    --     local func1 = func2
    --     func1(table.unpack(arg1))
    --   end
    -- )
    -- state:call("push",
    --   function()
    --     local arg1 = arg
    --     local func1 = func
    --     func1(table.unpack(arg1))
    --   end
    -- )
    -- buffer:addmsg(func2, arg)

    linda:send(nil, "task",
      function()
        local arg1 = arg
        local func1 = func
        func1(table.unpack(arg1))
      end
    )
  end

  local submit_works = function(count, func, ...)
    --local arg = {...}
    local works_count = math.ceil(count / (thread_count+1))
    local start = 0
    for i = 0, thread_count do
      local job_count = math.min(works_count, count-start)
      if job_count == 0 then break end
      submit(func, start, job_count, ...) -- table.unpack(arg)
      start = start + job_count
    end
  end

  -- мы можем передать какие нибудь произвольные сообщения в буфер
  -- чтобы проверить готовность тредов
  -- нет не можем
  local wait = function()
    -- local end_work_message = mtmsg.newbuffer()
    -- --end_work_message:nonblock(false) -- по идее по умолчанию false
    -- submit_works(self.thread_count, function(buffer_id)
    --   local buffer = mtmsg.buffer(buffer_id)
    --   --buffer:nonblock(false)
    --   buffer:addmsg(1)
    -- end,
    -- end_work_message:id())
    --
    -- for i = 0, self.thread_count-1 do
    --   buffer:nextmsg()
    -- end
    -- repeat
    --   mtmsg.sleep(0.01)
    -- until state:call("wait", thread_count)

    for i = 0, thread_count-1 do
      linda:send(nil, "task", 0)
    end

    linda:receive(nil, linda.batched, "ready", thread_count)
    for i = 0, thread_count-1 do
      linda:send(nil, "sync", 0)
    end
  end

  local compute = function()
    while true do
      --local func = state:call("pop", id)
      local key, func = linda:receive(0.0, "task")
      if type(func) ~= "function" then return end
      func()
    end
  end

  local free = function()
    --state:call("int")
    for i = 0, thread_count-1 do
      linda:send(nil, "task", 1)
    end

    for i = 0, thread_count-1 do
      --threads[i]:join()
      local a = threads[i][1]
    end
  end

  return {
    submit = submit,
    submit_works = submit_works,
    wait = wait,
    compute = compute,
    free = free
  }
end

-- сделать бы тут thread pool
--local thread_pool = thread_pool_create()

-- function norm()
--   print(rand)
--   return rand:norm()
-- end

function generate_plates(thread_count)
  local plates_count = 50
  --local rand = random_engine.new(1)
  print(1)
  --register_clonable()
  print(1)
  local rand = new_clonable()
  print(1)
  print(rand)
  --print(clonable)
  --print(clonable.__lanesclone)
  --clonable = {}
  --clonable.__lanesclone = function(self, obj)
  --  self = obj
  --end
  --setmetatable(rand, clonable)
  print(getmetatable(rand))
  print(getmetatable(rand).__lanesclone)
  --local thread_count = 8
  --local plates = thread_pool.create_table()

  lanes_clonable_user_data.__pairs = function(tbl)
    -- Iterator function takes the table and an index and returns the next index and associated value
    -- or nil to end iteration

    local function stateless_iter(tbl, k)
      local v
      -- Implement your own key,value selection logic in place of next
      k, v = next(tbl, k)
      if nil~=v then return k,v end
    end

    -- Return an iterator function, the table, starting point
    return stateless_iter, tbl, nil
  end

  lanes_clonable_user_data.__eq = function(tbl1, tbl2)
    return true
  end

  local lane_globals = {}
  lane_globals.lanes_clonable_user_data = lanes_clonable_user_data
  local g = lanes.gen("base", {globals = lane_globals}, function(rand)
    print(rand)
  end
  )

  h = g(rand)
	local from_lane = h[1]
  -- local thread_pool = thread_pool_create2(thread_count)
  -- print(rand)
  -- print(thread_pool.submit_works)
  -- thread_pool.submit_works(8, function(start, count, rand) -- map, plate_tile_indices, ocean_percentage, rand
  --   for i = start, start+count-1 do
  --     --plates[i] = assign_physical_properties(map, plate_tile_indices, ocean_percentage, i, rand)
  --     --print(1)
  --     --print("thread",1)
  --     print(rand:norm())
  --   end
  -- end, rand) --map, plate_tile_indices, ocean_percentage, rand

  --thread_pool.submit(function() print("kmvivawknkjwnldsvnvjoawjvnaojvn;oavndksjvv") end)

  -- это конечно идеальный случай
  -- с существующими ограничениями он практически невозможен
  -- короче нужно искать либо треды с юзердатой
  -- либо как то по умному вызывать из луа с++ треды (или по умному организовать это дело в с++ коде)
  -- либо перед началом работы перекопировать все что возможно в луа таблицы

  --effil.sleep(5)
  --thread_pool.wait()
  --thread_pool.free();
  print(1)

end

function test_function()
  print("from another state")
end

local PI = 3.141592653589793238462643383279502884197169399375105820974944592307816406286208998628034825342117067982148086513282306647093844609550582231725359408128481117450284102701938521105559644622948954930381964428810956659334461284756482337867831652712019091456485669234603486104543266482133936072602491412737245870066063155881748815209209628292540917153643678925903600113305305488204665213841469519415116094

function got_problems(error_msg)
	return "got_problems handler: " .. error_msg
end

function assign_physical_properties(map, plate_tile_indices, ocean_percentage, plate_index, rand)
  local min_drift_rate = -PI / 30.0
  local max_drift_rate =  PI / 30.0
  local min_spin_rate  = -PI / 30.0
  local max_spin_rate  =  PI / 30.0

  local min_oceanic_elevation     = -0.8
  local max_oceanic_elevation     = -0.3
  local min_continental_elevation =  0.1
  local max_continental_elevation =  0.5

  -- local tile_indices = plate_tile_indices[plate_index]

  local drift_axis = rand:unit3()
  local drift_rate = rand:closed(min_drift_rate, max_drift_rate)
  -- тут берем случайный тайл на плите, но вообще можно любой случайный тайл брать
  -- local rand_index = tile_indices[rand.index(tile_indices.size())]
  local rand_index = plate_tile_indices[rand:index(plate_tile_indices:size())]
  local spin_axis = map.points[map.tiles[rand_index].index]
  local spin_rate = rand:closed(min_spin_rate, max_spin_rate)
  local oceanic = rand:probability(ocean_percentage)
  local base_elevation = 0.0
  if oceanic then
    base_elevation = rand:closed(min_oceanic_elevation, max_oceanic_elevation)
  else
    base_elevation = rand:closed(min_continental_elevation, max_continental_elevation)
  end

  local tectonic_plate_info = tectonic_plate_t:new()
  tectonic_plate_info.drift_axis = drift_axis
  tectonic_plate_info.drift_rate = drift_rate
  tectonic_plate_info.spin_axis = spin_axis
  tectonic_plate_info.spin_rate = spin_rate
  tectonic_plate_info.base_elevation = base_elevation
  tectonic_plate_info.oceanic = oceanic

  return tectonic_plate_info
end

function generate_map(map)
  local plate_count = 50
  -- local array = vector:new(plate_count)
  -- submit_works(plate_count, function(start, count)
  --   for i = start, start+count-1 do
  --     array[i] = i
  --   end
  -- end)

  print(plate_count)
  print(map.tiles:size())
end
