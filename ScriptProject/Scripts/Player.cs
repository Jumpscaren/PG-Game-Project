using ScriptProject.Engine;
using ScriptProject.Engine.Constants;
using ScriptProject.EngineMath;
using ScriptProject.UserDefined;
using System;
using System.Collections.Generic;
using static ScriptProject.Engine.Input;
using ScriptProject.EngineFramework;

namespace ScriptProject.Scripts
{
    internal class Player : InteractiveCharacterBehaviour
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
        Timer attack_timer = new Timer();
        const float attack_angle = (float)Math.PI / 2.0f;
        const float between_attack_time = 0.45f;
        Timer between_attack_timer = new Timer();

        float health = 100.0f;

        const float max_speed = 4.0f;//8.1f;
        const float princess_speed = max_speed * 0.7f;
        const float drag_speed = 20.0f;
        const float attack_speed = 0.0f;// max_speed / 2.0f;

        GameObject princess;
        bool holding_princess = false;
        Princess princess_script = null;

        List<GameObject> game_objects;
        List<GameObject> remove_path = new List<GameObject>();

        Vector2 saved_position;
        Timer save_position_timer = new Timer();
        const float save_position_time = 5.0f;

        bool invincible_switch = false;
        bool is_invincble = false;
        Timer invincible_timer = new Timer();
        const float invincible_time = 1.0f;

        GameObject current_rope;
        Vector2 current_rope_end_positon;
        GameObject current_rope_hooked_game_object;
        Vector2 current_rope_difference_in_position;
        Sprite current_rope_sprite;

        Timer rope_animate_timer = new Timer();
        float rope_animation_speed = 40.0f;
        float rope_animation_time = 0.0f;

        Timer rope_respawn_timer = new Timer();
        const float rope_respawn_time = 8.0f;

        HoleManager holes = new HoleManager();

        List<GameObject> boomerangs = new List<GameObject>();
        const UInt16 max_boomerangs = 2;

        bool bow_shot = false;
        float bow_charge = 0.0f;
        float bow_charge_max = 100.0f;
        float bow_charge_increase = 200.0f;

        Timer princess_call_timer = new Timer();
        const float princess_call_time = 10.0f;

        InputBuffer inputBuffer = new InputBuffer();

        const float epsilion = 0.1f;

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

            camera = GameObject.TempFindGameObject("PlayerCamera");

            princess = GameObject.TempFindGameObject("Princess");
            princess_script = princess.GetComponent<Princess>();

            game_objects = GameObject.FindGameObjectsWithTag(UserTags.PuzzleBoxes);
            foreach (GameObject obj in game_objects)
            {
                Console.WriteLine(obj.GetTag());
            }

            rope_respawn_timer.SetTimeLimit(rope_respawn_time);
            invincible_timer.SetTimeLimit(invincible_time);
            save_position_timer.SetTimeLimit(save_position_time);
            attack_timer.SetTimeLimit(attack_time);
            between_attack_timer.SetTimeLimit(attack_time + between_attack_time);
            princess_call_timer.SetTimeLimit(princess_call_time);
        }

        void Update()
        {
            GetInput();
            bool stop_movement = !IsEffectOver() && GetEffect().StopMovement();

            if (health <= 0.0f)
            {
                health = 0.0f;
                Console.WriteLine("Player Restart");
                SceneManager.RestartActiveScene();
            }

            if (holes.ShouldDieInHole(body.GetVelocity().Length(), max_speed))
            {
                TakeDamage(null, 20.0f);
                game_object.transform.SetPosition(saved_position);
                body.SetVelocity(new Vector2());
            }

            if (save_position_timer.IsExpired())
            {
                if (!holes.InHoles())
                {
                    saved_position = game_object.transform.GetPosition();
                    save_position_timer.Start();
                }
            }

            bool show_sprite = true;
            if (!invincible_timer.IsExpired())
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
            if (!between_attack_timer.IsExpired())
            {
                current_speed = attack_speed;
            }

            if (attack_timer.IsExpired())
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
            else if (!attack && !stop_movement && !AnimationManager.IsAnimationPlaying(game_object, "Animations/KnightRunAnim.anim"))
            {
                AnimationManager.LoadAnimation(game_object, "Animations/KnightRunAnim.anim");
            }

            if (!holding_princess && between_attack_timer.IsExpired() && !stop_movement && inputBuffer.ConsumeBufferedInput(MouseButton.LEFT))
            {
                AnimationManager.LoadAnimation(game_object, "Animations/KnightAttack.anim");
                attack = true;
                hit_box_body.SetEnabled(true);
                attack_timer.Start();
                between_attack_timer.Start();

                if (velocity.Length() <= max_speed + epsilion)
                {
                    velocity = velocity.Normalize() * attack_speed;
                }
                current_speed = attack_speed;
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

            new_velocity = new_velocity.Normalize() * current_speed;
            Movement(velocity, new_velocity, current_speed, drag_speed, body);

            //new_velocity = new_velocity.Normalize() * current_speed;
            //if (velocity.Length() <= current_speed && new_velocity.Length() != 0.0f)
            //    velocity = new_velocity;
            ////else
            ////    velocity += new_velocity * Time.GetDeltaTime();
            //if (new_velocity.Length() == 0.0f && velocity.Length() <= current_speed)
            //    velocity = new Vector2(0.0f, 0.0f);
            //if (velocity.Length() > current_speed)
            //    velocity -= velocity.Normalize() * drag_speed * Time.GetDeltaTime();
            //if (new_velocity.Length() > 0.0f && velocity.Length() < current_speed)
            //    velocity = velocity.Normalize() * current_speed;

            //body.SetVelocity(velocity);

            if (stop_movement)
            {
                return;
            }

            sprite.FlipX(mouse_dir.x < epsilion);

            PrincessLogic();

            RopeLogic(new_velocity);

            BoomerangLogic(mouse_dir);

            BowLogic(mouse_dir);

            PrincessCall();
        }

        public override void TakeDamage(GameObject hit_object, float damage)
        {
            if (!is_invincble)
            {
                health -= damage;
                invincible_timer.Start();
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
                GameMaster.game_master.NextScene();
                //SceneManager.RestartActiveScene();
            }
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
            if (!attack_timer.IsExpired())
            {
                attack_time_rot = (1.0f - (attack_timer.GetTime() - Time.GetElapsedTime()) / attack_time) * attack_angle;
            }
            return calculated_rot - attack_angle / 2.0f + attack_time_rot;
        }


        void PrincessLogic()
        {
            float distance_from_princess = (game_object.transform.GetPosition() - princess.transform.GetPosition()).Length();
            princess_script.KnightHoldingPrincess(holding_princess);

            bool pressed_e = inputBuffer.ConsumeBufferedInput(Key.E);

            if (current_rope != null || holding_princess && pressed_e)
            {
                holding_princess = false;
                return;
            }

            if (distance_from_princess > 1.5f)
            {
                return;
            }

            if (pressed_e)
            {
                holding_princess = true;
            }
        }

        void DestroyRope()
        {
            GameObject.DeleteGameObject(current_rope);
            current_rope = null;
            current_rope_hooked_game_object = null;
            current_rope_sprite = null;

            rope_respawn_timer.Start();
        }

        void RopeLogic(Vector2 new_velocity)
        {
            if (!rope_respawn_timer.IsExpired())
            {
                return;
            }

            if (inputBuffer.ConsumeBufferedInput(Input.MouseButton.RIGHT))
            {
                if (current_rope != null)
                {
                    DestroyRope();
                    return;
                }

                GameObject rope = GameObject.CreateGameObject();
                Sprite rope_sprite = rope.AddComponent<Sprite>();
                Render.LoadTexture("../QRGameEngine/Textures/Rope.png", rope_sprite);

                Vector2 player_position = game_object.transform.GetPosition();
                Vector2 mouse_position = Input.GetMousePositionInWorld(camera);
                Vector2 distance = mouse_position - player_position;

                RaycastResult result = Physics.Raycast(player_position, distance.Normalize(),
                    PhysicCollisionCategory.AllCategories, UserCollisionCategories.FilterForPrincessBlockers, 0);

                if (result.intersected)
                {
                    current_rope = rope;
                    current_rope_end_positon = result.intersected_position;
                    current_rope_hooked_game_object = result.intersected_game_object;
                    current_rope_difference_in_position =
                        result.intersected_game_object.transform.GetPosition() - result.intersected_position;

                    rope_animation_time = (current_rope_end_positon - player_position).Length() / rope_animation_speed;
                    rope_animate_timer.SetTimeLimit(rope_animation_time);
                    rope_animate_timer.Start();

                    current_rope_sprite = rope_sprite;
                }
            }

            if (current_rope != null)
            {
                if (current_rope_hooked_game_object == null)
                {
                    DestroyRope();
                    return;
                }

                Vector2 player_position = game_object.transform.GetPosition();

                Vector2 rope_end_position_with_difference = current_rope_hooked_game_object.transform.GetPosition()
                - current_rope_end_positon - current_rope_difference_in_position;

                Vector2 distance = current_rope_end_positon - player_position + rope_end_position_with_difference;

                if (distance.Length() < 0.8f)
                {
                    DestroyRope();
                    return;
                }

                if (!Physics.RaycastCheckIfClosest(player_position, distance.Normalize(),
                    PhysicCollisionCategory.AllCategories, UserCollisionCategories.FilterForPrincessBlockersAndCharacters, 0,
                    current_rope_hooked_game_object))
                {
                    DestroyRope();
                    return;
                }

                Vector2 direction = distance.Normalize();
                if (!rope_animate_timer.IsExpired())
                {
                    Vector2 target_rope_position = current_rope_end_positon
                        - direction * distance.Length() * ((rope_animate_timer.GetTime() - Time.GetElapsedTime()) / rope_animation_time);
                    distance = target_rope_position - player_position + rope_end_position_with_difference;
                    direction = distance.Normalize();
                }

                current_rope.transform.SetPosition(player_position + distance / 2);
                current_rope.transform.SetScale(new Vector2(0.1f, distance.Length()));

                float rope_rotation = Vector2.Angle(direction, new Vector2(0.0f, 1.0f));
                current_rope.transform.SetLocalRotation(rope_rotation);

                current_rope_sprite.SetUV(new Vector2(), new Vector2(1.0f, distance.Length()));

                if (rope_animate_timer.IsExpired())
                {
                    float rope_velocity = max_speed * (3.5f - 0.5f / distance.Length());

                    Vector2 perpendicular_direction = direction.PerpendicularClockwise();
                    float projected_speed = Vector2.ProjectVectorOnVector(new_velocity, perpendicular_direction);

                    Vector2 velocity = direction * rope_velocity + perpendicular_direction * projected_speed * 2.0f;
                    if (velocity.Length() > rope_velocity)
                    {
                        velocity = velocity.Normalize() * rope_velocity;
                    }
                    body.SetVelocity(velocity);
                }
            }
        }

        void BoomerangLogic(Vector2 boomerang_initial_direction)
        {
            for (int i = 0; i < boomerangs.Count; ++i)
            {
                if (boomerangs[i] == null)
                {
                    boomerangs.RemoveAt(i);
                    --i;
                }
            }

            if (boomerangs.Count < max_boomerangs && inputBuffer.ConsumeBufferedInput(Key.R))
            {
                GameObject boomerang = GameObject.CreateGameObject();
                Sprite boomerang_sprite = boomerang.AddComponent<Sprite>();
                Render.LoadTexture("../QRGameEngine/Textures/Boomerang.png", boomerang_sprite);
                boomerang.transform.SetPosition(game_object.transform.GetPosition());
                boomerang.transform.SetScale(new Vector2(1.5f, 1.5f));

                boomerang.AddComponent<BoomerangScript>().SetInitialDirection(boomerang_initial_direction).SetOwner(game_object);

                boomerangs.Add(boomerang);
            }
        }

        void BowLogic(Vector2 arrow_direction)
        {
            if (Input.GetKeyDown(Key.Q))
            {
                bow_charge += bow_charge_increase * Time.GetDeltaTime();
                if (!bow_shot && bow_charge > bow_charge_max)
                {
                    GameObject arrow = GameObject.CreateGameObject();
                    arrow.AddComponent<Arrow>().InitArrow(game_object.transform.GetPosition(), arrow_direction);
                    bow_shot = true;
                }
            }
            else
            {
                bow_charge = 0.0f;
                bow_shot = false;
            }
        }

        void PrincessCall()
        {
            if (princess_call_timer.IsExpired() && inputBuffer.ConsumeBufferedInput(Key.F))
            {
                princess_script.RunToPosition(game_object.transform.GetPosition());
                princess_call_timer.Start();
            }
        }

        public void PrincessStopFollowPlayer()
        {
            holding_princess = false;
            princess_script.KnightHoldingPrincess(holding_princess);
        }

        void GetInput()
        {
            const float buffered_input_time = 0.5f;
            if (Input.GetMouseButtonPressed(Input.MouseButton.LEFT))
            {
                inputBuffer.AddBufferedInput(Input.MouseButton.LEFT, buffered_input_time);
            }
            if (Input.GetMouseButtonPressed(Input.MouseButton.RIGHT))
            {
                inputBuffer.AddBufferedInput(Input.MouseButton.RIGHT, buffered_input_time);
            }
            if (Input.GetKeyPressed(Input.Key.E))
            {
                inputBuffer.AddBufferedInput(Input.Key.E, buffered_input_time);
            }
            if (Input.GetKeyPressed(Input.Key.R))
            {
                inputBuffer.AddBufferedInput(Input.Key.R, buffered_input_time);
            }
            if (Input.GetKeyPressed(Input.Key.F))
            {
                inputBuffer.AddBufferedInput(Input.Key.F, buffered_input_time);
            }
        }

        public class HitBoxPlayer : HitBoxAction
        {
            float damage = 10.0f;
            //float knockback = 10.3f;
            float knockback = 8.3f;

            public override void OnHit(ScriptingBehaviour hit_box_script, InteractiveCharacterBehaviour hit_object_script)
            {
                if (hit_object_script is Princess)
                {
                    return;
                }

                hit_object_script.TakeDamage(hit_box_script.GetGameOjbect(), damage);

                float rot = hit_box_script.GetGameOjbect().GetParent().transform.GetLocalRotation();
                Vector2 dir = new Vector2((float)Math.Cos(rot), (float)Math.Sin(rot));

                hit_object_script.Knockback(dir, knockback);

                hit_object_script.SetEffect(new Effects.StunEffect(0.75f));
            }

            public override void OnHitAvoidGameObject(ScriptingBehaviour hit_box_script)
            {

            }
        }
    }
}
