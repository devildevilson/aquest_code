-- имеет смысл сделать расчет времени как "years:1", "months:1", "days:1"
-- округлять к большему? скорее всего, но с другой стороны если нужен 1 ход,
-- то можно просто написать time = 1

return {
  {
    id = "hire_physician_decision",
    name = "hire_physician_decision.name",
    description = "hire_physician_decision.desc",
    confirm_text = "hire_physician_decision.confirm_text",
    -- проверяет ли ии potential?
    ai_potential = { false },
    potential = { is_ruler = true },
    condition = { is_imprisoned = false },
    effect = {
      hidden_effect = {
  			add_character_flag = {
  				flag = "health_3001_hire_physician_decision_text",
  				time = 3 -- указываются ходы, нужно ли преобразовывать из дней, месяцев, лет в ходы? было бы неплохо
  			}
  		},

  		trigger_event = {
  			id = "health.3001",
  			time = 2
  		},
  		custom_tooltip = hire_physician_decision_effect_tooltip
    },
    -- numbers
    ai_will_do = { 0 },
    ai_check_frequency = { 0 },
    money_cost = { 0 },
    authority_cost = { 0 },
    esteem_cost = { 0 },
    influence_cost = { 0 },
    cooldown = { time = "years:1" }
  }
}
