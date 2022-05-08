#ifndef DEVILS_ENGINE_CORE_CORE_INTERACTION_H
#define DEVILS_ENGINE_CORE_CORE_INTERACTION_H

#include <cstdint>
#include <cstddef>
#include <string>
#include <array>
#include "utils/sol.h"
#include "declare_structures.h"
#include "script/header.h"
#include "script/context.h"
#include "render/shared_structures.h"

// добавится еще пара значений
#define INTERACTION_TYPES_LIST \
  INTERACTION_TYPE_FUNC(character)  \
  INTERACTION_TYPE_FUNC(province)   \
  INTERACTION_TYPE_FUNC(city)       \
  INTERACTION_TYPE_FUNC(title)      \
  INTERACTION_TYPE_FUNC(realm)      \
  INTERACTION_TYPE_FUNC(army)       \
  INTERACTION_TYPE_FUNC(hero_troop) \
  INTERACTION_TYPE_FUNC(war)        \
  
#define INTERACTION_FLAGS_LIST \
  INTERACTION_FLAG_FUNC(popup_on_receive) \
  INTERACTION_FLAG_FUNC(pause_on_receive) \
  INTERACTION_FLAG_FUNC(force_notification) \
  INTERACTION_FLAG_FUNC(ai_accept_negotiation) \
  INTERACTION_FLAG_FUNC(hidden) \
  INTERACTION_FLAG_FUNC(can_send_despite_rejection) \
  INTERACTION_FLAG_FUNC(ignores_pending_interaction_block) \
  INTERACTION_FLAG_FUNC(needs_recipient_to_open) /* по идее не в моем случае (?) */ \
  INTERACTION_FLAG_FUNC(show_answer_notification) \
  INTERACTION_FLAG_FUNC(show_effects_in_notification) \
  INTERACTION_FLAG_FUNC(ai_maybe) \
  INTERACTION_FLAG_FUNC(greeting) /* тут по идее только 2 значения */ \
  
// в цк3 это валидные опции для ai_targets, может быть еще парочка от меня добавится
#define INTERACTION_AI_TARGET_TYPES_LIST \
  INTERACTION_AI_TARGET_TYPE_FUNC(known_secrets) /* все персонажи секрет которых знает текущий */ \
  INTERACTION_AI_TARGET_TYPE_FUNC(hooked_characters) \
  INTERACTION_AI_TARGET_TYPE_FUNC(neighboring_rulers) \
  INTERACTION_AI_TARGET_TYPE_FUNC(peer_vassals) \
  INTERACTION_AI_TARGET_TYPE_FUNC(guests) \
  INTERACTION_AI_TARGET_TYPE_FUNC(dynasty) \
  INTERACTION_AI_TARGET_TYPE_FUNC(courtiers) \
  INTERACTION_AI_TARGET_TYPE_FUNC(prisoners) \
  INTERACTION_AI_TARGET_TYPE_FUNC(sub_realm_characters) \
  INTERACTION_AI_TARGET_TYPE_FUNC(realm_characters) \
  INTERACTION_AI_TARGET_TYPE_FUNC(vassals) \
  INTERACTION_AI_TARGET_TYPE_FUNC(liege) \
  INTERACTION_AI_TARGET_TYPE_FUNC(self) \
  INTERACTION_AI_TARGET_TYPE_FUNC(head_of_faith) \
  INTERACTION_AI_TARGET_TYPE_FUNC(spouses) \
  INTERACTION_AI_TARGET_TYPE_FUNC(family) \
  INTERACTION_AI_TARGET_TYPE_FUNC(children) \
  INTERACTION_AI_TARGET_TYPE_FUNC(primary_war_enemies) \
  INTERACTION_AI_TARGET_TYPE_FUNC(war_enemies) \
  INTERACTION_AI_TARGET_TYPE_FUNC(war_allies) \
  INTERACTION_AI_TARGET_TYPE_FUNC(scripted_relations) /* все персонажи с которыми есть заскриптованные отношения */ \

// с помощью скриптов мне нужно отправить предложение и обработать отказ или согласие
// по идее это дело тоже нужно компилировать?

// в цк3 в интеракциях выделяются следующие данные
// interface_priority = number              // в цк3 данные для меню интеракции
// common_interaction = yes/no              // данные для меню интеракции
// category = interaction_category_hostile  // данные для меню интеракции
// icon = key                               // какую иконку исользовать, по умолчанию: "interaction" (? defaults to the interaction key)
// 
// is_highlighted = trigger                 // должна ли интеракция быть как либо выделена в меню
// highlighted_reason = loc_key             // причина выделения (локализованный текст)
// on_decline_summary = dynamic description // специальный текст, который показывается под виджетом степени принятия действия, использовать когда необходимо привлечь внимание игрока к отказу
// special_interaction = type               // интеракция использует специальный интерфейс
// special_ai_interaction = type            // интеракция запускает определнные команды которые определяются интеракцией по этому типу
// interface = call_ally/marriage/grant_titles/etc.   // какой интерфейс использует интеракция
// scheme = elope/murder/etc.               // тип интриги которое стартуется интеракцией
// popup_on_receive = yes                   // интеракция должна явно давать о себе понять принимающей стороне
// force_notification = yes/no              // форсится специальное окно даже в случае автопринятия (наверное удобно использовать для объявления войн)
// pause_on_receive = yes/no                // игра паузится при принятии этой интеракции (в моем случае наверное игрок не должен мочь сделать ход, пока не выберет что то осмысленное)
// ai_accept_negotiation = yes/no           // если ии откажется от интеракции, то стартует цепочка эвентов с торгом, мы не должны показывать сообщения в духе "не согласится" потому что есть шанс что согласится во время торга
// hidden = yes                             // скрытая интеракция
// use_diplomatic_range = yes/no/trigger    // проверяет ли интеракция дипломатическую дальность (по умолчанию должно быть да)
// can_send_despite_rejection = yes         // интеракция все равно может быть отправлена и ии может отказать
// ignores_pending_interaction_block = yes  // есть актор - это игрок и тот уже отправил на рассмотрение интеракцию, можно ли ее отправить еще раз? (по умолчанию нет)
// send_name = loc_key                      // имя интеркции в контексте отправленности (???)
// needs_recipient_to_open = yes            // нужно ли интеракции установить ресипиента чтобы появилась позможность показать интеракцию (по умолчанию нет)
// show_answer_notification = no            // показать ли ответ если отправитель игрок (по умолчанию да)
// show_effects_in_notification = no        // должны ли эффекты интеракции быть показаны в окне нотификации, когда интеракция отправлена (по умолчанию да)
// 
// alert_icon = texture_path                // иконка тревоги (gfx/interface/icons/character_interactions/my_interaction_alert.dds)
// icon_small = texture_path                // маленькая иконка (gfx/interface/icons/character_interactions/my_interaction_small.dds)
// should_use_extra_icon = { always = yes } // когда показать дополнительную иконку (id тултипа <key>_extra_icon, не понимаю пока что где это может быть использовано)
// extra_icon = "gfx/<...>/hook_icon.dds"   // дополнительная иконка
// 
// target_type = type                       // тип таргета (в цк3 возможные типы: титул и никакой, а как же жена?)
// target_filter = type                     // там перечислены несколько хардкоженных фильтров
// 
// ai_maybe = yes                           // ии может ответить "может быть"
// ai_min_reply_days = 4                    // минимум дней до ответа ии
// ai_max_reply_days = 9                    // максимум дней до ответа ии
// 
// desc = loc_key                           // небольшое описание интеракции
// greeting = negative/positive             // тональность текста в запросе
// notification_text = loc_key              // текст запроса
// prompt = loc_key                         // текст который нужно показать под портретом (например "выбери опекуна")
// 
// cooldown = { years = x }                 // сколько времени откат интеракции
// cooldown_against_recipient = { years = x } // сколкьо времени откат для конкретного принимающего
// 
// is_shown = trigger                       // доступна ли интеракция между актором и принимающим
// is_valid_showing_failures_only = trigger // интеракция может быть выбрана в текущем состоянии, показывать только не выполнившиеся триггеры
// is_valid = trigger                       // интеракция может быть выбрана в текущем состоянии
// has_valid_target_showing_failures_only = trigger   // TODO
// has_valid_target = trigger               // TODO
// can_be_picked = trigger                  // TODO
// can_be_picked_title = trigger            // TODO
// auto_accept = yes/no/trigger             // является ли эта интеракция автоматически принимаемой
// can_send = trigger                       // может ли быть интеракция отправлена (в чем отличие от is_valid)
// can_be_blocked = trigger                 // может ли интеракция быть заблокирована принимающим (например хуком)
//  
// redirect = {}                            // изменяем перенаправление персонажей используя данные скоупа (??????)
// populate_actor_list = {}                 // фильтер для листа персонажей которые потом показываются в интерфейсе и могут быть выбраны
// populate_recipient_list = {}             // второй лист
// 
// localization_values = {}                 // переменные для локализации
// 
// options_heading = loc_key                // текст который показывается выше опций, описывает все опции в целом
// send_option = {                          // добавит опцию
//   is_shown = trigger                     // видна ли опция
//   is_valid = trigger                     // опцию можно выбрать
//   current_description = desc             // описание для тултипа
//   flag = flag_name                       // если выбрано, то в контексте будет задана булева переменная с именем флага
//   localization = loc_key                 // локализация для надписи опции
//   starts_enabled = trigger               // должен ли быть этот триггер включен при открытии окна? (по умолчанию нет)
//   can_be_changed = trigger               // опция может быть изменена из состояния по умолчанию
//   can_invalidate_interaction = bool      // если да то аи сделает can_send проверку вместо менее затратной проверки отказа принимающего и ai_will_do (по идее затратно)
// }
// // опции не должны мешать отправки интеракции (кроме случая отказа принимающего), так как мы предполагаем что они имеют место быть для ИИ (производительность), нужно использовать can_invalidate_interaction если проверка необходима
// send_options_exclusive = yes/no          // являются ли опциии эксклюзивными
// 
// on_send = effect                         // выполняется ровно во время отправки интеракции
// on_accept = effect                       // выполняется при принятии условий другой стороной
// on_decline = effect                      // выполняется при откаже от интеракции другой стороной
// on_blocked_effect = effect               // выполняется когда принимающая сторона блокирует интеракцию
// pre_auto_accept = effect                 // выполняется до любых других эффектов при автопринятии (в цк3 сказано что может выполнится до женитьбы, но наверное это не мой случай)
// on_auto_accept = effect                  // выполняется при автопринятии Only executes if the interaction was auto accepted
// 
// reply_item_key = loc_key                 // локализация для тултипа интеракции (??????)
// 
// // эти тексты показываются игроку когда тот отправляет интеракцию, означают какой ответ может дать принимающая сторона
// pre_answer_yes_key = loc_key             // текст для случая когда интеракция скорее всего будет принята
// pre_answer_no_key = loc_key              // текст для случая когда интеракция скорее всего НЕ будет принята
// pre_answer_maybe_key = loc_key           // текст для случая когда интеракция МОЖЕТ быть принята (вероятность (степень согласия?) записывается в специальную переменную, в моем случае должно быть иначе)
// pre_answer_maybe_breakdown_key = loc_key // текст для ситуации с шансом принятия
// 
// // эти тексты будут показаны игроку, когда тот отвечает на интеракцию
// answer_block_key = loc_key               // текст при блокировке
// answer_accept_key = loc_key              // текст при согласии
// answer_reject_key = loc_key              // текст при отказе
// answer_acknowledge_key = loc_key         // (непонятно, в цк3 неаккуратно скопирована предыдущая строка, возможно для случая когда выбора нет, и нас просто ставят в известность)
// 
// cost = {                                 // стоимость интеракции, будет вычтена после отправки интеракции
//   piety = {}
//   prestige = {}
//   gold = {}
//   renown = {}
// }
// 
// ai_set_target = {}                       // задает scope:target для того чтобы ии мог указать какую то конкретную цель, титулы этого не требуют (???)
// ai_targets = {
//   ai_recipients = type                   // кого ии подразумевает в качестве принимающей стороны (можно указать тип персонажа)
//                                          // наверное имеет смысл перечислить список типов где нибудь ниже
//   chance = 0-1                           // имеет смысл случайно отключать из проверки этот тип, для сохранения производительности
// }
// ai_target_quick_trigger = {              // быстрые условия для таргетов
//   adult = yes                            // взрослый
//   attracted_to_owner = yes               // цель должна быть (иметь возможность?) увлечена владельцем
//   owner_attracted = yes                  // владелец должен быть увлечен таргетом
//   prison = yes                           // цель должна быть в тюрьме
// }
// 
// ai_frequency = months                    // как часто ии должен проверять эту интеракцию
// ai_potential = trigger                   // попробует ли ии эту интеракцию
// ai_accept = mtth                         // ответит ли ии на эту интеракцию
// ai_will_do = mtth                        // насколько интеракция интересна ии (проверится после выбора цели)
// необходимо отметить, что для взаимодействий с титулами, каждый отдельный титул будет вычислен, и тот что получит наибольший ai_will_do будет использован, если у интеракции есть опции, то комбинация опций которая получит наибольший ai_will_do будет использована (возможно мой случай и не только для титулов)

namespace devils_engine {
  namespace core {
    struct interaction {
      static const structure s_type = structure::interaction;
      static const size_t max_variables_count = 16;
      static const size_t max_options_count = 8;
      
      enum class type {
#define INTERACTION_TYPE_FUNC(name) name,
        INTERACTION_TYPES_LIST
#undef INTERACTION_TYPE_FUNC

        count
      };
      
      enum class flags {
#define INTERACTION_FLAG_FUNC(name) name,
        INTERACTION_FLAGS_LIST
#undef INTERACTION_FLAG_FUNC
        
        count
      };
      
      enum class ai_target_type {
#define INTERACTION_AI_TARGET_TYPE_FUNC(name) name,
        INTERACTION_AI_TARGET_TYPES_LIST
#undef INTERACTION_AI_TARGET_TYPE_FUNC
        
        count
      };
      
      // target_filter // несколько предзаданных фильтров таргетов
      
      struct option {
        std::string id;
        script::string name_script; // строка которая видна в списке
        script::string description_script; // строка в тултипе
        script::condition potential;
        script::condition condition;
        script::condition starts_enabled;
        script::condition can_be_changed;
        // будет ли тут эффект вообще? это интеракция и опции здесь - это дополнительные флаги
        // эффект будет учитывать это дело
        //script::effect effect;
        script::number ai_will_do;
        std::string flag;
      };
      
      struct filter_data {
        std::string_view name;
        size_t index;
      };
      
      struct variable {
        std::string name;
        enum type type;
        // здесь также должен быть задан заранее написанный фильтр (например вернуть только титулы непосредственного владения)
        struct filter_data filter_data;
        //script::condition scripted_filter;
        script::number scripted_filter; // возвращаем вес из скрипта, по нему ии проверит предпочтительность данных в переменной
      };
      
      std::string id;
      script::string name_script;
      script::string description_script;
      enum type type;
      
      // как задать входные данные?
//       size_t input_count;
      //std::array<std::pair<std::string, script::script_data>, 16> input_array;
      // это наверное будет храниться непосредственно в script::script_data
      script::condition potential; // нужно использовать определенное соглашение по именам для переменных (например root и recipient)
      script::condition condition; // в цк3 разделяется на обычный и на скрипт в котором показывается только неудачи
      script::condition auto_accept; // это триггер автопринятия, 
      // то есть мы его должны проверить (когда?) и тогда персонаж которому мы отправляем это дело в любом случае выполнит on_accept
      script::condition is_highlighted;
      script::condition should_use_extra_icon;
      // has_valid_target_showing_failures_only
      // has_valid_target // не очень понимаю для чего это
      // can_be_picked
      // can_be_picked_title // тоже не понимаю
      script::condition can_send; // делаем эти проверки после задания всех переменных, перед непосредственно отправкой
      // can_be_blocked // полезная штука, пока не понимаю что с этим делать
      
      // redirect // перенаправление?
      // populate_actor_list // при женитьбе например нужно задать с одной и с другой стороны списки
      // populate_recipient_list
      // localization_values // полезная штука, тут только сопоставляем название и строку? или еще какие то числа могут появиться?
      
      // нужно зафорсить проверку дополнительных персонажей, к этим фильтрам нужно добавить предварительный фильтр для ии
      // типы для вторичных целей? вторичные цели могут быть и городами и титулами и проч
//       script::condition additional_actor_filter;
//       script::condition additional_recipient_filter;
      
      script::string on_decline_summary; // наверное скрипт
      // special_ai_interaction // нужно дать понять ии что это за интеракция
      // scheme                 // нужно дать понять ии запускает ли какую нибудь схему эта интеракция
      std::string interface_id; // возможно имеет смысл задать id интерфейса
      // use_diplomatic_range // скрипт
      std::string send_name; // просто строка
      std::string highlighted_reason; // это только строка?
      std::string notification_text;
      std::string prompt;
      std::string options_heading; // общее описание для опций (если имеются)
      std::string reply_item_key; // локализация ответа?
      
      std::string pre_answer_yes_key;
      std::string pre_answer_no_key;
      std::string pre_answer_maybe_key;
      std::string pre_answer_maybe_breakdown_key;
      std::string answer_block_key;
      std::string answer_accept_key;
      std::string answer_reject_key;
      std::string answer_acknowledge_key;
      
      script::number cooldown; // как сделать откат? нужно ввести мапу где это дело учтем
      script::number cooldown_against_recipient; // этот откат уйдет челику
      
      // вообще это можно подменить на что то другое, да будет неплохим вариантом например сделать эффект ДО акссепт
      // в этом эффекте например можно быстренько запомнить какие нибудь полезные вещи, 
      // и вообще скрипт вызывается последовательно с "усвоением" новых данных в скоупе
      // как это усвоение правильно сделать? оставить все данные дальше по вызовам
      // запомнить в контекст мы можем только в экшонах? скорее да чем нет
      //script::effect immediate; // ???
      
      script::effect on_send; // по идее война должна стартовать здесь
      // эффекты, нужно добавить проверки что где может использоваться (то есть вещи ниже используют только интеракции)
      // эти эффекты вызываются после send_options
      script::effect on_accept;
      script::effect on_decline; // другой персонаж может например отказаться от принятия подарка
      script::effect on_blocked_effect; // персонаж может заблокировать интеракцию, например с помощью хука
      // форсированное принятие, в каких случаях? например когда мы используем хук, 
      // как понять что мы использовали хук и нужно сделать авто акксепт?
      script::effect on_auto_accept;
//       script::effect pre_auto_accept; // ???
      
      script::number ai_accept;    // число с описанием
      script::number ai_will_do;   // смотрим будем ли мы это делать, как использовать?
      script::number ai_frequency; // насколько часто ии использует это дело
      script::condition ai_potential; // будет ли ии вообще рассматривать возможность это сделать?
      // ai_set_target // не понимаю для чего
      // ai_targets // задать фильтры целей для ии, чтобы он не по всем персонажамм проходился
      script::condition ai_target_quick_check; // быстрые проверки для таргетов, видимо после фильтров сверху
      
      script::number money_cost;
      script::number authority_cost;
      script::number esteem_cost;
      script::number influence_cost;
      // вообще наверное имеет смысл потом сделать скриптовые переменные и собственно в них хранить какие нибудь особые валюты
      
      // тут чаще всего нам нужно получить число, которое по условиям игры должно быть хотя бы 1 чтобы кнопку можно было бы нажать
      // некоторые из этих вещей могут быть специальными флажками (например хук как в цк3)
      uint32_t options_count;
      std::array<option, max_options_count> send_options; // тип примерно такой же как и опция в эвенте
      std::array<variable, max_variables_count> variables;
      
      render::image_t alert_icon;
      render::image_t icon;
      render::image_t icon_small;
      render::image_t extra_icon;
      
      size_t ai_min_reply_turns; // у меня считается не в днях а в ходах, так что тут числа будут не очень большими
      size_t ai_max_reply_turns;
      
      utils::bit_field<static_cast<size_t>(flags::count)> flag_container;
      
      interaction();
      
      inline bool set_flag(const enum flags flag, const bool val) { return flag_container.set(static_cast<size_t>(flag), val); }
      inline bool get_flag(const enum flags flag) const { return flag_container.get(static_cast<size_t>(flag)); }      
    };
    
    // нужно осознать что это будет запускаться ТОЛЬКО игроком в луа
    // что то такое нужно передать дальше персонажу (интеракцию и контекст)
    // у игрока опять придется составить compiled_interaction, а значит нужно тут
    // оставить уже сделанный контекст
    struct compiled_interaction {
      const core::character* character;
      const interaction* native;
      script::context ctx;
      // нам не так много всего нужно задать с точки зрения интеракции
      // только по сути эти четыре переменные, у контекст может содержать дополнительные данные - лучше контекст
//       script::object actor;
//       script::object recipient;
//       script::object secondary_actor;
//       script::object secondary_recipient;
      
      size_t id; // для того чтобы найти ожидающую интеракцию 
      
      // нужна ли тут какая то дополнительная проверка входных данных
      // вообще возможно... но для интерфейса пока без проверок оставим
      
      // как совместить данные которые мы запомним у игрока с вот этим? опять скопировать?
      // задать тут уникальный id по которому найдем ожидающую интеракцию у игрока
      // имеет смысл
      
      compiled_interaction();
      compiled_interaction(const core::character* character, const interaction* native, const script::context &ctx, const size_t &id = SIZE_MAX);
      compiled_interaction(const compiled_interaction &copy) = delete;
      compiled_interaction(compiled_interaction &&move) = default;
      compiled_interaction & operator=(const compiled_interaction &copy) = delete;
      compiled_interaction & operator=(compiled_interaction &&move) = default;
      
      // для войны мне нужно еще передать казус_белли, клаймата, титул (несколько?)
      // не особо понимаю как это сделать в текущей ситуации, с другой стороны можно сделать систему
      // где указать название переменной и фильтр к нему, как с этим работать в ии?
      // по идее - это вещи которые необходимо задать, так что по крайней мере понятно что делать
      // задаем переменные мы так: сначала определяем тип объекта, затем используем подготовленный фильтр,
      // фильтруем с помощью скрипта, если объектов после этого не осталось то интеракцию использовать не можем
      // как в случае войны задать казус_белли, откуда вообще казус_белли берется? так же как интеракцию
      // берем по актору, ресипиенту, затем нужно проверить кондитион
      void set_variable(const std::string_view &name, const sol::object &obj);
      
      bool is_pending() const;
      
      bool potential();
      bool condition();
      bool auto_accept();
      bool is_highlighted();
      bool can_send();
      bool send();
      
      // только при ожидании
      bool accept();
      bool decline();
      // block?
      
      // а для опций не нужны ли переменные? вообще могут пригодиться
      size_t options_count() const;
      
      std::string_view get_option_id(const size_t &index) const;
      std::string_view get_option_name(const size_t &index); // строка которая видна в списке
      std::string_view get_option_description(const size_t &index); // строка в тултипе
      std::string_view get_option_flag(const size_t &index) const;
      bool get_option_potential(const size_t &index);
      bool get_option_condition(const size_t &index);
      bool get_option_starts_enabled(const size_t &index);
      bool get_option_can_be_changed(const size_t &index);
      
      bool toggle_option(const size_t &index);
      bool option_state(const size_t &index);
      
      void draw_option_name(const size_t &index, const sol::function &func);
      void draw_option_description(const size_t &index, const sol::function &func);
      void draw_option_potential(const size_t &index, const sol::function &func);
      void draw_option_condition(const size_t &index, const sol::function &func);
      void draw_option_starts_enabled(const size_t &index, const sol::function &func);
      void draw_option_can_be_changed(const size_t &index, const sol::function &func);
      
      double money_cost();
      double authority_cost();
      double esteem_cost();
      double influence_cost();
      
      double cooldown();
      double cooldown_against_recipient();
      
      std::string_view get_name();
      std::string_view get_description();
      std::string_view get_on_decline_summary();
      
      void draw_name(const sol::function &func);
      void draw_description(const sol::function &func);
      void draw_on_decline_summary(const sol::function &func);
      
      void draw_potential(const sol::function &func);
      void draw_condition(const sol::function &func);
      void draw_auto_accept(const sol::function &func);
      void draw_is_highlighted(const sol::function &func);
      void draw_can_send(const sol::function &func);
      void draw_on_accept(const sol::function &func);
      void draw_on_decline(const sol::function &func);
      void draw_on_auto_accept(const sol::function &func);
//       void draw_pre_auto_accept(const sol::function &func);
      void draw_on_blocked_effect(const sol::function &func);
      void draw_on_send(const sol::function &func);
      
      void draw_cooldown(const sol::function &func);
      void draw_cooldown_against_recipient(const sol::function &func);
      void draw_money_cost(const sol::function &func);
      void draw_authority_cost(const sol::function &func);
      void draw_esteem_cost(const sol::function &func);
      void draw_influence_cost(const sol::function &func);
      
      // нужно ли это тут? вообще может пригодиться для дебага
      void draw_ai_accept(const sol::function &func);
      void draw_ai_will_do(const sol::function &func);
      void draw_ai_frequency(const sol::function &func);
      void draw_ai_potential(const sol::function &func);
      void draw_ai_target_quick_check(const sol::function &func);
    };
    
    bool validate_interaction(const size_t &index, const sol::table &table);
    void parse_interaction(core::interaction* interaction, const sol::table &table);
  }
}

#endif
