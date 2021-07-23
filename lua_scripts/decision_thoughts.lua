-- нужно начать с дизайна утилити функций, как назвать? effect_util, trigger, script

-- вводить концепцию функции? хотя по сути это будут отдельные куски таблиц, которые мы будем копировать туда сюда
-- local prov_function = {
--   random_province = {
--     random_tile = {
--       value = script_utils.this
--     }
--   }
-- }
--
-- return {
--   id = "decision123",
--   name = "d_name_id_123",
--   description = "d_description_id_123", -- было бы неплохо еще получать название и описание по условиям
--   input = { char_type, char_type, number, army_type },
--   potential = {
--
--   },
--   conditions = {
--     age = script_utils.more(40),
--     influence = script_utils.equal(200),
--     learning = script_utils.more_eq(30),
--     -- могут быть еще и сложные условия
--     learning_diff = { target = script_utils.context(2), value = script_utils.more(2), abs = true}
--   },
--   action = {
--     money = script_utils.add(10),
--     influence = script_utils.remove(100),
--     -- начинаем войну, где T1 - это титул, их можно перечислить через запятую
--     -- как получить несколько объектов контекста через запятую в таблице?
--     start_war = { cb = X, target = char, claimant = Z, target_titles = { T1 } },
--     spawn_army = {
--       levies = 40, -- обычные челики
--       -- тут наверное будет по другому, нужно указывать конкретные отряды и статы?
--       men_at_arms = { { type = t123, men = 40 }, },
--       -- как получать данные из объекта в этом случае? + тут нужно указать где в провинции
--       -- то есть рандомный тайл внутри провинции, здесь должен быть вызов метода?
--       -- в цк3 примерно так и записывается обращение к данным объекта, у нас тут получается
--       -- нужно делать функции и для получения данных, их дудет поменьше
--       location = "this.random_province().random_tile()",
--       origin = prov_index,
--       war = war,
--       inheritable = true,
--       uses_supply = true,
--       army = army,
--       name = "new_name"
--     },
--     any_cortier = {
--       condition = { age = script_utils.more(40) },
--       money = script_utils.add(10)
--     },
--     -- можно вот так получать объект контекста, так я генерю строку, потом по этой строке я должен буду догадаться
--     -- а хотя зачем строку? число, часть - тип, другая число, с другой стороны может быть коллизия, хотя врядли
--     [script_utils.context(1)] = {
--       [script_utils.if_condition(1)] = { -- как можно сделать кондитион? можно генерировать уникальную строчку для таблицы
--         condition = {}
--       },
--       [script_utils.if_condition(2)] = {
--         condition = {}
--       }
--     }
--
--
--   }
-- }

-- как обращаться к полям структуры у объекта в скопе или контексте?
-- по идее мы должны обращаться через те команды которые я здесь примерно определил
-- что может попасть в скоп? по идее так мы запомним контекст какой? проблема луа в том
-- что невозможно гарантировать последовательность данных в таблице, но
-- я могу гарантировать последовательность обращений к таблице тут должен быть приоритет какой?
-- есть разница? врядли, но так ч могу гарантировать простое и однозначное обращение к скопу
-- сколько скоп должен жить?

-- local function itr_decision_func(command_name, target, value, compare_type, additional_data_type, result)
--   print("Current command " .. command_name .. " ")
-- end

-- local vassal_cond = {
--   is_male = true,
--   is_married = false,
--   money = script_utils.less(100)
-- }

-- небольшой тест
return {
  id = "test_decision123",
  name = "decisions.names.test_decision123",
  --description = "decisions.descriptions.test_decision123",
  description = { -- ожидается строка
    {
      -- desc или str?
      string = "decisions.descriptions.test_decision123",
      -- условия
    },
    {
      string = "decisions.descriptions.test_decision456",
      -- условия
    }
  },
  -- не понятно пока что как и что задавать для эвента
  -- так теперь мы задаем конкретные имена для входных данных
  -- root - это специальное имя, также его можно указать просто без имени
  -- все остальные входные вещи должны задаваться по имени
  input = { root = script_utils.target.character }, -- , attacker = script_utils.target.realm
  type = script_utils.decision.minor,
  potential = {
    is_player = true,
    is_male = true,
    is_married = false
  },
  -- условие или условия?
  conditions = {
    --is_character = true, -- ???
    money = script_utils.less(100),
    strength = script_utils.less(10),
    -- если есть хотя бы один вассал по заданным условиям
    -- has_vassal = vassal_cond,
  },
  effects = {
    --add_money = script_utils.context(2)
    add_money = 20,
    -- lvalue, берем, условно, у атакующего государства лидера, а у него дипломатию
    -- как указать тип сравнения? можно в начале строки указать тип
    add_diplomacy = "less:context:attacker.leader.diplomacy",
    add_military = {
      value = 20,
      factor = 2,
      condition = { -- если в таком виде не выполняется, то что? ноль?
        -- проверяем получим ли мы эту прибавку
      }
    },
    -- если все условия проходят, то расчитывается как (5 * 1 + 11) * 2 + 22
    add_intellect = {
      value = 5, -- здесь также можно указать lvalue
      {
        factor = 1,
        value = 11,

        -- таблицу condition здесь указывать необязательно
        -- все что не factor и не value расценивается как условие
      },
      {
        factor = 2,
        value = 22,
        condition = { -- если в таком виде не выполняется, то что? ноль?
          -- проверяем получим ли мы эту прибавку
        }
      }
    },
    -- случайный один вассал, можно задать условия что и выше
    -- random_vassal = {
    --   condition = vassal_cond,
    --   add_money = 20
    -- }
    -- эту функцию, как и другие, можно будет поставить лишь раз в блок скрипта,
    -- нужны ли мне повторяющиеся блоки? для блоков AND, OR, NAND и проч нужны
    --[script_utils.context("character")] = {}
  }
}
