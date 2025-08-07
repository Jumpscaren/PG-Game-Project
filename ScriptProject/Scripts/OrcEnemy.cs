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

namespace ScriptProject.Scripts
{
    internal class OrcEnemy : InteractiveCharacterBehaviour
    {
        GameObject player_game_object;
        PathFindingActor actor;
        Transform transform;
        Vector2 last_position;
        GameObject hit_box;
        GameObject mid_block;
        DynamicBody body;
        Sprite sprite;
        float health = 20.0f;

        StaticBody hit_box_body;

        GameObject sprite_game_object;

        float charge_up = 0.0f;
        bool charged_up = false;
        const float charge_up_distance = 4.0f;
        const float charge_up_increase = 0.6f;
        const float charge_up_decrease = 0.3f;
        bool attack_ready = false;
        bool attacking = false;
        const float attack_range = 1.3f;
        const float attack_time = 0.4f;
        float attack_timer = 0.0f;
        const float attack_angle = (float)Math.PI / 2.0f;
        const float delay_time = 0.1f;
        float delay_timer = 0.0f;
        bool delay_attack = false;

        float max_speed = 2.0f;
        const float drag_speed = 20.0f;

        const float calculate_path_time = 0.02f;
        Timer calculate_path_timer = new Timer(calculate_path_time);

        RandomGenerator random_generator = new RandomGenerator();

        GameObject target = null;

        static GameObject only = null;

        bool dead = false;
        bool falling = false;
        float falling_speed = 1.0f;

        HoleManager holes = new HoleManager();

        public class OrcAngryEventData : EventSystem.BaseEventData
        {
            public GameObject orc_to_target;
        }

        public static int count = 0;

        public static int GetCount()
        {
            return count; 
        }

        void Start()
        {
            player_game_object = GameObject.TempFindGameObject("Player");
            actor = game_object.GetComponent<PathFindingActor>();
            transform = game_object.transform;
            body = game_object.GetComponent<DynamicBody>();

            sprite_game_object = GameObject.CreateGameObject();
            sprite = sprite_game_object.AddComponent<Sprite>();
            sprite.SetTexture(game_object.GetComponent<Sprite>().GetTexture());
            game_object.RemoveComponent<Sprite>();
            game_object.transform.SetZIndex(0);
            game_object.AddChild(sprite_game_object);

            max_speed += random_generator.RandomFloat(-0.3f, 0.3f);

            CreateHitBox();

            EventSystem.ListenToEvent("OrcAngry", game_object, OrcAngryEvent);

            game_object.SetName("Orc");

            target = player_game_object;

            last_position = actor.PathFind(target, 1);

            if (only != null)
            {
                //GameObject.DeleteGameObject(game_object);
            }
            else
            {
                //only = game_object;
                //for (int i = 0; i < 1000; ++i)
                //{
                //    GameObject new_game_object = GameObject.CreateGameObject();
                //    new_game_object.AddComponent<Sprite>();
                //    PrefabSystem.InstanceUserPrefab(new_game_object, "OrcEnemy");
                //    //Console.WriteLine("New Orc: " + new_game_object.GetEntityID());
                //}
            }

            ++count;
        }

        void Update()
        {
            if (target == null)
            {
                target = player_game_object;
            }

            Death();
            if (!dead)
            {
                Look();
                Move();
                Attack();
            }
        }

        void Remove()
        {
           // Console.WriteLine("Remove Event");
            EventSystem.StopListeningToEvent("OrcAngry", game_object, OrcAngryEvent);

            --count;
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

        void CreateHitBox()
        {
            hit_box = GameObject.CreateGameObject();
            hit_box_body = hit_box.AddComponent<StaticBody>();
            hit_box_body.SetEnabled(false);
            BoxCollider box_collider = hit_box.AddComponent<BoxCollider>();
            box_collider.SetTrigger(true);
            box_collider.SetHalfBoxSize(new Vector2(0.6f, 0.5f));
            hit_box.transform.SetPosition(new Vector2(0.7f, 0.0f));
            hit_box.SetName("OrcHitBox");
            hit_box.SetTag(UserTags.EnemyHitbox);

            HitBox hit_box_script = hit_box.AddComponent<HitBox>();
            hit_box_script.SetHitBoxAction(new HitBoxOrcEnemy());
            hit_box_script.SetAvoidGameObject(game_object);

            mid_block = GameObject.CreateGameObject();
            mid_block.AddChild(hit_box);
            game_object.AddChild(mid_block);
        }

        Vector2 right_dir = new Vector2(1.0f, 0.0f);
        void Look()
        {
            if (!IsEffectOver() && GetEffect().StopMovement())
            {
                return;
            }

            Vector2 player_position = target.transform.GetPosition();
            Vector2 player_dir = (player_position - game_object.transform.GetPosition()).Normalize();
            mid_block.transform.SetLocalRotation(GetMidBlockRotation(Vector2.Angle(player_dir, right_dir)));
            sprite.FlipX(player_dir.x < 0);
        }

        int times = 0;
        void Move()
        {
            Vector2 current_position = transform.GetPosition();
            Vector2 dir = last_position - current_position;
            Vector2 target_dir = target.transform.GetPosition() - current_position;
            if (dir.Length() < 0.1f || actor.NeedNewPathFind(target, 1))
            //if (calculate_path_timer.IsExpired())
            {
                //Console.WriteLine("Did this once " + (++times) + ", ent = " + game_object.GetEntityID());
                last_position = actor.PathFind(target, 1);
                dir = last_position - current_position;
                calculate_path_timer.Start();
            }

            //Console.WriteLine("Dir: " + dir);
            //last_position = actor.PathFind(target, 1);
            actor.DebugPath();
            if (target_dir.Length() < 1.01f)
            {
                dir = new Vector2();
            }

            Vector2 velocity = body.GetVelocity();
            float speed = max_speed;
            if (attacking)
            {
                speed = 0.0f;
            }

            Vector2 new_velocity = dir.Normalize() * speed;
            Movement(velocity, new_velocity, speed, drag_speed, body);
            //float new_velocity_length = new_velocity.Length();
            //if (velocity.Length() <= speed && new_velocity_length != 0.0f)
            //    velocity = new_velocity;
            ////else
            ////    velocity += new_velocity * Time.GetDeltaTime();
            //if (new_velocity_length == 0.0f && velocity.Length() <= speed)  
            //    velocity = new Vector2(0.0f, 0.0f);
            //if (velocity.Length() > speed)
            //    velocity -= velocity.Normalize() * drag_speed * Time.GetDeltaTime();
            //if (new_velocity_length > 0.0f && velocity.Length() < speed)
            //    velocity = velocity.Normalize() * speed;

            //body.SetVelocity(velocity);
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
                //GameObject new_game_object = GameObject.CreateGameObject();
                //new_game_object.AddComponent<Sprite>();
                //PrefabSystem.InstanceUserPrefab(new_game_object, "OrcEnemy");
                return;
            }

            if (dead && falling)
            {
                var scale = transform.GetScale();
                var rotation = transform.GetLocalRotation();
                scale.x -= falling_speed * Time.GetDeltaTime();
                scale.y -= falling_speed * Time.GetDeltaTime();
                rotation += falling_speed * Time.GetDeltaTime();
                falling_speed += 1.5f * Time.GetDeltaTime();
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

            float distance_to_player = (game_object.transform.GetPosition() - target.transform.GetPosition()).Length();
            if (distance_to_player < charge_up_distance)
            {
                if (!attack_ready && !attacking)
                {
                    charge_up += charge_up_increase * Time.GetDeltaTime();
                    if (charge_up > 1.0f)
                    {
                        charge_up = 1.0f;
                        charged_up = true;
                    }
                }
            }
            else
            {
                charge_up -= charge_up_decrease * Time.GetDeltaTime();
                if (charge_up < 0.0f)
                {
                    charge_up = 0.0f;
                }
                charged_up = false;
            }

            if (charged_up)
            {
                charge_up = 0.0f;
                charged_up = false;
                attack_ready = true;
            }

            if (!delay_attack && attack_ready && distance_to_player <= attack_range)
            {
                delay_attack = true;
                delay_timer = delay_time + Time.GetElapsedTime();
            }

            if (delay_attack && delay_timer < Time.GetElapsedTime())
            {
                hit_box_body.SetEnabled(true);
                attack_timer = attack_time + Time.GetElapsedTime();
                attack_ready = false;
                attacking = true;
                delay_attack = false;
            }

            if (attacking && attack_timer < Time.GetElapsedTime())
            {
                attacking = false;
                hit_box_body.SetEnabled(false);
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
            GameObject.DeleteGameObject(mid_block);
            game_object.RemoveComponent<CircleCollider>();
        }

        float GetMidBlockRotation(float calculated_rot)
        {
            float attack_time_rot = 0.0f;
            if (attack_timer > Time.GetElapsedTime() && attacking)
            {
                attack_time_rot = (1.0f - (attack_timer - Time.GetElapsedTime()) / attack_time) * attack_angle;
            }
            return calculated_rot - attack_angle / 2.0f + attack_time_rot;
        }

        void OrcAngryEvent(EventSystem.BaseEventData data)
        {
            OrcAngryEventData orc_event_data = (OrcAngryEventData)data;
            //Console.WriteLine("Entity id: " + game_object.GetEntityID());
            //Console.WriteLine("Orc Angry Entity Id: " + orc_event_data.orc_to_target.GetEntityID());

            if (orc_event_data.orc_to_target != game_object)
            {
                target = orc_event_data.orc_to_target;
            }
        }

        public class HitBoxOrcEnemy : HitBoxAction
        {
            float damage = 20.0f;
            float knockback = 10.3f;

            public override void OnHit(ScriptingBehaviour hit_box_script, InteractiveCharacterBehaviour hit_object_script)
            {
                float rot = hit_box_script.GetGameOjbect().GetParent().transform.GetLocalRotation();
                Vector2 dir = new Vector2((float)Math.Cos(rot), (float)Math.Sin(rot));

                hit_object_script.Knockback(dir, knockback);
                hit_object_script.TakeDamage(hit_box_script.GetGameOjbect(), damage);
            }

            public override void OnHitAvoidGameObject(ScriptingBehaviour hit_box_script)
            {

            }
        }
    }
}
