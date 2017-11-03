#include <robuffer.h>
#include <winrt\Windows.Storage.Streams.h>
#include "..\Pbr\GltfLoader.h"
#include "ControllerRendering.h"

using namespace std::placeholders;
using namespace DirectX;
using namespace Microsoft::WRL;
using namespace winrt::Windows::Storage::Streams;
using namespace winrt::Windows::UI::Input::Spatial;

namespace
{
    constexpr PCSTR TouchIndicatorNodeName = "TouchIndicator";
    constexpr PCSTR TouchIndicatorMaterialName = "TouchIndicator";

    // Decompose, interpolate each component, and then recompose.
    void InterpolateNode(const Pbr::Node& min, const Pbr::Node& max, float t, Pbr::Node& result)
    {
        XMVECTOR minScale, minRot, minTrans;
        XMVECTOR maxScale, maxRot, maxTrans;
        if (XMMatrixDecompose(&minScale, &minRot, &minTrans, min.GetTransform()) &&
            XMMatrixDecompose(&maxScale, &maxRot, &maxTrans, max.GetTransform()))
        {
            const XMMATRIX interpolatedMatrix =
                XMMatrixScalingFromVector(XMVectorLerp(minScale, maxScale, t)) *
                XMMatrixRotationQuaternion(XMQuaternionSlerp(minRot, maxRot, t)) *
                XMMatrixTranslationFromVector(XMVectorLerp(minTrans, maxTrans, t));
            result.SetTransform(interpolatedMatrix);
        }
    }

    void AddTouchpadTouchIndicator(Pbr::Model& controllerModel, _In_ ID3D11Device* device, _In_ ID3D11DeviceContext3* deviceContext, Pbr::Resources& pbrResources)
    {
        // Create a material for the touch indicator. Use emissive color so it is visible in all light conditions.
        std::shared_ptr<Pbr::Material> touchIndicatorMaterial = Pbr::Material::CreateFlat(
            device,
            pbrResources,
            TouchIndicatorMaterialName,
            Colors::Black /* base color */,
            0.5f /* roughness */,
            0.0f /* metallic */,
            Colors::DarkSlateBlue /* emissive */);
        touchIndicatorMaterial->Hidden = true;

        // Add a touch indicator primitive parented to the TOUCH node. This material will be hidden or visible based on the touch state.
        for (auto i = 0; i < controllerModel.GetNodeCount(); i++)
        {
            Pbr::Node& node = controllerModel.GetNode(i);
            if (node.Name == "TOUCH") // Add a touch indicator sphere to the TOUCH node.
            {
                // Create a new node parented to the TOUCH node. This node will positioned at the correct place.
                const Pbr::Node& touchIndicatorNode = controllerModel.AddNode(XMMatrixIdentity(), node.Index, TouchIndicatorNodeName);

                // Create a sphere primitive which is rooted on the TOUCH node.
                Pbr::PrimitiveBuilder touchSphere;
                touchSphere.CreateSphere(0.004f /* Diameter */, 5 /* Tessellation */, touchIndicatorNode.Index);

                controllerModel.AddPrimitive(Pbr::Primitive(device, touchSphere, touchIndicatorMaterial));
            }
        }
    }
}

namespace ControllerRendering
{
    ControllerModelKey GetControllerModelKey(SpatialInteractionSource const& source)
    {
        if (!source.Controller())
        {
            return {};
        }

        return std::make_tuple(source.Controller().ProductId(), source.Controller().VendorId(), source.Controller().Version(), source.Handedness());
    }

    std::future<std::shared_ptr<Pbr::Model>> ControllerRendering::LoadRenderModel(
        winrt::Windows::UI::Input::Spatial::SpatialInteractionSource const& source,
        _In_ ID3D11Device* device,
        _In_ ID3D11DeviceContext3* deviceContext,
        std::shared_ptr<Pbr::Resources> const& pbrResources)
    {
        IRandomAccessStreamWithContentType modelStream = co_await source.Controller().TryGetRenderableModelAsync();
        if (modelStream == nullptr) // Mesh data is optional. If not available, a null stream is returned.
        {
            co_return std::shared_ptr<Pbr::Model>();
        }

        // Gltf::FromGltfBinary is a synchronous operation and can be slow, so do it in a background context
        winrt::apartment_context originalContext;
        co_await winrt::resume_background();

        // Read all data out of the stream.
        DataReader dr(modelStream.GetInputStreamAt(0));
        const uint32_t readAmount = co_await dr.LoadAsync(static_cast<unsigned int>(modelStream.Size()));
        const IBuffer buffer = dr.DetachBuffer();

        uint8_t* rawBuffer;
        winrt::check_hresult(buffer.as<::Windows::Storage::Streams::IBufferByteAccess>()->Buffer(&rawBuffer));

        // Load the binary controller model data into a Pbr::Model
        const std::shared_ptr<Pbr::Model> model = Gltf::FromGltfBinary(device, *pbrResources, rawBuffer, buffer.Length());

        // Add a touchpad touch indicator node and primitive/material to the model.
        AddTouchpadTouchIndicator(*model, device, deviceContext, *pbrResources);

        // Switch back to the original context before returning the result.
        co_await originalContext;
        co_return model;
    }

    ArticulateValues GetArticulateValues(winrt::Windows::UI::Input::Spatial::SpatialInteractionSourceState const& sourceState)
    {
        ArticulateValues articulateValues;

        articulateValues.Grasp = sourceState.IsGrasped() ? 1.0f : 0.0f;
        articulateValues.Menu = sourceState.IsMenuPressed() ? 1.0f : 0.0f;
        articulateValues.Select = (float)sourceState.SelectPressedValue();
        articulateValues.ThumbstickPress = sourceState.ControllerProperties().IsThumbstickPressed() ? 1.0f : 0.0f;
        articulateValues.ThumbstickX = (float)(sourceState.ControllerProperties().ThumbstickX() / 2) + 0.5f;
        articulateValues.ThumbstickY = (float)(-sourceState.ControllerProperties().ThumbstickY() / 2) + 0.5f;
        articulateValues.TouchpadPress = sourceState.ControllerProperties().IsTouchpadPressed() ? 1.0f : 0.0f;

        // If the the touchpad is pressed, use the touch location to control touchpad tilting, otherwise keep it centered.
        articulateValues.TouchIndicatorVisisble = sourceState.ControllerProperties().IsTouchpadTouched();
        articulateValues.TouchpadPressX = articulateValues.TouchIndicatorVisisble ? 0.5f : (float)sourceState.ControllerProperties().TouchpadX();
        articulateValues.TouchpadPressY = articulateValues.TouchIndicatorVisisble ? 0.5f : -(float)sourceState.ControllerProperties().TouchpadY();
        articulateValues.TouchpadTouchX = (float)(sourceState.ControllerProperties().TouchpadX() / 2) + 0.5f;
        articulateValues.TouchpadTouchY = (float)(-sourceState.ControllerProperties().TouchpadY() / 2) + 0.5f;

        return articulateValues;
    }

    void ArticulateControllerModel(ArticulateValues const& articulateValues, Pbr::Model& model)
    {
        // Every articulatable node in the model has three children, two which define the extents of the motion and one (VALUE) which holds the interpolated value.
        // In some cases, there nodes are nested to create combined transformations, like the X and Y movements of the thumbstick.
        auto interpolateNode = [&](Pbr::Node& rootNode, PCSTR minName, PCSTR maxName, float t)
        {
            const std::optional<Pbr::NodeIndex_t> minChild = model.FindFirstNode(minName, rootNode.Index);
            const std::optional<Pbr::NodeIndex_t> maxChild = model.FindFirstNode(maxName, rootNode.Index);
            const std::optional<Pbr::NodeIndex_t> valueChild = model.FindFirstNode("VALUE", rootNode.Index);
            if (minChild && maxChild && valueChild)
            {
                InterpolateNode(model.GetNode(minChild.value()), model.GetNode(maxChild.value()), t, model.GetNode(valueChild.value()));
            }
        };

        for (uint32_t i = 0; i < model.GetNodeCount(); i++)
        {
            Pbr::Node& node = model.GetNode(i);
            if (node.Name == "GRASP") { interpolateNode(node, "UNPRESSED", "PRESSED", articulateValues.Grasp); }
            else if (node.Name == "MENU") { interpolateNode(node, "UNPRESSED", "PRESSED", articulateValues.Menu); }
            else if (node.Name == "SELECT") { interpolateNode(node, "UNPRESSED", "PRESSED", articulateValues.Select); }
            else if (node.Name == "THUMBSTICK_PRESS") { interpolateNode(node, "UNPRESSED", "PRESSED", articulateValues.ThumbstickPress); }
            else if (node.Name == "THUMBSTICK_X") { interpolateNode(node, "MIN", "MAX", articulateValues.ThumbstickX); }
            else if (node.Name == "THUMBSTICK_Y") { interpolateNode(node, "MIN", "MAX", articulateValues.ThumbstickY); }
            else if (node.Name == "TOUCHPAD_PRESS") { interpolateNode(node, "UNPRESSED", "PRESSED", articulateValues.TouchpadPress); }
            else if (node.Name == "TOUCHPAD_PRESS_X") { interpolateNode(node, "MIN", "MAX", articulateValues.TouchpadPressX); }
            else if (node.Name == "TOUCHPAD_PRESS_Y") { interpolateNode(node, "MIN", "MAX", articulateValues.TouchpadPressY); }
            else if (node.Name == "TOUCHPAD_TOUCH_X") { interpolateNode(node, "MIN", "MAX", articulateValues.TouchpadTouchX); }
            else if (node.Name == "TOUCHPAD_TOUCH_Y") { interpolateNode(node, "MIN", "MAX", articulateValues.TouchpadTouchY); }
        }

        // Show or hide touch indicator by showing or hiding the material exclusively used by the touch indicator.
        for (uint32_t i = 0; i < model.GetPrimitiveCount(); i++)
        {
            std::shared_ptr<Pbr::Material>& primitiveMaterial = model.GetPrimitive(i).GetMaterial();
            if (primitiveMaterial->Name == TouchIndicatorMaterialName) { primitiveMaterial->Hidden = !articulateValues.TouchIndicatorVisisble; }
        }
    }
}