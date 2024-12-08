using ScriptProject.Engine;
using ScriptProject.EngineMath;
using ScriptProject.UserDefined;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Security.Policy;
using System.Text;
using System.Threading.Tasks;
using static ScriptProject.Scripts.OrcCarrier;

namespace ScriptProject.Scripts
{
    internal class Player : InteractiveCharacterInterface
    {
        DynamicBody body;
        Sprite sprite;
        AnimatableSprite anim_sprite;
        bool attack = false;
        GameObject mid_block = null;
        GameObject hit_box;
        StaticBody hit_box_body;
        GameObject camera;
        const float attack_time = 0.1f;
        float attack_timer = 0.0f;
        const float attack_angle = (float)Math.PI / 2.0f;
        const float between_attack_time = 0.45f;
        float between_attack_timer = 0.0f;

        float health = 100.0f;

        const float max_speed = 4.0f;//8.1f;
        const float princess_speed = max_speed * 0.7f;
        const float drag_speed = 20.0f;
        const float attack_speed = max_speed / 2.0f;

        GameObject princess;
        bool holding_princess = false;
        Princess princess_script = null;

        List<GameObject> game_objects;
        List<GameObject> remove_path = new List<GameObject>();

        Vector2 saved_position;
        float save_position_timer = 0.0f;
        float save_position_time = 5.0f;

        bool invincible_switch = false;
        bool is_invincble = false;
        float invincible_timer = 0.0f;
        float invincible_time = 1.0f;

        HoleManager holes = new HoleManager();

        void Start()
        {
            body = game_object.GetComponent<DynamicBody>();
            sprite = game_object.GetComponent<Sprite>();
            anim_sprite = game_object.GetComponent<AnimatableSprite>();
            hit_box = GameObject.CreateGameObject();
            hit_box.SetName("Attack_Box");
            hit_box_body = hit_box.AddComponent<StaticBody>();
            hit_box_body.SetEnabled(false);
            BoxCollider box_collider = hit_box.AddComponent<BoxCollider>();
            box_collider.SetTrigger(true);
            box_collider.SetHalfBoxSize(new Vector2(0.6f, 0.5f));
            hit_box.transform.SetPosition(new Vector2(0.7f, 0.0f));
            hit_box.SetName("HitBox");
            hit_box.SetTag(UserTags.PlayerHitbox);
            HitBox hit_box_script = hit_box.AddComponent<HitBox>();
            hit_box_script.SetHitBoxAction(new HitBoxPlayer());
            hit_box_script.SetAvoidGameObject(game_object);
            mid_block = GameObject.CreateGameObject();
            mid_block.AddChild(hit_box);
            game_object.AddChild(mid_block);

            //{
            //    GameObject hit_box1 = GameObject.CreateGameObject();
            //    hit_box1.SetName("Attack_Box");
            //    hit_box1.AddComponent<StaticBody>().SetEnabled(false);
            //    box_collider = hit_box1.AddComponent<BoxCollider>();
            //    box_collider.SetTrigger(true);
            //    box_collider.SetHalfBoxSize(new Vector2(0.6f, 0.5f));
            //    hit_box1.transform.SetPosition(0.7f, 0.0f);
            //    hit_box1.SetName("HitBox1");
            //    mid_block.AddChild(hit_box1);
            //}

            camera = GameObject.TempFindGameObject("PlayerCamera");

            princess = GameObject.TempFindGameObject("Princess");
            princess_script = princess.GetComponent<Princess>();

            game_objects = GameObject.FindGameObjectsWithTag(UserTags.PuzzleBoxes);
            foreach (GameObject obj in game_objects)
            {
                Console.WriteLine(obj.GetTag());
            }
        }

        void Update()
        {
            if (health <= 0.0f)
            {
                health = 0.0f;
                Console.WriteLine("Player Restart");
                SceneManager.RestartActiveScene();
            }

            if(holes.ShouldDieInHole(body.GetVelocity().Length(), max_speed))
            {
                TakeDamage(null, 20.0f);
                game_object.transform.SetPosition(saved_position);
                body.SetVelocity(new Vector2());
            }

            if (save_position_timer < Time.GetElapsedTime())
            {
                save_position_timer = Time.GetElapsedTime() + save_position_time;
                if (!holes.InHoles())
                {
                    saved_position = game_object.transform.GetPosition();
                }
            }

            bool show_sprite = true;
            if (invincible_timer > Time.GetElapsedTime())
            {
                invincible_switch = !invincible_switch;
                show_sprite = invincible_switch;
            }
            else
            {
                is_invincble = false;
            }
            sprite.SetShow(show_sprite);

            float current_speed = max_speed;
            if (holding_princess)
            {
                current_speed = princess_speed;
            }
            if (between_attack_timer >= Time.GetElapsedTime())
            {
                current_speed = attack_speed;
            }

            if (attack_timer < Time.GetElapsedTime())
            {
                hit_box_body.SetEnabled(false);
            }

            Vector2 velocity = body.GetVelocity();
            Vector2 new_velocity = new Vector2();
            if (Input.GetKeyDown(Input.Key.W))
                new_velocity.y += max_speed;
            if (Input.GetKeyDown(Input.Key.S))
                new_velocity.y -= max_speed;
            if (Input.GetKeyDown(Input.Key.D))
            {
                new_velocity.x += max_speed;
            }
            if (Input.GetKeyDown(Input.Key.A))
            {
                new_velocity.x -= max_speed;
            }

            if (new_velocity.Length() < 0.01f && !attack)
            {
                if (!AnimationManager.IsAnimationPlaying(game_object, "Animations/KnightIdle.anim"))
                    AnimationManager.LoadAnimation(game_object, "Animations/KnightIdle.anim");
            }
            else if (!attack && !AnimationManager.IsAnimationPlaying(game_object, "Animations/KnightRunAnim.anim"))
            {
                AnimationManager.LoadAnimation(game_object, "Animations/KnightRunAnim.anim");
            }

            if (!holding_princess && Input.GetMouseButtonPressed(Input.MouseButton.LEFT) && between_attack_timer < Time.GetElapsedTime())
            {
                AnimationManager.LoadAnimation(game_object, "Animations/KnightAttack.anim");
                attack = true;
                hit_box_body.SetEnabled(true);
                attack_timer = attack_time + Time.GetElapsedTime();
                between_attack_timer = attack_timer + between_attack_time;
            }

            if (!AnimationManager.IsAnimationPlaying(game_object, "Animations/KnightAttack.anim"))
            {
                attack = false;
            }

            Vector2 mouse_position = Input.GetMousePositionInWorld(camera);
            Vector2 mouse_dir = (mouse_position - game_object.transform.GetPosition()).Normalize();
            Vector2 right_dir = new Vector2(1.0f, 0.0f);
            float calculated_rot = Vector2.Angle(mouse_dir, right_dir);

            mid_block.transform.SetLocalRotation(GetMidBlockRotation(calculated_rot));
            sprite.FlipX(mouse_dir.x < 0);

            new_velocity = new_velocity.Normalize() * current_speed;
            if (velocity.Length() <= current_speed && new_velocity.Length() != 0.0f)
                velocity = new_velocity;
            //else
            //    velocity += new_velocity * Time.GetDeltaTime();
            if (new_velocity.Length() == 0.0f && velocity.Length() <= current_speed)
                velocity = new Vector2(0.0f, 0.0f);
            if (velocity.Length() > current_speed)
                velocity -= velocity.Normalize() * drag_speed * Time.GetDeltaTime();
            if (new_velocity.Length() > 0.0f && velocity.Length() < current_speed)
                velocity = velocity.Normalize() * current_speed;

            body.SetVelocity(velocity);

            PrincessLogic();
        }

        public override void TakeDamage(GameObject hit_object, float damage)
        {
            if (!is_invincble)
            {
                health -= damage;
                invincible_timer = invincible_time + Time.GetElapsedTime();
                is_invincble = true;
            }
        }

        public override void Knockback(Vector2 dir, float knockback)
        {
            body.SetVelocity(dir * knockback);
        }

        void BeginCollision(GameObject collided_game_object)
        {
            //Console.WriteLine("Enter - " + collided_game_object.GetName());
            if (collided_game_object.GetName() == "Bouncer")
            {
                Vector2 direction = game_object.transform.GetPosition() - collided_game_object.transform.GetPosition();
                body.SetVelocity(direction.Normalize() * 20.0f);

                foreach (GameObject obj in remove_path)
                {
                    GameObject.DeleteGameObject(obj);
                }
                remove_path.Clear();

                foreach (GameObject obj in game_objects)
                {
                    GameObject.DeleteGameObject(obj);
                    GameObject new_game_object = GameObject.CreateGameObject();
                    new_game_object.AddComponent<Sprite>();
                    new_game_object.transform.SetPosition(obj.transform.GetPosition());
                    new_game_object.transform.SetZIndex(2);
                    PrefabSystem.InstanceUserPrefab(new_game_object, "Prefab2");
                    remove_path.Add(new_game_object);
                }
                game_objects.Clear();
            }

            if (holes.AddHole(collided_game_object, false, max_speed, body.GetVelocity().Length()) == null)
            {
                TakeDamage(null, 20.0f);
                game_object.transform.SetPosition(saved_position);
                body.SetVelocity(new Vector2());
                Console.WriteLine("Enter - Hole");
                Console.WriteLine("ETag - " + (int)collided_game_object.GetTag());
            }

            if (holding_princess && collided_game_object.GetTag() == UserTags.Finish)
            {
                Console.WriteLine("Finished Level");
                SceneManager.RestartActiveScene();
            }

            if (collided_game_object.GetTag() == UserTags.Player)
            {
                //Console.WriteLine("Enter");
                //Console.WriteLine(normal);
            }

            //if (collided_game_object.GetTag() == UserTags.EnemyHitbox)
            //{
            //    health -= 20.0f;
            //    float rot = collided_game_object.GetParent().transform.GetLocalRotation();
            //    Vector2 dir = new Vector2((float)Math.Cos(rot), (float)Math.Sin(rot));
            //    body.SetVelocity(dir * 11.0f);

            //    PrincessStopFollowPlayer();
            //}
        }

        void EndCollision(GameObject collided_game_object)
        {
            if (collided_game_object.GetTag() == UserTags.Player)
            {
                //Console.WriteLine("Exit");
            }

            holes.RemoveHole(collided_game_object);
        }

        float GetMidBlockRotation(float calculated_rot)
        {
            float attack_time_rot = 0.0f;
            if (attack_timer > Time.GetElapsedTime())
            {
                attack_time_rot = (1.0f - (attack_timer - Time.GetElapsedTime()) / attack_time) * attack_angle;
            }
            return calculated_rot - attack_angle / 2.0f + attack_time_rot;
        }


        void PrincessLogic()
        {
            float distance_from_princess = (game_object.transform.GetPosition() - princess.transform.GetPosition()).Length();
            princess_script.KnightHoldingPrincess(holding_princess);

            if (holding_princess && Input.GetKeyPressed(Input.Key.E))
            {
                holding_princess = false;
                return;
            }

            if (distance_from_princess > 1.5f)
            {
                return;
            }

            if (Input.GetKeyPressed(Input.Key.E))
            {
                holding_princess = true;
            }
        }

        public void PrincessStopFollowPlayer()
        {
            holding_princess = false;
            princess_script.KnightHoldingPrincess(holding_princess);
        }

        public class HitBoxPlayer : HitBoxAction
        {
            float damage = 10.0f;
            float knockback = 10.3f;

            public override void OnHit(ScriptingBehaviour hit_box_script, InteractiveCharacterInterface hit_object_script)
            {
                if (hit_object_script is Princess)
                {
                    return;
                }

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
