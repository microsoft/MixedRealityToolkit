//
// Copyright (C) Microsoft. All rights reserved.
//

using UnityEditor;

namespace UAudioTools
{
    [CustomEditor(typeof(UAudioMiniManager))]
    public class UAudioMiniManagerEditor : UAudioManagerBaseEditor<MiniAudioEvent>
    {
        private void OnEnable()
        {
            this.myTarget = (UAudioMiniManager)target;
            SetUpEditor();
        }

        public override void OnInspectorGUI()
        {
            DrawInspectorGUI(true);
        }
    }
}