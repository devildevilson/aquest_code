-- ~(%f|%a)(~%f|%a)
-- (qasfsfsq%fsdqsdsf)(...)
-- !(teos|muchos)(batata|fus)

local pattern_strings_table = {
  -- необычные центры слов
  s = {
    "ach", "ack", "ad", "age", "ald", "ale", "an", "ang", "ar", "ard",
    "as", "ash", "at", "ath", "augh", "aw", "ban", "bel", "bur", "cer",
    "cha", "che", "dan", "dar", "del", "den", "dra", "dyn", "ech", "eld",
    "elm", "em", "en", "end", "eng", "enth", "er", "ess", "est", "et",
    "gar", "gha", "hat", "hin", "hon", "ia", "ight", "ild", "im", "ina",
    "ine", "ing", "ir", "is", "iss", "it", "kal", "kel", "kim", "kin",
    "ler", "lor", "lye", "mor", "mos", "nal", "ny", "nys", "old", "om",
    "on", "or", "orm", "os", "ough", "per", "pol", "qua", "que", "rad",
    "rak", "ran", "ray", "ril", "ris", "rod", "roth", "ryn", "sam",
    "say", "ser", "shy", "skel", "sul", "tai", "tan", "tas", "ther",
    "tia", "tin", "ton", "tor", "tur", "um", "und", "unt", "urn", "usk",
    "ust", "ver", "ves", "vor", "war", "wor", "yer"
  },

  -- гласные
  v = {
    "a", "e", "i", "o", "u", "y"
  },

  -- сложные гласные
  V = {
    "a", "e", "i", "o", "u", "y", "ae", "ai", "au", "ay", "ea", "ee",
    "ei", "eu", "ey", "ia", "ie", "oe", "oi", "oo", "ou", "ui"
  },

  -- согласные
  c = {
    "b", "c", "d", "f", "g", "h", "j", "k", "l", "m", "n", "p", "q", "r",
    "s", "t", "v", "w", "x", "y", "z"
  },

  --
  B = {
    "b", "bl", "br", "c", "ch", "chr", "cl", "cr", "d", "dr", "f", "g",
    "h", "j", "k", "l", "ll", "m", "n", "p", "ph", "qu", "r", "rh", "s",
    "sch", "sh", "sl", "sm", "sn", "st", "str", "sw", "t", "th", "thr",
    "tr", "v", "w", "wh", "y", "z", "zh"
  },

  C = {
    "b", "c", "ch", "ck", "d", "f", "g", "gh", "h", "k", "l", "ld", "ll",
    "lt", "m", "n", "nd", "nn", "nt", "p", "ph", "q", "r", "rd", "rr",
    "rt", "s", "sh", "ss", "st", "t", "th", "v", "w", "y", "z"
  },

  -- смешные прозвища
  i = {
    "air", "ankle", "ball", "beef", "bone", "bum", "bumble", "bump",
    "cheese", "clod", "clot", "clown", "corn", "dip", "dolt", "doof",
    "dork", "dumb", "face", "finger", "foot", "fumble", "goof",
    "grumble", "head", "knock", "knocker", "knuckle", "loaf", "lump",
    "lunk", "meat", "muck", "munch", "nit", "numb", "pin", "puff",
    "skull", "snark", "sneeze", "thimble", "twerp", "twit", "wad",
    "wimp", "wipe"
  },

  -- уменьшительно ласкательные (?)
  m = {
    "baby", "booble", "bunker", "cuddle", "cuddly", "cutie", "doodle",
    "foofie", "gooble", "honey", "kissie", "lover", "lovey", "moofie",
    "mooglie", "moopie", "moopsie", "nookum", "poochie", "poof",
    "poofie", "pookie", "schmoopie", "schnoogle", "schnookie",
    "schnookum", "smooch", "smoochie", "smoosh", "snoogle", "snoogy",
    "snookie", "snookum", "snuggy", "sweetie", "woogle", "woogy",
    "wookie", "wookum", "wuddle", "wuddly", "wuggy", "wunny"
  },

  M = {
    "boo", "bunch", "bunny", "cake", "cakes", "cute", "darling",
    "dumpling", "dumplings", "face", "foof", "goo", "head", "kin",
    "kins", "lips", "love", "mush", "pie", "poo", "pooh", "pook", "pums"
  },

  -- части для тупых имен
  D = {
    "b", "bl", "br", "cl", "d", "f", "fl", "fr", "g", "gh", "gl", "gr",
    "h", "j", "k", "kl", "m", "n", "p", "th", "w"
  },

  d = {
    "elch", "idiot", "ob", "og", "ok", "olph", "olt", "omph", "ong",
    "onk", "oo", "oob", "oof", "oog", "ook", "ooz", "org", "ork", "orm",
    "oron", "ub", "uck", "ug", "ulf", "ult", "um", "umb", "ump", "umph",
    "un", "unb", "ung", "unk", "unph", "unt", "uzz"
  }
}

local template_string = {
  string_table = {}
}

local function add(self, p)
  self.next[#self.next+1] = p
end

local function reverse(self)
  local table = self.next
  assert(#table == 1)
  local str = table[0]:func()
  return string.reverse(str)
end

local function capitalize(self)
  local table = self.next
  assert(#table == 1)
  local function first_to_upper(str)
    return (str:gsub("^%l", string.upper))
  end
  local str = table[0]:func()
  return first_to_upper(str)
end

local function random(self)
  local random_engine = template_string.random_engine
  assert(random_engine ~= nil)
  local table = self.next
  if #table == 0 then return "" end
  local val = random_engine:index(#table) -- рандом должен быть другой
  return table[val]:func()
end

local function sequence(self)
  local table = self.next
  local str = ""
  for i = 1, #table do
    str = str .. table[i]:func()
  end

  return str
end

local function create_producer(string_func)
  return {
    func = string_func,
    add = add,
    next = {}
  }
end

local function is_random(p)
  return p.func == random -- сравниваем по ссылкам, по идее нам этого достаточно
end

local function is_sequence(p)
  return p.func == sequence -- сравниваем по ссылкам, по идее нам этого достаточно
end

local function is_reverse(p)
  return p.func == reverse -- сравниваем по ссылкам, по идее нам этого достаточно
end

local function is_capitalize(p)
  return p.func == capitalize -- сравниваем по ссылкам, по идее нам этого достаточно
end

function template_string.set_table(self, literal, table)
  assert(type(literal) == "string")
  assert(#literal == 1)
  assert(type(table) == "table")
  self.string_table[literal] = table;
end

function template_string.set_pattern(self, pattern)
  assert(type(pattern) == "string")
  -- тут нужно обойти паттерн и составить функции генерации
  -- то есть нужно по символам составить цепочку функции

  self.pattern = nil

  local stack = {}

  local function submit_to_stack(p)
    while is_reverse(stack[#stack]) || is_capitalize(stack[#stack]) do
      local tmp = stack[#stack]
      stack[#stack] = nil
      tmp:add(p)
      p = tmp
    end

    stack[#stack]:add(p)
  end

  local function add_word(word)
    local new_word = ""
    local p = create_producer(function (self) return word end)
    submit_to_stack(p)

    return new_word
  end

  local function add_table(table_literal)
    assert(template_string.string_table[table_literal] ~= nil)
    local p = create_producer(function (self)
      local random_engine = template_string.random_engine
      assert(random_engine ~= nil)
      local table = template_string.string_table[table_literal]
      local val = random_engine:index(#table)
      return table[val]
    end)

    submit_to_stack(p)
  end

  local word = ""
  stack[#stack+1] = create_producer(sequence)
  assert(is_sequence(stack[#stack]))
  for i = 1, #str do
    local c = str:sub(i,i)

    if c == '(' then
      if word ~= "" then word = add_word(word) end

      stack[#stack+1] = create_producer(random)
      local p = create_producer(sequence)
      stack[#stack]:add(p)
      stack[#stack+1] = p
    elseif c == ')' then
      if word ~= "" then word = add_word(word) end
      assert(#stack > 2)
      stack[#stack] = nil
      local cur = stack[#stack]
      stack[#stack] = nil

      submit_to_stack(cur)
    elseif c == '|' then -- или одно слово (указанное в паттерне) или другое
      if word ~= "" then word = add_word(word) end
      assert(#stack > 1)
      assert(is_sequence(stack[#stack]))
      stack[#stack] = nil
      local p = create_producer(sequence)
      stack[#stack]:add(p)
      stack[#stack+1] = p
    elseif c == '%' then -- берем из таблицы
      if word ~= "" then word = add_word(word) end
      local next = i+1
      assert(#str >= next)
      local table_literal = str:sub(next,next)
      add_table(table_literal)
      i=next
    elseif c == '~' then -- обратный порядок
      if word ~= "" then word = add_word(word) end
      local p = create_producer(reverse)
      stack[#stack+1] = p
    elseif c == '!' then -- увеличиваем первую букву
      if word ~= "" then word = add_word(word) end
      local p = create_producer(capitalize)
      stack[#stack+1] = p
    else word = word .. c end
  end

  assert(#stack == 1)
  self.pattern = stack[#stack]
  stack[#stack] = nil
  stack = nil

  collectgarbage("collect")
end

function template_string.produce(self)
  -- по идее здесь будет довольно длинная рекурсия
  return self.pattern:func()
end

function template_string.set_random(self, random_engine)
  self.random_engine = random_engine
end

for k,v in pairs(pattern_strings_table) do
  assert(type(k) == "string")
  assert(#k == 1)
  template_string:set_table(k, v)
end

return template_string
