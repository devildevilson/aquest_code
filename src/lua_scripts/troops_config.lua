local troop_type1 = {
  id = "troop_type1",
  name_id = "troops.troop_type1.name",
  description_id = "troops.troop_type1.desc",
  -- нужно еще указать тип формирования, там должна храниться иконка + название типа
  --formation_type = "heavy_footman", -- пока так оставим
  stats = { -- по идее статы с 0 можно не указывать
    accuracy = 50,
    ammo = 0,
    armor = 62,
    charge = 17,
    fire_resistance = 25,
    initiative = 10,
    magic_resistance = 0,
    maintenance = 10,
    maintenance_factor = 0,
    max_hp = 10000,
    melee_armor_piercing = 10,
    melee_attack = 32,
    melee_damage = 40,
    melee_defence = 59,
    melee_fire = 0,
    melee_magic = 0,
    melee_siege = 0,
    morale = 55,
    morale_damage = 0,
    provision = 10,
    provision_factor = 0,
    range_armor_piercing = 0,
    range_attack = 0,
    range_damage = 0,
    range_defence = 0,
    range_fire = 0,
    range_magic = 0,
    range_siege = 0,
    reinforce = 10,
    reinforce_factor = 0,
    reloading = 10 --[[?]],
    siege_armor = 5,
    speed = 23,
    troop_size = 120,
    vision = 3,
  }
}

return {
  troop_type1
}
