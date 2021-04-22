local methods = {
  push_right = function(self, value)
    assert(value ~= nil)
    self.tail = self.tail + 1
    self[self.tail] = value
  end,
  push_left = function(self, value)
    assert(value ~= nil)
    self[self.head] = value
    self.head = self.head - 1
  end,
  pop_right = function(self)
    if self:is_empty() then return nil end
    local r = self[self.tail]
    self[self.tail] = nil
    self.tail = self.tail - 1
    return r
  end,
  pop_left = function(self)
    if self:is_empty() then return nil end
    self.head = self.head + 1
    local r = self[self.head]
    self[self.head] = nil
    return r
  end,
  right = function(self) return self[self.tail] end,
  left = function(self) return self[self.head] end,
  size = function(self) return self.tail - self.head end,
  is_empty = function(self) return self:size() == 0 end,
  clear = function(self)
    local k = next(self)
    while k ~= nil do
      self[k] = nil
      k = next(self)
    end
    --for k,_ in pairs(self) do self[k] = nil end
    self.tail = 0
    self.head = 0
  end
}

local function new() return setmetatable({head = 0, tail = 0}, {__index = methods}) end
return {new = new}
