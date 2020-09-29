#include "serializator_helper.h"

#include "bin/core_structures.h"
// #include "protobuf/test1.pb.h"
#include "lz4.h"
#include "lz4hc.h"
#include "globals.h"
//#include "sha256.h"
#include <fstream>
#include <filesystem>
#include "cista/cista.h"
//#include "cryptopp/sha.h"
//#include "cryptopp/hc256.h"
#include "picosha2.h"
#include "salsa20.h"
#include "linear_rng.h"
#include "bin/seasons.h"

#include <iostream>

// #include <gzip/compress.hpp>
// #include <gzip/config.hpp>
// #include <gzip/decompress.hpp>
// #include <gzip/utils.hpp>
// #include <gzip/version.hpp>

// typedef void(apate_quest::map_data::*fptr)(const std::string &);
// typedef const std::string & (apate_quest::map_data::*get_ptr)(int index) const;
// typedef int (apate_quest::map_data::*get_size_ptr)() const;
//
// const fptr write_funcs[] = {
//   nullptr,
//   &apate_quest::map_data::add_provinces,
//   &apate_quest::map_data::add_building_type,
//   &apate_quest::map_data::add_city_type,
//   &apate_quest::map_data::add_city,
//   &apate_quest::map_data::add_trait,
//   &apate_quest::map_data::add_modificator,
//   &apate_quest::map_data::add_troop_type,
//   &apate_quest::map_data::add_decision,
//   &apate_quest::map_data::add_religion_group,
//   nullptr,
//   &apate_quest::map_data::add_culture,
//   &apate_quest::map_data::add_law,
//   &apate_quest::map_data::add_event,
//   &apate_quest::map_data::add_titulus,
//   &apate_quest::map_data::add_character,
//   nullptr,                                 // это я еще не определил
//   nullptr,                                 // это скорее всего будет описано у персонажа
//   &apate_quest::map_data::add_hero_troop,
//   &apate_quest::map_data::add_army,
// };
//
// const get_ptr read_funcs[] = {
//   nullptr,
//   &apate_quest::map_data::provinces,
//   &apate_quest::map_data::building_type,
//   &apate_quest::map_data::city_type,
//   &apate_quest::map_data::city,
//   &apate_quest::map_data::trait,
//   &apate_quest::map_data::modificator,
//   &apate_quest::map_data::troop_type,
//   &apate_quest::map_data::decision,
//   &apate_quest::map_data::religion_group,
//   nullptr,
//   &apate_quest::map_data::culture,
//   &apate_quest::map_data::law,
//   &apate_quest::map_data::event,
//   &apate_quest::map_data::titulus,
//   &apate_quest::map_data::character,
//   nullptr,
//   nullptr,
//   &apate_quest::map_data::hero_troop,
//   &apate_quest::map_data::army,
// };
//
// const get_size_ptr get_size_funcs[] = {
//   nullptr,
//   &apate_quest::map_data::provinces_size,
//   &apate_quest::map_data::building_type_size,
//   &apate_quest::map_data::city_type_size,
//   &apate_quest::map_data::city_size,
//   &apate_quest::map_data::trait_size,
//   &apate_quest::map_data::modificator_size,
//   &apate_quest::map_data::troop_type_size,
//   &apate_quest::map_data::decision_size,
//   &apate_quest::map_data::religion_group_size,
//   nullptr,
//   &apate_quest::map_data::culture_size,
//   &apate_quest::map_data::law_size,
//   &apate_quest::map_data::event_size,
//   &apate_quest::map_data::titulus_size,
//   &apate_quest::map_data::character_size,
//   nullptr,
//   nullptr,
//   &apate_quest::map_data::hero_troop_size,
//   &apate_quest::map_data::army_size,
// };
//
// static_assert(static_cast<int32_t>(devils_engine::core::structure::count) == sizeof(write_funcs) / sizeof(write_funcs[0]));
// static_assert(static_cast<int32_t>(devils_engine::core::structure::count) == sizeof(read_funcs) / sizeof(read_funcs[0]));
// static_assert(static_cast<int32_t>(devils_engine::core::structure::count) == sizeof(get_size_funcs) / sizeof(get_size_funcs[0]));

namespace devils_engine {
  namespace utils {
//     std::string make_technical(const std::string &s) {
//       char buf[s.size()];
//       memset(buf, 0, sizeof(buf[0]) * s.size());
//       size_t counter = 0;
//       for (size_t i = 0; i < s.size(); ++i) {
//         const char c = s[i];
//
//       }
//     }

    constexpr const auto MODE = cista::mode::WITH_VERSION | cista::mode::WITH_INTEGRITY; // opt. versioning + check sum

    namespace data = cista::offset;

    struct cista_serializator {
      struct tile_data {
        float height;
        uint32_t biome;
      };

//       data::string world_name;
//       data::string technical_name;
//       data::string world_settings;
      uint32_t rand_seed;
      uint32_t noise_seed;
      uint32_t current_season;
      data::array<data::array<float, 4>, 4> matrix;

      data::array<data::vector<data::string>, static_cast<size_t>(core::structure::count)> data_array;
      data::array<render::biome_data_t, core::seasons::maximum_biomes> biomes;
      data::vector<tile_data> tiles_array; //  world_serializator::tiles_count
      data::vector<uint8_t> tiles_seasons;
    };

    world_serializator::world_serializator() : ptr(new cista_serializator) { ptr->tiles_array.resize(world_serializator::tiles_count);  }
    world_serializator::~world_serializator() { delete ptr; ptr = nullptr; }
    void world_serializator::set_name(const std::string_view &name) { world_name = name; }
    void world_serializator::set_name(std::string &&name) { world_name = std::move(name); }
    void world_serializator::set_technical_name(const std::string_view &name) { technical_name = name; }
    void world_serializator::set_technical_name(std::string &&name) { technical_name = std::move(name); }
    void world_serializator::set_settings(const std::string_view &name) { world_settings = name; }
    void world_serializator::set_settings(std::string &&name) { world_settings = std::move(name); }
    void world_serializator::set_rand_seed(const uint32_t &seed) { ptr->rand_seed = seed; }
    void world_serializator::set_noise_seed(const uint32_t &seed) { ptr->noise_seed = seed; }
    void world_serializator::add_data(const core::structure &type, const std::string &data) {
      const uint32_t index = static_cast<uint32_t>(type);
      ptr->data_array[index].push_back(data);
    }

    void world_serializator::add_data(const core::structure &type, std::string &&data) {
      const uint32_t index = static_cast<uint32_t>(type);
      ptr->data_array[index].emplace_back(std::move(data));
    }

    void world_serializator::set_world_matrix(const glm::mat4 &mat) {
      ASSERT(mat.length() == 4);
      ASSERT(mat[0].length() == 4);
      for (int i = 0; i < mat.length(); ++i) {
        for (int j = 0; j < mat.length(); ++j) {
          ptr->matrix[i][j] = mat[i][j];
        }
      }
    }

    void world_serializator::set_tile_data(const uint32_t &index, const tile_data &data) {
      ASSERT(index < core::map::hex_count_d(core::map::detail_level));
      ptr->tiles_array[index] = {data.height, data.biome};
    }

    void world_serializator::copy_seasons(core::seasons* s) {
      ptr->current_season = s->current_season;
      ptr->tiles_seasons.resize(core::seasons::data_count);
      memcpy(ptr->tiles_seasons.data(), s->data, core::seasons::data_count);
      memcpy(ptr->biomes.data(), s->biomes, sizeof(s->biomes[0]) * core::seasons::maximum_biomes);
    }

    using array = std::array<world_serializator::container, static_cast<size_t>(core::structure::count)>;

//     void setup_core_structures(array &arr, const uint32_t &arr_index, apate_quest::map_data* data, fptr func) {
//       const size_t index = arr_index;
//       for (size_t i = 0; i < arr[index].data.size(); ++i) {
//         //std::invoke(func, data, std::move(arr[index].data[i]));
//         std::invoke(func, data, arr[index].data[i]);
//       }
// //       arr[index].data.clear();
//     }

    void fill_encryptor_data(size_t* key, size_t* iv) {
      const splitmix64::state s = {0x32a665c};
      splitmix64::state states[5];
      states[0] = splitmix64::rng(s);
      for (uint32_t i = 1; i < 5; ++i) {
        states[i] = splitmix64::rng(states[i-1]);
      }

      key[0] = splitmix64::get_value(states[0]);
      key[1] = splitmix64::get_value(states[1]);
      key[2] = splitmix64::get_value(states[2]);
      key[3] = splitmix64::get_value(states[3]);

      iv[0]  = splitmix64::get_value(states[4]);
      //iv[1]  = splitmix64::get_value(states[5]);
      //iv[2]  = splitmix64::get_value(states[6]);
      //iv[3]  = splitmix64::get_value(states[7]);
    }

    void world_serializator::serialize() {
//       GOOGLE_PROTOBUF_VERIFY_VERSION;
//
//       apate_quest::map_data serialization_container;
//       serialization_container.set_seed(seed);
//
//       auto map_rotation = new apate_quest::map_data::mat4();
//       for (uint32_t i = 0; i < 4; ++i) {
//         auto row = map_rotation->add_row();
//         for (uint32_t j = 0; j < 4; ++j) {
//           row->add_val(mat[i][j]);
//         }
//       }
//
//       serialization_container.set_allocated_map_rotation(map_rotation);
//
//       for (uint32_t i = 0; i < static_cast<uint32_t>(core::structure::count); ++i) {
//         if (write_funcs[i] == nullptr) continue;
//         setup_core_structures(data_container, i, &serialization_container, write_funcs[i]);
//       }
//
//       for (size_t i = 0; i < core::map::hex_count_d(core::map::detail_level); ++i) {
//         auto ptr = serialization_container.add_tile_datas();
//         ptr->set_height(tiles[i].height);
//         ptr->set_biome(tiles[i].biome);
//       }
//
//       std::string raw_data;
//       const bool ret = serialization_container.SerializeToString(&raw_data);
//       if (!ret) throw std::runtime_error("Could not serialize map data");
//
//       const int32_t max_bound = LZ4_compressBound(raw_data.size());
//       if (max_bound <= 0) throw std::runtime_error("Compressor returns bad size");
//
//       std::string mem(max_bound, '\0');
//       ASSERT(mem.size() == size_t(max_bound));
//       const int32_t compressed_data_size = LZ4_compress_HC(raw_data.c_str(), mem.data(), raw_data.size(), mem.size(), LZ4HC_CLEVEL_MAX);
//       if (compressed_data_size <= 0) throw std::runtime_error("Could not compress map data");
//       PRINT_VAR("raw        data size", raw_data.size())
//       PRINT_VAR("max bound  data size", max_bound)
//       PRINT_VAR("compressed data size", compressed_data_size)
//       PRINT_VAR("compress ratio", float(compressed_data_size) / float(raw_data.size()))
//
//       SHA256 sha256;
//       sha256.add(mem.data(), compressed_data_size);
//       uint8_t buffer1[SHA256::HashBytes];
//       memset(buffer1, 0, sizeof(buffer1[0]) * SHA256::HashBytes);
//       sha256.getHash(buffer1);
// //       uint8_t buffer2[SHA256::HashBytes];
// //       memset(buffer2, 0, sizeof(buffer2[0]) * SHA256::HashBytes);
// //       sha256.getHash(buffer2);
//
// //       ASSERT(memcmp(buffer1, buffer2, sizeof(buffer1[0]) * SHA256::HashBytes) == 0);
//
//       std::filesystem::path current_path(global::root_directory());
// #ifndef _NDEBUG
//       std::filesystem::directory_entry current_dir(current_path);
//       ASSERT(current_dir.exists());
//       ASSERT(current_dir.is_directory());
// #endif
//       current_path /= "saves";
//       std::error_code code;
//       std::filesystem::create_directory(current_path, code); // создает директорию
//       if (code) throw std::runtime_error("Could not create dir. " + code.message());
//
//       // теперь нужно придумать какое то название
//       // желательно организовать сохранения в отдельные папки по адекватному названию
//       // самое адекватное это конечно дать человеку назвать мир, а потом создать папку по этому названию
//       // название по всей видимости нужно дать в самом конце, как быть с анонимным генератором?
//       // для анонимного генератора название мира возможно нужно сгенерировать самостоятельно
//       // + нужно сохранить настройки и какой то доступ к генератору
//       // название мира нужно сделать техническим, для того чтобы все папки поддерживались
//       current_path /= technical_name;
//       std::filesystem::create_directory(current_path, code);
//       if (code) throw std::runtime_error("Could not create dir. " + code.message());
//
//       current_path /= "world_data";
//
//       const char end_of_string = '\0';
//
//       std::ofstream map_data_file(current_path, std::ios::out | std::ios::binary);
//       if (!map_data_file.is_open()) throw std::runtime_error("Could not open world_data file");
//       map_data_file.write(world_name.data(), world_name.size());
//       map_data_file.write(&end_of_string, 1);
//       map_data_file.write(technical_name.data(), technical_name.size());
//       map_data_file.write(&end_of_string, 1);
//       map_data_file.write(world_settings.data(), world_settings.size());
//       map_data_file.write(&end_of_string, 1);
//       const size_t write_size = raw_data.size();
//       map_data_file.write(reinterpret_cast<const char*>(&write_size), sizeof(write_size));
//       map_data_file.write(mem.data(), compressed_data_size);
//       map_data_file.write(reinterpret_cast<const char*>(buffer1), SHA256::HashBytes);
//
//       // для сериализации нужно еще записать размер данных в сыром массиве
//       // возможно еще какие то данные, для того чтобы игроку показать когда тот будет выбирать сгенерированный мир
//       // явно нужно записать название какое нибудь, изображение? какое изображение? у нас карта сферическая
//
//       // как то так выглядит сериализация
//
//       google::protobuf::ShutdownProtobufLibrary();

      std::filesystem::path current_path(global::root_directory());
#ifndef _NDEBUG
      std::filesystem::directory_entry current_dir(current_path);
      ASSERT(current_dir.exists());
      ASSERT(current_dir.is_directory());
#endif
      current_path /= "saves";
      std::error_code code;
      std::filesystem::create_directory(current_path, code); // создает директорию
      if (code) throw std::runtime_error("Could not create dir. " + code.message());

      // теперь нужно придумать какое то название
      // желательно организовать сохранения в отдельные папки по адекватному названию
      // самое адекватное это конечно дать человеку назвать мир, а потом создать папку по этому названию
      // название по всей видимости нужно дать в самом конце, как быть с анонимным генератором?
      // для анонимного генератора название мира возможно нужно сгенерировать самостоятельно
      // + нужно сохранить настройки и какой то доступ к генератору
      // название мира нужно сделать техническим, для того чтобы все папки поддерживались
      current_path /= technical_name;
      std::filesystem::create_directory(current_path, code);
      if (code) throw std::runtime_error("Could not create dir. " + code.message());

      //cista::buf mmap{cista::mmap{"data"}};
      const auto buf = cista::serialize<MODE>(*ptr);
// #ifndef _NDEBUG
//       {
//         std::ofstream map_data_file(std::string(current_path) + "/world_data_raw", std::ios::out | std::ios::binary);
//         map_data_file.write(reinterpret_cast<const char*>(buf.data()), buf.size());
//       }
// #endif
      const int32_t max_bound = LZ4_compressBound(buf.size());
      if (max_bound <= 0) throw std::runtime_error("Compressor returns bad size");

      std::vector<uint8_t> mem(max_bound, 0);
      ASSERT(mem.size() == size_t(max_bound));
      //std::string compressed_data = gzip::compress(reinterpret_cast<const char*>(buf.data()), buf.size(), Z_BEST_COMPRESSION);
      const int32_t compressed_data_size = LZ4_compress_HC(reinterpret_cast<const char*>(buf.data()), reinterpret_cast<char*>(mem.data()), buf.size(), mem.size(), LZ4HC_CLEVEL_MAX);
      //if (compressed_data_size <= 0) throw std::runtime_error("Could not compress map data");
      PRINT_VAR("raw        data size", buf.size())
        PRINT_VAR("max bound  data size", max_bound)
        PRINT_VAR("compressed data size", compressed_data_size)
        PRINT_VAR("compress ratio", float(compressed_data_size) / float(buf.size()))
        //PRINT_VAR("compressed data size", compressed_data.size())
        //PRINT_VAR("compress ratio", float(compressed_data.size()) / float(buf.size()))
  // #ifndef _NDEBUG
  //       {
  //         std::ofstream map_data_file(std::string(current_path) + "/world_data_compressed", std::ios::out | std::ios::binary);
  //         map_data_file.write(reinterpret_cast<const char*>(mem.data()), compressed_data_size);
  //       }
  // #endif

      cipher::salsa20 enc;
      size_t key[cipher::salsa20::key_size / sizeof(size_t)];
      size_t iv[cipher::salsa20::iv_size / sizeof(size_t)];

      //using namespace CryptoPP;
      //HC256::Encryption enc;
      //ASSERT(enc.DefaultKeyLength() == 32);
      //ASSERT(enc.IVSize() == 32);
      //size_t key[32 / sizeof(size_t)];
      //size_t iv[32 / sizeof(size_t)];
      fill_encryptor_data(key, iv);
      enc.set_key(reinterpret_cast<uint8_t*>(key));
      enc.set_iv(reinterpret_cast<uint8_t*>(iv));
      //std::vector<uint8_t> out_message(compressed_data_size, 0);
      std::vector<uint8_t> out_message(compressed_data_size, 0);
      constexpr const size_t num_block_per_chunk = 1;
      constexpr const size_t chunk_size = num_block_per_chunk * cipher::salsa20::block_size;
      //uint8_t chunk[chunk_size];
      size_t num_chunks = compressed_data_size / chunk_size;
      size_t remainder_size = compressed_data_size % chunk_size;

      {
        cipher::salsa20 enc;
        size_t key[cipher::salsa20::key_size / sizeof(size_t)];
        size_t iv[cipher::salsa20::iv_size / sizeof(size_t)];
        fill_encryptor_data(key, iv);
        enc.set_key(reinterpret_cast<uint8_t*>(key));
        enc.set_iv(reinterpret_cast<uint8_t*>(iv));
        const std::string plain_text = "salkgjajs;;algjg[[opewttwtewgekvlwkdnvsjkvdkvsdnvsnvskdvjvsnkljckmlanjlkvnl;rvnajbnl;nnba;fnkjdpbmkndsbms;lkbnd;knasklbnzsq[wlgggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggg";
        std::string cipher_text(plain_text.size(), '\0');
        std::string test_text(plain_text.size(), '\0');
        size_t num_chunks = plain_text.size() / chunk_size;
        size_t remainder_size = plain_text.size() % chunk_size;
        //for (size_t i = 0; i < num_chunks; ++i) {
        //  enc.process_blocks(reinterpret_cast<const uint8_t*>(plain_text.data()), reinterpret_cast<uint8_t*>(cipher_text.data()), num_block_per_chunk);
        //}

        //if (remainder_size != 0) {
          enc.process_bytes(reinterpret_cast<const uint8_t*>(plain_text.data()), reinterpret_cast<uint8_t*>(cipher_text.data()), plain_text.size());
        //}

        {
          cipher::salsa20 enc;
          size_t key[cipher::salsa20::key_size / sizeof(size_t)];
          size_t iv[cipher::salsa20::iv_size / sizeof(size_t)];
          fill_encryptor_data(key, iv);
          enc.set_key(reinterpret_cast<uint8_t*>(key));
          enc.set_iv(reinterpret_cast<uint8_t*>(iv));
          //for (size_t i = 0; i < num_chunks; ++i) {
          //  enc.process_blocks(reinterpret_cast<const uint8_t*>(cipher_text.data()), reinterpret_cast<uint8_t*>(test_text.data()), num_block_per_chunk);
          //}

          //if (remainder_size != 0) {
            enc.process_bytes(reinterpret_cast<const uint8_t*>(cipher_text.data()), reinterpret_cast<uint8_t*>(test_text.data()), plain_text.size());
          //}

          assert(test_text == plain_text);
          //PRINT_VAR("test_text", test_text)
          //PRINT_VAR("plain_text", plain_text)
        }
      }

      //for (size_t i = 0; i < num_chunks; ++i) {
      //  enc.process_blocks(reinterpret_cast<const uint8_t*>(mem.data()), out_message.data(), num_block_per_chunk);
      //}

      //if (remainder_size != 0) {
        enc.process_bytes(reinterpret_cast<const uint8_t*>(mem.data()), out_message.data(), compressed_data_size);
      //}

      //enc.SetKeyWithIV(reinterpret_cast<const uint8_t*>(key), 32, reinterpret_cast<const uint8_t*>(iv), 32);
      //enc.ProcessString(out_message.data(), reinterpret_cast<const uint8_t*>(mem.data()), compressed_data_size);

      //SHA512 hasher;
      //hasher.Update(out_message.data(), compressed_data_size);
      ASSERT(picosha2::k_digest_size == hash_size);
      picosha2::hash256(out_message, hash, hash+hash_size);
      //hasher.Final(hash);

      current_path /= "world_data";

      const char end_of_string = '\0';

      std::ofstream map_data_file(current_path, std::ios::out | std::ios::binary);
      if (!map_data_file.is_open()) throw std::runtime_error("Could not open world_data file");
      map_data_file.write(world_name.data(), world_name.size());
      map_data_file.write(&end_of_string, 1);
      map_data_file.write(technical_name.data(), technical_name.size());
      map_data_file.write(&end_of_string, 1);
      map_data_file.write(world_settings.data(), world_settings.size());
      map_data_file.write(&end_of_string, 1);
      const size_t write_size = buf.size();
      map_data_file.write(reinterpret_cast<const char*>(&write_size), sizeof(write_size));
      map_data_file.write(reinterpret_cast<const char*>(out_message.data()), compressed_data_size);
      map_data_file.write(reinterpret_cast<const char*>(hash), hash_size);
    }

//     void fill_structure_data(array &arr, const uint32_t &arr_index, apate_quest::map_data* data, const size_t &count, get_ptr ptr) {
//       const uint32_t index = arr_index;
//       for (size_t i = 0; i < count; ++i) {
//         const std::string &d = std::invoke(ptr, data, i);
//         arr[index].data.push_back(d);
//       }
//     }

    void world_serializator::deserialize(const std::string &path) {
      std::filesystem::path p(path);
      std::filesystem::directory_entry file(p);
      if (!file.exists()) throw std::runtime_error("Could not find file " + path);
      if (!file.is_regular_file()) throw std::runtime_error("Bad file " + path);
      if (p.filename() != "world_data") throw std::runtime_error("File " + path + " is not world data file");

      std::ifstream map_data_file(p, std::ios::in | std::ios::binary);
      {
        const int64_t prev_pos = map_data_file.tellg();
        ASSERT(prev_pos != -1);
        char c;
        map_data_file.read(&c, 1);
        while (c != '\0') { map_data_file.read(&c, 1); }
        const int64_t pos = map_data_file.tellg();
        ASSERT(pos != -1);
        map_data_file.seekg(prev_pos);
        world_name.resize(pos - prev_pos - 1);
        map_data_file.read(world_name.data(), world_name.size());
        ASSERT(pos-1 == map_data_file.tellg());
        map_data_file.seekg(pos); // это скорее всего не нужно
      }

      {
        const int64_t prev_pos = map_data_file.tellg();
        ASSERT(prev_pos != -1);
        char c;
        map_data_file.read(&c, 1);
        while (c != '\0') { map_data_file.read(&c, 1); }
        const int64_t pos = map_data_file.tellg();
        ASSERT(pos != -1);
        map_data_file.seekg(prev_pos);
        technical_name.resize(pos - prev_pos - 1);
        map_data_file.read(technical_name.data(), technical_name.size());
        ASSERT(pos-1 == map_data_file.tellg());
        map_data_file.seekg(pos); // это скорее всего не нужно
      }

      {
        const int64_t prev_pos = map_data_file.tellg();
        ASSERT(prev_pos != -1);
        char c;
        map_data_file.read(&c, 1);
        while (c != '\0') { map_data_file.read(&c, 1); }
        const int64_t pos = map_data_file.tellg();
        ASSERT(pos != -1);
        map_data_file.seekg(prev_pos);
        world_settings.resize(pos - prev_pos - 1);
        map_data_file.read(world_settings.data(), world_settings.size());
        ASSERT(pos-1 == map_data_file.tellg());
        map_data_file.seekg(pos);
      }

      size_t output_size = 0;
      {
        char buf[sizeof(output_size)];
        map_data_file.read(buf, sizeof(output_size));
        memcpy(&output_size, buf, sizeof(output_size));
        //ASSERT(output_size == 3849016); // почему то иногда меняются размеры выходного буфера, но при этом вроде бы все данные одинаковые
        PRINT_VAR("raw        data size", output_size)
      }

      //using namespace CryptoPP;
      std::vector<uint8_t> raw_data(output_size, 0);
      {
        const int64_t pos = map_data_file.tellg();
        ASSERT(pos >= 0);
        map_data_file.seekg(0, std::ios::end);
        const int64_t last_pos = map_data_file.tellg();
        ASSERT(last_pos >= 0);
        ASSERT(size_t(last_pos) >= pos + hash_size);
        const size_t compressed_data_size = last_pos - pos - hash_size;
        ASSERT(compressed_data_size < INT32_MAX);
        PRINT_VAR("compressed data size", compressed_data_size)
        map_data_file.seekg(pos);

        std::vector<uint8_t> mem(compressed_data_size, 0);
        ASSERT(mem.size() == compressed_data_size);
        map_data_file.read(reinterpret_cast<char*>(mem.data()), mem.size());

        //HC256::Decryption dec;
        //ASSERT(dec.DefaultKeyLength() == 32);
        //ASSERT(dec.IVSize() == 32);
        cipher::salsa20 dec;
        size_t key[cipher::salsa20::key_size / sizeof(size_t)];
        size_t iv[cipher::salsa20::iv_size / sizeof(size_t)];
        fill_encryptor_data(key, iv);
        dec.set_key(reinterpret_cast<uint8_t*>(key));
        dec.set_iv(reinterpret_cast<uint8_t*>(iv));

        constexpr const size_t num_block_per_chunk = 1;
        constexpr const size_t chunk_size = num_block_per_chunk * cipher::salsa20::block_size;
        size_t num_chunks = compressed_data_size / chunk_size;
        size_t remainder_size = compressed_data_size % chunk_size;
        std::vector<uint8_t> dec_message(compressed_data_size, 0);
        //dec.SetKeyWithIV(reinterpret_cast<const uint8_t*>(key), 32, reinterpret_cast<const uint8_t*>(iv), 32);
        //dec.ProcessString(dec_message.data(), mem.data(), compressed_data_size);
        //for (size_t i = 0; i < num_chunks; ++i) {
        //  dec.process_blocks(reinterpret_cast<const uint8_t*>(mem.data()), dec_message.data(), num_block_per_chunk);
        //}

        //if (remainder_size != 0) {
          dec.process_bytes(reinterpret_cast<const uint8_t*>(mem.data()), dec_message.data(), compressed_data_size);
        //}

        const int32_t ret = LZ4_decompress_safe(reinterpret_cast<const char*>(dec_message.data()), reinterpret_cast<char*>(raw_data.data()), dec_message.size(), raw_data.size());
        if (ret < 0) throw std::runtime_error("Bad decompression");
      }

//       std::string saved_data;
//       {
//         const int64_t pos = map_data_file.tellg();
//         ASSERT(pos != -1);
//         map_data_file.seekg(0, std::ios::end);
//         const int64_t last_pos = map_data_file.tellg();
//         ASSERT(last_pos >= pos + SHA256::HashBytes);
//         const size_t compressed_data_size = last_pos - pos - SHA256::HashBytes;
//         ASSERT(compressed_data_size < INT32_MAX);
//         PRINT_VAR("compressed data size", compressed_data_size)
//         map_data_file.seekg(pos);
//
//         std::string mem(compressed_data_size, '\0');
//         ASSERT(mem.size() == compressed_data_size);
//         map_data_file.read(mem.data(), mem.size());
//
//         saved_data.resize(output_size);
//         const int32_t ret = LZ4_decompress_safe_partial(mem.data(), saved_data.data(), mem.size(), saved_data.size(), INT32_MAX);
//         if (ret < 0) throw std::runtime_error("Bad decompression");
//       }

      {
        auto p = cista::deserialize<cista_serializator, MODE>(raw_data);
        *ptr = *p;
      }

//       {
//         apate_quest::map_data serialization_container;
//         serialization_container.ParseFromString(saved_data);
//
//         seed = serialization_container.seed();
//         const auto map_rot = serialization_container.map_rotation();
//         for (int32_t i = 0; i < map_rot.row_size(); ++i) {
//           const auto row = map_rot.row(i);
//           for (int32_t j = 0; j < row.val_size(); ++j) {
//             const float val = row.val(i);
//             mat[i][j] = val;
//           }
//         }
//
//         for (uint32_t i = 0; i < static_cast<uint32_t>(core::structure::count); ++i) {
//           if (get_size_funcs[i] == nullptr) continue;
//           ASSERT((get_size_funcs[i] == nullptr) == (read_funcs[i] == nullptr));
//           fill_structure_data(data_container, i, &serialization_container, std::invoke(get_size_funcs[i], serialization_container), read_funcs[i]);
//         }
//       }

      // нужно еще прочитать хеш для будущих сохранений в этом мире (или мы используем техническое имя? хеш всегда 32 последних байта)
      {
        const int64_t pos = map_data_file.tellg();
        ASSERT(pos != -1);
        //static_assert(hash_size == SHA256::HashBytes);
#ifndef _NDEBUG
        map_data_file.seekg(0, std::ios::end);
        const int64_t last_pos = map_data_file.tellg();
        const uint32_t size = last_pos - pos;
        ASSERT(size == hash_size);
        map_data_file.seekg(pos);
#endif
        map_data_file.read(reinterpret_cast<char*>(hash), hash_size);
      }
    }

    std::string_view world_serializator::get_name() const {
      return world_name;
    }

    std::string_view world_serializator::get_technical_name() const {
      return technical_name;
    }

    std::string_view world_serializator::get_settings() const {
      return world_settings;
    }

    uint32_t world_serializator::get_rand_seed() const { return ptr->noise_seed; }
    uint32_t world_serializator::get_noise_seed() const { return ptr->rand_seed; }

    uint32_t world_serializator::get_data_count(const core::structure &type) const {
      const uint32_t index = static_cast<uint32_t>(type);
      ASSERT(type < core::structure::count);
      return ptr->data_array[index].size();
    }

    std::string_view world_serializator::get_data(const core::structure &type, const uint32_t &index) const {
      const uint32_t data_index = static_cast<uint32_t>(type);
      ASSERT(type < core::structure::count);
      const auto &arr = ptr->data_array[data_index];
      if (index >= arr.size()) throw std::runtime_error("Bad data index");
      return arr[index];
    }

    glm::mat4 world_serializator::get_world_matrix() const {
      glm::mat4 mat;
      ASSERT(mat.length() == 4);
      ASSERT(mat[0].length() == 4);
      for (int i = 0; i < mat.length(); ++i) {
        for (int j = 0; j < mat.length(); ++j) {
          mat[i][j] = ptr->matrix[i][j];
        }
      }
      return mat;
    }

    world_serializator::tile_data world_serializator::get_tile_data(const uint32_t &index) const {
      if (index >= tiles_count) throw std::runtime_error("Bad tile data index");
      return {ptr->tiles_array[index].height, ptr->tiles_array[index].biome};
    }

    void world_serializator::fill_seasons(core::seasons* s) const {
      s->current_season = ptr->current_season;
      //ptr->tiles_seasons.resize(core::seasons::data_count);
      ASSERT(ptr->tiles_seasons.size() == core::seasons::data_count);
      memcpy(s->data, ptr->tiles_seasons.data(), core::seasons::data_count);
      memcpy(s->biomes, ptr->biomes.data(), sizeof(s->biomes[0]) * core::seasons::maximum_biomes);
    }

    const uint8_t* world_serializator::get_hash() const {
      return hash;
    }
  }
}
