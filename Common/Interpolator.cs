using UnityEngine;

namespace HoloToolkit
{
    public class Interpolator : MonoBehaviour
    {
        public const float SmallNumber = 0.0000001f;

        // The movement speed in meters per second
        [HideInInspector]
        public float PositionPerSecond = 30.0f;

        // The rotation speed, in degrees per second
        [HideInInspector]
        public float RotationDegreesPerSecond = 720.0f;

        // Adjusts rotation speed based on angular distance
        [HideInInspector]
        public float RotationSpeedScaler = 0.0f;

        // The amount to scale per second
        [HideInInspector]
        public float ScalePerSecond = 5.0f;

        // Lerp the estimated targets towards the object each update,
        // slowing and smoothing movement.
        [HideInInspector]
        public bool SmoothLerpToTarget = false;
        [HideInInspector]
        public float SmoothPositionLerpRatio = .5f;
        [HideInInspector]
        public float SmoothRotationLerpRatio = .5f;
        [HideInInspector]
        public float SmoothScaleLerpRatio = .5f;

        // Position data
        private Vector3 targetPosition;
        public bool AnimatingPosition { get; private set; }

        // Rotation data
        private Quaternion targetRotation;
        public bool AnimatingRotation { get; private set; }

        // Local Rotation data
        private Quaternion targetLocalRotation;
        public bool AnimatingLocalRotation { get; private set; }

        // Scale data
        private Vector3 targetLocalScale;
        public bool AnimatingLocalScale { get; private set; }

        public event System.Action InterpolationStarted;
        public event System.Action InterpolationDone;

        public Vector3 PositionVelocity { get; private set; }
        private Vector3 oldPosition = Vector3.zero;

        public bool Running
        {
            get
            {
                return (AnimatingPosition || AnimatingRotation || AnimatingLocalRotation || AnimatingLocalScale);
            }
        }

        public void Awake()
        {
            targetPosition = transform.position;
            targetRotation = transform.rotation;
            targetLocalRotation = transform.localRotation;
            targetLocalScale = transform.localScale;

            enabled = false;
        }

        public void SetTargetPosition(Vector3 target)
        {
            bool wasRunning = Running;

            targetPosition = target;

            float magsq = (targetPosition - transform.position).sqrMagnitude;
            if (magsq > SmallNumber)
            {
                AnimatingPosition = true;
                enabled = true;

                if (InterpolationStarted != null && !wasRunning)
                {
                    InterpolationStarted();
                }
            }
            else
            {
                // set immediately to prevent accumulation of error
                transform.position = target;
                AnimatingPosition = false;
            }
        }

        public void SetTargetRotation(Quaternion target)
        {
            bool wasRunning = Running;

            targetRotation = target;

            if (Quaternion.Dot(transform.rotation, target) < 1.0f)
            {
                AnimatingRotation = true;
                enabled = true;

                if (InterpolationStarted != null && !wasRunning)
                {
                    InterpolationStarted();
                }
            }
            else
            {
                // set immediately to prevent accumulation of error
                transform.rotation = target;
                AnimatingRotation = false;
            }
        }

        public void SetTargetLocalRotation(Quaternion target)
        {
            bool wasRunning = Running;

            targetLocalRotation = target;

            if (Quaternion.Dot(transform.localRotation, target) < 1.0f)
            {
                AnimatingLocalRotation = true;
                enabled = true;

                if (InterpolationStarted != null && !wasRunning)
                {
                    InterpolationStarted();
                }
            }
            else
            {
                // set immediately to prevent accumulation of error
                transform.localRotation = target;
                AnimatingLocalRotation = false;
            }

        }

        public void SetTargetLocalScale(Vector3 target)
        {
            bool wasRunning = Running;

            targetLocalScale = target;

            float magsq = (targetLocalScale - transform.localScale).sqrMagnitude;
            if (magsq > Mathf.Epsilon)
            {
                AnimatingLocalScale = true;
                enabled = true;

                if (InterpolationStarted != null && !wasRunning)
                {
                    InterpolationStarted();
                }
            }
            else
            {
                // set immediately to prevent accumulation of error
                transform.localScale = target;
                AnimatingLocalScale = false;
            }
        }

        /// <summary>
        /// Interpolates smoothly to a target position.
        /// </summary>
        /// <param name="start"></param>
        /// <param name="target"></param>
        /// <param name="deltaTime"></param>
        /// <param name="speed"></param>
        /// <returns>New interpolated position closer to target</returns>
        public static Vector3 NonLinearInterpolateTo(Vector3 start, Vector3 target, float deltaTime, float speed)
        {
            // If no interpolation speed, jump to target value.
            if (speed <= 0.0f)
            {
                return target;
            }

            Vector3 distance = (target - start);

            // When close enough, jump to the target
            if (distance.sqrMagnitude <= Mathf.Epsilon)
            {
                return target;
            }

            // Apply the delta, then clamp so we don't overshoot the target
            Vector3 deltaMove = distance * Mathf.Clamp(deltaTime * speed, 0.0f, 1.0f);

            return start + deltaMove;
        }

        public void Update()
        {
            bool interpOccuredThisFrame = false;

            if (AnimatingPosition)
            {
                Vector3 lerpTargetPosition = targetPosition;
                if (SmoothLerpToTarget)
                {
                    lerpTargetPosition = Vector3.Lerp(transform.position, lerpTargetPosition, SmoothPositionLerpRatio);
                }

                Vector3 newPosition = NonLinearInterpolateTo(transform.position, lerpTargetPosition, Time.deltaTime, PositionPerSecond);
                if ((targetPosition - newPosition).sqrMagnitude <= SmallNumber)
                {
                    // Snap to final position
                    newPosition = targetPosition;
                    AnimatingPosition = false;
                }
                else
                {
                    interpOccuredThisFrame = true;
                }

                transform.position = newPosition;

                //calculate interpolatedVelocity and store position for next frame
                PositionVelocity = oldPosition - newPosition;
                oldPosition = newPosition;
            }

            // Determine how far we need to rotate
            if (AnimatingRotation)
            {
                Quaternion lerpTargetRotation = targetRotation;
                if (SmoothLerpToTarget)
                {
                    lerpTargetRotation = Quaternion.Lerp(transform.rotation, lerpTargetRotation, SmoothRotationLerpRatio);
                }

                float angleDiff = Quaternion.Angle(transform.rotation, lerpTargetRotation);
                float speedScale = 1.0f + (Mathf.Pow(angleDiff, RotationSpeedScaler) / 180.0f);
                float ratio = Mathf.Clamp01((speedScale * RotationDegreesPerSecond * Time.deltaTime) / angleDiff);

                if (angleDiff < float.Epsilon)
                {
                    AnimatingRotation = false;
                    transform.rotation = targetRotation;
                }
                else
                {
                    // Only lerp rotation here, as ratio is NaN if angleDiff is 0.0f
                    transform.rotation = Quaternion.Slerp(transform.rotation, targetRotation, ratio);
                    interpOccuredThisFrame = true;
                }
            }

            // Determine how far we need to rotate
            if (AnimatingLocalRotation)
            {
                Quaternion lerpTargetLocalRotation = targetLocalRotation;
                if (SmoothLerpToTarget)
                {
                    lerpTargetLocalRotation = Quaternion.Lerp(transform.localRotation, lerpTargetLocalRotation, SmoothRotationLerpRatio);
                }

                float angleDiff = Quaternion.Angle(transform.localRotation, lerpTargetLocalRotation);
                float speedScale = 1.0f + (Mathf.Pow(angleDiff, RotationSpeedScaler) / 180.0f);
                float ratio = Mathf.Clamp01((speedScale * RotationDegreesPerSecond * Time.deltaTime) / angleDiff);

                if (angleDiff < float.Epsilon)
                {
                    AnimatingLocalRotation = false;
                    transform.localRotation = targetLocalRotation;
                }
                else
                {
                    // Only lerp rotation here, as ratio is NaN if angleDiff is 0.0f
                    transform.localRotation = Quaternion.Slerp(transform.localRotation, targetLocalRotation, ratio);
                    interpOccuredThisFrame = true;
                }
            }


            if (AnimatingLocalScale)
            {
                Vector3 lerpTargetLocalScale = targetLocalScale;
                if (SmoothLerpToTarget)
                {
                    lerpTargetLocalScale = Vector3.Lerp(transform.localScale, lerpTargetLocalScale, SmoothScaleLerpRatio);
                }

                Vector3 newScale = NonLinearInterpolateTo(transform.localScale, lerpTargetLocalScale, Time.deltaTime, ScalePerSecond);
                if ((targetLocalScale - newScale).sqrMagnitude <= SmallNumber)
                {
                    // Snap to final scale
                    newScale = targetLocalScale;
                    AnimatingLocalScale = false;
                }
                else
                {
                    interpOccuredThisFrame = true;
                }

                transform.localScale = newScale;
            }

            // If all interpolations have completed, stop updating
            if (!interpOccuredThisFrame)
            {
                if (InterpolationDone != null)
                {
                    InterpolationDone();
                }
                enabled = false;
            }
        }

        /// <summary>
        /// Snaps to the final target and stops interpolating
        /// </summary>
        public void SnapToTarget()
        {
            if (enabled)
            {
                transform.position = TargetPosition;
                transform.rotation = TargetRotation;
                transform.localRotation = TargetLocalRotation;
                transform.localScale = TargetLocalScale;

                AnimatingPosition = false;
                AnimatingLocalScale = false;
                AnimatingRotation = false;
                AnimatingLocalRotation = false;

                enabled = false;

                if (InterpolationDone != null)
                {
                    InterpolationDone();
                }
            }
        }

        /// <summary>
        /// Stops the interpolation regardless if it has reached the target
        /// </summary>
        public void StopInterpolating()
        {
            if (enabled)
            {
                Reset();

                if (InterpolationDone != null)
                {
                    InterpolationDone();
                }
            }
        }

        public void Reset()
        {
            targetPosition = transform.position;
            targetRotation = transform.rotation;
            targetLocalRotation = transform.localRotation;
            targetLocalScale = transform.localScale;

            AnimatingPosition = false;
            AnimatingRotation = false;
            AnimatingLocalRotation = false;
            AnimatingLocalScale = false;

            enabled = false;
        }

        public Vector3 TargetPosition
        {
            get
            {
                if (AnimatingPosition)
                {
                    return targetPosition;
                }
                return transform.position;
            }
        }

        public Quaternion TargetRotation
        {
            get
            {
                if (AnimatingRotation)
                {
                    return targetRotation;
                }
                return transform.rotation;
            }
        }

        public Quaternion TargetLocalRotation
        {
            get
            {
                if (AnimatingLocalRotation)
                {
                    return targetLocalRotation;
                }
                return transform.localRotation;
            }
        }

        public Vector3 TargetLocalScale
        {
            get
            {
                if (AnimatingLocalScale)
                {
                    return targetLocalScale;
                }
                return transform.localScale;
            }
        }
    }
}