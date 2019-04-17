using System.Collections.Generic;

namespace CalibrationPlugin.Editor
{
    public class ArUcoCorners
    {
        public int markerId { get; set; }
        public System.Numerics.Vector3 topLeft { get; set; }
        public System.Numerics.Vector3 topRight { get; set; }
        public System.Numerics.Vector3 bottomLeft { get; set; }
        public System.Numerics.Vector3 bottomRight { get; set; }

        internal static ArUcoCorners NativeToEditor(Native.ArUcoCorners native)
        {
            var editor = new ArUcoCorners();
            editor.markerId = native.markerId;
            editor.topLeft = native.topLeft;
            editor.topRight = native.topRight;
            editor.bottomRight = native.bottomRight;
            editor.bottomLeft = native.bottomLeft;
            return editor;
        }

        internal static Native.ArUcoCorners EditorToNative(ArUcoCorners editor)
        {
            var native = new Native.ArUcoCorners();
            native.markerId = editor.markerId;
            native.topLeft = editor.topLeft;
            native.topRight = editor.topRight;
            native.bottomRight = editor.bottomRight;
            native.bottomLeft = editor.bottomLeft;
            return native;
        }
    };

    public class UnityCameraOrientation
    {
        public System.Numerics.Vector3 position { get; set; }
        public System.Numerics.Quaternion rotation { get; set; }

        internal static UnityCameraOrientation NativeToEditor(Native.UnityCameraOrientation native)
        {
            var editor = new UnityCameraOrientation();
            editor.position = native.position;
            editor.rotation = native.rotation;
            return editor;
        }

        internal static Native.UnityCameraOrientation EditorToNative(UnityCameraOrientation editor)
        {
            var native = new Native.UnityCameraOrientation();
            native.position = editor.position;
            native.rotation = editor.rotation;
            return native;
        }
    };

    public class Intrinsics
    {
        public double reprojectionError { get; set; }
        public System.Numerics.Vector2 focalLength { get; set; }
        public System.Numerics.Vector2 principalPoint { get; set; }
        public System.Numerics.Vector3 radialDistortion { get; set; }
        public System.Numerics.Vector2 tangentialDistortion { get; set; }

        internal static Intrinsics NativeToEditor(Native.Intrinsics native)
        {
            var editor = new Intrinsics();
            editor.reprojectionError = native.reprojectionError;
            editor.focalLength = native.focalLength;
            editor.principalPoint = native.principalPoint;
            editor.radialDistortion = native.radialDistortion;
            editor.tangentialDistortion = native.tangentialDistortion;
            return editor;
        }
    };

    public class MultiIntrinsics
    {
        public Intrinsics Standard { get; set; }
        public Intrinsics PreCalcMatrix { get; set; }
        public Intrinsics StandardPositionCorrected { get; set; }
        public Intrinsics PreCalcPositionCorrected { get; set; }

        internal static MultiIntrinsics NativeToEditor(Native.MultiIntrinsics native)
        {
            var editor = new MultiIntrinsics();
            editor.Standard = Intrinsics.NativeToEditor(native.Standard);
            editor.PreCalcMatrix = Intrinsics.NativeToEditor(native.PreCalcMatrix);
            editor.StandardPositionCorrected = Intrinsics.NativeToEditor(native.StandardPositionCorrected);
            editor.PreCalcPositionCorrected = Intrinsics.NativeToEditor(native.PreCalcPositionCorrected);
            return editor;
        }
    };

    public class Calibration
    {
        private Native.Calibration calibration;

        public Calibration()
        {
            calibration = new Native.Calibration();
        }

        public bool ProcessImage(byte[] image, int imageWidth, int imageHeight, ArUcoCorners[] worldCorners, UnityCameraOrientation orientation)
        {
            var nativeCorners = new Native.ArUcoCorners[worldCorners.Length];
            for (int i = 0; i < worldCorners.Length; i++)
            {
                nativeCorners[i] = ArUcoCorners.EditorToNative(worldCorners[i]);
            }

            return calibration.ProcessImage(image, imageWidth, imageHeight, nativeCorners, UnityCameraOrientation.EditorToNative(orientation));
        }

        public MultiIntrinsics ProcessIntrinsics()
        {
            return MultiIntrinsics.NativeToEditor(calibration.ProcessIntrinsics());
        }
    }
}
