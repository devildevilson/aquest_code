#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_COMMAND_USERDATA
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#include <Nuklear/nuklear.h>

// #ifndef NUKLEAR_HEADER_H
// #define NUKLEAR_HEADER_H
// 
// struct nuklear_data {
//   nk_context ctx;
//   nk_font *font;
//   nk_font_atlas atlas;
//   nk_draw_null_texture null;
//   nk_buffer cmds;
// };
// 
// // функции?
// // тут по крайне мере будет структура с переменными нуклира
// // ну и функции поди тоже нужны
// // нет, наверное тут все же ничего не будет
// 
// // struct nuklear_data {
// //   nk_context ctx;
// //   nk_font *font;
// //   nk_font_atlas atlas;
// //   nk_draw_null_texture null;
// // };
// 
// // namespace nk {
// //   void init(nuklear_data &data);
// //   void deinit(nuklear_data &data);
// //   
// //   // в основном тут будет инициализация
// // };
// // 
// // struct nkvec2 {
// //   float x;
// //   float y;
// // };
// // 
// // // тут нужен минимальный эффективный набор полезных штук для гуи
// // class NuklearGui {
// // public:
// //   NuklearGui();
// //   ~NuklearGui();
// //   
// //   void inputBegin();
// //   void mouseInput(const int32_t &x, const int32_t &y);
// //   void inputKey(const nk_keys &key, const uint32_t &down);
// //   void inputButton(const nk_buttons &btn, const uint32_t &x, const uint32_t &y, const uint32_t &down);
// //   void inputScroll(const nkvec2 &val);
// //   void inputChar(const char &c);
// //   void inputGlyph(const nk_glyph &g);
// //   void inputUnicode(const nk_rune &rune);
// //   void inputEnd();
// //   
// //   nk_command* begin();
// //   nk_command* next(nk_command* cmd);
// //   nk_flags convert(nk_buffer* cmds, nk_buffer* verts, nk_buffer* idx, nk_convert_config* cfg);
// //   nk_draw_command* drawBegin(nk_buffer* buf);
// //   nk_draw_command* drawEnd(nk_buffer* buf);
// //   nk_draw_command* drawNext(nk_draw_command* cmd, nk_buffer* buf);
// //   
// //   // я не уверен нужно ли мне это все??
// //   
// //   nk_context* getContext();
// // private:
// //   nk_context ctx;
// //   nk_font *font;
// //   nk_font_atlas atlas;
// //   nk_draw_null_texture null;
// // };
// 
// #endif
