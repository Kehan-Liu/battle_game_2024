#include "lkh_tank.h"
#include "battle_game/core/bullets/bullets.h"
#include "battle_game/core/game_core.h"
#include "battle_game/graphics/graphics.h"

namespace battle_game::unit {

namespace {
uint32_t lkh_tank_body_model_index = 0xffffffffu;
uint32_t lkh_tank_turret_model_index = 0xffffffffu;
  }

LKHTank::LKHTank(GameCore *game_core, uint32_t id, uint32_t player_id)
    : Unit(game_core, id, player_id) {
  if (!~lkh_tank_body_model_index) {
    auto mgr = AssetsManager::GetInstance();
    {
      /* Tank Body */
      std::vector<ObjectVertex> body_vertices;
      std::vector<uint32_t> body_indices;
      const int body_precision = 120;
      const float body_inv_precision = 1.0f / float(body_precision);
      for (int i = 0; i < body_precision; i++) {
        auto theta = (float(i) + 0.5f) * body_inv_precision;
        theta *= glm::pi<float>() * 2.0f;
        auto sin_theta = std::sin(theta);
        auto cos_theta = std::cos(theta);
        body_vertices.push_back({{sin_theta * 1.0f, cos_theta * 1.0f},
                                   {0.0f, 0.0f},
                                   {1.0f, 1.0f, 1.0f, 1.0f}});
        body_indices.push_back(i);
        body_indices.push_back((i + 1) % body_precision);
        body_indices.push_back(body_precision);
      }
      body_vertices.push_back(
          {{0.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}});
      lkh_tank_body_model_index =
          mgr->RegisterModel(body_vertices, body_indices);
    }

    {
      /* Tank Turret */
      std::vector<ObjectVertex> turret_vertices;
      std::vector<uint32_t> turret_indices;
      const int precision = 60;
      const float inv_precision = 1.0f / float(precision);
      for (int i = 0; i < precision; i++) {
        auto theta = (float(i) + 0.5f) * inv_precision;
        theta *= glm::pi<float>() * 2.0f;
        auto sin_theta = std::sin(theta);
        auto cos_theta = std::cos(theta);
        turret_vertices.push_back({{sin_theta * 0.5f, cos_theta * 0.5f},
                                   {0.0f, 0.0f},
                                   {0.7f, 0.7f, 0.7f, 1.0f}});
        turret_indices.push_back(i);
        turret_indices.push_back((i + 1) % precision);
        turret_indices.push_back(precision);
      }
      turret_vertices.push_back(
          {{0.0f, 0.0f}, {0.0f, 0.0f}, {0.7f, 0.7f, 0.7f, 1.0f}});
      turret_vertices.push_back(
          {{-0.1f, 0.0f}, {0.0f, 0.0f}, {0.7f, 0.7f, 0.7f, 1.0f}});
      turret_vertices.push_back(
          {{0.1f, 0.0f}, {0.0f, 0.0f}, {0.7f, 0.7f, 0.7f, 1.0f}});
      turret_vertices.push_back(
          {{-0.1f, 1.2f}, {0.0f, 0.0f}, {0.7f, 0.7f, 0.7f, 1.0f}});
      turret_vertices.push_back(
          {{0.1f, 1.2f}, {0.0f, 0.0f}, {0.7f, 0.7f, 0.7f, 1.0f}});
      turret_vertices.push_back(
          {{-0.1f, -1.2f}, {0.0f, 0.0f}, {0.7f, 0.7f, 0.7f, 1.0f}});
      turret_vertices.push_back(
          {{0.1f, -1.2f}, {0.0f, 0.0f}, {0.7f, 0.7f, 0.7f, 1.0f}});
      turret_indices.push_back(precision + 1 + 0);
      turret_indices.push_back(precision + 1 + 1);
      turret_indices.push_back(precision + 1 + 2);
      turret_indices.push_back(precision + 1 + 1);
      turret_indices.push_back(precision + 1 + 2);
      turret_indices.push_back(precision + 1 + 3);
      turret_indices.push_back(precision + 1 + 0);
      turret_indices.push_back(precision + 1 + 1);
      turret_indices.push_back(precision + 1 + 4);
      turret_indices.push_back(precision + 1 + 1);
      turret_indices.push_back(precision + 1 + 4);
      turret_indices.push_back(precision + 1 + 5);
      lkh_tank_turret_model_index =
          mgr->RegisterModel(turret_vertices, turret_indices);
    }
  }
}

void LKHTank::Render() {
  battle_game::SetTransformation(position_, rotation_);
  battle_game::SetTexture(0);
  battle_game::SetColor(game_core_->GetPlayerColor(player_id_));
  battle_game::DrawModel(lkh_tank_body_model_index);
  battle_game::SetRotation(turret_rotation_);
  battle_game::DrawModel(lkh_tank_turret_model_index);
}

void LKHTank::Update() {
  TankMove();
  TurretRotate(glm::radians(90.0f));
  Fire();
}

void LKHTank::TankMove() {
  if (move_count_down_ == 0) {
    auto player = game_core_->GetPlayer(player_id_);
    if (player) {
      auto &input_data = player->GetInputData();
      auto new_position = input_data.mouse_cursor_position;
      if (!game_core_->IsBlockedByObstacles(new_position) && input_data.mouse_button_clicked[GLFW_MOUSE_BUTTON_LEFT]) {
        game_core_->PushEventMoveUnit(id_, new_position);
        move_count_down_ = kTickPerSecond;  // Move interval 0.5 second.
      }
    }
  }
  if (move_count_down_) {
    move_count_down_--;
  }
}

void LKHTank::TurretRotate(float rotate_angular_speed) {
  auto player = game_core_->GetPlayer(player_id_);
  if (player) {
    auto &input_data = player->GetInputData();
    float rotation_offset = 0.0f;
    if (input_data.key_down[GLFW_KEY_A]) {
      rotation_offset += 1.0f;
    }
    if (input_data.key_down[GLFW_KEY_D]) {
      rotation_offset -= 1.0f;
    }
    turret_rotation_ += rotation_offset * rotate_angular_speed * kSecondPerTick;
  }
}

void LKHTank::Fire() {
  if (fire_count_down_ == 0) {
    auto player = game_core_->GetPlayer(player_id_);
    if (player) {
      auto &input_data = player->GetInputData();
      if (input_data.key_down[GLFW_KEY_W] && !input_data.key_down[GLFW_KEY_S]) {
        auto velocity = Rotate(glm::vec2{0.0f, 20.0f}, turret_rotation_);
        GenerateBullet<bullet::CannonBall>(
            position_ + Rotate({0.0f, 1.2f}, turret_rotation_),
            turret_rotation_, GetDamageScale(), velocity);
        fire_count_down_ = kTickPerSecond;  // Fire interval 1 second.
      }
      if (input_data.key_down[GLFW_KEY_S] && !input_data.key_down[GLFW_KEY_W]) {
        auto velocity = Rotate(glm::vec2{0.0f, -20.0f}, turret_rotation_);
        GenerateBullet<bullet::CannonBall>(
            position_ + Rotate({0.0f, -1.2f}, turret_rotation_),
            turret_rotation_, GetDamageScale(), velocity);
        fire_count_down_ = kTickPerSecond;
      }
    }
  }
  if (fire_count_down_) {
    fire_count_down_ --;
  }
}

bool LKHTank::IsHit(glm::vec2 position) const {
  position = WorldToLocal(position);
  return glm::length(position) < 1.0f;
}

const char *LKHTank::UnitName() const {
  return "Inverse Tank";
}

const char *LKHTank::Author() const {
  return "Liu Kehan";
}

}  // namespace battle_game::unit