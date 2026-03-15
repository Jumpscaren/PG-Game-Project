using ScriptProject.Engine;
using ScriptProject.Engine.Constants;
using ScriptProject.EngineFramework;
using ScriptProject.EngineMath;
using ScriptProject.Scripts.Effects;
using ScriptProject.Scripts.Modules;
using ScriptProject.UserDefined;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Security.Policy;
using System.Text;
using System.Threading.Tasks;
using static ScriptProject.Scripts.OrcEnemy;
using static ScriptProject.Scripts.Player;

namespace ScriptProject.Scripts
{
    internal class OrcMonkey : InteractiveCharacterBehaviour
    {
        GameObject player_game_object;
        PathFindingActor actor;
        Transform transform;
        Vector2 last_position;
        DynamicBody body;
        Sprite sprite;

        float health = 100.0f;

        GameObject sprite_game_object;

        const float WAIT_ATTACK_TIME = 3.0f;
        const float BETWEEN_ATTACK_TIME = 1.0f;
        Timer attack_timer = new Timer(WAIT_ATTACK_TIME);
        const float ATTACK_RADIUS = 10.0f;

        bool attack = true;
        bool charge_attack = true;
        int attack_count = 0;
        const int MAX_ATTACK_COUNT = 2;

        float max_speed = 2.0f;
        const float drag_speed = 20.0f;

        const float calculate_path_time = 0.5f;
        Timer calculate_path_timer = new Timer(calculate_path_time);

        RandomGenerator random_generator = new RandomGenerator();

        GameObject target = null;

        bool dead = false;
        bool falling = false;
        float falling_speed = 1.0f;

        HoleManager holes = new HoleManager();

        GameObject shadow_game_object;
        Vector2 shadow_offset = new Vector2(0.0f, 0.0f);
        Vector2 base_shadow_scale;
        Sprite shadow_sprite;
        const float SHADOW_ALPHA = 0.5f;

        GameObject inbetween_game_object;
        AnimatableSprite inbetween_anim_sprite;
        Sprite inbetween_sprite;

        enum JumpState { None, Jump, Above, Fall };
        JumpState jump_state = JumpState.None;
        const float IN_AIR_TIME = 0.6f;
        const float IN_ABOVE_TIME = 0.2f;
        Timer in_air_timer = new Timer(IN_AIR_TIME);
        Timer above_timer = new Timer(IN_ABOVE_TIME);
        const float MIN_JUMP_SPEED = 3.0f / IN_AIR_TIME;

        GameObject impact_dust_game_object;
        AnimatableSprite impact_dust_anim_sprite;

        GameObject hit_box_game_object;
        StaticBody hit_box_body;
        float hit_box_radius = 2.25f;
        Timer hit_box_timer = new Timer(0.2f);

        EffectHolderModule effect_holder = new EffectHolderModule();

        void Start()
        {
            player_game_object = GameObject.TempFindGameObject("Player");
            actor = game_object.GetComponent<PathFindingActor>();
            actor.SetShowPath(true);
            transform = game_object.transform;
            body = game_object.GetComponent<DynamicBody>();

            base_shadow_scale = game_object.transform.GetScale();

            sprite_game_object = GameObject.CreateGameObject();
            sprite_game_object.transform.SetScale(transform.GetScale());
            //sprite_game_object.transform.SetLocalZIndex(player_game_object.transform.GetZIndex() + 0.1f);
            transform.SetScale(new Vector2(1.0f, 1.0f));
            sprite = sprite_game_object.AddComponent<Sprite>();
            sprite.SetTexture(game_object.GetComponent<Sprite>().GetTexture());
            game_object.RemoveComponent<Sprite>();
            game_object.transform.SetZIndex(0);

            inbetween_game_object = GameObject.CreateGameObject();
            inbetween_anim_sprite = inbetween_game_object.AddComponent<AnimatableSprite>();
            inbetween_sprite = inbetween_game_object.AddComponent<Sprite>();
            inbetween_sprite.SetShow(false);
            inbetween_game_object.transform.SetLocalZIndex(0);

            inbetween_game_object.AddChild(sprite_game_object);
            game_object.AddChild(inbetween_game_object);

            max_speed += random_generator.RandomFloat(-0.3f, 0.3f);

            game_object.SetName("Orc");

            target = player_game_object;

            last_position = transform.GetPosition();

            shadow_game_object = GameObject.CreateGameObject();
            shadow_game_object.transform.SetLocalZIndex(sprite_game_object.transform.GetZIndex() + 0.1f);
            shadow_sprite = shadow_game_object.AddComponent<Sprite>();
            Render.LoadTexture("../QRGameEngine/Textures/Shadow.png", shadow_sprite);

            shadow_offset.y = -game_object.transform.GetScale().y / 2.0f;

            hit_box_game_object = GameObject.CreateGameObject();
            hit_box_body = hit_box_game_object.AddComponent<StaticBody>();
            hit_box_body.SetEnabled(false);
            CircleCollider circle_collider = hit_box_game_object.AddComponent<CircleCollider>();
            circle_collider.SetTrigger(true);
            circle_collider.SetRadius(hit_box_radius);
            attack_timer.Start();

            HitBox hit_box_script = hit_box_game_object.AddComponent<HitBox>();
            hit_box_script.SetHitBoxAction(new HitBoxOrcMonkey(), game_object);
            hit_box_script.SetAvoidGameObject(game_object);

            game_object.AddChild(hit_box_game_object);

            impact_dust_game_object = GameObject.CreateGameObject();
            impact_dust_game_object.transform.SetScale(new Vector2(4.0f, 4.0f));
            impact_dust_game_object.AddComponent<Sprite>();
            impact_dust_anim_sprite = impact_dust_game_object.AddComponent<AnimatableSprite>();
            impact_dust_anim_sprite.SetAnimationSpeed(1.5f);
        }

        void FixedUpdate()
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
                Jump();
            }
            ShadowLogic();

            sprite.SetAddativeColor(inbetween_sprite.GetAddativeColor());
        }

        void Remove()
        {
            GameObject.DeleteGameObject(shadow_game_object);
        }

        public override void SetEffect(Effect effect)
        {
            effect_holder.SetEffect(this, effect);
        }

        public override void TakeDamage(GameObject hit_object, float damage)
        {
            if (jump_state != JumpState.None)
            {
                return;
            }

            health -= damage;
            if (health <= 0.0f)
            {
                dead = true;
            }

            AnimationManager.LoadAnimation(game_object, "Animations/HurtTest.anim");
        }

        public override void Knockback(Vector2 dir, float knockback)
        {
            if (jump_state != JumpState.None)
            {
                return;
            }

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

        void Look()
        {
            if (!effect_holder.IsEffectOver() && effect_holder.IsEffect<StunEffect>())
            {
                return;
            }

            Vector2 player_position = target.transform.GetPosition();
            Vector2 player_dir = (player_position - game_object.transform.GetPosition()).Normalize();
            sprite.FlipX(player_dir.x < 0);
        }

        void Move()
        {
            if (jump_state == JumpState.None)
            {
                Walk();
            }
            else if (jump_state == JumpState.Above)
            {
                body.SetVelocity(Vector2.Zero);
            }
            else
            {
                InJump();
            }
        }

        void Walk()
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

            if (target_dir.Length() < 1.01f)
            {
                dir = new Vector2();
            }

            float speed = max_speed;
            Vector2 new_velocity = dir.Normalize() * speed;

            if (!hit_box_timer.IsExpired())
            {
                speed = 0.0f;
            }
            CharacterMovementModule.FixedMovement(new_velocity, speed, drag_speed, body, effect_holder);
        }

        void InJump()
        {
            const float MAX_ALTER_SPEED = 6.0f;
            const float CLOSE_TO_TARGET = 1.0f;
            const float EPSILON = 0.01f;

            Vector2 current_position = transform.GetPosition();

            Vector2 target_velocity = target.GetComponent<DynamicBody>().GetVelocity();
            float change_velocity = 0.0f;
            if (target_velocity.Length() > EPSILON)
            {
                change_velocity = MAX_ALTER_SPEED / target_velocity.Length();
            }
            Vector2 altered_velocity = body.GetVelocity() + target_velocity * PhysicConstants.TIME_STEP * change_velocity;
            body.SetVelocity(altered_velocity);

            Vector2 distance_to_target = (target.transform.GetPosition() - current_position);
            if (distance_to_target.Length() < CLOSE_TO_TARGET)
            {
                body.SetVelocity(distance_to_target.Normalize() * MIN_JUMP_SPEED * distance_to_target.Length());
            }
        }

        void ShadowLogic()
        {
            Vector2 inbetween_scale = inbetween_game_object.transform.GetScale();

            shadow_game_object.transform.SetPosition(transform.GetPosition() + shadow_offset);
            shadow_game_object.transform.SetScale(base_shadow_scale * inbetween_scale.x);

            Vector4 addative_color = shadow_sprite.GetAddativeColor();
            addative_color.a = SHADOW_ALPHA / inbetween_scale.x;
            shadow_game_object.GetComponent<Sprite>().SetAddativeColor(addative_color);
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
                scale.x -= falling_speed * Time.GetFixedDeltaTime();
                scale.y -= falling_speed * Time.GetFixedDeltaTime();
                rotation += falling_speed * Time.GetFixedDeltaTime();
                falling_speed += 1.5f * Time.GetFixedDeltaTime();
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
            if (!effect_holder.IsEffectOver() && effect_holder.IsEffect<StunEffect>())
            {
                return;
            }

            float distance_to_player = (game_object.transform.GetPosition() - target.transform.GetPosition()).Length();
            if (distance_to_player < ATTACK_RADIUS)
            {
                if (!charge_attack && attack_timer.IsExpired())
                {
                    attack_timer.Start();
                    charge_attack = true;
                }
            }
            else
            {
                attack_timer.Start();
                charge_attack = false;
            }

            if (charge_attack && attack_timer.IsExpired())
            {
                attack = true;
                charge_attack = false;
                attack_count++;

                if (attack_count == MAX_ATTACK_COUNT)
                {
                    attack_timer.SetTimeLimit(WAIT_ATTACK_TIME);
                    attack_count = 0;
                }
                else
                {
                    attack_timer.SetTimeLimit(BETWEEN_ATTACK_TIME);
                }
            }
            
        }

        void Jump()
        {
            if (!effect_holder.IsEffectOver() && effect_holder.IsEffect<StunEffect>())
            {
                return;
            }

            if (jump_state == JumpState.Jump && in_air_timer.IsExpired())
            {
                jump_state = JumpState.Above;
                above_timer.Start();
            }
            if (jump_state == JumpState.Above && above_timer.IsExpired())
            {
                AnimationManager.LoadAnimation(inbetween_game_object, "Animations/MonkeyOrcFall.anim");
                jump_state = JumpState.Fall;
                body.SetVelocity(Vector2.Zero);
            }
            if (jump_state == JumpState.Fall && !AnimationManager.IsAnimationPlaying(inbetween_game_object, "Animations/MonkeyOrcFall.anim"))
            {
                hit_box_timer.Start();
                hit_box_body.SetEnabled(true);
                jump_state = JumpState.None;

                impact_dust_game_object.transform.SetPosition(game_object.transform.GetPosition());
                AnimationManager.LoadAnimation(impact_dust_game_object, "Animations/JumpEffect.anim");
            }
            if (jump_state == JumpState.None && attack)
            {
                AnimationManager.LoadAnimation(inbetween_game_object, "Animations/MonkeyOrcJump.anim");

                Vector2 distance_to_player = target.transform.GetPosition() - game_object.transform.GetPosition() + target.GetComponent<DynamicBody>().GetVelocity() * PhysicConstants.TIME_STEP;
                float speed = distance_to_player.Length() / in_air_timer.GetTimeLimit();
                if (speed < MIN_JUMP_SPEED)
                {
                    speed = MIN_JUMP_SPEED;
                }
                body.SetVelocity(distance_to_player.Normalize() * speed);

                attack = false;
                jump_state = JumpState.Jump;
                in_air_timer.Start();
                sprite_game_object.transform.SetLocalZIndex(0.1f);
            }
            if (jump_state == JumpState.None && !AnimationManager.IsAnimationPlaying(inbetween_game_object, "Animations/MonkeyOrcFall.anim"))
            {
                sprite_game_object.transform.SetLocalZIndex(1.0f);
            }

            if (hit_box_timer.IsExpired() && hit_box_body.GetEnabled())
            {
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
            game_object.RemoveComponent<CircleCollider>();
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

        public class HitBoxOrcMonkey : HitBoxAction
        {
            float damage = 20.0f;
            float knockback = 10.0f;

            public override void OnHit(GameObject hit_box_owner_game_object, ScriptingBehaviour hit_box_script, InteractiveCharacterBehaviour hit_object_script)
            {
                Vector2 dir = hit_object_script.GetGameOjbect().transform.GetPosition() - hit_box_owner_game_object.transform.GetPosition();

                float knockback_multiplier = 1.0f / dir.Length();
                if (knockback_multiplier < 0.67f)
                {
                    knockback_multiplier = 0.67f;
                }
                if (knockback_multiplier > 1.0f)
                {
                    knockback_multiplier = 1.0f;
                }

                hit_object_script.Knockback(dir.Normalize(), knockback * knockback_multiplier);
                hit_object_script.TakeDamage(hit_box_script.GetGameOjbect(), damage);
            }

            public override void OnHitAvoidGameObject(ScriptingBehaviour hit_box_script)
            {

            }
        }
    }
}
