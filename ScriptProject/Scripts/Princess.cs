using ScriptProject.Engine;
using ScriptProject.EngineMath;
using ScriptProject.UserDefined;
using System;
using ScriptProject.EngineFramework;

namespace ScriptProject.Scripts
{
    internal class Princess : InteractiveCharacterBehaviour
    {
        DynamicBody body;
        Sprite sprite;
        PathFindingActor actor;
        const float max_speed = 1.0f;
        const float drag_speed = 20.0f;
        const float start_to_move_time = 5;
        float start_to_move_timer = 0;
        const float new_random_direction_time = 3;
        float new_random_direction_timer = 0;
        Vector2 random_direction;
        RandomGenerator random_generator = new RandomGenerator();

        GameObject sprite_game_object;

        bool follow_player = false;
        DynamicBody player_body;
        GameObject player;
        Player player_script;
        const float max_distance_to_player = 0.75f;

        float health = 100.0f;

        GameObject grabbed_by_orc = null;
        DynamicBody grabbed_body = null;

        bool rescue_state = false;
        float rescue_time = 10.0f;
        Timer rescue_timer = new Timer();

        GameObject target = null;
        const float target_speed = 2.75f;

        void Start()
        {
            body = game_object.GetComponent<DynamicBody>();
            sprite = game_object.GetComponent<Sprite>();
            actor = game_object.GetComponent<PathFindingActor>();
            random_direction = new Vector2(0, 0);
            start_to_move_timer = Time.GetElapsedTime() + start_to_move_time;
            new_random_direction_timer = start_to_move_time + new_random_direction_time;

            player = GameObject.TempFindGameObject("Player");
            player_body = player.GetComponent<DynamicBody>();
            player_script = player.GetComponent<Player>();

            sprite_game_object = GameObject.CreateGameObject();
            sprite = sprite_game_object.AddComponent<Sprite>();
            sprite.SetTexture(game_object.GetComponent<Sprite>().GetTexture());
            game_object.RemoveComponent<Sprite>();
            game_object.transform.SetZIndex(0);
            game_object.AddChild(sprite_game_object);

            rescue_timer.SetTimeLimit(rescue_time);
        }

        Vector2 grabbed_position_change = new Vector2(0.0f, 0.4f);
        void Update()
        {
            if (rescue_state)
            {
                if (rescue_timer.IsExpired())
                {
                    Console.WriteLine("Princess Rescue Restart");
                    SceneManager.RestartActiveScene();
                }

                return;
            }

            body.SetEnabled(grabbed_by_orc == null);
            if (grabbed_by_orc != null)
            {
                Vector2 direction = grabbed_by_orc.transform.GetPosition() - game_object.transform.GetPosition();
                float distance = direction.Length();
                const float max_distance_to_orc = 0.0f;
                if (distance > max_distance_to_orc)
                {
                    game_object.transform.SetPosition(grabbed_position_change + grabbed_by_orc.transform.GetPosition() - direction.Normalize() * max_distance_to_orc);
                }

                var player_velocity = grabbed_body.GetVelocity();
                //body.SetVelocity(player_velocity);
                sprite.FlipX(player_velocity.x < 0);
                return;
            }
            grabbed_by_orc = null;

            if (health <= 1e-05)
            {
                health = 0.0f;
                Console.WriteLine("Princess Restart");
                SceneManager.RestartActiveScene();
            }

            if (follow_player)
            {
                Vector2 direction_to_player = player.transform.GetPosition() - game_object.transform.GetPosition();
                float distance_to_player = direction_to_player.Length();
                if (distance_to_player > 1.1f)//0.8f)
                {
                    follow_player = false;
                    player_script.PrincessStopFollowPlayer();
                }

                start_to_move_timer = Time.GetElapsedTime() + start_to_move_time;
                new_random_direction_timer = start_to_move_timer + new_random_direction_time;

                if (distance_to_player > max_distance_to_player)
                {
                    game_object.transform.SetPosition(player.transform.GetPosition() - direction_to_player.Normalize() * max_distance_to_player);
                }

                var player_velocity = player_body.GetVelocity();
                //body.SetVelocity(player_velocity);
                sprite.FlipX(player_velocity.x < 0);
                return;
            }

            if (start_to_move_timer >= Time.GetElapsedTime())
            {
                random_direction = Vector2.Zero;
            }

            if (new_random_direction_timer < Time.GetElapsedTime())
            {
                random_direction = new Vector2(random_generator.RandomFloat(-1.0f, 1.0f), random_generator.RandomFloat(-1.0f, 1.0f)).Normalize();
                new_random_direction_timer = Time.GetElapsedTime() + new_random_direction_time;
            }

            if (!PathFindingActor.IsPositionInWorld(random_direction * 1.5f + game_object.transform.GetPosition()))
            {
                Console.WriteLine("Not in world");
                random_direction = Vector2.Zero;
                new_random_direction_timer = 0.0f;
            }

            bool is_running_to_target = RunToTarget();

            if (!is_running_to_target)
            {
                Vector2 new_velocity = random_direction;
                new_velocity = new_velocity.Normalize() * max_speed;
                Movement(body.GetVelocity(), new_velocity, max_speed, drag_speed, body);
            }

            Vector2 velocity = body.GetVelocity();
            sprite.FlipX(velocity.x < 0);
            //if (velocity.Length() <= max_speed && new_velocity.Length() != 0.0f)
            //    velocity = new_velocity;
            ////else
            ////    velocity += new_velocity * Time.GetDeltaTime();
            //if (new_velocity.Length() == 0.0f && velocity.Length() <= max_speed)
            //    velocity = new Vector2(0.0f, 0.0f);
            //if (velocity.Length() > max_speed)
            //    velocity -= velocity.Normalize() * drag_speed * Time.GetDeltaTime();
            //if (new_velocity.Length() > 0.0f && velocity.Length() < max_speed)
            //    velocity = velocity.Normalize() * max_speed;

            //body.SetVelocity(velocity);
        }

        public override void TakeDamage(GameObject hit_object, float damage)
        {
            health -= damage;
            player_script.PrincessStopFollowPlayer();
            follow_player = false;
            if (hit_object.GetTag() == UserTags.EnemyHitbox)
            {
                OrcEnemy.OrcAngryEventData event_data = new OrcEnemy.OrcAngryEventData();
                event_data.orc_to_target = hit_object.GetParent().GetParent();
                EventSystem.SendEvent("OrcAngry", event_data);
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

            if (grabbed_by_orc != null)
            {
                return;
            }

            if (collided_game_object.GetTag() == UserTags.EnemyHitbox)
            {
                health -= 20.0f;
                float rot = collided_game_object.GetParent().transform.GetLocalRotation();
                Vector2 dir = new Vector2((float)Math.Cos(rot), (float)Math.Sin(rot));
                body.SetVelocity(dir * 10.3f);
            }
        }

        bool RunToTarget()
        {
            if (target == null)
            {
                return false;
            }

            if (follow_player)
            {
                target = null;
                return false;
            }

            Vector2 current_position = game_object.transform.GetPosition();
            //Vector2 last_position = actor.PathFind(target, 1);
            //Vector2 dir = last_position - current_position;
            Vector2 dir = new Vector2();
            Vector2 target_dir = target.transform.GetPosition() - current_position;
            if (target_dir.Length() < 1.01f)
            {
                dir = new Vector2();
                target = null;
            }

            Vector2 velocity = body.GetVelocity();
            float speed = target_speed;
            Vector2 new_velocity = dir.Normalize() * speed;

            Movement(velocity, new_velocity, speed, drag_speed, body);

            return true;
        }

        public void KnightHoldingPrincess(bool holding)
        {
            follow_player = holding;

            if (holding && rescue_state)
            {
                rescue_state = false;
                sprite.FlipY(false);
                rescue_time = rescue_timer.GetTime() - Time.GetElapsedTime();
                if (rescue_time < 3.0f)
                {
                    rescue_time = 3.0f;
                }
                rescue_timer.SetTimeLimit(rescue_time);
                Console.WriteLine(rescue_time);
            }
        }

        public void GrabbedByOrc(GameObject orc)
        {
            grabbed_by_orc = orc;
            grabbed_body = orc.GetComponent<DynamicBody>();
            follow_player = false;
            player_script.PrincessStopFollowPlayer();
            target = null;
        }

        public GameObject GetGrabbedByOrc()
        {
            return grabbed_by_orc;
        }

        public void SetRescueState()
        {
            grabbed_by_orc = null;
            rescue_state = true;
            rescue_timer.Start();
            sprite.FlipY(true);
        }

        public bool GetRescueState()
        {
            return rescue_state;
        }

        public void RunToPosition(Vector2 position)
        {
            GameObject game_object_node = PathFindingActor.GetGameObjectNodeByPosition(position);

            if (game_object_node != null)
            {
                target = game_object_node;
            }
        }
    }
}
