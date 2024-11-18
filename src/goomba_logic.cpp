#include "goomba_logic.hpp"
#include "ai_system.hpp"
#include "world_system.hpp"
#include <cassert>
#include <sstream>
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>

void GoombaLogic::goomba_ceiling_death(Entity hostile) {
    Sprite& goombaCeilingSprite = registry.sprites.get(hostile);
    goombaCeilingSprite = g_texture_paths->at(TEXTURE_ASSET_ID::CEILING_FALL);

    //Motion& goombaCeilingMotion = registry.motions.get(hostile);
    //goombaCeilingMotion.scale = GOOMBA_CEILING_FALL_SCALE;

    registry.gravity.emplace(hostile, std::move(Gravity()));
    registry.damages.remove(hostile);
    registry.healths.remove(hostile);
    registry.bounding_box.remove(hostile);
    registry.projectileTimers.remove(hostile);
}

// Update the ceiling goomba's falling sprite to its dead sprite
void GoombaLogic::goomba_ceiling_splat(Entity hostile) {
    Sprite& goombaCeilingSprite = registry.sprites.get(hostile);
    goombaCeilingSprite = g_texture_paths->at(TEXTURE_ASSET_ID::GOOMBA_DEAD);
}

void GoombaLogic::goomba_land_death(Entity hostile) {
    Sprite& goombaLandSprite = registry.sprites.get(hostile);
    goombaLandSprite = g_texture_paths->at(TEXTURE_ASSET_ID::GOOMBA_DEAD);

    Motion& hostile_motion = registry.motions.get(hostile);
    hostile_motion.velocity = { 0,0 };
    hostile_motion.scale = GOOMBA_LAND_IDLE_SCALE;

    registry.bounding_box.remove(hostile);
    registry.patrol_ais.remove(hostile);
    registry.damages.remove(hostile);
    registry.healths.remove(hostile);
}

void GoombaLogic::goomba_flying_death(Entity hostile) {

}

void GoombaLogic::goomba_get_damaged(Entity hostile, Entity m_weapon) {
    if (registry.healths.has(hostile)) {
        Health& hostile_health = registry.healths.get(hostile);
        Damage damage = registry.damages.get(m_weapon);
        hostile_health.current_health -= damage.damage_dealt;

        // If the goomba isnt dead yet, change their current sprite to their hit sprite
        if (hostile_health.current_health > 0) {
            if (!registry.recentDamageTimers.has(hostile)) {
                registry.recentDamageTimers.emplace(hostile, std::move(RecentlyDamagedTimer()));
            }
            // Change the ceilingGoombas sprite
            Sprite& goombaSprite = registry.sprites.get(hostile);
            Motion& goombaMotion = registry.motions.get(hostile);
            if (registry.hostiles.get(hostile).type == HostileType::GOOMBA_FLYING) {

            }
            else if (registry.hostiles.get(hostile).type == HostileType::GOOMBA_CEILING) {
                goombaSprite = g_texture_paths->at(TEXTURE_ASSET_ID::CEILING_HIT);
                goombaMotion.scale = GOOMBA_CEILING_HIT_SCALE;
            }
            // Change the landGoombas sprite
            else {
                goombaSprite = g_texture_paths->at(TEXTURE_ASSET_ID::GOOMBA_WALK_HIT);
                goombaMotion.scale = GOOMBA_LAND_HIT_SCALE;
            }
        }
        else {
            if (registry.hostiles.get(hostile).type == HostileType::GOOMBA_CEILING) {
                goomba_ceiling_death(hostile);
            }
            else {
                goomba_land_death(hostile);
            }
        }
    }
}

// If the goomba is currently using its damaged sprite, revert it back to its idle sprite
void GoombaLogic::update_damaged_goomba_sprites(float delta_time) {
    for (Entity entity : registry.recentDamageTimers.entities) {
        if (!registry.sprites.has(entity)) {
            continue;
        }
        RecentlyDamagedTimer& damaged_timer = registry.recentDamageTimers.get(entity);
        damaged_timer.counter_ms -= delta_time;
        if (damaged_timer.counter_ms <= 0 && registry.healths.has(entity)) {
            Sprite& goombaSprite = registry.sprites.get(entity);
            Motion& goombaMotion = registry.motions.get(entity);
            // change flying goombas sprite
            if (registry.hostiles.get(entity).type == HostileType::GOOMBA_FLYING) {

            } 
            // change ceiling goombas sprite
            else if (registry.hostiles.get(entity).type == HostileType::GOOMBA_CEILING) {
                goombaSprite = g_texture_paths->at(TEXTURE_ASSET_ID::CEILING_IDLE);
                goombaMotion.scale = GOOMBA_CEILING_IDLE_SCALE;
            }
            else {
                goombaSprite = g_texture_paths->at(TEXTURE_ASSET_ID::GOOMBA_WALK_IDLE);
                goombaMotion.scale = GOOMBA_LAND_IDLE_SCALE;
            }
            registry.recentDamageTimers.remove(entity);
        }
    }
}

// Counts down to when the ceiling goomba can attack again
void GoombaLogic::update_goomba_projectile_timer(float delta_time, Entity current_room) {
    for (Entity entity : registry.projectileTimers.entities) {
        ProjectileTimer& projectile_counter = registry.projectileTimers.get(entity);
        projectile_counter.elapsed_time -= delta_time;
        // TODO for Kuter: should this remain here?
        if (projectile_counter.elapsed_time <= 0 && registry.rooms.get(current_room).has(entity)) {
            AISystem::ceiling_goomba_attack(entity, current_room);
            projectile_counter.elapsed_time = projectile_counter.max_time;
        }
    }
}




