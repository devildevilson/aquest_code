-- тут сразу понятно что будет невероятно много обращений к контексту,
-- было бы неплохо как нидубь сократить обращения к хеш мапе
-- нужно по этим именам задать переменные по индексу
-- скорее всего придется переписать довольно много вещей

-- нужно будет много чего изменить, чтобы это не выглядело совсем как копия

-- в какой то момент добавиться поиск жены, для него нужно делать фильтр
-- в поиске жены требуется 4 объекта: чел, вайфу, владелец чела, владелец вайфу
-- причем неочевидно кто актор и кто ресипиент (но скорее всего владельцы персонажей)
-- по фильтру я в интерфейсе должен буду найти нужно персонажа

return {
  -- {
  --   id = "declare_war_interaction",
  -- 	--category = "interaction_category_diplomacy",
  -- 	--common_interaction = yes
  -- 	--special_interaction = declare_war_interaction
  -- 	--interface = declare_war
  -- 	--interface_priority = 70
  --   type = character_interaction, -- нужно уаказать тип у интеракции
  -- 	popup_on_receive = true, -- нам нужно дать понять игроку, что началась война
  -- 	pause_on_receive = true, -- пауза? нужно чтобы игрок как то отреагировал на сообщение конечно
  --
  --   name = "declare_war_name",
  -- 	description = "declare_war_desc",
  --
  -- 	potential = {
  -- 		--NAND = { ["context:recipient"] = { equals_to = "context:actor" } },
  -- 		--["context:actor"] = { NAND = { is_at_war_with = "context:recipient" } },
  --     ["context:recipient"] = { is_landed = true, not_equals_to = "context:actor", NAND = { is_at_war_with = "context:actor" } },
  -- 	},
  --
  --   -- название такое себе (наверное condition)
  --   -- кажется эти условия видны только тогда когда некоторые из них проваливаются
  -- 	condition = {
  -- 		["context:actor"] = {
  -- 			NAND = {
  --         has_trait = "incapable",
  --         has_raised_armies = true,
  --         is_allied_in_war = "context:recipient"
  --       },
  -- 			can_attack_in_hierarchy = "context:recipient", -- ???
  -- 			has_any_display_cb_on = "context:recipient", -- display?
  -- 			custom_description = { text = "is_not_bankrupt", gold = 1 }, -- по умолчанию >=
  -- 			is_imprisoned = false,
  -- 			{
  --         -- нету листов пока еще
  -- 				condition = { has_variable_list = "subjugation_offer_under_consideration" },
  -- 				custom_description = {
  --           -- этот текст попадет как аргумент в таблице
  -- 					text = "is_not_considering_offer_of_subjugation",
  -- 					NAND = {
  -- 						is_target_in_variable_list = { -- нету возможности проверить листы
  -- 							name = "subjugation_offer_under_consideration",
  -- 							target = "context:recipient"
  -- 						}
  -- 					}
  -- 				}
  -- 			}
  -- 		},
  -- 		--["context:recipient"] = {
  -- 		["context:recipient"] = {
  --       NAND = { has_strong_hook = "context:actor" },
  -- 			{
  -- 				condition = { is_imprisoned = true },
  --         -- проверить кто тюремщик можно без проверки, у меня сейчас ипризонер - это реалм
  --         -- хотя наверное нужно разделить собственно импризонера и тюрьму
  -- 				NAND = { imprisoner = "context:actor" }
  -- 			}
  -- 		}
  -- 	},
  --
  --   -- для чего это? проверка таргета?
  -- 	has_valid_target_showing_failures_only = {
  -- 		custom_description = {
  -- 			text = "declare_war_hook_on_liege",
  -- 			OR = { war_declarer_needs_hook_on_liege = false, always = "context:hook" } -- always (в этом контексте это проверка существует ли переменная hook)
  -- 		},
  -- 		["context:actor"] = {
  -- 			NOR = {
  -- 				is_at_war_with = "context:recipient",
  -- 				custom_description = { -- A vassal cannot go to war against someone their Liege is already at war with
  -- 					text = "liege_is_at_war_with_recipient",
  -- 					object = "context:recipient",
  -- 					liege = { is_at_war_with = "context:recipient" }
  -- 				}
  -- 			}
  -- 		}
  -- 	},
  --
  -- 	send_options_exclusive = false,
  -- 	send_option = {
  -- 		potential = { war_declarer_needs_hook_on_liege = true },
  -- 		condition = { ["context:actor"] = { has_usable_hook = "liege" } },
  -- 		flag = "hook",
  -- 		localization = "WAR_LIEGE_HOOK",
  -- 		can_invalidate_interaction = true -- ???
  -- 	},
  -- 	should_use_extra_icon = {
  -- 		war_declarer_needs_hook_on_liege = true,
  -- 		["context:actor"] = { has_usable_hook = "liege" }
  -- 	},
  --   -- иконку нужно задать иначе
  -- 	--extra_icon = "gfx/interface/icons/character_interactions/hook_icon.dds",
  --
  --   -- почему тут нет функции старт_вар?
  -- 	on_accept = {
  -- 		["context:actor"] = {
  --       -- почему этого не было?
  --       start_war = {},
  --
  -- 			--Feedback!
  -- 			hidden_effect = {
  -- 				send_interface_toast = {
  -- 					title = "declare_war_interaction_notification",
  -- 					left_icon = "context:actor",
  -- 					right_icon = "context:recipient",
  -- 					custom_tooltip = "declare_war_interaction_notification_tooltip",
  -- 					show_as_tooltip = { condition = { always = "context:hook" }, use_hook = "liege" }
  -- 				}
  -- 			},
  --
  --       -- обещали ли вассалу войну?
  -- 			{
  -- 				condition = {
  --           -- как сделать это? где эта переменная запоминается?
  --           -- у нас должна быть механика обещанных войн (не только войну можно пообещать)
  -- 					{ exists = "var:promised_war" },
  -- 					["var:promised_war"] = {
  -- 						is_alive = true,
  -- 						NOT = { current = { equals_to = "context:recipient" } }
  -- 					}
  -- 				}
  -- 				trigger_event = {
  -- 					id = "vassal.2610",
  -- 					days = { 7, 10 }
  -- 				}
  -- 			},
  --
  -- 			{
  -- 				condition = { always = "context:hook" },
  -- 				use_hook = "liege"
  -- 			},
  --
  -- 			-- порвем со своей любовью из-за войны
  -- 			{
  -- 				condition = {
  -- 					has_relation_lover = "context:recipient",
  -- 					NAND = { has_relation_rival = "context:recipient" }, -- To enable really strange love stories
  -- 					has_war = { -- any_character_war
  -- 						casus_belli = { -- у меня проверяется сама война
  -- 							primary_attacker = "context:actor",
  -- 							primary_defender = "context:recipient",
  -- 							any_target_title = { count = 1 }
  -- 						}
  -- 					}
  -- 				},
  -- 				["context:recipient"] = {
  -- 					trigger_event = {
  -- 						id = "lover.0103", -- название будет более нормальным
  -- 						days = { 14, 30 }
  -- 					}
  -- 				}
  -- 			}
  --
  -- 			-- Invalidate any wars your vassals have going against recipient atm
  -- 			["context:recipient"] = {
  -- 				{
  -- 					condition = {
  -- 						has_war = { -- _character
  -- 							primary_attacker = {
  -- 								is_vassal_or_below_of = "context:actor",
  -- 								{ -- Ai should never invalidate a player's war!
  -- 									condition = { current = { is_ai = false } }, -- наверное не куррент а рут или прев (нет, прев это war)
  -- 									["context:actor"] = { is_ai = false }
  -- 								}
  -- 							}
  -- 						}
  -- 					},
  -- 					every_war = { -- _character
  -- 						condition = {
  -- 							primary_attacker = {
  -- 								is_vassal_or_below = "context:actor",
  -- 								{ -- Ai should never invalidate a player's war!
  -- 									condition = { current = { is_ai = false } },
  -- 									["context:actor"] = { is_ai = false }
  -- 								}
  -- 							}
  -- 						},
  -- 						show_as_tooltip = { end_war = "invalidated" }, -- происходит ли здесь что то?
  -- 						primary_attacker = { trigger_event = "war_event.1001" }
  -- 					}
  -- 				}
  -- 			}
  -- 		}
  -- 	},
  --
  -- 	auto_accept = true
  -- }, -- declare_war_interaction

  -- в цк3 мы сюда отправляем пачку титулов, у меня пока что нет инструментов для работы с массивами
  -- да и нужно ли мне отправлять пачками? я могу из интерфейса пройтись по каждому титулу
  -- в общем то лучше наверное все же из интерфейса пройтись
  {
    id = "﻿grant_titles_interaction",
    --category = "interaction_category_vassal",
    --common_interaction = true,
    name = "grant_titles_interaction_name",
    description = "grant_titles_interaction_desc",

    type = "character",
    interface_id = "grant_titles",

    -- для ии нужно указать что ожидается в этой интеракции (как и для объявления войны)
    -- ai_interaction_type = "give_title", -- вообще наверное нужно указывать через запятую эти свойства

    -- указывается таргет, как в моем случае приходит таргет?
    -- мне нужно так или иначе задать и проверить входные данные, возможно фильтр (?)
    --target_type = "title",
    --target_filter = "actor_domain_titles",
    --interface_priority = 60,

    -- нужно указать титул который передадим другой стороне, что тут еще можно сделать?
    -- предпочтительность титула, можно вернуть из scripted_filter, не bool а число
    -- тогда ии добавит титул по весам
    variables = {
      {
        name = "title",
        type = "title",
        filter = "directly_owned_titles",
        -- вернем здесь число, которое обозначает вес, 0 - плохой вариант, игнорируем
        -- если везде получаем 0, то игнорируем интеракцию
        scripted_filter = { 1 }
      }
    },

    -- можно еще задать вычисление чисел, которые может использовать ии при принятии решений
    -- нужно по крайней мере указать что лучше больше или меньше
    -- эти числа пихнем в контекст, так же как и переменные, возможно имеет смысл какие то не пихать
    -- с таким большим количеством чисел неочевидно как считать вес, так что наверное нет
    -- numbers = {
    --   {
    --     name = "happiness",
    --     less_is_better = false,
    --     script = {}
    --   }
    -- },

    -- actor character giving the titles
    -- recipient character receiving the titles

    potential = {
      --NAND = { ["context:actor"] = { is_equals_to = "context:recipient" } },
      ["context:recipient"] = {
        not_equals_to = "context:actor", -- is_not_equals_to????
        OR = {
        	is_liege_or_above = "context:actor",
        	is_pool_guest_of = "context:actor" -- что тут?
        }
      }
    },

    condition = {
      -- тогут быть выданы титулы *кем*, это что? отдельный скрипт? ну да нужно еще проверить может ли ресипиент принять титул
      ["context:actor"] = {
        NAND = { is_at_war_with = "context:recipient" },
        has_held_title = { count = 1, true }, -- held? это титулы которые находятся в прямом владении персонажа?
      },
      ["context:recipient"] = {
        NAND = { has_trait = "devoted" }, -- монахи не могут наследовать поэтому им титулы давать не нужно
        can_be_granted_titles_by = { RULER = "context:actor" },
        is_diplomatically_available = true,
        {
          condition = { is_ruler = false },
          is_imprisoned = false
        }
      },
      custom_description = {
        text = "is_not_theocratic_court_chaplain",
        subject = "context:recipient",
        NAND = {
          ["context:actor.faith"] = { has_doctrine = "doctrine_theocracy_temporal" }, -- что делать с доктринами? по идее это проверки прав
          ["context:recipient"] = {
            faith = { has_doctrine = "doctrine_theocracy_temporal" },
            has_council_position = "councillor_court_chaplain"
          }
        }
      }
    },

        -- в описании сказано что это должна быть быстрая проверка титула для того чтобы собрать список титулов
        can_be_picked_title = {
        ["context:target"] = {
        is_leased_out = false,
        {
          -- если титул передается игровому наследнику и тип наследования - разбиение реалма среди наследников,
          -- пишем для этих условий особое описание
        	condition = {
        		["context:recipient"] = { equals_to = "context:actor.player_heir" },
        		["context:actor"] = { has_partition_succession_realm_law_trigger = true }
        	},
        	custom_description = {
        		text = "partition_primary_heir_unfair",
        		subject = "context:recipient",
        		current_heir = "context:recipient",
        	}
        }
        },

        custom_description = {
        text = "grant_titles_interaction_children_not_allowed_temple",
        subject = "context:recipient",
        NOR = {
        	AND = {
        		["context:recipient"] = { is_adult = false },
        		["context:target"] = { -- проверяем титул
        			tier = tier_barony,
        			title_province = { has_building_with_flag = "temple" }
        		}
        	},
        	AND = {
        		["context:recipient"] = { is_adult = false },
        		["context:target"] = {
        			tier = tier_county,
        			title_province = {
        				is_county_capital = true,
        				has_building_with_flag = "temple"
        			}
        		}
        	}
        } -- NOR
        } -- custom_description
        }, -- can_be_picked_title

        -- может ли чел отказаться от титула? вообще это был бы интересный опыт
        auto_accept = true,

        on_auto_accept = { ["context:recipient"] = { trigger_event = "char_interaction.0110" } },

        on_accept = {
        every_in_list = {
        -- обходим лист, в котором лежат видимо титулы, которые мы хотим передать
        -- в моем случае, надо будет в луа пройтись по этим титулам видимо и проверить каждый отдельно
        list = "target_titles",
        save_temporary_scope_as = this_title, -- текущий титул в массиве
        selector = {
          {
        		condition = { exists = "context:landed_title" },
        		["context:recipient"] = {
              -- в зависимости от того какого тира у нас титул, изменяем отношение
              -- скрыто уменьшаем недовольство фракции, запоминаем титул во временном листе
        			selector = {
                {
        					condition = { ["context:this_title.tier"] = "tier_barony" },
        					hidden_effect = {
        						add_opinion = {
        							target = "context:actor",
        							modifier = "received_title_barony"
        						}
        					},
        					["context:this_title"] = { add_to_temporary_list = "titles_to_grant" },
        					hidden_effect = {
        						{
        							condition = { is_a_faction_member = true },
        							add_joined_faction_discontent = -5
        						}
        					}
                },

        				{ -- else if
        					condition = { ["context:this_title.tier"] = "tier_county" },
        					hidden_effect = {
        						add_opinion = {
        							target = "context:actor",
        							modifier = "received_title_county"
        						}
        					},
        					["context:this_title"] = { add_to_temporary_list = "titles_to_grant" },
        					hidden_effect = {
        						{
        							condition = { is_a_faction_member = true },
        							add_joined_faction_discontent = -10
        						}
        					}
        				},

        				{ -- else if
        					condition = { ["context:this_title.tier"] = "tier_duchy" },
        					hidden_effect = {
        						add_opinion = {
        							target = "context:actor",
        							modifier = "received_title_duchy"
        						}
        					},
        					["context:this_title"] = { add_to_temporary_list = "titles_to_grant" },
        					hidden_effect = {
        						{
        							condition = { is_a_faction_member = true },
        							add_joined_faction_discontent = -20
        						}
        					}
        				},

        				{ -- else if
        					condition = { ["context:this_title.tier"] = "tier_kingdom" },
        					hidden_effect = {
        						add_opinion = {
        							target = "context:actor",
        							modifier = "received_title_kingdom"
        						}
        					},
        					["context:this_title"] = { add_to_temporary_list = "titles_to_grant" },
        					hidden_effect = {
        						{
        							condition = { is_a_faction_member = true },
        							add_joined_faction_discontent = -40
        						}
        					}
        				},

        				{ -- else if
        					condition = { ["context:this_title.tier"] = "tier_empire" },
        					add_opinion = {
        						target = "context:actor",
        						modifier = "received_title_empire"
        					},
        					hidden_effect = {
        						{
        							condition = { is_a_faction_member = true },
        							add_joined_faction_discontent = -60
        						}
        					},
        					["context:this_title"] = { add_to_temporary_list = "titles_to_grant" }
        				}
              } -- selector
        		}, -- ["context:recipient"]

        		-- Promised a vassal this title they had a claim on (vassal.2901)
            --
        		{
        			condition = {
        				["context:actor"] = {
                  -- у актора может лежать переменная "обещали титул персонажу такому то"
                  -- проверяем является ли это обещаение персонажу которому мы пытаемся сейчас передать титулы
        					exists = "var:was_promised_title",
        					["var:was_promised_title"] = "context:recipient"
        				}
        			},
        			{
        				condition = { -- если landed_title в контексте совпадает с титулом который мы пообещали
        					["context:recipient"] = { exists = "var:promised_title" },
        					["context:landed_title"] = { equals_to = "context:recipient.var:promised_title" }
        				}, -- то запускаем эвент (мы передали вассалу землю которую он хочет)
        				["context:actor"] = {
        					trigger_event = {
        						id = "vassal.2910",
        						days = { 7, 14 }
        					}
        				}
        			}
        		}
          }
        }, -- selector
        { -- если пытаемся передать титул бастарду, то выводим предупреждение
        	condition = { ["context:recipient"] = { has_trait = "bastard" }, },
        	custom_tooltip = "grant_title_to_bastard_dynasty_warning"
        }
        }, -- every_in_list

        -- в зависимости от тира титула выводим предупреждение
        {
        condition = { any_in_list = { list = "target_titles", tier = "tier_empire" } },
        custom_tooltip = "grant_title_modifier_stack_empires"
        },
        {
        condition = { any_in_list = { list = "target_titles", tier = "tier_kingdom" } },
        custom_tooltip = "grant_title_modifier_stack_kingdoms"
        },
        {
        condition = { any_in_list = { list = "titles_to_grant", tier = "tier_duchy" } },
        custom_tooltip = "grant_title_modifier_stack_duchies"
        },
        {
        condition = { any_in_list = { list = "titles_to_grant", tier = "tier_county" } },
        custom_tooltip = "grant_title_modifier_stack_counties"
        },
        {
        condition = { any_in_list = { list = "titles_to_grant", tier = "tier_barony" } },
        custom_tooltip = "grant_title_modifier_stack_baronies"
        },
        {
        condition = { ["context:recipient"] = { is_a_faction_member = true } },
        custom_tooltip = "grant_title_modifier_stack_discontent"
        },

        -- расчитываем изменения уровня стреса (будет ли это вообще у меня?)
        ["context:actor"] = {
        {
          -- опять же в цк3 нет явной передачи титула
          give_title = { target = "context:recipient", title = "context:title" }
        },

        -- эти проверки нужно вынести отдельно в фильтр?
        {
        	condition = { -- если в листе нету баронств с неверным типом владения
        		any_in_list = {
        			list = target_titles,
        			{
        				condition = { tier = tier_barony },
        				["title_province.barony"] = { has_wrong_holding_type = false },
        			},
        			count = 1
        		}
        	},

        	-- гностики (религия) теряют стресс отдавая титулы
        	{
            -- если в веровании есть доктрина (у меня наверное слово религия будет использоваться)
        		condition = { ["context:actor"] = { faith = { has_doctrine_parameter = "granting_titles_gives_stress" } } },
        		["context:actor"] = {
        			add_stress = {
        				0,

			          -- базовое значение стреса изменяется в зависимости от тира титула
                every_in_list = {
                  list = "target_titles",

                  -- приводим к ближайшему числу делимому на 5, возможно имеет смысл сделать отдельной операцией
                  multiply = {
                    5,
                    round = {
                      divide = {
                        -- модифицируем значение под тир титула
                        multiply = {
                          divide = {
                            add = { 1, "this.tier" },
                            "context:actor.primary_title.tier"
                          }, -- divide
                          add = { -- вычисляем стресс в зависимости от тира титула
                            0,

                            -- у меня равно - это не равенство а '>=', тут нужно в обратную сторону проверить
                            selector = {
                              {
                                condition = { ["this.tier"] = "tier_county" },
                                minor_stress_loss -- константа (10)
                              },
                              {
                                condition = { ["this.tier"] = "tier_duchy" },
                                multiply = { 1.25, medium_stress_loss } -- константа (20) * коэф (1.25)
                              },
                              {
                                condition = { ["this.tier"] = "tier_kingdom" },
                                multiply = { 2, major_stress_loss } -- константа (40) * коэф (2)
                              },
                              {
                                condition = { ["this.tier"] = "tier_empire" },
                                multiply = { 2, monumental_stress_loss } -- константа (100) * коэф (2)
                              }
                            },

                            -- Lose more stress for giving away titles of your primary tier; lose less stress for giving away titles far below your primary in tier.
                            -- этот множитель он общий? я так понимаю да
                            -- multiply = {
                            -- 	value = this.tier
                            -- 	add = 1
                            -- 	divide = context:actor.primary_title.tier
                            -- }

                            -- Round to the nearest multiple of 5.
                            -- divide = 5
                            -- round = yes
                            -- multiply = 5

                            -- For a Duke, this will be x1.25 for Duchies (-30) and x1 for Counties(-10)
                            -- For a King, this will be x1.25 for a Kingdom (-100), x1 for a Duchy (-25) and x0.75 for Counties(-10)
                            -- For an Emperor, this will be x1.25 for an Empire(-200), x1 for a Kingdom(-80), x0.75 for a Duchy(-20) and x0.4 for a Counties (-5)
                          } -- add
                        }, -- multiply
                        5
                      } -- divide
                    } -- floor
                  } -- multiply
                } -- every_in_list
              } -- add_stress
            } -- ["context:actor"]
          },

          -- жадные и амбициозные правители набирают стресс если отдают титулы которые могут удержать
        	{
        		condition = {
        			-- является ли персонаж жадным или амбициозным
        			OR = { -- луа не гарантирует последовательность проверки (!)
        				{ has_trait = "greedy" },
        				{ has_trait = "ambitious" }
        			},
              -- это стат персонажа по идее
        			domain_size = script_utils.less_eq("domain_limit"), -- не превышаем лимит домена
        			any_in_list = {
        				list = "target_titles",
                -- количество доменов складывается по баронствам в цк3 (у меня видимо будет по городам)
                -- проверяем есть ли хотя бы один титул который учитывается в лимите
        				tier = "tier_county"
        			},

              -- исключение для персонажей у которых превышается лимит герцогств, и которые отдают это герцогство (и связанные с ним графства и далее)
        			{
        				condition = { has_too_many_held_duchies_trigger = true },
        				NAND = {
        					-- передаем хотя бы одно герцогство
        					any_in_list = {
        						list = "target_titles",
        						title_counts_towards_too_many_duchies_trigger = true, -- считается ли оверлимитом (отдельный триггер)
        						save_temporary_scope_as = "granted_duchy",

        						-- проверяем чтобы ВСЕ каунти были юридической частью этого герцогства
        						any_in_list = {
        							list = "target_titles",
        							--count = all,
        							{
        								condition = { tier = "tier_county" },
        								de_jure_liege = "context:granted_duchy"
        							}
        						} -- any_in_list
        					} -- any_in_list
        				} -- NAND
        			}
        		}, -- condition
            -- расчитываем изменение стресса
            -- selector = {
            -- 	{
            -- 		condition = { domain_size = domain_limit },
            -- 		stress_impact = {
            -- 			greedy = minor_stress_impact_gain,
            -- 			ambitious = minor_stress_impact_gain
            -- 		}
            -- 	},
            -- 	{
            -- 		condition = { domain_size < domain_limit },
            -- 		stress_impact = {
            -- 			greedy = medium_stress_impact_gain,
            -- 			ambitious = medium_stress_impact_gain
            -- 		}
            -- 	}
            -- } -- selector
          }
        },

        selector = {
    			{
    				condition = {
    					-- дополнительные проверки если есть длс
    					has_fp1_dlc_trigger = true,
    					fp1_remove_stele_new_holder_trigger = { TITLE = "context:target" }
    				},
    				custom_tooltip = "runestone_grant_title_warning"
    			},
    		  {
    				condition = {
    					["context:target"] = { exists = "var:ancestor_to_bury" },
              -- изменять имеет смысл только если передаем титулы другой династии
    					NAND = { dynasty = "context:previous_holder.dynasty" }
    				},
    				custom_tooltip = "runestone_grant_title_warning"
    			}
        }, -- selector

        -- отправим сообщение актору
        -- hidden_effect = {
        --   selector = {
        -- 		{
        -- 			condition = {
        -- 				any_in_list = {
        -- 					list = "target_titles",
        -- 					count = 1,
        -- 					NOT = { tier = "context:actor.highest_held_title_tier" },
        -- 				},
        -- 				["context:recipient"] = {
        -- 					any_held_title = {
        -- 						tier > tier_barony,
        -- 						NOT = { is_in_list = target_titles }
        -- 					}
        -- 				}
        -- 			}, -- condition
        -- 			random_in_list = {
        -- 				list = target_titles,
        -- 				save_temporary_scope_as = loc_title
        -- 			},
        -- 			send_interface_toast = {
        -- 				title = "grant_titles_interaction_notification",
        -- 				left_icon = "context:actor",
        -- 				right_icon = "context:recipient",
        -- 				custom_tooltip = "grant_titles_interaction_notification_effect_2"
        -- 			}
        -- 		},
        -- 		{
        -- 			condition = {
        -- 				any_in_list = {
        -- 					list = "target_titles",
        -- 					count = 1,
        -- 					NOT = { tier = "context:actor.highest_held_title_tier" }
        -- 				},
        -- 				["context:recipient"] = {
        -- 					any_held_title = {
        --             tier = "tier_barony",
        -- 						NOT = { is_in_list = "target_titles" }
        -- 					}
        -- 				}
        -- 			},
        -- 			random_in_list = {
        -- 				list = "target_titles",
        -- 				save_temporary_scope_as = "loc_title",
        -- 			},
        -- 			send_interface_toast = {
        -- 				title = "grant_titles_interaction_notification",
        -- 				left_icon = "context:actor",
        -- 				right_icon = "context:recipient",
        -- 				custom_tooltip = "grant_titles_interaction_notification_effect_3"
        -- 			}
        -- 		},
        -- 		{
        -- 			condition = {
        -- 				["context:recipient"] = {
        -- 					any_in_list = { -- станет независимым, но сохранит хозяина более высокого ранга
        -- 						list = "target_titles",
        -- 						tier = "context:actor.highest_held_title_tier"
        -- 					},
        -- 					top_liege = { equals_to = "context:actor.top_liege" }
        -- 				}
        -- 			},
        -- 			send_interface_toast = {
        -- 				title = "grant_titles_interaction_notification",
        -- 				left_icon = "context:actor",
        -- 				right_icon = "context:recipient",
        -- 				custom_tooltip = "grant_titles_interaction_notification_effect_5"
        -- 			}
        -- 		},
        -- 		{
        -- 			condition = {
        -- 				["context:recipient"] = {
        -- 					any_in_list = { -- станет независимым
        -- 						list = "target_titles",
        -- 						tier = "context:actor.highest_held_title_tier"
        -- 					}
        -- 				}
        -- 			},
        -- 			send_interface_toast = {
        -- 				title = "grant_titles_interaction_notification",
        -- 				left_icon = "context:actor",
        -- 				right_icon = "context:recipient",
        -- 				custom_tooltip = "grant_titles_interaction_notification_effect_4"
        -- 			}
        -- 		},
        -- 		{
        -- 			send_interface_toast = {
        -- 				title = "grant_titles_interaction_notification",
        -- 				left_icon = "context:actor",
        -- 				right_icon = "context:recipient",
        -- 				custom_tooltip = "grant_titles_interaction_notification_effect"
        -- 			}
        -- 		}
        --   } -- selector
        -- } -- hidden_effect
      } -- ["context:actor"]
    }, -- on_accept
    ai_will_do = { 1 }
  }, -- ﻿grant_titles_interaction

  {
    id = "declare_war_interaction2",

    name = "name",
    description = "description",

    type = "character",

    potential = {
      ["context:recipient"] = { is_landed = true, not_equals_to = "context:actor", NAND = { is_at_war_with = "context:actor" } },
    },

    condition = {
      ["context:actor"] = {
        NAND = {
          has_trait = "incapable",
          has_raised_armies = true,
          is_allied_in_war = "context:recipient"
        },
        can_attack_in_hierarchy = "context:recipient", -- ???
        has_any_display_cb_on = "context:recipient", -- display?
        custom_description = { text = "is_not_bankrupt", gold = 1 }, -- по умолчанию >=
        is_imprisoned = false,
        -- subjugation_offer_under_consideration - расматривающиеся соглашения нужно сделать иначе (через статус реалма)
      },
      ["context:recipient"] = {
        NAND = { has_strong_hook = "context:actor" },
        {
          condition = { is_imprisoned = true },
          -- у меня сейчас импризонер - это реалм, должен быть персонаж
          -- хотя наверное нужно разделить собственно импризонера и тюрьму
          imprisoner = { not_equals_to = "context:actor" }
        }
      }
    },

    -- таргет? наверное чать этих проверок будет в кондишене

    -- проверка есть ли хук чтобы стартануть войну с хозяином, пока что отключу
    -- send_options_exclusive = false,
    -- send_option = {
    -- 	potential = { war_declarer_needs_hook_on_liege = true },
    -- 	condition = { ["context:actor"] = { has_usable_hook = "liege" } },
    -- 	flag = "hook",
    -- 	localization = "WAR_LIEGE_HOOK",
    -- 	can_invalidate_interaction = true -- ???
    -- },
    -- should_use_extra_icon = {
    -- 	war_declarer_needs_hook_on_liege = true,
    -- 	["context:actor"] = { has_usable_hook = "liege" }
    -- },

    -- переменным было бы неплохо задать либо несколько фильтров, либо фильры по иерархии
    -- хотя наверное лучше несколько
    variables = {
      {
        name = "cb",
        type = "casus_belli",
        -- мы должны проверить казус белли как? фильтр по тем казус белли которые могут использовать
        -- персонажи, ну то есть тут теперь требуется передать две перменные? да
        -- является ли казус белли единственным особым случаем? пока что не могу придумать другие
        -- сколько фильтров для казус белли? один? я так понимаю для казус белли
        -- ничего больше указывать не нужно
      },
      -- война у нас довольно часто не за титул, например война за независимость, по идее для нее
      -- нужно только посмотреть чтобы казус белли был валиден, остальное не нужно
      {
        name = "title",
        type = "title",
        target = "recipient", -- по умолчанию actor
        -- проверим так ли необходима переменная, в общем в чем заключается проверка?
        is_necessary = {
          ["context:cb"] = {
            title_is_necessary = true
          }
        },
        -- титул выбирается у recipient в этот раз, то есть нужно задать таргет для фильтра
        -- из чего задается таргет для фильтра? по идее из того что уже есть, то есть
        -- либо actor либо recipient либо предыдущая переменная
        filter = "directly_owned_titles", -- может в таблице несколько фильтров задавать?
      },
      {
        name = "claimant",
        type = "character",
        -- тут сложно: должен быть персонаж из числа тех что имеют клайм на титул и при этом
        -- персонажи должны быть подчиненными actor'а (или им самим), но не все цб
        -- требуют претензии, в тех что не требуется можно поставить actor'а
      },
    },

    auto_accept = true,

    on_accept = {
      ["context:actor"] = {
        start_war = {
          target = "context:recipient",
          cb = "context:cb",
          claimant = "context:actor",
          titles = { "context:title" }
        },

        -- дадим понять игроку что произошло
        hidden_effect = {
          send_interface_toast = {
            title = "declare_war_interaction_notification",
            left_icon = "context:actor",
            right_icon = "context:recipient",
            custom_tooltip = "declare_war_interaction_notification_tooltip",
            show_as_tooltip = { condition = { always = "context:hook" }, use_hook = "liege" }
          }
        },

        -- обещали ли вассалу войну?
        -- {
        -- 	condition = {
        --     -- как сделать это? где эта переменная запоминается?
        --     -- у нас должна быть механика обещанных войн (не только войну можно пообещать)
        -- 		{ exists = "var:promised_war" },
        -- 		["var:promised_war"] = {
        -- 			is_alive = true,
        -- 			NOT = { current = { equals_to = "context:recipient" } }
        -- 		}
        -- 	}
        -- 	trigger_event = {
        -- 		id = "vassal.2610",
        -- 		days = { 7, 10 }
        -- 	}
        -- },

        {
          condition = { always = "context:hook" },
          use_hook = "liege"
        },

        -- порвем со своей любовью из-за войны
        {
          condition = {
            has_love_relationship = "context:recipient",
            NAND = { has_bad_relationship = "context:recipient" }, -- To enable really strange love stories
            -- здесь вообще зачем войну проверять? мы же ее здесь добавляем
            -- has_war = { -- any_character_war
            -- 	--casus_belli = { -- у меня проверяется сама война
            -- 		primary_attacker = { equals_to = "context:actor" },
            -- 		primary_defender = { equals_to = "context:recipient" },
            -- 		any_target_title = { count = 1 }
            -- 	--}
            -- }
          },
          ["context:recipient"] = {
            trigger_event = {
              id = "lover.0103", -- название будет более нормальным
              days = { 14, 30 }
            }
          }
        },
      }, -- ["context:actor"]

			-- любая война в которой вассалы воют против текущего ресипиента должна быть закончена
			["context:recipient"] = {
				{
					-- condition = {
					-- 	has_war = { -- _character
					-- 		primary_attacker = {
					-- 			is_vassal_or_below_of = "context:actor",
					-- 			{ -- Ai should never invalidate a player's war!
					-- 				condition = { current = { is_ai = false } }, -- наверное не куррент а рут или прев (нет, прев это war)
					-- 				["context:actor"] = { is_ai = false }
					-- 			}
					-- 		}
					-- 	}
					-- },
					every_war = { -- _character
						condition = {
							primary_attacker = {
								is_vassal_or_below = "context:actor",
								{ -- Ai should never invalidate a player's war!
									condition = { is_ai = false },
									["context:actor"] = { is_ai = false }
								}
							}
						},
						show_as_tooltip = { end_war = "invalidated" }, -- происходит ли здесь что то?
						primary_attacker = { trigger_event = "war_event.1001" }
					}
				}
			} -- ["context:recipient"]
    }, -- on_accept
    ai_will_do = { 1 }
  } -- declare_war_interaction2
}


--NAND = { ["context:actor"] = { is_at_war_with = "context:recipient" } },
--NAND = { ["context:recipient"] = { has_trait = "devoted" } },
--["context:actor"] = { compare = { op = "more", any_held_title = { true }, 1 } },
--["context:actor"] = { has_held_title = { count = 1, true } },
--["context:recipient"] = { is_diplomatically_available = true },
-- {
-- 	condition = { ["context:recipient"] = { is_ruler = false } },
-- 	["context:recipient"] = { is_imprisoned = false }
-- }
