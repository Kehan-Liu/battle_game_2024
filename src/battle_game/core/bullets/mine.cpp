#include "battle_game/core/bullets/mine.h"

#include "battle_game/core/game_core.h"
#include "battle_game/core/particles/particles.h"
#include <iostream>

namespace battle_game::bullet {

namespace {
uint32_t mine_model_index = 0xffffffffu;
uint32_t mine_light_index = 0xffffffffu;
}

Mine::Mine(GameCore *core,
           uint32_t id,
           uint32_t unit_id,
           uint32_t player_id,
           glm::vec2 position,
           float rotation,
           float damage_scale)
    : Bullet(core, id, unit_id, player_id, position, 0.0f, damage_scale) {
  if (!~mine_model_index) {
    auto mgr = AssetsManager::GetInstance();
    {
      /*Mine Body*/
      std::vector<ObjectVertex> mine_vertices;
      std::vector<uint32_t> mine_indices;
      const int precision = 60;
      const float inv_precision = 1.0f / float(precision);
      for (int i = 0; i < precision; i++) {
        auto theta = (float(i) + 0.5f) * inv_precision;
        theta *= glm::pi<float>() * 2.0f;
        auto sin_theta = std::sin(theta);
        auto cos_theta = std::cos(theta);
        mine_vertices.push_back({{sin_theta * 0.5f, cos_theta * 0.5f},
                   {0.0f, 0.0f},
                   {0.0f, 0.3f, 0.0f, 0.8f}});
        mine_indices.push_back(i);
        mine_indices.push_back((i + 1) % precision);
        mine_indices.push_back(precision);
      }
      mine_vertices.push_back(
          {{0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.5f, 0.0f, 0.8f}});
      mine_model_index = mgr->RegisterModel(mine_vertices, mine_indices);
        }

        {
      /*Mine Light*/
      std::vector<ObjectVertex> light_vertices;
      std::vector<uint32_t> light_indices;
      const int light_precision = 30;
      const float light_inv_precision = 1.0f / float(light_precision);
      for (int i = 0; i < light_precision; i++) {
        auto theta = (float(i) + 0.5f) * light_inv_precision;
        theta *= glm::pi<float>() * 2.0f;
        auto sin_theta = std::sin(theta);
        auto cos_theta = std::cos(theta);
        light_vertices.push_back({{sin_theta * 0.15f, cos_theta * 0.15f},
                                  {0.0f, 0.0f},
                                  {1.0f, 0.0f, 0.0f, 1.0f}});
        light_indices.push_back(i);
        light_indices.push_back((i + 1) % light_precision);
        light_indices.push_back(light_precision);
      }
      light_vertices.push_back(
          {{0.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}});
      mine_light_index = mgr->RegisterModel(light_vertices, light_indices);
    }
  }
}

void Mine::Render() {
  if (hide_count_down_ > 0) {
    hide_count_down_--;
    battle_game::SetTransformation(position_);
    battle_game::SetColor(glm::vec4{1.0f, 1.0f, 1.0f, 1.0f});
    battle_game::SetTexture(0);
    battle_game::DrawModel(mine_model_index);
    bool light = (hide_count_down_ % (light_count_down_ + dark_count_down_)) <
                 light_count_down_;
    if (light) {
      battle_game::DrawModel(mine_light_index);
    }

  }
}

void Mine::Update() {
  bool explode = false;
  auto &units = game_core_->GetUnits();
  for (auto &unit : units) {
    if (unit.first == unit_id_) {
      continue;
    }
    if (unit.second->IsHit(position_)) {
      game_core_->PushEventDealDamage(unit.first, id_, damage_scale_ * 20.0f);
      explode = true;
    }
  }
  if (explode) {
    game_core_->PushEventRemoveBullet(id_);
  }
}

Mine::~Mine() {
  for (int i = 0; i < 5; i++) {
    game_core_->PushEventGenerateParticle<particle::Smoke>(
        position_, 0.0F, game_core_->RandomInCircle() * 2.0f, 0.5f,
        glm::vec4{0.0f, 0.0f, 0.0f, 1.0f}, 30.0f);
  }
}

} // namespace battle_game::bullet