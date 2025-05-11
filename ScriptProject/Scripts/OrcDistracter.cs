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
    internal class OrcDistracter : InteractiveCharacterBehaviour
    {
        GameObject player_game_object;
        GameObject princess_game_object;
        Transform transform;
        Vector2 last_position;
        DynamicBody body;
        Sprite sprite;
        float health = 20.0f;

        bool attack_ready = false;
        bool attacking = false;
        const float attack_time = 0.4f;
        float attack_timer = 0.0f;
        const float delay_time = 0.1f;
        float delay_timer = 0.0f;
        bool delay_attack = false;

        float teleport_timer = 0.0f;
        float teleport_time = 10.0f;

        float max_speed = 4.0f;
        const float drag_speed = 20.0f;

        RandomGenerator random_generator = new RandomGenerator();

        GameObject target = null;

        public static int count = 0;

        Princess princess_script = null;

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

            transform = game_object.transform;
            body = game_object.GetComponent<DynamicBody>();
            sprite = game_object.GetComponent<Sprite>();
            max_speed += random_generator.RandomFloat(-0.3f, 0.3f);

            EventSystem.ListenToEvent("OrcAngry", game_object, OrcAngryEvent);

            game_object.SetName("OrcDistracter");

            target = player_game_object;

            ++count;

            teleport_timer = Time.GetElapsedTime() + teleport_time;
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
                PrincessLogic();
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

        Vector2 right_dir = new Vector2(1.0f, 0.0f);
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

            bool stop_movement = !IsEffectOver() && GetEffect().StopMovement();

            if (teleport_timer < Time.GetElapsedTime() && !stop_movement)
            {
                teleport_timer = Time.GetElapsedTime() + teleport_time;
                body.SetVelocity(new Vector2());

                ListSetGameObject nodes = new ListSetGameObject();
                PathFindingActor.GetRandomNodes(game_object, nodes, 3);
                List<GameObject> nodes_list = nodes.GetData();

                GameObject teleport_game_object = null;
                foreach (GameObject node in nodes_list)
                {
                    if (teleport_game_object == null)
                    {
                        teleport_game_object = node;
                        continue;
                    }

                    float length_to_old_node = (current_position - teleport_game_object.transform.GetPosition()).Length();
                    float length_to_new_node = (current_position - node.transform.GetPosition()).Length();

                    bool old_node_inside = length_to_old_node >= 3.0f && length_to_old_node <= 7.0f;
                    bool new_node_inside = length_to_new_node >= 3.0f && length_to_new_node <= 7.0f;

                    if (!old_node_inside && new_node_inside)
                    {
                        teleport_game_object = node;
                    }
                    else if (old_node_inside && new_node_inside)
                    {
                        if (length_to_new_node > length_to_old_node)
                        {
                            teleport_game_object = node;
                        }
                    }
                    else if (!old_node_inside && !new_node_inside)
                    {
                        if (length_to_new_node > 3.0f && length_to_new_node < length_to_old_node)
                        {
                            teleport_game_object = node;
                        }
                    }
                }

                ListSetGameObject characters = new ListSetGameObject();
                CharactersInterface.GetInteractiveCharacters(characters);

                Vector2 diff = teleport_game_object.transform.GetPosition() - transform.GetPosition();
                foreach (GameObject character in characters.GetData())
                {
                    if ((character.transform.GetPosition() - transform.GetPosition()).Length() < 2.0f)
                    {
                        character.transform.SetPosition(character.transform.GetPosition() + diff);
                    }
                }

                transform.SetPosition(teleport_game_object.transform.GetPosition());
                attack_ready = true;
            }

            Vector2 velocity = body.GetVelocity();
            float speed = max_speed;
            if (attacking)
            {
                speed = 0.0f;
            }

            if (velocity.Length() > speed)
                velocity -= velocity.Normalize() * drag_speed * Time.GetDeltaTime();
            else
            {
                velocity = new Vector2();
            }

            body.SetVelocity(velocity);
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

            if (!delay_attack && attack_ready)
            {
                delay_attack = true;
                delay_timer = delay_time + Time.GetElapsedTime();
            }

            if (delay_attack && delay_timer < Time.GetElapsedTime())
            {
                attack_timer = attack_time + Time.GetElapsedTime();
                attack_ready = false;
                attacking = true;
                delay_attack = false;

                GameObject fireball = GameObject.CreateGameObject();
                fireball.AddComponent<Sprite>();
                fireball.transform.SetPosition(transform.GetPosition());
                PrefabSystem.InstanceUserPrefab(fireball, "Fireball");
                fireball.GetComponent<ProjectileScript>().SetCreator(this.game_object);
            }

            if (attacking && attack_timer < Time.GetElapsedTime())
            {
                attacking = false;
            }
        }

        void PrincessLogic()
        {
            if (!IsEffectOver() && GetEffect().StopMovement())
            {
                return;
            }

            if (target != princess_game_object)
            {
                return;
            }

            float distance_to_princess = (game_object.transform.GetPosition() - princess_game_object.transform.GetPosition()).Length();
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
    }
}
