# HoloToolkit
The HoloToolkit is a collection of scripts and components intended to accelerate the development of holographic applications targeting Windows Holographic---

---
## Common
Any classes that are referenced from multiple subdirectories within the HoloToolkit.

### Interpolator.cs
A MonoBehaviour that interpolates an transform's Position, Rotation or Scale.

---
## Gaze
Any classes that are based on the user's gaze.

### SimpleTagalong.cs
A Tagalong that stays at a fixed distance from the camera and always seeks to have a part of itself in the view frustum of the camera.

### AdvancedTagalong.cs
A Tagalong that extends SimpleTagalong that allows for specifying the minimum and target percentage of the object to keep in the view frustum of the camera.

### ComplexTagalong.cs
A Tagalong that extends AdvancedTagalong that keeps the Tagalong object in front of other Holograms including the Spatial Mapping Mesh.

### FixedAngularSize.cs
Causes a Hologram to maintain a fixed angular size, which is to say it occupies the same pixels in the view regardless of its distance from the camera.

### Billboard.cs
Causes a Hologram to rotate so it is always facing towards the camera.