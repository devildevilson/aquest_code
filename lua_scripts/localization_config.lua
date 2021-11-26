-- локализация, все сложно с локализацией, возможно ее нужно хранить в луа
-- потому что луа позволяет сделать легко вложенные таблицы, с другой стороны
-- данные в луа занимают очень много памяти, с учетом локализации, не получится ли
-- бессмысленно большого расхода памяти? строки которые я тут храню мне еще нужно форматировать
-- это значит, нужно предоставить какой то способ найти в строке управляющую последовательность
-- и подставить нужную переменную, частично локализация будет зависеть от того как я задизайню
-- функции интерфейса, что делать с интерфейсом? локализацию можно сделать похожей на
-- https://github.com/kikito/i18n.lua , суть в общем простая, нужно брать строчку по ключу
-- таблицы верхнего уровня - это локали, в самой либе сделана и плюрялизация и форматирование,
-- мне же форматирование нужно кастомное, и врядли я сильно завишу от плюрялизации, но мне пригодится
-- изменять окончания, в общем чаще всего это делается созданием дополнительной строки
-- для формата строк можно использовать fmt + нужно написать способ форматировать запись типа:
-- [event_stack[4]] или [protagonist.name()] одна из этих записей (или обе) должна привести
-- к встраиванию ссылки на персонажа, на эту ссылку можно навестись и получить какую нибудь полезную информацию
-- + рядом со ссылкой должен быть портрет, делающий примерно тоже самое

-- плюрялизацию можно сверху прикрутить к этому

return {
  -- таблица верхнего уровня всегда должна быть конкретной локалью
  -- и должна поддерживать до 8 символов (64бита) в названии, мы будем ее хранить
  -- в спп контейнере
  en = {
    month = {
      -- нужно ввести какие то ограничения на то что мы можем тут хранить
      -- понятное дело прежде всего мы уберем юзердату, поток, функцию
      -- и оставим только таблицу, строку или число
      -- + должен быть способ указать таблицу имен для культуры
      jan = "january",
      feb = "february",
      mar = "march",
      apr = "april",
      may = "may",
      jun = "june",
      jul = "july",
      aug = "august",
      sep = "september",
      oct = "october",
      nov = "november",
      dec = "december"
    },
    cultures = {
      english = {
        name = "English",
        description = "whats here?",
        character_names = { -- вот это нужно еще как то генерировать, можно грузить с диска, потребуется os.lines
          "ehnam khussan", "rharen ramum", "stongem Pridelight", "nuu Crowbane", "fozin barsk", "for stumitsk",
          "gurbum Meadowdancer", "on Mourninghammer", "buerhoter zevrunskub", "do-nud jendet", "golmadoos chudegokya",
          "gelvad vantonza", "lep chiem", "chuih ing", "dozescies ricobral", "rartul evurgu", "bhaser baka",
          "jarded bistan", "irvidd Gloomwater", "beu Titantail", "visem mursk", "fom sirkirsk", "gromom Hallowedbash",
          "orth Daymantle", "deru-duez najovuhk", "rar-veod vovuft", "brinolded gilzikuku", "gonrek trorgami",
          "fuiy yi", "muing taom", "robrertir rorneme", "darnis dostulo", "bazud khakam", "rezur bheidam",
          "sturstodd Oatbend", "grilledd Duskmore", "sedgef nog", "gram chenuz", "bremoth Skychaser",
          "oth Marblehide", "ran-vad-koz veprundrak", "ho-vuh jenzuld", "dadjindak ilzarkyaku",
          "fasdir nirira", "zium ziay", "qiy mian", "gridristol pergoldol", "hirvis costaldir"
        }
      },
      culture1 = {
        name = "Culture1",
        description = "whats here?"
      }
    },
    culture_groups = {
      culture_group1 = {
        name = "Culture group",
        description = "whats here?",
      }
    }
  }
}
