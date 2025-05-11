using ScriptProject.Engine;
using ScriptProject.EngineMath;
using ScriptProject.UserDefined;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ScriptProject.Scripts
{
    internal class SwitchScript : ScriptingBehaviour
    {

        List<GameObject> replace_blocks;
        GameObject player;
        const float Max_Length_To_Player = 1.5f;
        bool switched = false;

        void Start()
        {
            replace_blocks = GameObject.FindGameObjectsWithTag(UserTags.Replace);
            player = GameObject.TempFindGameObject("Player");
        }

        void Update()
        {
            if (switched)
            {
                return;
            }

            float length_to_player = (game_object.transform.GetPosition() - player.transform.GetPosition()).Length();
            if (length_to_player < Max_Length_To_Player)
            {
                if (Input.GetKeyDown(Input.Key.E))
                {
                    switched = true;
                    Render.LoadTexture("../QRGameEngine/Textures/UglySwitchOn.png", game_object.GetComponent<Sprite>());

                    foreach (var replace in replace_blocks)
                    {
                        GameObject.DeleteGameObject(replace);
                        GameObject new_game_object = GameObject.CreateGameObject();
                        new_game_object.AddComponent<Sprite>();
                        new_game_object.transform.SetPosition(replace.transform.GetPosition());
                        new_game_object.transform.SetZIndex(2);
                        ReplaceBlockScript script = replace.GetComponent<ReplaceBlockScript>();
                        PrefabSystem.InstanceUserPrefab(new_game_object, script.replace_prefab_name);
                    }
                }
            }
        }
    }
}
