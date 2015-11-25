using UnityEngine;

namespace HoloToolkit
{
    public class ComplexTagalong : AdvancedTagalong
    {
        [Range(3, 11), Tooltip("The number of rays to cast horizontally across the Tagalong.")]
        public int HorizontalRayCount = 3;
        [Range(3, 11), Tooltip("The number of rays to cast vertically across the Tagalong.")]
        public int VerticalRayCount = 3;
        [Tooltip("Don't allow the Tagalong to come closer than this distance.")]
        public float MinimumTagalongDistance = 1.0f;
        [Tooltip("If true, the Tagalong object maintains a fixed angular size.")]
        public bool MaintainFixedSize = true;

        [Tooltip("Useful for visualizing the Raycasts used for determining the depth to place the Tagalong. Set to 'None' to disable.")]
        public Light DebugPointLight;

        [Tooltip("The speed to update the Tagalong's distance when compensating for depth (meters/second).")]
        public float DistanceUpdateSpeed = 4.0f;

        private float defaultTagalongDistance;

        protected override void Start()
        {
            // Remember the default for distance.
            defaultTagalongDistance = TagalongDistance;

            // If the specified minumum distance for the tagalong would be within the 
            // camera's near clipping plane, adjust it to be 10% beyond the near
            // clipping plane
            if (Camera.main.nearClipPlane > MinimumTagalongDistance)
            {
                MinimumTagalongDistance = Camera.main.nearClipPlane * 1.1f;
            }

            // The EnforceDistance functionality of the SimmpleTagalong would have
            // a detrimental effect on the ComplexTagalong's desired behavior.
            // Disable that flag here.
            EnforceDistance = false;

            // Add the FixedAngularSize script if MaintainFixedSize is true
            if (MaintainFixedSize)
            {
                gameObject.AddComponent<FixedAngularSize>();
            }

            base.Start();
        }

        protected override void Update()
        {
            base.Update();

            if (!interpolator.AnimatingPosition)
            {
                // If we aren't animating towards a new position, check to see if
                // we need to update the Tagalong's position because it is behind
                // some other hologram or the Spatial Mapping mesh.
                Vector3 newPosition;
                if (AdjustTagalongDistance(out newPosition))
                {
                    interpolator.PositionPerSecond = DistanceUpdateSpeed;
                    interpolator.SetTargetPosition(newPosition);
                    TagalongDistance = Mathf.Min(defaultTagalongDistance, Vector3.Distance(Camera.main.transform.position, newPosition));
                }
            }
        }

        private bool AdjustTagalongDistance(out Vector3 newPosition)
        {
            bool needsUpdating = false;

            // Get the actual width and height of the Tagalong's BoxCollider
            float width = tagalongCollider.size.x * transform.lossyScale.x;
            float height = tagalongCollider.size.y * transform.lossyScale.y;

            Vector3 cameraPosition = Camera.main.transform.position;

            // Find the lower-left corner of the Tagalong's BoxCollider.
            Vector3 lowerLeftCorner = transform.position - (transform.right * (width / 2)) - (transform.up * (height / 2));

            // Cast a grid of rays across the Tagalong's collider. Keep track of
            // of the closest hit, ignoring collisions with ourselves and those
            // that are closer than MinimumColliderDistance.
            RaycastHit closestHit = new RaycastHit();
            float closestHitDistance = float.PositiveInfinity;
            RaycastHit[] allHits;
            for (int x = 0; x < HorizontalRayCount; x++)
            {
                Vector3 xCoord = lowerLeftCorner + transform.right * (x * width / (HorizontalRayCount - 1));
                for (int y = 0; y < VerticalRayCount; y++)
                {
                    Vector3 targetCoord = xCoord + transform.up * (y * height / (VerticalRayCount - 1));

                    allHits = Physics.RaycastAll(cameraPosition, targetCoord - cameraPosition, defaultTagalongDistance * 1.5f);
                    for (int h = 0; h < allHits.Length; h++)
                    {
                        if (allHits[h].distance >= MinimumTagalongDistance &&
                            allHits[h].distance < closestHitDistance &&
                            !allHits[h].transform.IsChildOf(transform))
                        {
                            closestHit = allHits[h];
                            closestHitDistance = closestHit.distance;
                            if (DebugPointLight != null)
                            {
                                Light clonedLight = Instantiate(DebugPointLight, closestHit.point, Quaternion.identity) as Light;
                                clonedLight.color = Color.red;
                                DestroyObject(clonedLight, 1.0f);
                            }
#if UNITY_EDITOR
                            Debug_DrawLine(debug_drawLines, cameraPosition, targetCoord, Color.red);
#endif // UNITY_EDITOR
                        }
                    }
                }
            }

            // If we hit something, the closestHitDistance will be < infinity
            needsUpdating = closestHitDistance < float.PositiveInfinity;
            if (needsUpdating)
            {
                // The closestHitDistance is a straight-line from the camera to the
                // point on the collider that was hit. Unless the closest hit was
                // encountered on the center Raycast, using the distance found will
                // actually push the tagalong too far away, and part of the object
                // that was hit will show through the Tagalong. We can fix that
                // with a little thing we like to call Trigonometry.
                Vector3 cameraToTransformPosition = transform.position - cameraPosition;
                Vector3 cameraToClosestHitPoint = closestHit.point - cameraPosition;
                float angleBetween = Vector3.Angle(cameraToTransformPosition, cameraToClosestHitPoint);
                closestHitDistance = closestHitDistance * Mathf.Cos(angleBetween * Mathf.Deg2Rad);

                // Make sure we aren't trying to move too close.
                closestHitDistance = Mathf.Max(closestHitDistance, MinimumTagalongDistance);
            }
            else if (TagalongDistance != defaultTagalongDistance)
            {
                // If we didn't hit anything but the TagalongDistance is different
                // from the defaultTagalongDistance, we still need to update.
                needsUpdating = true;
                closestHitDistance = defaultTagalongDistance;
            }

            newPosition = cameraPosition + (transform.position - cameraPosition).normalized * closestHitDistance;
            return needsUpdating;
        }
    }
}