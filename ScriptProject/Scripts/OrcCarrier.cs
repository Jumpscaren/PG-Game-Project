using ScriptProject.Engine;
using ScriptProject.EngineMath;
using ScriptProject.UserDefined;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Security.Cryptography.X509Certificates;
using System.Text;
using System.Threading.Tasks;
using static ScriptProject.Scripts.OrcEnemy;

namespace ScriptProject.Scripts
{
    internal class OrcCarrier : InteractiveCharacterInterface
    {
        GameObject player_game_object;
        GameObject princess_game_object;
        PathFindingActor actor;
        Transform transform;
        Vector2 last_position;
        GameObject hit_box;
        GameObject mid_block;
        DynamicBody body;
        Sprite sprite;
        float health = 20.0f;

        StaticBody hit_box_body;

        float charge_up = 0.0f;
        bool charged_up = false;
        const float charge_up_distance = 4.0f;
        const float charge_up_increase = 1.0f;
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

        float max_speed = 4.0f;
        const float drag_speed = 20.0f;

        RandomGenerator random_generator = new RandomGenerator();

        GameObject target = null;

        static GameObject only = null;

        public static int count = 0;

        bool grabbed_princess = false;
        Princess princess_script = null;

        GameObject door_game_object;

        GameObject old_target = null;

        bool dead = false;
        bool falling = false;
        float falling_speed = 1.0f;

        HoleManager holes = new HoleManager();

        public static int GetCount()
        {
            return count;
        }

        void Start()
        {
            player_game_object = GameObject.TempFindGameObject("Player");
            princess_game_object = GameObject.TempFindGameObject("Princess");
            princess_script = princess_game_object.GetComponent<Princess>();

            door_game_object = GameObject.FindGameObjectWithTag(UserTags.StartDoor);
            Console.WriteLine(door_game_object);

            actor = game_object.GetComponent<PathFindingActor>();
            transform = game_object.transform;
            body = game_object.GetComponent<DynamicBody>();
            sprite = game_object.GetComponent<Sprite>();
            max_speed += random_generator.RandomFloat(-0.3f, 0.3f);

            CreateHitBox();

            EventSystem.ListenToEvent("OrcAngry", game_object, OrcAngryEvent);

            game_object.SetName("OrcCarrier");

            target = princess_game_object;

            last_position = actor.PathFind(target, 1);

            if (only != null)
            {
                //GameObject.DeleteGameObject(game_object);
            }
            else
            {
                only = game_object;
                //for (int i = 0; i < 10; ++i)
                //{
                //    GameObject new_game_object = GameObject.CreateGameObject();
                //    new_game_object.AddComponent<Sprite>();
                //    PrefabSystem.InstanceUserPrefab(new_game_object, "OrcEnemy");
                //}
            }

            ++count;
        }

        void Update()
        {
            if (target == null)
            {
                target = princess_game_object;
            }

            GameObject grabbed_by_orc = princess_script.GetGrabbedByOrc();
            if (grabbed_by_orc == game_object)
            {
                target = door_game_object;
            }
            else if ((grabbed_by_orc != game_object && grabbed_by_orc != null) || princess_script.GetRescueState())
            {
                target = player_game_object;
            }
            else
            {
                target = princess_game_object;
            }

            Death();
            if (!dead)
            {
                Look();
                Move();
                Attack();
                PrincessLogic();

                Vector2 target_dir = target.transform.GetPosition() - transform.GetPosition();
                if (target == door_game_object && target_dir.Length() < 0.1f)
                {
                    GameObject.DeleteGameObject(game_object);
                    princess_script.SetRescueState();
                    GameObject new_game_object = GameObject.CreateGameObject();
                    new_game_object.AddComponent<Sprite>();
                    PrefabSystem.InstanceUserPrefab(new_game_object, "OrcCarrier");
                }
            }
        }

        void Remove()
        {
            Console.WriteLine("Remove Event");
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
        }

        public override void Knockback(Vector2 dir, float knockback)
        {
            body.SetVelocity(dir * knockback);
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
            hit_box_script.SetHitBoxAction(new HitBoxOrcCarrier());
            hit_box_script.SetAvoidGameObject(game_object);

            mid_block = GameObject.CreateGameObject();
            mid_block.AddChild(hit_box);
            game_object.AddChild(mid_block);
        }

        Vector2 right_dir = new Vector2(1.0f, 0.0f);
        void Look()
        {
            Vector2 player_position = target.transform.GetPosition();
            Vector2 player_dir = (player_position - game_object.transform.GetPosition()).Normalize();
            mid_block.transform.SetLocalRotation(GetMidBlockRotation(Vector2.Angle(player_dir, right_dir)));
            sprite.FlipX(player_dir.x < 0);
        }

        void Move()
        {
            Vector2 current_position = transform.GetPosition();
            Vector2 dir = last_position - current_position;
            Vector2 target_dir = target.transform.GetPosition() - current_position;

            //Console.WriteLine(target != old_target);

            if (target != old_target || dir.Length() < 0.1f || actor.NeedNewPathFind(target, 1))
            {
                //Console.WriteLine("Did this once " + (++times) + ", ent = " + game_object.GetEntityID());
                last_position = actor.PathFind(target, 1);
                //Console.WriteLine(last_position);
                dir = last_position - current_position;
                old_target = target;
            }
            //Console.WriteLine("Dir: " + dir);
            //last_position = actor.PathFind(target, 1);
            actor.DebugPath();
            if (!(target != princess_game_object || target != door_game_object) && target_dir.Length() < 1.01f)
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
            float new_velocity_length = new_velocity.Length();
            if (velocity.Length() <= speed && new_velocity_length != 0.0f)
                velocity = new_velocity;
            //else
            //    velocity += new_velocity * Time.GetDeltaTime();
            if (new_velocity_length == 0.0f && velocity.Length() <= speed)
                velocity = new Vector2(0.0f, 0.0f);
            if (velocity.Length() > speed)
                velocity -= velocity.Normalize() * drag_speed * Time.GetDeltaTime();
            if (new_velocity_length > 0.0f && velocity.Length() < speed)
                velocity = velocity.Normalize() * speed;

            body.SetVelocity(velocity);
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
                //PrefabSystem.InstanceUserPrefab(new_game_object, "OrcCarrier");
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
            if (target == princess_game_object || target == door_game_object)
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

        void PrincessLogic()
        {
            if (target != princess_game_object)
            {
                return;
            }

            float distance_to_princess = (game_object.transform.GetPosition() - princess_game_object.transform.GetPosition()).Length();

            if (distance_to_princess > 1.3f || grabbed_princess)
            {
                return;
            }

            grabbed_princess = true;
            princess_game_object.GetComponent<Princess>().GrabbedByOrc(game_object);
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
            OrcEnemy.OrcAngryEventData orc_event_data = (OrcEnemy.OrcAngryEventData)data;
            //Console.WriteLine("Entity id: " + game_object.GetEntityID());
            //Console.WriteLine("Orc Angry Entity Id: " + orc_event_data.orc_to_target.GetEntityID());

            float distance_to_new_target = (orc_event_data.orc_to_target.transform.GetPosition() - game_object.transform.GetPosition()).Length();
            float distance_to_princess = (orc_event_data.orc_to_target.transform.GetPosition() - princess_game_object.transform.GetPosition()).Length();

            if (orc_event_data.orc_to_target != game_object && distance_to_new_target * 1.5f < distance_to_princess)
            {
                target = orc_event_data.orc_to_target;
            }
        }

        public class HitBoxOrcCarrier : HitBoxAction
        {
            float damage = 5.0f;
            float knockback = 10.3f;

            public override void OnHit(ScriptingBehaviour hit_box_script, InteractiveCharacterInterface hit_object_script)
            {
                hit_object_script.TakeDamage(hit_box_script.GetGameOjbect(), damage);

                float rot = hit_box_script.GetGameOjbect().GetParent().transform.GetLocalRotation();
                Vector2 dir = new Vector2((float)Math.Cos(rot), (float)Math.Sin(rot));

                hit_object_script.Knockback(dir, knockback);
            }

            public override void OnHitAvoidGameObject(ScriptingBehaviour hit_box_script)
            {

            }
        }
    }
}
