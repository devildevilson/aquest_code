-- в государстве что нам важно показать? текущие права, политические силы государства,
-- налоги, вассалы, двор (?)

local function government_view(character)
  local realm = character.self_realm

  for title in realm:titles() do

  end

  for vassal in realm:vassals() do

  end

  for prisoner in realm:prisoners() do

  end

  for courtier in realm:courtiers() do

  end

  -- че делать с циклом? сейчас realm.state_rights_count - это оффсет + каунт
  -- название неверное, но используя одну переменную, можно избежать ошибок
  -- не придется писать сумму каждый раз, для чего мне каунт вообще может пригодиться? может,
  -- но у меня каунт лежит в utils.realm_rights.count, поэтому тут достаточно сделать переменную end
  -- может быть стоит назвать переменную begin? по аналогии с с++
  for i = realm.state_rights_start, realm.state_rights_end do
    -- по этому имени нужно будет получить наверное строку локализации
    local right_name = utils.get_right_name(i)
    local has_right = realm:has_right(i)
  end

  -- реалм-государство, обычно авторитарное с единственным лидером, но в этом случае по идее его не существует
  if realm.state:valid() then
    local r = realm.state:get()
  end

  if realm.council:valid() then
    local r = realm.council:get()
    for i = r.power_rights_start, r.power_rights_end do
      -- обходим права парламента, верное имя получаем по индексу, в реальности скорее всего
      -- нужно будет делать отдельную функцию с локализацией всех прав, а там обходом не отделаешься
      local right_name = utils.get_right_name(i)
      local has_right = r:has_right(i)
    end
  end

  if realm.tribunal:valid() then
    local r = realm.tribunal:get()
  end

  if realm.assembly:valid() then
    local r = realm.assembly:get()
  end

  if realm.clergy:valid() then
    local r = realm.clergy:get()
  end

  -- почти все перечисления в реалме скорее всего не будут использовать цикл (?)
  for i = realm.stats_start, realm.stats_end do
    local stat_name = utils.get_stat_name(i)
    -- возможно будет не лишним указать как посчиталась переменная?
    -- в принципе можно указать в тултипе
    local stat = realm:get_stat(i)
    local base_stat = realm:get_base_stat(i)
  end

  for i = realm.resources_start, realm.resources_end do
    local stat_name = utils.get_stat_name(i)
    local resource = realm:get_resource(i)
  end

  -- обходим войны и союзы

  -- господин текущего реалма
  if realm.liege:valid() then
    local r = realm.liege:get()
  end
end

-- у нас может быть ситуация такая: религия неорганизована, но при этом в стране у монахов
-- есть институт, с какой то реальной властью, на основе института в государстве может быть сделана
-- организованность всей религии (то есть примерно как то так было с христианством)
-- что мне нужно в этом случае? мне нужно вообще то говоря и понимать ситуацию в целом в религии
-- и немаловажно понимать ситуацию с религией внутри страны
local function religion_view()

end

-- пока на этом все, все вышеперечисленное нужно уместить в интерфейсе
-- перечисления придворных и заключенных скорее всего уйдут в другое место
-- перечисление титулов займут не очень много места, важно указать вассалов
-- где разместить статы? видимо будут сверху, где то рядом должны еще находиться
-- права и законы государства, к этому еще механику электоров нужно предусмотреть

return government_view
