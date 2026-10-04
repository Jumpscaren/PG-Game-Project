using ScriptProject.Engine;
using ScriptProject.Engine.Constants;
using ScriptProject.EngineMath;
using ScriptProject.UserDefined;
using System;
using System.Collections.Generic;
using static ScriptProject.Engine.Input;
using ScriptProject.EngineFramework;
using ScriptProject.Scripts.Effects;
using ScriptProject.Scripts.Modules;

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
        BoxCollider hit_box_collider;
        static readonly Vector2 BASE_HALF_BOX_SIZE = new Vector2(0.6f, 0.5f);
        StaticBody hit_box_body;
        HitBoxPlayer hit_box_action;

        GameObject camera;
        const float attack_time = 0.1f;
        Timer attack_timer = new Timer();
        const float attack_combo_time_1 = 0.1f;
        const float attack_combo_time_2 = 0.15f;
        const float attack_combo_time_3 = 0.23f;
        const float attack_angle = (float)Math.PI / 2.0f;
        //const float between_attack_time = 0.45f;
        //const float between_attack_time = 0.35f;
        const float between_attack_time = 0.3f;
        Timer between_attack_timer = new Timer();

        const int MAX_COMBO_ATTACKS = 3;
        Timer combo_timer = new Timer();
        bool save_combo = false;
        Timer wait_for_next_attack_timer = new Timer();
        int attack_number = 0;
        static readonly Vector2 COMBO_HALF_BOX_INCREASE = new Vector2(0.1f, 0.5f);

        List<GameObject> slash_game_objects = new List<GameObject>();

        float health = 100.0f;

        GameObject sprite_game_object;
        AnimatableSprite sprite_anim_sprite;

        const float max_speed = 4.0f;//8.1f;
        //const float max_speed = 100.0f;
        const float princess_speed = max_speed * 0.7f;
        const float drag_speed = 20.0f;
        const float attack_speed = 0.0f;// max_speed / 2.0f;
        float current_speed = 0.0f;

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

        const float roll_animation_speed = 2.0f;
        const float roll_time = 1.2f / roll_animation_speed;
        Timer roll_timer = new Timer(roll_time);
        const float between_rolls_time = 0.5f;
        Timer between_rolls_timer = new Timer(between_rolls_time + roll_time);
        Vector2 roll_direction = new Vector2(0.0f, 0.0f);
        float roll_speed = 8.5f;

        InputBuffer inputBuffer = new InputBuffer();

        const float epsilion = 0.1f;

        GameObject damage_hit_box_game_object;
        PlayerDamageHitBox damage_hit_box_script;
        EffectHolderModule effect_holder = new EffectHolderModule();

        Sprite arm_sprite;
        GameObject mid_arm_block;

        readonly Vector2 right_dir = new Vector2(1.0f, 0.0f);

        void Start()
        {
            body = game_object.GetComponent<DynamicBody>();
            game_object.GetComponent<Sprite>();
            anim_sprite = game_object.GetComponent<AnimatableSprite>();

            game_object.GetComponent<CircleCollider>().SetRadius(0.2f);
            game_object.RemoveComponent<CircleCollider>();

            CapsuleCollider capsule = game_object.AddComponent<CapsuleCollider>();
            capsule.SetRadius(0.2f);
            capsule.SetPoints(new Vector2(-0.1f, 0.0f), new Vector2(0.1f, 0.0f));

            sprite_game_object = GameObject.CreateGameObject();
            Console.WriteLine("Sprite: " + sprite_game_object.GetEntityID());
            sprite = sprite_game_object.AddComponent<Sprite>();
            sprite_anim_sprite = sprite_game_object.AddComponent<AnimatableSprite>();
            sprite.SetTexture(game_object.GetComponent<Sprite>().GetTexture());
            sprite.PixelScale();
            game_object.RemoveComponent<Sprite>();
            game_object.transform.SetZIndex(0);
            game_object.AddChild(sprite_game_object);

            damage_hit_box_game_object = GameObject.CreateGameObject();
            damage_hit_box_script = damage_hit_box_game_object.AddComponent<PlayerDamageHitBox>();
            damage_hit_box_script.Init(game_object);
            game_object.AddChild(damage_hit_box_game_object);

            hit_box = GameObject.CreateGameObject();
            hit_box.SetName("Attack_Box");
            hit_box_body = hit_box.AddComponent<StaticBody>();
            hit_box_body.SetEnabled(false);
            hit_box_collider = hit_box.AddComponent<BoxCollider>();
            hit_box_collider.SetTrigger(true);
            hit_box_collider.SetHalfBoxSize(BASE_HALF_BOX_SIZE);
            hit_box.transform.SetPosition(new Vector2(0.7f, 0.0f));
            hit_box.SetName("HitBox");
            hit_box.SetTag(UserTags.PlayerHitbox);
            HitBox hit_box_script = hit_box.AddComponent<HitBox>();
            hit_box_action = new HitBoxPlayer();
            hit_box_script.SetHitBoxAction(hit_box_action, game_object);
            hit_box_script.SetAvoidGameObject(damage_hit_box_game_object);
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

            CreateArm();
        }

        void Update()
        {
            GetInput();
        }

        void FixedUpdate()
        {
            //GetInput();
            bool stop_movement = !effect_holder.IsEffectOver() && effect_holder.IsEffect<StunEffect>();
            //Console.WriteLine("Stop: " + stop_movement);

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

            if ((!roll_timer.IsExpired() || is_invincble) && damage_hit_box_script.IsHitBoxActive())
            {
                damage_hit_box_script.DeactivateHitBox();
            }
            else if (!damage_hit_box_script.IsHitBoxActive())
            {
                damage_hit_box_script.ActivateHitBox();
            }

                current_speed = max_speed;
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

            Vector2 mouse_position = Input.GetMousePositionInWorld(camera);
            Vector2 mouse_dir = (mouse_position - game_object.transform.GetPosition()).Normalize();

            if (roll_timer.IsExpired())
            {
                string run_animation_name = "Animations/KnightRunAnim.anim";
                run_animation_name = "Animations/KnightTestRunAnim.anim";
                if (new_velocity.Length() < 0.01f && !attack)
                {
                    if (!AnimationManager.IsAnimationPlaying(sprite_game_object, "Animations/KnightIdle.anim"))
                    {
                        AnimationManager.LoadAnimation(sprite_game_object, "Animations/KnightIdle.anim");
                    }
                }
                else if (!attack && !stop_movement && !AnimationManager.IsAnimationPlaying(sprite_game_object, run_animation_name))
                {
                    AnimationManager.LoadAnimation(sprite_game_object, run_animation_name);
                }

                AttackLogic(stop_movement, mouse_dir);

                if (!AnimationManager.IsAnimationPlaying(sprite_game_object, "Animations/KnightAttack2.anim"))
                {
                    attack = false;
                }
            }
            else
            {
                new_velocity = roll_direction;
                //current_speed = roll_speed;
            }

            float calculated_rot = Vector2.Angle(mouse_dir, right_dir);
            if (!roll_timer.IsExpired())
            {
                calculated_rot = Vector2.Angle(roll_direction, right_dir);
            }

            mid_block.transform.SetLocalRotation(GetMidBlockRotation(calculated_rot));

            Vector2 velocity = body.GetVelocity();
            new_velocity = new_velocity.Normalize() * current_speed;
            CharacterMovementModule.FixedMovement(new_velocity, current_speed, drag_speed, body, effect_holder);

            for (int i = 0; i < slash_game_objects.Count; ++i)
            {
                if (!AnimationManager.IsAnimationPlaying(slash_game_objects[i], "Animations/SlashAttackAnimation.anim"))
                {
                    GameObject.DeleteGameObject(slash_game_objects[i]);
                    slash_game_objects.RemoveAt(i);
                    --i;
                }
            }

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

            RollLogic(new_velocity.Normalize());

            ArmRotation(mouse_dir);
        }

        public override bool Interactable()
        {
            return false;
        }

        public override void SetEffect(Effect effect)
        {
            effect_holder.SetEffect(damage_hit_box_script, effect);
        }

        public override bool ShouldEffectBeSet(Effect effect)
        {
            if (effect is StunEffect && (!roll_timer.IsExpired() || is_invincble))
            {
                return false;
            }
            return true;
        }

        public override void TakeDamage(GameObject hit_object, float damage)
        {
            if (!roll_timer.IsExpired() || is_invincble)
            {
                return;
            }

            health -= damage;
            invincible_timer.Start();
            is_invincble = true;
            AnimationManager.LoadAnimation(game_object, "Animations/HurtTest.anim");
            ResetAttackCombo();
        }

        public override void Knockback(Vector2 dir, float knockback)
        {
            if (!roll_timer.IsExpired() || is_invincble)
            {
                return;
            }

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

        void ResetAttackCombo()
        {
            hit_box_action.ResetDamage();
            hit_box_collider.SetHalfBoxSize(BASE_HALF_BOX_SIZE);
            hit_box_action.ResetKnockback();
            hit_box_action.ResetStunEffectTime();

            if (attack_number < 2)
            {
                Console.WriteLine("Combo Failed");
            }
            attack_number = 0;
        }

        void AttackLogic(bool stop_movement, Vector2 attack_dir)
        {
            if (stop_movement)
            {
                return;
            }

            if (!between_attack_timer.IsExpired())
            {
                if (inputBuffer.CheckBufferedInput(MouseButton.LEFT) && attack_number <= 2 && !combo_timer.IsExpired())
                {
                    save_combo = true;
                }
                return;
            }
            bool combo_saved = save_combo;
            save_combo = false;

            if (!combo_saved && attack_number > 0 && between_attack_timer.IsExpired() && wait_for_next_attack_timer.IsExpired())
            {
                wait_for_next_attack_timer.SetTimeLimit(0.3f);
                wait_for_next_attack_timer.Start();
                attack_number = 0;
                ResetAttackCombo();
            }

            bool skip_attack = holding_princess || !wait_for_next_attack_timer.IsExpired() || !inputBuffer.ConsumeBufferedInput(MouseButton.LEFT);
            if (skip_attack)
            {
                return;
            }

            if ((!combo_saved && attack_number != 0 && combo_timer.IsExpired()) || attack_number >= MAX_COMBO_ATTACKS)
            {
                ResetAttackCombo();
            }

            //Refactorera, don't use int, stupid
            float attack_velocity_increase = 0.0f;
            if (attack_number == 0)
            {
                attack_velocity_increase = 0.5f;
                //between_attack_timer.SetTimeLimit(attack_time + between_attack_time * 0.5f);
                between_attack_timer.SetTimeLimit(attack_combo_time_1 + between_attack_time);
                attack_timer.SetTimeLimit(attack_combo_time_1);
                Console.WriteLine("First Combo");
            }
            if (attack_number == 1)
            {
                attack_velocity_increase = 2.5f;
                Console.WriteLine("Second Combo");
                between_attack_timer.SetTimeLimit(attack_combo_time_2 + between_attack_time);
                attack_timer.SetTimeLimit(attack_combo_time_2);
                //between_attack_timer.SetTimeLimit(attack_time + between_attack_time * 1.5f);
                hit_box_action.SetDamage(HitBoxPlayer.MEDIUM_DAMAGE);
                hit_box_action.SetKnockback(HitBoxPlayer.MEDIUM_KNOCKBACK);
                hit_box_action.SetStunEffectTime(HitBoxPlayer.MEDIUM_STUN_EFFECT_TIME);
                hit_box_collider.SetHalfBoxSize(BASE_HALF_BOX_SIZE + COMBO_HALF_BOX_INCREASE * 0.5f);
            }
            if (attack_number == 2)
            {
                const float time_between_attacks = 0.6f;
                //attack_velocity_increase = 4.5f;
                attack_velocity_increase = 6.0f;
                between_attack_timer.SetTimeLimit(attack_combo_time_3 + between_attack_time);
                attack_timer.SetTimeLimit(attack_combo_time_3);
                wait_for_next_attack_timer.SetTimeLimit(between_attack_timer.GetTimeLimit() + time_between_attacks);
                wait_for_next_attack_timer.Start();
                //between_attack_timer.SetTimeLimit(attack_time + between_attack_time * 2.5f);
                hit_box_action.SetDamage(HitBoxPlayer.HIGH_DAMAGE);
                hit_box_action.SetKnockback(HitBoxPlayer.HIGH_KNOCKBACK);
                hit_box_action.SetStunEffectTime(HitBoxPlayer.HIGH_STUN_EFFECT_TIME);
                hit_box_collider.SetHalfBoxSize(BASE_HALF_BOX_SIZE + COMBO_HALF_BOX_INCREASE * 1.5f);
                Console.WriteLine("Third/Last Combo");
            }

            if (attack_number != 2)
            {
                combo_timer.SetTimeLimit(between_attack_timer.GetTimeLimit());
                combo_timer.Start();
                Console.WriteLine("Combo Start");
            }

            Console.WriteLine("Attack number is " + attack_number);
            attack_number++;

            //AnimationManager.LoadAnimation(sprite_game_object, "Animations/KnightAttack.anim");
            AnimationManager.LoadAnimation(sprite_game_object, "Animations/KnightAttack2.anim");
            attack = true;
            hit_box_body.SetEnabled(true);
            attack_timer.Start();
            between_attack_timer.Start();

            Vector2 velocity = body.GetVelocity();
            float previous_speed = velocity.Length();
            if (velocity.Length() <= max_speed + epsilion)
            {
                velocity = velocity.Normalize() * attack_speed;
            }

            velocity += attack_dir * attack_velocity_increase;
            body.SetVelocity(velocity);
            current_speed = attack_speed;

            //Create Slash Sprite
            GameObject slash_sprite = GameObject.CreateGameObject();
            Vector2 position = (hit_box.transform.GetPosition() - mid_block.transform.GetPosition()).Length() * attack_dir;
            slash_sprite.transform.SetPosition(game_object.transform.GetPosition() + position + attack_dir * (hit_box_collider.GetHalfBoxSize().x - 0.3f));
            slash_sprite.transform.SetLocalRotation(mid_block.transform.GetLocalRotation() - attack_angle / 2.0f);
            slash_sprite.transform.SetScale(new Vector2(2.0f * (2.0f * hit_box_collider.GetHalfBoxSize().y), 2.0f * hit_box_collider.GetHalfBoxSize().x));
            Sprite sprite = slash_sprite.AddComponent<Sprite>();
            Render.LoadTexture("../QRGameEngine/Textures/TestAttack2.png", sprite);
            sprite.SetMaterial("SlashAttackMaterial");

            slash_sprite.AddComponent<AnimatableSprite>();
            AnimationManager.LoadAnimation(slash_sprite, "Animations/SlashAttackAnimation.anim");

            slash_game_objects.Add(slash_sprite);
        }

        float GetMidBlockRotation(float calculated_rot)
        {
            float attack_time_rot = 0.0f;
            if (!attack_timer.IsExpired())
            {
                attack_time_rot = (1.0f - (attack_timer.GetTime() - Time.GetElapsedTime()) / attack_timer.GetTimeLimit()) * attack_angle;
            }
            return calculated_rot - attack_angle / 2.0f + attack_time_rot;
        }

        float GetMidArmBlockRotation(float calculated_rot)
        {
            if (!attack_timer.IsExpired())
            {
                // Need the attack time to decrease as the attack hit box gets bigger
                float rot = GetMidBlockRotation(calculated_rot);
                float angle_pi = (float)Math.PI / 2.0f;
                //if (attack_number == 1)
                //{
                //    angle_pi = (float)Math.PI / 4.0f;
                //}
                //if (attack_number == 2)
                //{
                //    angle_pi = (float)Math.PI / 3.0f;
                //}
                float down_rot = rot - angle_pi;
                float attack_time_rot = down_rot + (1.0f - (attack_timer.GetTime() - Time.GetElapsedTime()) / attack_timer.GetTimeLimit()) * 2.0f * angle_pi;
                return attack_time_rot;
            }

            return calculated_rot;
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
                    Console.WriteLine("Right Clicked");
                    DestroyRope();
                    return;
                }

                Vector2 player_position = game_object.transform.GetPosition();
                Vector2 mouse_position = Input.GetMousePositionInWorld(camera);
                Vector2 distance = mouse_position - player_position;

                RaycastResult result = Physics.Raycast(player_position, distance.Normalize(),
                    PhysicCollisionCategory.AllCategories, UserCollisionCategories.FilterForPrincessBlockers, 0);

                if (result.intersected)
                {
                    GameObject rope = GameObject.CreateGameObject();
                    Sprite rope_sprite = rope.AddComponent<Sprite>();
                    Render.LoadTexture("../QRGameEngine/Textures/Rope.png", rope_sprite);

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
                    Console.WriteLine("Hooked GameObject is null");
                    DestroyRope();
                    return;
                }

                Vector2 player_position = game_object.transform.GetPosition();

                Vector2 rope_end_position_with_difference = current_rope_hooked_game_object.transform.GetPosition()
                - current_rope_end_positon - current_rope_difference_in_position;

                Vector2 distance = current_rope_end_positon - player_position + rope_end_position_with_difference;

                const float MINIMUM_DISTANCE = 0.8f;
                if (distance.Length() < MINIMUM_DISTANCE)
                {
                    Console.WriteLine("Distance is too small");
                    DestroyRope();
                    return;
                }

                RaycastResult raycast_result = Physics.Raycast(player_position, distance.Normalize(),
                    PhysicCollisionCategory.AllCategories, UserCollisionCategories.FilterForPrincessBlockersAndCharacters, 0);

                if (raycast_result.intersected)
                {
                    Vector2 distance_to_current_hooked_target = current_rope_hooked_game_object.transform.GetPosition() - player_position;
                    Vector2 distance_to_new_closest_target = raycast_result.intersected_game_object.transform.GetPosition() - player_position;

                    if (distance_to_new_closest_target.Length() < distance_to_current_hooked_target.Length())
                    {
                        Console.WriteLine("Object Blocks the path to the target");
                        DestroyRope();
                        return;
                    }
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

                boomerang.AddComponent<BoomerangScript>().SetInitialDirection(boomerang_initial_direction).SetOwner(game_object, damage_hit_box_game_object);

                boomerangs.Add(boomerang);
            }
        }

        void BowLogic(Vector2 arrow_direction)
        {
            if (Input.GetKeyDown(Key.Q))
            {
                bow_charge += bow_charge_increase * PhysicConstants.TIME_STEP;
                if (!bow_shot && bow_charge > bow_charge_max)
                {
                    GameObject arrow = GameObject.CreateGameObject();
                    arrow.AddComponent<Arrow>().InitArrow(game_object.transform.GetPosition(), arrow_direction, damage_hit_box_game_object);
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

        void RollLogic(Vector2 player_direction)
        {
            if (between_rolls_timer.IsExpired() && player_direction.Length() >= epsilion && inputBuffer.ConsumeBufferedInput(Key.LSHIFT))
            {
                roll_timer.Start();
                between_rolls_timer.Start();
                roll_direction = player_direction;
                sprite_anim_sprite.SetAnimationSpeed(roll_animation_speed);
                body.SetVelocity(player_direction.Normalize() * roll_speed);
            }

            if (!roll_timer.IsExpired())
            {
                if (!AnimationManager.IsAnimationPlaying(sprite_game_object, "Animations/KnightRoll.anim"))
                {
                    AnimationManager.LoadAnimation(sprite_game_object, "Animations/KnightRoll.anim");
                }
            }
            else
            {
                sprite_anim_sprite.SetAnimationSpeed(1.0f);
            }
        }

        public void PrincessStopFollowPlayer()
        {
            holding_princess = false;
            princess_script.KnightHoldingPrincess(holding_princess);
        }

        void CreateArm()
        {
            const float pixel_per_unit = 32.0f;
            Vector2 arm_offset_in_pixels = new Vector2(6.0f, 4.0f);
            const float z_mid_arm_block_offset = 0.9f;
            const float z_arm_offset = -0.01f;

            GameObject arm = GameObject.CreateGameObject();
            arm_sprite = arm.AddComponent<Sprite>();
            Render.LoadTexture("../QRGameEngine/Textures/SwordHand.png", arm_sprite);

            arm_sprite.PixelScale();
            mid_arm_block = GameObject.CreateGameObject();
            mid_arm_block.AddChild(arm);

            game_object.AddChild(mid_arm_block);

            mid_arm_block.transform.SetLocalZIndex(z_mid_arm_block_offset);
            arm.transform.SetLocalZIndex(z_arm_offset);

            arm.transform.SetLocalPosition(arm_offset_in_pixels / pixel_per_unit);
        }

        void ArmRotation(Vector2 mouse_dir)
        {
            Vector2 arm_mouse_dir = new Vector2(mouse_dir);
            if (mouse_dir.x < epsilion)
            {
                arm_mouse_dir.x *= -1.0f;
            }

            float arm_calculated_rot = Vector2.Angle(arm_mouse_dir, right_dir);
            mid_arm_block.transform.SetLocalRotation(GetMidArmBlockRotation(arm_calculated_rot));

            mid_arm_block.transform.FlipXLocally(mouse_dir.x < epsilion);
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
            if (Input.GetKeyPressed(Input.Key.LSHIFT))
            {
                inputBuffer.AddBufferedInput(Input.Key.LSHIFT, buffered_input_time);
            }
        }

        public class HitBoxPlayer : HitBoxAction
        {
            public const float BASE_DAMAGE = 10.0f;
            public const float BASE_KNOCKBACK = 4.3f;
            public const float BASE_STUN_EFFECT_TIME = 0.15f;

            public const float MEDIUM_DAMAGE = BASE_DAMAGE * 1.5f;
            public const float MEDIUM_KNOCKBACK = BASE_KNOCKBACK * 1.5f;
            public const float MEDIUM_STUN_EFFECT_TIME = BASE_STUN_EFFECT_TIME * 1.5f;

            public const float HIGH_DAMAGE = 40.0f;
            public const float HIGH_KNOCKBACK = 8.3f;
            public const float HIGH_STUN_EFFECT_TIME = 0.75f;

            float damage = BASE_DAMAGE;
            float knockback = BASE_KNOCKBACK;
            float stun_effect_time = BASE_STUN_EFFECT_TIME;

            public void SetDamage(float damage)
            {
                this.damage = damage;
            }

            public void ResetDamage()
            {
                damage = BASE_DAMAGE;
            }

            public void SetKnockback(float knockback)
            {
                this.knockback = knockback;
            }

            public void ResetKnockback()
            {
                knockback = BASE_KNOCKBACK;
            }

            public void SetStunEffectTime(float stun_effect_time)
            {
                this.stun_effect_time = stun_effect_time;
            }

            public void ResetStunEffectTime()
            {
                stun_effect_time = BASE_STUN_EFFECT_TIME;
            }

            public override void OnHit(GameObject hit_box_owner_game_object, ScriptingBehaviour hit_box_script, InteractiveCharacterBehaviour hit_object_script)
            {
                if (hit_object_script is Princess)
                {
                    return;
                }

                float rot = hit_box_script.GetGameOjbect().GetParent().transform.GetLocalRotation();
                Vector2 dir = new Vector2((float)Math.Cos(rot), (float)Math.Sin(rot));

                hit_object_script.Knockback(dir, knockback);
                //hit_object_script.TakeDamage(hit_box_script.GetGameOjbect(), damage);
                hit_object_script.TakeDamage(hit_box_owner_game_object, damage);
                hit_object_script.SetEffect(new Effects.StunEffect(stun_effect_time));
            }

            public override void OnHitAvoidGameObject(ScriptingBehaviour hit_box_script)
            {

            }
        }
    }
}
