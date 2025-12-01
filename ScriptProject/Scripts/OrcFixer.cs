using ScriptProject.Engine;
using ScriptProject.EngineMath;
using ScriptProject.UserDefined;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Security.Policy;
using System.Text;
using System.Threading.Tasks;
using static ScriptProject.Scripts.OrcCarrier;
using static ScriptProject.Scripts.Player;
using ScriptProject.EngineFramework;
using ScriptProject.Engine.Constants;

namespace ScriptProject.Scripts
{
    internal class OrcFixer : InteractiveCharacterBehaviour
    {
        GameObject player_game_object;
        PathFindingActor actor;
        Transform transform;
        Vector2 last_position;
        DynamicBody body;
        Sprite sprite;
        float health = 20.0f;

        GameObject sprite_game_object;

        bool attack_ready = false;
        bool attacking = false;
        const float attack_time = 0.4f;
        Timer attack_timer = new Timer(attack_time);
        const float delay_time = 0.8f;
        Timer delay_timer = new Timer(delay_time);
        bool delay_attack = false;

        const float attack_distance = 5.0f;

        float max_speed = 2.0f;
        const float drag_speed = 20.0f;

        const float calculate_path_time = 0.2f;
        Timer calculate_path_timer = new Timer(calculate_path_time);

        RandomGenerator random_generator = new RandomGenerator();

        GameObject target = null;
        bool is_target_spawner = false;

        bool dead = false;
        bool falling = false;
        float falling_speed = 1.0f;

        HoleManager holes = new HoleManager();

        List<GameObject> fence_spawners;
        const float health_per_fix_tick = 10.0f;
        const float fix_delay_time = 1.0f;
        Timer fix_delay_timer = new Timer(fix_delay_time);
        const float fix_distance = 0.5f;
        bool fix_ready = false;

        void Start()
        {
            player_game_object = GameObject.TempFindGameObject("Player");
            actor = game_object.GetComponent<PathFindingActor>();
            actor.SetShowPath(true);
            transform = game_object.transform;
            body = game_object.GetComponent<DynamicBody>();

            sprite_game_object = GameObject.CreateGameObject();
            sprite_game_object.transform.SetScale(transform.GetScale());
            transform.SetScale(new Vector2(1.0f, 1.0f));
            sprite = sprite_game_object.AddComponent<Sprite>();
            sprite.SetTexture(game_object.GetComponent<Sprite>().GetTexture());
            game_object.RemoveComponent<Sprite>();
            game_object.transform.SetZIndex(0);
            game_object.AddChild(sprite_game_object);

            max_speed += random_generator.RandomFloat(-0.3f, 0.3f);

            EventSystem.ListenToEvent("OrcAngry", game_object, OrcAngryEvent);

            game_object.SetName("OrcFixer");

            target = player_game_object;

            last_position = transform.GetPosition();

            fence_spawners = GameObject.FindGameObjectsWithTag(UserTags.FenceSpawner);
        }

        void FixedUpdate()
        {
            SetTarget();
            is_target_spawner = target.GetTag() == UserTags.FenceSpawner;

            Death();
            if (!dead)
            {
                Look();
                Move();

                if (!is_target_spawner)
                {
                    Attack();
                }
                else
                {
                    Fix();
                }
            }
        }

        void Remove()
        {
           // Console.WriteLine("Remove Event");
            EventSystem.StopListeningToEvent("OrcAngry", game_object, OrcAngryEvent);
        }

        public override void TakeDamage(GameObject hit_object, float damage)
        {
            health -= damage;
            if (health <= 0.0f)
            {
                dead = true;
            }

            AnimationManager.LoadAnimation(game_object, "Animations/HurtTest.anim");
        }

        public override void Knockback(Vector2 dir, float knockback)
        {
            body.SetVelocity(dir * knockback);
        }

        void BeginCollision(GameObject collided_game_object)
        {
            if (collided_game_object.GetName() == "Bouncer")
            {
                Vector2 direction = game_object.transform.GetPosition() - collided_game_object.transform.GetPosition();
                body.SetVelocity(direction.Normalize() * 20.0f);
            }

            if (holes.AddHole(collided_game_object, dead, max_speed, body.GetVelocity().Length()) == null)
            {
                DieByFalling();
            }
        }

        void EndCollision(GameObject collided_game_object)
        {
            holes.RemoveHole(collided_game_object);
        }

        void SetTarget()
        {
            if (target != null && target.GetTag() == UserTags.FenceSpawner)
            {
                return;
            }

            Spawner lowest_health_spawner = null;
            foreach (GameObject fence_spawner in fence_spawners)
            {
                Spawner spawner = fence_spawner.GetComponent<Spawner>();
                if (lowest_health_spawner == null || (spawner.GetHealth() < lowest_health_spawner.GetHealth()))
                {
                    lowest_health_spawner = spawner;
                }
            }

            if (lowest_health_spawner != null && lowest_health_spawner.GetGameOjbect() != target)
            {
                if (lowest_health_spawner.GetHealth() / lowest_health_spawner.GetMaxHealth() < 0.5f)
                {
                    target = lowest_health_spawner.GetGameOjbect();
                }
                else
                {
                    target = player_game_object;
                }
            }

            if (target == null)
            {
                target = player_game_object;
            }
        }

        void Look()
        {
            if (!IsEffectOver() && GetEffect().StopMovement())
            {
                return;
            }

            Vector2 player_position = target.transform.GetPosition();
            Vector2 player_dir = (player_position - game_object.transform.GetPosition()).Normalize();
            sprite.FlipX(player_dir.x < 0);
        }

        void Move()
        {
            Vector2 current_position = transform.GetPosition();
            Vector2 dir = last_position - current_position;
            Vector2 target_dir = target.transform.GetPosition() - current_position;

            if (calculate_path_timer.IsExpired())
            {
                actor.PathFind(target, 1);
                calculate_path_timer.Start();
                last_position = current_position;
                dir = last_position - current_position;
            }

            if (dir.Length() < 0.1f)
            {
                last_position = actor.GetNextNodePosition(1);
                dir = last_position - current_position;
            }
            last_position = actor.GetCurrentNodePosition();

            dir = last_position - current_position;

            if ((target.transform.GetPosition() - current_position).Length() < 2.0f)
            {
                dir = target.transform.GetPosition() - current_position;
            }

            if (!is_target_spawner && target_dir.Length() < attack_distance)
            {
                dir = new Vector2();
                attack_ready = true;
            }

            if (is_target_spawner && target_dir.Length() < fix_distance)
            {
                dir = new Vector2();
                fix_ready = true;
            }

            Vector2 velocity = body.GetVelocity();
            float speed = max_speed;
            if (attacking)
            {
                speed = 0.0f;
            }

            Vector2 new_velocity = dir.Normalize() * speed;
            FixedMovement(velocity, new_velocity, speed, drag_speed, body);
        }

        void Death()
        {
            if (!dead && holes.ShouldDieInHole(body.GetVelocity().Length(), max_speed))
            {
                DieByFalling();
            }

            if (health <= 0.0f && !falling)
            {
                health = 0.0f;
                GameObject.DeleteGameObject(game_object);
                return;
            }

            if (dead && falling)
            {
                var scale = transform.GetScale();
                var rotation = transform.GetLocalRotation();
                scale.x -= falling_speed * GetDeltaTime();
                scale.y -= falling_speed * GetDeltaTime();
                rotation += falling_speed * GetDeltaTime();
                falling_speed += 1.5f * GetDeltaTime();
                transform.SetScale(scale);
                transform.SetLocalRotation(rotation);
                if (scale.x < 0.01f)
                {
                    GameObject.DeleteGameObject(game_object);
                }
            }
        }

        void Attack()
        {
            if (!IsEffectOver() && GetEffect().StopMovement())
            {
                return;
            }

            if (!delay_attack && attack_ready)
            {
                delay_attack = true;
                delay_timer.Start();
            }

            if (delay_attack && delay_timer.IsExpired())
            {
                attack_timer.Start();
                attack_ready = false;
                attacking = true;
                delay_attack = false;

                GameObject hammer = GameObject.CreateGameObject();
                Vector2 target_direction = target.transform.GetPosition() - game_object.transform.GetPosition();
                hammer.AddComponent<Hammer>().InitHammer(game_object.transform.GetPosition(), target_direction.Normalize(), game_object);
            }

            if (attacking && attack_timer.IsExpired())
            {
                attacking = false;
            }
        }

        void Fix()
        {
            attacking = false;

            if (!IsEffectOver() && GetEffect().StopMovement())
            {
                return;
            }

            if (target == null)
            {
                return;
            }

            Spawner spawner = target.GetComponent<Spawner>();
            if (fix_ready && fix_delay_timer.IsExpired())
            {
                spawner.TakeDamage(null, -health_per_fix_tick);
                fix_delay_timer.Start();
                fix_ready = false;
            }

            const float epsilon = 0.01f;
            if (spawner.GetHealth() >= spawner.GetMaxHealth() - epsilon)
            {
                target = null;
            }
        }

        void DieByFalling()
        {
            if (dead)
            {
                return;
            }

            TakeDamage(null, 100.0f);
            body.SetVelocity(new Vector2());
            falling = true;
            game_object.RemoveComponent<CircleCollider>();
        }

        void OrcAngryEvent(EventSystem.BaseEventData data)
        {
            OrcEnemy.OrcAngryEventData orc_event_data = (OrcEnemy.OrcAngryEventData)data;
            //Console.WriteLine("Entity id: " + game_object.GetEntityID());
            //Console.WriteLine("Orc Angry Entity Id: " + orc_event_data.orc_to_target.GetEntityID());

            if (orc_event_data.orc_to_target != game_object)
            {
                target = orc_event_data.orc_to_target;
            }
        }

        float GetDeltaTime()
        {
            return PhysicConstants.TIME_STEP;
            //return Time.GetDeltaTime();
        }
    }
}
