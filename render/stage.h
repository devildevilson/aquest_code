#ifndef STAGE_H
#define STAGE_H

namespace devils_engine {
  namespace game {
    struct image_resources_t;
  }
  
  namespace render {
    class context;
    
    class stage {
    public:
      virtual ~stage() {}
      virtual void begin() = 0;
      virtual void proccess(context* ctx) = 0;
      virtual void clear() = 0;
    };
    
    class pipeline_stage {
    public:
      virtual ~pipeline_stage() {}
      virtual void recreate_pipelines(const game::image_resources_t* resource) = 0;
    };
  }
}

#endif
