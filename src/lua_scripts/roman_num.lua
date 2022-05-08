local romans = {
  {1000, "M"},
  {900, "CM"}, {500, "D"}, {400, "CD"}, {100, "C"},
   {90, "XC"},  {50, "L"},  {40, "XL"},  {10, "X"},
    {9, "IX"},   {5, "V"},   {4, "IV"},   {1, "I"}
}

local function to_roman_numerals(k)
  k = tonumber(k)
  if not k or k ~= k then error("Bad number") end
  if k == math.huge then error("Unable to convert infinity") end
  k = math.floor(k)

  if k <= 0 then return "" end

  local ret = ""
  for _, v in ipairs(romans) do
    local val, let = table.unpack(v)
    while k >= val do
      k = k - val
      ret = ret .. let
    end
  end

  return ret
end

local map = { M = 1000, D = 500, C = 100, L = 50, X = 10, V = 5, I = 1 }

local function to_number(str)
  str = str:upper()
  local ret = 0
  local i = 1
  local strlen = string.len(str)
  while i < strlen do
    local z1, z2 = map[string.sub(str, i, i)], map[string.sub(str, i+1, i+1)]
    if z1 == nil or z2 == nil then error("Bad roman number " .. str) end
    if z1 < z2 then
      ret = ret + (z2 - z1)
      i = i + 2
    else
      ret = ret + z1
      i = i + 1
    end
  end

  -- проверяем последний символ
  if i <= strlen then ret = ret + map[string.sub(str, i, i)] end

  return ret
end

-- print(1, to_roman_numerals(1))
-- print(4, to_roman_numerals(4))
-- print(5, to_roman_numerals(5))
-- print(6, to_roman_numerals(6))
-- print(9, to_roman_numerals(9))
-- print(501, to_roman_numerals(501))
-- print(1024, to_roman_numerals(1024))
-- print(9876, to_roman_numerals(9876))
-- print(944, to_roman_numerals(944))
-- print(500, to_roman_numerals(500))
-- print(600, to_roman_numerals(600))
-- print(369, to_roman_numerals(369))
-- print(0, to_roman_numerals(0))
-- print(2012, to_roman_numerals(2012))
-- print(99, to_roman_numerals(99))
-- print(999, to_roman_numerals(999))
-- print(1001, to_roman_numerals(1001))

assert(1 == to_number("I"))
assert(4 == to_number("IV"))
assert(5 == to_number("V"))
assert(6 == to_number("VI"))
assert(9 == to_number("IX"))
assert(501 == to_number("DI"))
assert(1024 == to_number("MXXIV"))
assert(9876 == to_number("MMMMMMMMMDCCCLXXVI"))
assert(944 == to_number("CMXLIV"))
assert(500 == to_number("DM"))
assert(600 == to_number("DMC"))
assert(369 == to_number("CCCLXIX"))
assert(8 == to_number("IIIIIV"))
assert(2012 == to_number("MMXII"))
assert(5 == to_number("VX"))
assert(99 == to_number("IC"))
assert(999 == to_number("IM"))
assert(1001 == to_number("MI"))

return { to_roman_numerals = to_roman_numerals, to_number = to_number }
