#pragma once
#include "battle_game/core/bullet.h"

namespace battle_game::bullet {
class Mine : public Bullet {
 public:
  Mine(GameCore *core,
       uint32_t id,
       uint32_t unit_id,
       uint32_t player_id,
       glm::vec2 position,
       float rotation,
       float damage_scale);
  ~Mine() override;
  void Render() override;
  void Update() override;

 private:
  uint32_t light_count_down_{6};
  uint32_t dark_count_down_{6};
  uint32_t hide_count_down_{180};
};
}