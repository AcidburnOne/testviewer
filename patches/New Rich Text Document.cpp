diff --git a/VULKAN_BOOTSTRAP_NOTES.md b/VULKAN_BOOTSTRAP_NOTES.md
new file mode 100644
index 0000000000..289299f34a
--- /dev/null
+++ b/VULKAN_BOOTSTRAP_NOTES.md
@@ -0,0 +1,73 @@
+# Firestorm Vulkan bootstrap 0.11
+
+Base: official Firestorm `Firestorm_Release_7.2.4.80712`.
+
+This source milestone includes the renderer-backend selection boundary,
+case-insensitive `OpenGL`/`Vulkan` parsing, an opt-in
+`FIRESTORM_RENDER_BACKEND` environment variable, and guaranteed fallback to
+OpenGL when Vulkan is not compiled or unavailable. It also dynamically probes
+`vulkan-1.dll` on Windows (`libvulkan.so.1` on Linux), records the loader API
+version, and emits startup diagnostics without requiring Vulkan SDK headers.
+
+When configured with `-DFIRESTORM_VULKAN_EXPERIMENTAL=ON`, milestone 0.3 uses
+the Vulkan 1.2 SDK to create a real instance and logical device, selects a
+graphics queue, prefers a discrete GPU, and logs the selected adapter. The
+default option remains OFF so existing Firestorm builds are unaffected.
+
+On Windows, milestone 0.4 enables the required surface and swapchain
+extensions, attaches to Firestorm's existing `HWND`, verifies that the graphics
+queue can present, chooses an sRGB surface format and FIFO present mode, and
+creates a swapchain using the surface's supported extent and image count.
+
+Milestone 0.5 creates an image view and framebuffer for every swapchain image,
+a clear/store render pass, a resettable graphics command pool, primary command
+buffers, and two frames of acquire/render semaphores plus signaled fences. The
+resize callback safely waits for the device and rebuilds the presentation
+resources when the Win32 client area changes.
+
+Milestone 0.6 adds a complete validation-frame transaction: wait for the frame
+fence, acquire an image, record a clear render pass, submit with semaphore
+dependencies, and present. It distinguishes success, unavailable state,
+out-of-date/suboptimal swapchains, and fatal errors. Validation presentation is
+off by default and requires `FIRESTORM_VULKAN_VALIDATE_PRESENT=1`.
+
+Milestone 0.7 adds reproducible GLSL sources and embedded SPIR-V modules for a
+vertex/fragment pair, a dynamic-viewport graphics pipeline, and the first
+Vulkan draw call. The opt-in validation frame now displays a colored triangle
+without vertex buffers by using `gl_VertexIndex`.
+
+Milestone 0.8 defines a real interleaved position/color vertex layout, selects
+compatible host-visible and device-local memory types, uploads vertex and
+16-bit index data through temporary staging buffers, and issues the first
+`vkCmdDrawIndexed` call from GPU-local buffers.
+
+Milestone 0.9 extracts allocation, mapping, ownership and staged device-local
+uploads into the reusable RAII `LLVulkanBuffer` class. It also introduces
+`LLRenderMeshSource`, a backend-neutral owned snapshot of Firestorm's planar
+`LLVertexBuffer` attributes and 16/32-bit index data. Production draw calls are
+not redirected yet.
+
+Milestone 0.10 gives each `LLVertexBuffer` a stable backend identity and data
+revision, adds a bounded revision-aware Vulkan GPU mesh cache, and observes
+indexed and non-indexed production draws when
+`FIRESTORM_VULKAN_CAPTURE_DRAWS=1`. Cache hits avoid repeat uploads; mutations
+replace stale resources. OpenGL still performs every production draw.
+
+Milestone 0.11 translates triangle, strip, fan, point and line modes into an
+explicit Vulkan topology model and records indexed/non-indexed ranges in an
+8,192-entry shadow command queue. OpenGL line loops are counted as unsupported
+instead of being mistranslated. Queue and unsupported-mode diagnostics are
+exposed for validation builds.
+
+It is **not yet an installable Vulkan viewer**. The next implementation step is
+pipeline variants and Firestorm render-state mapping.
+Shipping a
+renamed stock executable at this point would be misleading.
+
+Run the dependency-free boundary test from the repository root:
+
+```bash
+doc/vulkan/build-selection-test.sh
+```
+
+See `doc/vulkan/README.md` for the migration sequence and distribution rules.
diff --git a/doc/vulkan/README.md b/doc/vulkan/README.md
new file mode 100644
index 0000000000..e561e26e26
--- /dev/null
+++ b/doc/vulkan/README.md
@@ -0,0 +1,227 @@
+# Firestorm Vulkan backend
+
+This branch starts a side-by-side Vulkan backend for Firestorm 7.2.4. OpenGL
+remains the production backend and mandatory fallback.
+
+Set `FIRESTORM_RENDER_BACKEND=vulkan` to request Vulkan in experimental builds.
+Until the Vulkan device/swapchain implementation lands, selection deliberately
+falls back to OpenGL. This prevents an unfinished backend from producing a
+viewer that cannot start.
+
+## Milestones
+
+1. Backend selection and guaranteed OpenGL fallback (included).
+2. Dependency-free Vulkan loader/version probe and startup diagnostics (included).
+3. Vulkan SDK option, instance, GPU and graphics-queue selection (included).
+4. Win32 Vulkan surface, presentation verification and swapchain (included).
+5. Image views, render pass, framebuffers, commands, synchronization and resize recreation (included).
+6. Opt-in acquire/clear/submit/present validation frame (included).
+7. Embedded SPIR-V shaders and first graphics pipeline/triangle (included).
+8. Memory selection, staging uploads, device-local vertex/index buffers (included).
+9. Reusable RAII buffers and `LLVertexBuffer` CPU-data bridge (included).
+10. Revision-aware Vulkan mesh cache and opt-in draw observation (included).
+11. Primitive/range translation and bounded shadow command queue (included).
+12. Cacheable pipeline keys plus blend, depth, cull, polygon and topology state mapping (included).
+13. Lazy Vulkan pipeline variants and queued position/color draw execution (included).
+14. Per-draw camera transforms, push constants and swapchain depth attachments (included).
+15. Texture-binding capture, samplers, descriptor sets and fallback image (included).
+16. Thread-safe decoded texture mirroring and texture-coordinate shading (included).
+17. GPU mip generation, anisotropy, alpha cutoff and initial material flags (included).
+18. Normal transforms, basic directional lighting, sRGB decode and shader families (included).
+19. Diffuse/normal/specular descriptors, tangents and initial PBR mapping (included).
+20. Scalar PBR factors, occlusion/emissive maps and environment groundwork (included).
+21. Rigged attachment/object matrix-palette skinning (included).
+22. Classic baked-avatar body palette and packed-weight skinning (included).
+23. Above-water and underwater draw classification and initial shading (included).
+24. Initial EEP sky, cloud, sun, and moon environment shading (included).
+25. Shadow-caster classification and light-space draw capture (included).
+26. Vulkan shadow-map depth target and caster replay (included).
+27. Per-mesh shadow receivers and comparison-filtered sampling (included).
+28. Probe-enabled PBR classification and analytic IBL fallback (included).
+29. Native reflection/irradiance cube-map arrays and fallback sampling (included).
+30. Firestorm probe metadata, volume selection, and spatial blending (included).
+31. Live Firestorm probe-face mirroring and multi-layer cube arrays (included).
+32. UI composition, GPU culling, batching and dynamic resolution.
+
+Configure an SDK-enabled development build with
+`-DFIRESTORM_VULKAN_EXPERIMENTAL=ON`. The option requires Vulkan 1.2 headers
+and loader libraries and is OFF by default.
+
+For an SDK-enabled Windows build, request the experimental context with
+`FIRESTORM_RENDER_BACKEND=vulkan`. Set
+`FIRESTORM_VULKAN_VALIDATE_PRESENT=1` to additionally clear and present one
+dark-blue Vulkan frame with a colored triangle during startup. This validation present is intentionally
+opt-in because OpenGL remains Firestorm's active renderer.
+
+Set `FIRESTORM_VULKAN_CAPTURE_DRAWS=1` to observe production
+`LLVertexBuffer` draw calls and populate the bounded Vulkan mesh cache. OpenGL
+still executes each draw; this mode measures data compatibility without
+changing visible output.
+
+Captured draws now carry a deduplicated Vulkan pipeline key. Blend factors,
+depth testing/writes/comparison, back-face culling and primitive topology are
+translated from Firestorm's tracked render state. Polygon mode currently uses
+the safe fill default; direct legacy `glPolygonMode` call sites are diagnosed
+for conversion to a tracked state wrapper in a later integration pass.
+
+The validation render pass lazily creates bounded Vulkan pipeline variants and
+executes compatible queued meshes with Firestorm's planar position and color
+attribute layout. This is still a geometry bring-up path using validation
+shaders. Milestone 14 adds model-view-projection push constants,
+OpenGL-to-Vulkan clip conversion, and device-local depth attachments so queued
+geometry can be depth-tested in Firestorm camera space. Textures, materials,
+and shader parity are still required before this can replace a visible scene
+pass.
+
+Milestone 15 captures texture unit 0 identity, wrap mode, filtering and mip
+state on every shadow draw. Vulkan now owns a combined-image-sampler descriptor
+layout, bounded descriptor pool, sampler cache, and a staged device-local white
+fallback image. Unsupported, missing, or not-yet-decoded images resolve to that
+fallback rather than causing OpenGL readbacks or invalid Vulkan descriptors.
+
+Milestone 16 observes supported uncompressed `LLImageGL` uploads while decoded
+pixels are still available, converts luminance/RGB/RGBA inputs to RGBA8 on the
+CPU worker, and queues Vulkan work for the render thread. The bounded image
+cache uses staging copies and safe descriptor retargeting when texture content
+changes. Captured meshes now bind Firestorm's planar texture-coordinate stream,
+and the fragment shader multiplies the sampled diffuse texture by vertex color.
+Milestone 17 generates complete mip chains with Vulkan blits when the selected
+format supports linear filtering. Anisotropic samplers are enabled only when
+the GPU advertises the feature and are capped at 16x or the device limit.
+Firestorm shader state contributes alpha-mask cutoff, lighting, and sRGB flags
+to each captured draw; the fragment shader receives them through push
+constants.
+
+Milestone 18 binds Firestorm's planar normal stream and captures an
+inverse-transpose normal matrix per draw. Pipeline keys classify unlit,
+alpha-mask, lit, and lit-alpha shader families. The bring-up shader performs
+manual sRGB decode for mirrored UNORM textures and a basic directional-light
+calculation with ambient fill. This is intentionally a first lighting model;
+Firestorm's local lights, environment probes, PBR channels, and atmospheric
+lighting remain for the advanced-material passes.
+
+Milestone 19 resolves diffuse, normal, and specular or metallic-roughness
+textures from the currently bound Firestorm shader's actual sampler channels.
+The Vulkan pipeline layout exposes three compatible material descriptor sets,
+and captured geometry binds Firestorm's tangent stream. The shader constructs a
+tangent-space basis for normal maps and adds a first specular highlight model.
+GLTF draws are identified and interpret the material texture's green/blue
+channels as roughness/metallic inputs.
+
+Milestone 20 caches Firestorm's metallic, roughness, and emissive uniform
+updates on the active shader and captures them with each GLTF draw. Five Vulkan
+material sets now cover diffuse, normal, metallic-roughness/specular,
+occlusion, and emissive maps. Scalar factors are stored in the padding lanes of
+the normal-transform push constants, keeping the block at Vulkan's guaranteed
+128-byte minimum. The shader applies roughness/metallic factors, ambient
+occlusion, emissive contribution, and a small Fresnel environment term. Full
+reflection-probe sampling and Firestorm environment parity remain later work.
+
+Milestone 21 captures Firestorm's uploaded avatar/object `matrixPalette` by
+shader revision and mirrors it into bounded Vulkan storage-buffer descriptors.
+Rigged draws with both `weight4` and integer joint streams select a skinned
+pipeline family, bind those streams as Vulkan vertex attributes, and perform
+four-influence position, normal, and tangent skinning in the vertex shader.
+An identity palette keeps validation and non-rigged draws descriptor-complete.
+This establishes rigged-mesh deformation; specialized avatar faces, hair,
+deferred lighting, and complete avatar-material parity remain later work.
+
+Milestone 22 adds Firestorm's classic baked-avatar deformation path alongside
+rigged attachments. It captures the legacy 45-vector palette uploaded through
+`uniform4fv`, binds the scalar `weight` stream, and reproduces Firestorm's
+packed adjacent-joint interpolation for positions, normals, and tangents.
+Dedicated body and alpha-mask shader families use projection-only transforms
+because the legacy palette already outputs view-space vertices. Rigid eyeballs
+continue through the existing lit/material classification. Cloth wind,
+deferred avatar lighting, shadows, and impostor composition remain later work.
+
+Milestone 23 tags Firestorm's above-water and underwater shader programs and
+propagates that classification into dedicated Vulkan pipeline families. The
+initial fragment path samples the bound water normal map, perturbs the surface,
+and applies separate deep-water/reflection tints, Fresnel response, and opacity
+for cameras above and below the surface. This is the first environment pass;
+animated dual-normal blending, scene-color refraction, water fog, reflection
+probes, and sun shadows remain later parity work.
+
+Milestone 24 tags Firestorm's EEP sky dome, cloud layer, sun disc, and moon
+programs and assigns a dedicated Vulkan shader family to each. The initial sky
+path renders a horizon-to-zenith gradient, clouds resolve Firestorm's bound
+noise sampler and derive soft density, and the textured sun/moon paths apply
+separate warm and cool spectral tints while preserving alpha blending. Full
+EEP atmospheric uniforms, dual-texture transitions, stars, HDRI skies, cloud
+animation, and celestial depth ordering remain later parity work.
+
+Milestone 25 tags Firestorm's static, tree, rigged-object, classic-avatar,
+alpha-mask, alpha-blend, GLTF, and cube shadow programs. Captured casters retain
+the light-space model-view-projection matrix active during Firestorm's shadow
+pass and select static, alpha, skinned, or avatar Vulkan shadow families.
+Shadow-only records are explicitly isolated from swapchain color rendering,
+preventing depth casters from leaking into the visible validation pass. Vulkan
+depth images, cascade scheduling, alpha-tested depth shaders, and shadow
+sampling land in the following shadow-map milestone.
+
+Milestone 26 creates a device-local 2048x2048 sampled depth image, depth-only
+render pass, and framebuffer. Before swapchain color rendering, captured
+light-space casters are replayed through lazy depth-only pipeline variants with
+constant/slope depth bias. Opaque casters use vertex-only pipelines, while
+alpha shadow families keep texture sampling and cutoff discard. Rigged object
+and classic avatar casters bind their cached Vulkan skin palettes. Cascade
+selection, stabilization, receiver sampling, PCF filtering, and multiple local
+light shadow maps remain later work.
+
+Milestone 27 binds the shadow depth image through a hardware comparison
+sampler and supplies matching caster transforms to lit color draws as
+instance-rate matrix attributes. Double-buffered device-local transform data
+preserves the 128-byte push-constant guarantee and follows frame fences.
+Receivers project into shadow space and apply a bounded 3x3 PCF kernel with a
+small receiver bias and out-of-frustum rejection. An explicit render-pass
+dependency makes depth writes visible to fragment sampling. Cascades,
+cross-object receiver matching, temporal stabilization, and environment-probe
+integration remain later work.
+
+Milestone 28 propagates Firestorm's reflection-probe feature state into
+captured draws and dedicated probe-material pipeline keys. Probe-enabled PBR
+surfaces receive separate analytic irradiance and radiance terms: metallic
+suppresses diffuse environment light, Schlick Fresnel colors the specular
+response, and roughness filters the reflected sky approximation. This is a
+stable fallback and receiver-side bring-up path. Mirroring Firestorm's native
+cube-map arrays, probe-volume selection, box projection, spatial blending, and
+per-probe intensity data remain the next environment milestone.
+
+Milestone 29 allocates native cube-compatible Vulkan images for a 128px HDR
+radiance probe with eight roughness mip levels and a 32px irradiance probe.
+Both six-face arrays are device-local, initialized to safe environment fallback
+values, transitioned for shader reads, and exposed through cube-array
+descriptors with trilinear mip filtering. Probe-enabled PBR surfaces now sample
+native irradiance by normal direction and radiance by reflected direction with
+roughness-driven LOD. Importing Firestorm's generated faces and metadata,
+multiple array layers, local volume selection, box projection, and probe
+blending remain the next milestone.
+
+Milestone 30 mirrors Firestorm's packed probe metadata without reading back its
+OpenGL UBO. Box transforms, camera-space sphere centers/radii, irradiance and
+radiance scales, fade weights, cube indices, and priorities are queued into
+double-buffered Vulkan storage buffers. Per-draw model-view matrices extend the
+instance stream so fragments can evaluate probe volumes in the same view space.
+The PBR shader blends overlapping sphere and box influences and normalizes
+their weighted environment contribution.
+
+Milestone 31 expands both native probe images to 256 cube layers and adds a
+bounded live-face queue between Firestorm's OpenGL probe generator and Vulkan.
+After Firestorm completes face six, every radiance face and roughness mip, or
+each irradiance face, is read as RGBA16F and uploaded into the matching Vulkan
+cube index. Per-subresource barriers keep untouched probes shader-readable,
+and queued updates for the same face are coalesced. The PBR shader now samples
+the cube layer carried by Firestorm's packed probe metadata. This explicit
+readback is the portable correctness path; external-memory interop can replace
+it later without changing the renderer-facing queue contract.
+
+The embedded SPIR-V arrays are generated from the GLSL files in
+`indra/llrender/shaders/vulkan`. Install `@webgpu/glslang` locally and run
+`node doc/vulkan/generate-validation-shaders.js` when those shaders change.
+
+## Distribution
+
+Experimental builds must install beside official Firestorm and use a separate
+shader cache. They must not overwrite files in an official installation or
+identify themselves as an official Firestorm release.
diff --git a/doc/vulkan/build-selection-test.sh b/doc/vulkan/build-selection-test.sh
new file mode 100755
index 0000000000..e994489a81
--- /dev/null
+++ b/doc/vulkan/build-selection-test.sh
@@ -0,0 +1,30 @@
+#!/usr/bin/env bash
+set -euo pipefail
+
+root="$(cd "$(dirname "$0")/../.." && pwd)"
+tmp_base="${TMPDIR:-$root/.tmp}"
+mkdir -p "$tmp_base"
+tmp="$(mktemp -d "$tmp_base/firestorm-vulkan-test.XXXXXXXXXX")"
+trap 'rm -rf "$tmp"' EXIT
+
+# The production source includes the viewer precompiled header. Strip that one
+# include for this dependency-free boundary test.
+sed '/#include "linden_common.h"/d' \
+    "$root/indra/llrender/llrenderbackend.cpp" > "$tmp/llrenderbackend.cpp"
+sed -e '/#include "linden_common.h"/d' -e 's/#if LL_WINDOWS/#if 0/' \
+    -e 's/#elif LL_LINUX/#elif 1/' \
+    "$root/indra/llrender/llvulkanprobe.cpp" > "$tmp/llvulkanprobe.cpp"
+sed '/#include "linden_common.h"/d' \
+    "$root/indra/llrender/llvulkancontext.cpp" > "$tmp/llvulkancontext.cpp"
+
+"${CXX:-c++}" -std=c++17 -Wall -Wextra -Werror \
+    -I"$root/indra/llrender" \
+    "$tmp/llrenderbackend.cpp" \
+    "$tmp/llvulkanprobe.cpp" \
+    "$tmp/llvulkancontext.cpp" \
+    "$root/doc/vulkan/test_backend_selection.cpp" \
+    -ldl \
+    -o "$tmp/test_backend_selection"
+
+"$tmp/test_backend_selection"
+echo "renderer backend selection tests passed"
diff --git a/doc/vulkan/build-vulkan-context-test.sh b/doc/vulkan/build-vulkan-context-test.sh
new file mode 100755
index 0000000000..39ddd6d632
--- /dev/null
+++ b/doc/vulkan/build-vulkan-context-test.sh
@@ -0,0 +1,37 @@
+#!/usr/bin/env bash
+set -euo pipefail
+
+if [[ $# -ne 1 ]]; then
+    echo "usage: $0 /path/to/Vulkan-Headers/include" >&2
+    exit 2
+fi
+
+root="$(cd "$(dirname "$0")/../.." && pwd)"
+tmp_base="${TMPDIR:-$root/.tmp}"
+mkdir -p "$tmp_base"
+tmp="$(mktemp -d "$tmp_base/firestorm-vulkan-context.XXXXXXXXXX")"
+trap 'rm -rf "$tmp"' EXIT
+
+sed '/#include "linden_common.h"/d' \
+    "$root/indra/llrender/llvulkancontext.cpp" > "$tmp/llvulkancontext.cpp"
+sed '/#include "linden_common.h"/d' \
+    "$root/indra/llrender/llvulkanbuffer.cpp" > "$tmp/llvulkanbuffer.cpp"
+
+"${CXX:-c++}" -std=c++20 -Wall -Wextra -Werror \
+    -DFIRESTORM_VULKAN_EXPERIMENTAL=1 \
+    -I"$1" -I"$root/indra/llrender" \
+    "$tmp/llvulkancontext.cpp" \
+    "$tmp/llvulkanbuffer.cpp" \
+    "$root/doc/vulkan/test_vulkan_context.cpp" \
+    -Wl,-l:libvulkan.so.1 \
+    -o "$tmp/test_vulkan_context"
+
+set +e
+"$tmp/test_vulkan_context"
+status=$?
+set -e
+if [[ $status -eq 77 ]]; then
+    echo "Vulkan runtime test skipped: no usable GPU/ICD in this environment"
+    exit 0
+fi
+exit "$status"
diff --git a/doc/vulkan/generate-validation-shaders.js b/doc/vulkan/generate-validation-shaders.js
new file mode 100644
index 0000000000..103b949435
--- /dev/null
+++ b/doc/vulkan/generate-validation-shaders.js
@@ -0,0 +1,22 @@
+const fs = require('fs');
+const path = require('path');
+const glslang = require('@webgpu/glslang')();
+
+const root = path.resolve(__dirname, '../..');
+const shaderDir = path.join(root, 'indra/llrender/shaders/vulkan');
+const output = path.join(root, 'indra/llrender/llvulkanvalidationshaders.h');
+
+function compile(name, stage) {
+    const source = fs.readFileSync(path.join(shaderDir, name), 'utf8');
+    return glslang.compileGLSL(source, stage);
+}
+
+function array(name, words) {
+    const body = Array.from(words, word => `    0x${word.toString(16).padStart(8, '0')}u`).join(',\n');
+    return `inline constexpr std::uint32_t ${name}[] = {\n${body}\n};`;
+}
+
+const vertex = compile('validation.vert', 'vertex');
+const fragment = compile('validation.frag', 'fragment');
+const header = `// Generated SPIR-V for the Vulkan bootstrap validation pipeline.\n// Source GLSL lives in shaders/vulkan. Regenerate both arrays together.\n#ifndef LL_LLVULKANVALIDATIONSHADERS_H\n#define LL_LLVULKANVALIDATIONSHADERS_H\n\n#include <cstdint>\n\nnamespace LLVulkanValidationShaders\n{\n${array('validation_vertex_spirv', vertex)}\n\n${array('validation_fragment_spirv', fragment)}\n\n} // namespace LLVulkanValidationShaders\n\n#endif\n`;
+fs.writeFileSync(output, header);
diff --git a/doc/vulkan/test_backend_selection.cpp b/doc/vulkan/test_backend_selection.cpp
new file mode 100644
index 0000000000..66d14a7ecb
--- /dev/null
+++ b/doc/vulkan/test_backend_selection.cpp
@@ -0,0 +1,54 @@
+#include "llrenderbackend.h"
+#include "llvulkanprobe.h"
+#include "llvulkancontext.h"
+#include "llvulkanvalidationshaders.h"
+
+#include <cassert>
+#include <cstring>
+
+int main()
+{
+    assert(LLRenderBackendSelection::fromString("vulkan") == LLRenderBackend::VULKAN);
+    assert(LLRenderBackendSelection::fromString("VK") == LLRenderBackend::VULKAN);
+    assert(LLRenderBackendSelection::fromString("garbage") == LLRenderBackend::OPENGL);
+    assert(LLRenderBackendSelection::resolve(LLRenderBackend::VULKAN, false, true) == LLRenderBackend::OPENGL);
+    assert(LLRenderBackendSelection::resolve(LLRenderBackend::VULKAN, true, false) == LLRenderBackend::OPENGL);
+    assert(LLRenderBackendSelection::resolve(LLRenderBackend::VULKAN, true, true) == LLRenderBackend::VULKAN);
+    assert(std::strcmp(LLRenderBackendSelection::name(LLRenderBackend::OPENGL), "OpenGL") == 0);
+    const LLVulkanProbeResult probe = LLVulkanProbe::run();
+    assert(!probe.loader_available || probe.api_version != 0 || !probe.error.empty());
+    assert(!probe.versionString().empty());
+    // The dependency-free test intentionally compiles without the experimental
+    // SDK option and verifies the safe stub rather than creating a GPU device.
+    assert(!LLVulkanContext::instance().initialize());
+    assert(!LLVulkanContext::instance().ready());
+    assert(!LLVulkanContext::instance().error().empty());
+    assert(!LLVulkanContext::instance().attachWindow(nullptr, 1280, 720));
+    assert(!LLVulkanContext::instance().presentationReady());
+    assert(LLVulkanContext::instance().cachedMeshCount() == 0);
+    assert(LLVulkanContext::instance().queuedDrawCount() == 0);
+    assert(LLVulkanContext::instance().pipelineKeyCount() == 0);
+    assert(LLVulkanContext::instance().pipelineVariantCount() == 0);
+    assert(LLVulkanContext::instance().unsupportedDrawCount() == 0);
+    assert(LLVulkanContext::instance().unsupportedStateCount() == 0);
+    assert(LLVulkanContext::instance().executedDrawCount() == 0);
+    assert(LLVulkanContext::instance().descriptorCount() == 0);
+    assert(LLVulkanContext::instance().cachedTextureCount() == 0);
+    const unsigned char white_pixel[4] = {255, 255, 255, 255};
+    LLVulkanContext::instance().observeTextureUpload(
+        1, 1, 1, 4, white_pixel, sizeof(white_pixel), false);
+    assert(LLVulkanContext::instance().descriptorCount() == 0);
+    LLVulkanPipelineKey pipeline_a;
+    LLVulkanPipelineKey pipeline_b;
+    assert(pipeline_a == pipeline_b);
+    pipeline_b.blend_enabled = true;
+    assert(!(pipeline_a == pipeline_b));
+    LLVulkanContext::instance().clearQueuedDraws();
+    assert(LLVulkanContext::instance().drawValidationFrame(0.f, 0.f, 0.f) ==
+           LLVulkanFrameResult::NOT_READY);
+    assert(std::strcmp(LLVulkanContext::instance().frameResultName(
+        LLVulkanFrameResult::PRESENTED), "presented") == 0);
+    assert(LLVulkanValidationShaders::validation_vertex_spirv[0] == 0x07230203u);
+    assert(LLVulkanValidationShaders::validation_fragment_spirv[0] == 0x07230203u);
+    return 0;
+}
diff --git a/doc/vulkan/test_vulkan_context.cpp b/doc/vulkan/test_vulkan_context.cpp
new file mode 100644
index 0000000000..510291bf95
--- /dev/null
+++ b/doc/vulkan/test_vulkan_context.cpp
@@ -0,0 +1,18 @@
+#include "llvulkancontext.h"
+
+#include <iostream>
+
+int main()
+{
+    LLVulkanContext& context = LLVulkanContext::instance();
+    if (!context.initialize())
+    {
+        std::cerr << "Vulkan context initialization failed: " << context.error() << '\n';
+        // Automake-compatible skip code: useful for CI machines that have the
+        // loader and headers but no physical GPU/ICD.
+        return 77;
+    }
+    std::cout << "Vulkan device selected: " << context.deviceName() << '\n';
+    context.shutdown();
+    return 0;
+}
diff --git a/indra/CMakeLists.txt b/indra/CMakeLists.txt
index 8d8d5d6de9..ef09b73d96 100644
--- a/indra/CMakeLists.txt
+++ b/indra/CMakeLists.txt
@@ -22,6 +22,14 @@ set(ROOT_PROJECT_NAME "Firestorm" CACHE STRING
     "The root project/makefile/solution name. Defaults to Firestorm.")
 project(${ROOT_PROJECT_NAME})
 
+option(FIRESTORM_VULKAN_EXPERIMENTAL
+       "Build the incomplete experimental Vulkan renderer bootstrap" OFF)
+if (FIRESTORM_VULKAN_EXPERIMENTAL)
+  find_package(Vulkan 1.2 REQUIRED)
+  add_compile_definitions(FIRESTORM_VULKAN_EXPERIMENTAL=1)
+  message(STATUS "Building the experimental Vulkan bootstrap")
+endif()
+
 set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake"  "${CMAKE_CURRENT_BINARY_DIR}")
 
 include(conanbuildinfo OPTIONAL RESULT_VARIABLE USE_CONAN )
@@ -222,4 +230,3 @@ if (LL_TESTS)
   # individual apps can add themselves as dependencies
   add_subdirectory(${INTEGRATION_TESTS_PREFIX}integration_tests)
 endif (LL_TESTS)
-
diff --git a/indra/llrender/CMakeLists.txt b/indra/llrender/CMakeLists.txt
index 69f56434ed..366843f125 100644
--- a/indra/llrender/CMakeLists.txt
+++ b/indra/llrender/CMakeLists.txt
@@ -24,6 +24,11 @@ set(llrender_SOURCE_FILES
     llimagegl.cpp
     llpostprocess.cpp
     llrender.cpp
+    llrendermeshsource.cpp
+    llrenderbackend.cpp
+    llvulkanprobe.cpp
+    llvulkancontext.cpp
+    llvulkanbuffer.cpp
     llrender2dutils.cpp
     llrendernavprim.cpp
     llrendersphere.cpp
@@ -56,6 +61,14 @@ set(llrender_HEADER_FILES
     llimagegl.h
     llpostprocess.h
     llrender.h
+    llrendermeshsource.h
+    llrenderbackend.h
+    llvulkanprobe.h
+    llvulkancontext.h
+    llvulkanbuffer.h
+    llvulkanvalidationshaders.h
+    shaders/vulkan/validation.vert
+    shaders/vulkan/validation.frag
     llrender2dutils.h
     llrendernavprim.h
     llrendersphere.h
@@ -106,5 +119,9 @@ target_link_libraries(llrender
         ll::freetype
         OpenGL::GL
         OpenGL::GLU
+        ${CMAKE_DL_LIBS}
         )
 
+if (FIRESTORM_VULKAN_EXPERIMENTAL)
+  target_link_libraries(llrender Vulkan::Vulkan)
+endif()
diff --git a/indra/llrender/llgl.cpp b/indra/llrender/llgl.cpp
index 49905232a0..7c748f079b 100644
--- a/indra/llrender/llgl.cpp
+++ b/indra/llrender/llgl.cpp
@@ -2508,6 +2508,12 @@ void LLGLState::dumpStates()
     }
 }
 
+bool LLGLState::isEnabled(LLGLenum state)
+{
+    const auto found = sStateMap.find(state);
+    return found != sStateMap.end() && found->second != GL_FALSE;
+}
+
 void LLGLState::checkStates(GLboolean writeAlpha)
 {
     if (!gDebugGL)
@@ -3024,4 +3030,3 @@ extern "C"
 }
 #endif
 
-
diff --git a/indra/llrender/llgl.h b/indra/llrender/llgl.h
index 1d2399e995..d31cda0578 100644
--- a/indra/llrender/llgl.h
+++ b/indra/llrender/llgl.h
@@ -247,6 +247,7 @@ public:
     // what we expect
     //  writeAlpha - whether or not writing to alpha channel is expected
     static void checkStates(GLboolean writeAlpha = GL_TRUE);
+    static bool isEnabled(LLGLenum state);
 
 protected:
     static boost::unordered_map<LLGLenum, LLGLboolean> sStateMap;
diff --git a/indra/llrender/llglslshader.cpp b/indra/llrender/llglslshader.cpp
index a268ea07bb..a8551834d1 100644
--- a/indra/llrender/llglslshader.cpp
+++ b/indra/llrender/llglslshader.cpp
@@ -1317,6 +1317,10 @@ void LLGLSLShader::uniform1f(U32 index, GLfloat x)
 {
     LL_PROFILE_ZONE_SCOPED_CATEGORY_SHADER;
     llassert(sCurBoundShaderPtr == this);
+    if (index == LLShaderMgr::METALLIC_FACTOR)
+        mMetallicFactor = x;
+    else if (index == LLShaderMgr::ROUGHNESS_FACTOR)
+        mRoughnessFactor = x;
 
     if (mProgramObject)
     {
@@ -1543,6 +1547,8 @@ void LLGLSLShader::uniform3fv(U32 index, U32 count, const GLfloat* v)
 {
     LL_PROFILE_ZONE_SCOPED_CATEGORY_SHADER;
     llassert(sCurBoundShaderPtr == this);
+    if (index == LLShaderMgr::EMISSIVE_COLOR && count > 0 && v)
+        mEmissiveStrength = std::max(v[0], std::max(v[1], v[2]));
 
     if (mProgramObject)
     {
@@ -1570,6 +1576,12 @@ void LLGLSLShader::uniform4fv(U32 index, U32 count, const GLfloat* v)
 {
     LL_PROFILE_ZONE_SCOPED_CATEGORY_SHADER;
     llassert(sCurBoundShaderPtr == this);
+    if (index == LLShaderMgr::AVATAR_MATRIX && count > 0 && v)
+    {
+        mAvatarMatrixPalette.assign(v, v + static_cast<std::size_t>(count) * 4);
+        mLegacyAvatarMatrixPalette = true;
+        ++mAvatarMatrixPaletteRevision;
+    }
 
     if (mProgramObject)
     {
@@ -1668,6 +1680,12 @@ void LLGLSLShader::uniformMatrix3x4fv(U32 index, U32 count, GLboolean transpose,
 {
     LL_PROFILE_ZONE_SCOPED_CATEGORY_SHADER;
     llassert(sCurBoundShaderPtr == this);
+    if (index == LLShaderMgr::AVATAR_MATRIX && count > 0 && v)
+    {
+        mAvatarMatrixPalette.assign(v, v + static_cast<std::size_t>(count) * 12);
+        mLegacyAvatarMatrixPalette = false;
+        ++mAvatarMatrixPaletteRevision;
+    }
 
     if (mProgramObject)
     {
@@ -2025,6 +2043,7 @@ void LLGLSLShader::setMinimumAlpha(F32 minimum)
 {
     LL_PROFILE_ZONE_SCOPED_CATEGORY_SHADER;
     gGL.flush();
+    mMinimumAlpha = minimum;
     uniform1f(LLShaderMgr::MINIMUM_ALPHA, minimum);
 }
 
diff --git a/indra/llrender/llglslshader.h b/indra/llrender/llglslshader.h
index 272a99aaa5..39217f6296 100644
--- a/indra/llrender/llglslshader.h
+++ b/indra/llrender/llglslshader.h
@@ -60,6 +60,13 @@ public:
     bool hasHeroProbes = false;
     bool isPBRTerrain = false;
     bool hasTonemap = false;
+    bool isWater = false;
+    bool isUnderwater = false;
+    bool isSky = false;
+    bool isCloud = false;
+    bool isSun = false;
+    bool isMoon = false;
+    bool isShadow = false;
 };
 
 // ============= Structure for caching shader uniforms ===============
@@ -233,6 +240,13 @@ public:
     void uniformMatrix4fv(const LLStaticHashedString& uniform, U32 count, GLboolean transpose, const GLfloat* v);
 
     void setMinimumAlpha(F32 minimum);
+    F32 getMinimumAlpha() const { return mMinimumAlpha; }
+    F32 getMetallicFactor() const { return mMetallicFactor; }
+    F32 getRoughnessFactor() const { return mRoughnessFactor; }
+    F32 getEmissiveStrength() const { return mEmissiveStrength; }
+    const std::vector<F32>& getAvatarMatrixPalette() const { return mAvatarMatrixPalette; }
+    U64 getAvatarMatrixPaletteRevision() const { return mAvatarMatrixPaletteRevision; }
+    bool isLegacyAvatarMatrixPalette() const { return mLegacyAvatarMatrixPalette; }
 
     void vertexAttrib4f(U32 index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
     void vertexAttrib4fv(U32 index, GLfloat* v);
@@ -312,6 +326,13 @@ public:
     S32 mShaderLevel;
     S32 mShaderGroup; // see LLGLSLShader::eGroup
     bool mUniformsDirty;
+    F32 mMinimumAlpha = 0.f;
+    F32 mMetallicFactor = 1.f;
+    F32 mRoughnessFactor = 1.f;
+    F32 mEmissiveStrength = 0.f;
+    std::vector<F32> mAvatarMatrixPalette;
+    U64 mAvatarMatrixPaletteRevision = 0;
+    bool mLegacyAvatarMatrixPalette = false;
     LLShaderFeatures mFeatures;
     std::vector< std::pair< std::string, GLenum > > mShaderFiles;
     std::string mName;
diff --git a/indra/llrender/llglstates.h b/indra/llrender/llglstates.h
index 9bb980d7ad..67ac13455a 100644
--- a/indra/llrender/llglstates.h
+++ b/indra/llrender/llglstates.h
@@ -42,6 +42,10 @@ public:
 
     void checkState();
 
+    static bool depthEnabled() { return sDepthEnabled != GL_FALSE; }
+    static bool depthWriteEnabled() { return sWriteEnabled != GL_FALSE; }
+    static GLenum depthFunction() { return sDepthFunc; }
+
     GLboolean mPrevDepthEnabled;
     GLenum mPrevDepthFunc;
     GLboolean mPrevWriteEnabled;
diff --git a/indra/llrender/llimagegl.cpp b/indra/llrender/llimagegl.cpp
index 5ed1f3b687..fea05fb3ce 100644
--- a/indra/llrender/llimagegl.cpp
+++ b/indra/llrender/llimagegl.cpp
@@ -39,6 +39,8 @@
 #include "llgl.h"
 #include "llglslshader.h"
 #include "llrender.h"
+#include "llrenderbackend.h"
+#include "llvulkancontext.h"
 #include "llwindow.h"
 #include "llframetimer.h"
 #include <unordered_set>
@@ -751,6 +753,22 @@ bool LLImageGL::setImage(const U8* data_in, bool data_hasmips /* = false */, S32
 
     const bool is_compressed = isCompressed();
 
+#if FIRESTORM_VULKAN_EXPERIMENTAL && LL_WINDOWS
+    static const bool capture_vulkan_textures =
+        LLRenderBackendSelection::meshCaptureRequestedFromEnvironment();
+    if (capture_vulkan_textures && data_in && !data_hasmips && !is_compressed &&
+        mFormatType == GL_UNSIGNED_BYTE)
+    {
+        const std::uint64_t identity = usename ? usename : mTexName;
+        const std::uint32_t width = getWidth(mCurrentDiscardLevel);
+        const std::uint32_t height = getHeight(mCurrentDiscardLevel);
+        LLVulkanContext::instance().observeTextureUpload(
+            identity, width, height, mComponents, data_in,
+            static_cast<std::size_t>(width) * height * mComponents,
+            mUseMipMaps);
+    }
+#endif
+
     if (mUseMipMaps)
     {
         //set has mip maps to true before binding image so tex parameters get set properly
@@ -2660,4 +2678,3 @@ void LLImageGLThread::run()
     gGL.shutdown();
     mWindow->destroySharedContext(mContext);
 }
-
diff --git a/indra/llrender/llrender.cpp b/indra/llrender/llrender.cpp
index 25d16bf6b4..dbd545cdee 100644
--- a/indra/llrender/llrender.cpp
+++ b/indra/llrender/llrender.cpp
@@ -117,10 +117,12 @@ static const GLenum sGLBlendFactor[] =
 };
 
 LLTexUnit::LLTexUnit(S32 index)
-    : mCurrTexType(TT_NONE),
+    : mIndex(index),
     mCurrTexture(0),
+    mCurrTexType(TT_NONE),
     mHasMipMaps(false),
-    mIndex(index)
+    mAddressMode(TAM_WRAP),
+    mFilterOption(TFO_BILINEAR)
 {
     llassert_always(index < (S32)LL_NUM_TEXTURE_LAYERS);
 }
@@ -207,6 +209,8 @@ void LLTexUnit::bindFast(LLTexture* texture)
     }
     glBindTexture(sGLTextureType[gl_tex->getTarget()], mCurrTexture);
     mHasMipMaps = gl_tex->mHasMipMaps;
+    mAddressMode = gl_tex->mAddressMode;
+    mFilterOption = gl_tex->mFilterOption;
     if (gl_tex->mTexOptionsDirty)
     {
         gl_tex->mTexOptionsDirty = false;
@@ -229,6 +233,9 @@ bool LLTexUnit::bind(LLTexture* texture, bool for_rendering, bool forceBind)
         {
             if (gl_tex->getTexName()) //if texture exists
             {
+                mAddressMode = gl_tex->mAddressMode;
+                mFilterOption = gl_tex->mFilterOption;
+                mHasMipMaps = gl_tex->mHasMipMaps;
                 //in audit, replace the selected texture by the default one.
                 if ((mCurrTexture != gl_tex->getTexName()) || forceBind)
                 {
@@ -316,6 +323,8 @@ bool LLTexUnit::bind(LLImageGL* texture, bool for_rendering, bool forceBind, S32
         stop_glerror();
         texture->updateBindStats();
         mHasMipMaps = texture->mHasMipMaps;
+        mAddressMode = texture->mAddressMode;
+        mFilterOption = texture->mFilterOption;
         if (texture->mTexOptionsDirty)
         {
             stop_glerror();
@@ -352,6 +361,8 @@ bool LLTexUnit::bind(LLCubeMap* cubeMap)
             mCurrTexture = cubeMap->mImages[0]->getTexName();
             glBindTexture(GL_TEXTURE_CUBE_MAP, mCurrTexture);
             mHasMipMaps = cubeMap->mImages[0]->mHasMipMaps;
+            mAddressMode = cubeMap->mImages[0]->mAddressMode;
+            mFilterOption = cubeMap->mImages[0]->mFilterOption;
             cubeMap->mImages[0]->updateBindStats();
             if (cubeMap->mImages[0]->mTexOptionsDirty)
             {
@@ -472,6 +483,7 @@ void LLTexUnit::setTextureAddressMode(eTextureAddressMode mode)
 
 void LLTexUnit::setTextureAddressModeFast(eTextureAddressMode mode, eTextureType tex_type)
 {
+    mAddressMode = mode;
     glTexParameteri(sGLTextureType[tex_type], GL_TEXTURE_WRAP_S, sGLAddressMode[mode]);
     glTexParameteri(sGLTextureType[tex_type], GL_TEXTURE_WRAP_T, sGLAddressMode[mode]);
     if (tex_type == TT_CUBE_MAP || tex_type == TT_CUBE_MAP_ARRAY || tex_type == TT_TEXTURE_3D)
@@ -491,6 +503,7 @@ void LLTexUnit::setTextureFilteringOption(LLTexUnit::eTextureFilterOptions optio
 
 void LLTexUnit::setTextureFilteringOptionFast(LLTexUnit::eTextureFilterOptions option, eTextureType tex_type)
 {
+    mFilterOption = option;
     if (option == TFO_POINT)
     {
         glTexParameteri(sGLTextureType[tex_type], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
diff --git a/indra/llrender/llrender.h b/indra/llrender/llrender.h
index 27c2091f4b..c8a6bc15cb 100644
--- a/indra/llrender/llrender.h
+++ b/indra/llrender/llrender.h
@@ -223,6 +223,9 @@ public:
     U32 getCurrTexture(void) { return mCurrTexture; }
 
     eTextureType getCurrType(void) { return mCurrTexType; }
+    eTextureAddressMode getTextureAddressMode() const { return mAddressMode; }
+    eTextureFilterOptions getTextureFilterOption() const { return mFilterOption; }
+    bool hasMipMaps() const { return mHasMipMaps; }
 
     void setHasMipMaps(bool hasMips) { mHasMipMaps = hasMips; }
 
@@ -233,6 +236,8 @@ protected:
     U32                 mCurrTexture;
     eTextureType        mCurrTexType;
     bool                mHasMipMaps;
+    eTextureAddressMode mAddressMode;
+    eTextureFilterOptions mFilterOption;
 
     void debugTextureUnit(void);
     GLint getTextureSource(eTextureBlendSrc src);
@@ -471,6 +476,11 @@ public:
     void blendFunc(eBlendFactor color_sfactor, eBlendFactor color_dfactor,
                eBlendFactor alpha_sfactor, eBlendFactor alpha_dfactor);
 
+    eBlendFactor getBlendColorSource() const { return mCurrBlendColorSFactor; }
+    eBlendFactor getBlendColorDest() const { return mCurrBlendColorDFactor; }
+    eBlendFactor getBlendAlphaSource() const { return mCurrBlendAlphaSFactor; }
+    eBlendFactor getBlendAlphaDest() const { return mCurrBlendAlphaDFactor; }
+
     LLLightState* getLight(U32 index);
     void setAmbientLightColor(const LLColor4& color);
 
diff --git a/indra/llrender/llrenderbackend.cpp b/indra/llrender/llrenderbackend.cpp
new file mode 100644
index 0000000000..924e5f72a3
--- /dev/null
+++ b/indra/llrender/llrenderbackend.cpp
@@ -0,0 +1,76 @@
+/**
+ * @file llrenderbackend.cpp
+ * @brief Renderer backend selection boundary for OpenGL and Vulkan.
+ *
+ * $LicenseInfo:firstyear=2026&license=viewerlgpl$
+ * Copyright (C) 2026 Firestorm Vulkan contributors
+ *
+ * This library is free software; you can redistribute it and/or
+ * modify it under the terms of the GNU Lesser General Public License
+ * as published by the Free Software Foundation; version 2.1 only.
+ * $/LicenseInfo$
+ */
+
+#include "linden_common.h"
+#include "llrenderbackend.h"
+
+#include <algorithm>
+#include <cctype>
+#include <cstdlib>
+
+LLRenderBackend LLRenderBackendSelection::requestedFromEnvironment()
+{
+    const char* value = std::getenv("FIRESTORM_RENDER_BACKEND");
+    return value ? fromString(value) : LLRenderBackend::OPENGL;
+}
+
+bool LLRenderBackendSelection::validationPresentRequestedFromEnvironment()
+{
+    const char* value = std::getenv("FIRESTORM_VULKAN_VALIDATE_PRESENT");
+    if (!value)
+    {
+        return false;
+    }
+    std::string normalized(value);
+    std::transform(normalized.begin(), normalized.end(), normalized.begin(),
+                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
+    return normalized == "1" || normalized == "true" || normalized == "yes";
+}
+
+bool LLRenderBackendSelection::meshCaptureRequestedFromEnvironment()
+{
+    const char* value = std::getenv("FIRESTORM_VULKAN_CAPTURE_DRAWS");
+    if (!value)
+        return false;
+    std::string normalized(value);
+    std::transform(normalized.begin(), normalized.end(), normalized.begin(),
+                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
+    return normalized == "1" || normalized == "true" || normalized == "yes";
+}
+
+LLRenderBackend LLRenderBackendSelection::fromString(const std::string& value)
+{
+    std::string normalized(value);
+    std::transform(normalized.begin(), normalized.end(), normalized.begin(),
+                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
+    return normalized == "vulkan" || normalized == "vk"
+        ? LLRenderBackend::VULKAN
+        : LLRenderBackend::OPENGL;
+}
+
+const char* LLRenderBackendSelection::name(LLRenderBackend backend)
+{
+    return backend == LLRenderBackend::VULKAN ? "Vulkan" : "OpenGL";
+}
+
+LLRenderBackend LLRenderBackendSelection::resolve(LLRenderBackend requested,
+                                                   bool vulkan_compiled,
+                                                   bool vulkan_runtime_available)
+{
+    if (requested == LLRenderBackend::VULKAN &&
+        vulkan_compiled && vulkan_runtime_available)
+    {
+        return LLRenderBackend::VULKAN;
+    }
+    return LLRenderBackend::OPENGL;
+}
diff --git a/indra/llrender/llrenderbackend.h b/indra/llrender/llrenderbackend.h
new file mode 100644
index 0000000000..c425bb0a32
--- /dev/null
+++ b/indra/llrender/llrenderbackend.h
@@ -0,0 +1,41 @@
+/**
+ * @file llrenderbackend.h
+ * @brief Renderer backend selection boundary for OpenGL and Vulkan.
+ *
+ * $LicenseInfo:firstyear=2026&license=viewerlgpl$
+ * Copyright (C) 2026 Firestorm Vulkan contributors
+ *
+ * This library is free software; you can redistribute it and/or
+ * modify it under the terms of the GNU Lesser General Public License
+ * as published by the Free Software Foundation; version 2.1 only.
+ * $/LicenseInfo$
+ */
+
+#ifndef LL_LLRENDERBACKEND_H
+#define LL_LLRENDERBACKEND_H
+
+#include <string>
+
+enum class LLRenderBackend
+{
+    OPENGL,
+    VULKAN
+};
+
+class LLRenderBackendSelection
+{
+public:
+    static LLRenderBackend requestedFromEnvironment();
+    static bool validationPresentRequestedFromEnvironment();
+    static bool meshCaptureRequestedFromEnvironment();
+    static LLRenderBackend fromString(const std::string& value);
+    static const char* name(LLRenderBackend backend);
+
+    // Vulkan remains opt-in until the backend reaches feature parity.  A failed
+    // probe must always return OpenGL so experimental builds remain usable.
+    static LLRenderBackend resolve(LLRenderBackend requested,
+                                   bool vulkan_compiled,
+                                   bool vulkan_runtime_available);
+};
+
+#endif
diff --git a/indra/llrender/llrendermeshsource.cpp b/indra/llrender/llrendermeshsource.cpp
new file mode 100644
index 0000000000..d41611e85f
--- /dev/null
+++ b/indra/llrender/llrendermeshsource.cpp
@@ -0,0 +1,66 @@
+/**
+ * @file llrendermeshsource.cpp
+ * @brief Backend-neutral snapshot of Firestorm vertex/index data.
+ *
+ * $LicenseInfo:firstyear=2026&license=viewerlgpl$
+ * Copyright (C) 2026 Firestorm Vulkan contributors
+ * $/LicenseInfo$
+ */
+
+#include "linden_common.h"
+#include "llrendermeshsource.h"
+#include "llvertexbuffer.h"
+
+#include <cstring>
+
+LLRenderMeshSource LLRenderMeshSource::fromVertexBuffer(
+    const LLVertexBuffer& source)
+{
+    std::array<std::uint32_t, attribute_count> offsets{};
+    static_assert(attribute_count == LLVertexBuffer::TYPE_MAX,
+                  "Mesh bridge must track every LLVertexBuffer attribute");
+    for (std::size_t attribute = 0; attribute < attribute_count; ++attribute)
+    {
+        offsets[attribute] = source.getOffset(
+            static_cast<LLVertexBuffer::AttributeType>(attribute));
+    }
+    return fromRaw(source.getMappedData(), source.getSize(),
+                   source.getMappedIndices(), source.getIndicesSize(),
+                   source.getNumVerts(), source.getNumIndices(),
+                   source.getIndicesSize() && source.getNumIndices()
+                       ? source.getIndicesSize() / source.getNumIndices() : 0,
+                   source.getTypeMask(), offsets);
+}
+
+LLRenderMeshSource LLRenderMeshSource::fromRaw(
+    const void* vertices, std::size_t vertex_bytes,
+    const void* indices, std::size_t index_bytes,
+    std::uint32_t vertices_count, std::uint32_t indices_count,
+    std::uint32_t indices_stride, std::uint32_t mask,
+    const std::array<std::uint32_t, attribute_count>& offsets)
+{
+    LLRenderMeshSource result;
+    if (!vertices || vertex_bytes == 0 || vertices_count == 0)
+        return result;
+    result.vertex_data.resize(vertex_bytes);
+    std::memcpy(result.vertex_data.data(), vertices, vertex_bytes);
+    if (indices && index_bytes > 0)
+    {
+        result.index_data.resize(index_bytes);
+        std::memcpy(result.index_data.data(), indices, index_bytes);
+    }
+    result.attribute_offsets = offsets;
+    result.vertex_count = vertices_count;
+    result.index_count = indices_count;
+    result.index_stride = indices_stride;
+    result.type_mask = mask;
+    return result;
+}
+
+bool LLRenderMeshSource::valid() const
+{
+    return !vertex_data.empty() && vertex_count > 0 &&
+           ((index_count == 0 && index_data.empty()) ||
+            (index_count > 0 && !index_data.empty() &&
+             (index_stride == 2 || index_stride == 4)));
+}
diff --git a/indra/llrender/llrendermeshsource.h b/indra/llrender/llrendermeshsource.h
new file mode 100644
index 0000000000..72a4883e80
--- /dev/null
+++ b/indra/llrender/llrendermeshsource.h
@@ -0,0 +1,44 @@
+/**
+ * @file llrendermeshsource.h
+ * @brief Backend-neutral snapshot of Firestorm vertex/index data.
+ *
+ * $LicenseInfo:firstyear=2026&license=viewerlgpl$
+ * Copyright (C) 2026 Firestorm Vulkan contributors
+ * $/LicenseInfo$
+ */
+
+#ifndef LL_LLRENDERMESHSOURCE_H
+#define LL_LLRENDERMESHSOURCE_H
+
+#include <array>
+#include <cstddef>
+#include <cstdint>
+#include <vector>
+
+class LLVertexBuffer;
+
+class LLRenderMeshSource
+{
+public:
+    static constexpr std::size_t attribute_count = 14;
+
+    static LLRenderMeshSource fromVertexBuffer(const LLVertexBuffer& source);
+    static LLRenderMeshSource fromRaw(
+        const void* vertices, std::size_t vertex_bytes,
+        const void* indices, std::size_t index_bytes,
+        std::uint32_t vertex_count, std::uint32_t index_count,
+        std::uint32_t index_stride, std::uint32_t type_mask,
+        const std::array<std::uint32_t, attribute_count>& offsets);
+
+    bool valid() const;
+
+    std::vector<std::uint8_t> vertex_data;
+    std::vector<std::uint8_t> index_data;
+    std::array<std::uint32_t, attribute_count> attribute_offsets{};
+    std::uint32_t vertex_count = 0;
+    std::uint32_t index_count = 0;
+    std::uint32_t index_stride = 0;
+    std::uint32_t type_mask = 0;
+};
+
+#endif
diff --git a/indra/llrender/llvertexbuffer.cpp b/indra/llrender/llvertexbuffer.cpp
index b5f9588227..3abfbbc6fb 100644
--- a/indra/llrender/llvertexbuffer.cpp
+++ b/indra/llrender/llvertexbuffer.cpp
@@ -29,6 +29,8 @@
 #include "llfasttimer.h"
 #include "llsys.h"
 #include "llvertexbuffer.h"
+#include "llvulkancontext.h"
+#include "llrenderbackend.h"
 // #include "llrender.h"
 #include "llglheaders.h"
 #include "llrender.h"
@@ -36,6 +38,7 @@
 #include "llshadermgr.h"
 #include "llglslshader.h"
 #include "llmemory.h"
+#include <atomic>
 #include <glm/gtc/type_ptr.hpp>
 
 //Next Highest Power Of Two
@@ -916,6 +919,13 @@ void LLVertexBuffer::clone(LLVertexBuffer& target) const
 
 void LLVertexBuffer::drawRange(U32 mode, U32 start, U32 end, U32 count, U32 indices_offset) const
 {
+#if FIRESTORM_VULKAN_EXPERIMENTAL && LL_WINDOWS
+    static const bool capture_vulkan_meshes =
+        LLRenderBackendSelection::meshCaptureRequestedFromEnvironment();
+    if (capture_vulkan_meshes)
+        LLVulkanContext::instance().observeDraw(*this, mode, indices_offset,
+                                                count, true);
+#endif
     llassert(validateRange(start, end, count, indices_offset));
     llassert(mGLBuffer == sGLRenderBuffer);
     llassert(mGLIndices == sGLRenderIndices);
@@ -928,6 +938,13 @@ void LLVertexBuffer::drawRange(U32 mode, U32 start, U32 end, U32 count, U32 indi
 
 void LLVertexBuffer::drawRangeFast(U32 mode, U32 start, U32 end, U32 count, U32 indices_offset) const
 {
+#if FIRESTORM_VULKAN_EXPERIMENTAL && LL_WINDOWS
+    static const bool capture_vulkan_meshes =
+        LLRenderBackendSelection::meshCaptureRequestedFromEnvironment();
+    if (capture_vulkan_meshes)
+        LLVulkanContext::instance().observeDraw(*this, mode, indices_offset,
+                                                count, true);
+#endif
     glDrawRangeElements(sGLMode[mode], start, end, count, mIndicesType,
         (GLvoid*)(indices_offset * (size_t)mIndicesStride));
 }
@@ -941,6 +958,13 @@ void LLVertexBuffer::draw(U32 mode, U32 count, U32 indices_offset) const
 
 void LLVertexBuffer::drawArrays(U32 mode, U32 first, U32 count) const
 {
+#if FIRESTORM_VULKAN_EXPERIMENTAL && LL_WINDOWS
+    static const bool capture_vulkan_meshes =
+        LLRenderBackendSelection::meshCaptureRequestedFromEnvironment();
+    if (capture_vulkan_meshes)
+        LLVulkanContext::instance().observeDraw(*this, mode, first,
+                                                count, false);
+#endif
     llassert(first + count <= mNumVerts);
     llassert(mGLBuffer == sGLRenderBuffer);
     llassert(mGLIndices == sGLRenderIndices);
@@ -1013,8 +1037,11 @@ void LLVertexBuffer::cleanupClass()
 
 //----------------------------------------------------------------------------
 
+static std::atomic<U64> sNextBackendBufferIdentity{1};
+
 LLVertexBuffer::LLVertexBuffer(U32 typemask)
 :   LLRefCount(),
+    mBackendIdentity(sNextBackendBufferIdentity.fetch_add(1)),
     mTypeMask(typemask)
 {
     //zero out offsets
@@ -1428,6 +1455,7 @@ void LLVertexBuffer::_unmapBuffer()
     {
         return;
     }
+    ++mDataRevision;
 
     struct SortMappedRegion
     {
@@ -1936,7 +1964,3 @@ void LLVertexBuffer::setIndexData(const U32* data, U32 offset, U32 count)
     }
     flush_vbo(GL_ELEMENT_ARRAY_BUFFER, offset * sizeof(U32), (offset + count) * sizeof(U32) - 1, (U8*)data, mMappedIndexData);
 }
-
-
-
-
diff --git a/indra/llrender/llvertexbuffer.h b/indra/llrender/llvertexbuffer.h
index 78c449fe2f..857f0b3c20 100644
--- a/indra/llrender/llvertexbuffer.h
+++ b/indra/llrender/llvertexbuffer.h
@@ -258,6 +258,8 @@ public:
     bool hasDataType(AttributeType type) const { return ((1 << type) & getTypeMask()); }
     U32 getSize() const                     { return mSize; }
     U32 getIndicesSize() const              { return mIndicesSize; }
+    U64 getBackendIdentity() const          { return mBackendIdentity; }
+    U64 getDataRevision() const             { return mDataRevision; }
     U8* getMappedData() const               { return mMappedData; }
     U8* getMappedIndices() const            { return mMappedIndexData; }
     U32 getOffset(AttributeType type) const { return mOffsets[type]; }
@@ -288,6 +290,8 @@ protected:
     U32     mNumIndices = 0;    // Number of indices allocated
     U32     mIndicesType = GL_UNSIGNED_SHORT; // type of indices in index buffer
     U32     mIndicesStride = 2;     // size of each index in bytes
+    U64     mBackendIdentity = 0;   // stable cache key across pointer reuse
+    U64     mDataRevision = 1;      // incremented whenever mapped data changes
     U32     mOffsets[TYPE_MAX]; // byte offsets into mMappedData of each attribute
 
     U8* mMappedData = nullptr;  // pointer to currently mapped data (NULL if unmapped)
diff --git a/indra/llrender/llvulkanbuffer.cpp b/indra/llrender/llvulkanbuffer.cpp
new file mode 100644
index 0000000000..5a95f41b50
--- /dev/null
+++ b/indra/llrender/llvulkanbuffer.cpp
@@ -0,0 +1,147 @@
+/**
+ * @file llvulkanbuffer.cpp
+ * @brief RAII Vulkan buffer allocation and staging upload helper.
+ *
+ * $LicenseInfo:firstyear=2026&license=viewerlgpl$
+ * Copyright (C) 2026 Firestorm Vulkan contributors
+ * $/LicenseInfo$
+ */
+
+#include "linden_common.h"
+#include "llvulkanbuffer.h"
+
+#if FIRESTORM_VULKAN_EXPERIMENTAL
+
+#include <cstring>
+
+LLVulkanBuffer::LLVulkanBuffer(VkPhysicalDevice physical_device, VkDevice device)
+    : mPhysicalDevice(physical_device), mDevice(device)
+{
+}
+
+LLVulkanBuffer::~LLVulkanBuffer()
+{
+    reset();
+}
+
+bool LLVulkanBuffer::selectMemoryType(std::uint32_t allowed_types,
+                                      VkMemoryPropertyFlags required,
+                                      std::uint32_t& selected) const
+{
+    VkPhysicalDeviceMemoryProperties properties{};
+    vkGetPhysicalDeviceMemoryProperties(mPhysicalDevice, &properties);
+    for (std::uint32_t index = 0; index < properties.memoryTypeCount; ++index)
+    {
+        if ((allowed_types & (1u << index)) != 0 &&
+            (properties.memoryTypes[index].propertyFlags & required) == required)
+        {
+            selected = index;
+            return true;
+        }
+    }
+    return false;
+}
+
+bool LLVulkanBuffer::allocate(VkDeviceSize size, VkBufferUsageFlags usage,
+                              VkMemoryPropertyFlags memory_properties)
+{
+    reset();
+    const VkBufferCreateInfo buffer_info{
+        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, nullptr, 0, size, usage,
+        VK_SHARING_MODE_EXCLUSIVE, 0, nullptr};
+    if (vkCreateBuffer(mDevice, &buffer_info, nullptr, &mBuffer) != VK_SUCCESS)
+        return false;
+
+    VkMemoryRequirements requirements{};
+    vkGetBufferMemoryRequirements(mDevice, mBuffer, &requirements);
+    std::uint32_t memory_type = 0;
+    if (!selectMemoryType(requirements.memoryTypeBits, memory_properties,
+                          memory_type))
+    {
+        reset();
+        return false;
+    }
+    const VkMemoryAllocateInfo allocation_info{
+        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, nullptr,
+        requirements.size, memory_type};
+    if (vkAllocateMemory(mDevice, &allocation_info, nullptr, &mMemory) != VK_SUCCESS ||
+        vkBindBufferMemory(mDevice, mBuffer, mMemory, 0) != VK_SUCCESS)
+    {
+        reset();
+        return false;
+    }
+    mSize = size;
+    return true;
+}
+
+bool LLVulkanBuffer::upload(const void* data, VkDeviceSize size)
+{
+    if (!valid() || !data || size > mSize)
+        return false;
+    void* mapped = nullptr;
+    if (vkMapMemory(mDevice, mMemory, 0, size, 0, &mapped) != VK_SUCCESS)
+        return false;
+    std::memcpy(mapped, data, static_cast<std::size_t>(size));
+    vkUnmapMemory(mDevice, mMemory);
+    return true;
+}
+
+bool LLVulkanBuffer::uploadDeviceLocal(const void* data, VkDeviceSize size,
+                                       VkBufferUsageFlags final_usage,
+                                       VkCommandPool command_pool, VkQueue queue)
+{
+    LLVulkanBuffer staging(mPhysicalDevice, mDevice);
+    if (!staging.allocate(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
+                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
+                          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) ||
+        !staging.upload(data, size) ||
+        !allocate(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | final_usage,
+                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
+        return false;
+
+    VkCommandBuffer command = VK_NULL_HANDLE;
+    const VkCommandBufferAllocateInfo allocate_info{
+        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, nullptr,
+        command_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1};
+    VkResult status = vkAllocateCommandBuffers(mDevice, &allocate_info, &command);
+    if (status == VK_SUCCESS)
+    {
+        const VkCommandBufferBeginInfo begin_info{
+            VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr,
+            VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr};
+        status = vkBeginCommandBuffer(command, &begin_info);
+    }
+    if (status == VK_SUCCESS)
+    {
+        const VkBufferCopy copy{0, 0, size};
+        vkCmdCopyBuffer(command, staging.handle(), mBuffer, 1, &copy);
+        status = vkEndCommandBuffer(command);
+    }
+    if (status == VK_SUCCESS)
+    {
+        const VkSubmitInfo submit_info{
+            VK_STRUCTURE_TYPE_SUBMIT_INFO, nullptr, 0, nullptr, nullptr,
+            1, &command, 0, nullptr};
+        status = vkQueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
+    }
+    if (status == VK_SUCCESS)
+        status = vkQueueWaitIdle(queue);
+    if (command != VK_NULL_HANDLE)
+        vkFreeCommandBuffers(mDevice, command_pool, 1, &command);
+    if (status != VK_SUCCESS)
+        reset();
+    return status == VK_SUCCESS;
+}
+
+void LLVulkanBuffer::reset()
+{
+    if (mBuffer != VK_NULL_HANDLE)
+        vkDestroyBuffer(mDevice, mBuffer, nullptr);
+    if (mMemory != VK_NULL_HANDLE)
+        vkFreeMemory(mDevice, mMemory, nullptr);
+    mBuffer = VK_NULL_HANDLE;
+    mMemory = VK_NULL_HANDLE;
+    mSize = 0;
+}
+
+#endif // FIRESTORM_VULKAN_EXPERIMENTAL
diff --git a/indra/llrender/llvulkanbuffer.h b/indra/llrender/llvulkanbuffer.h
new file mode 100644
index 0000000000..54867f3756
--- /dev/null
+++ b/indra/llrender/llvulkanbuffer.h
@@ -0,0 +1,52 @@
+/**
+ * @file llvulkanbuffer.h
+ * @brief RAII Vulkan buffer allocation and staging upload helper.
+ *
+ * $LicenseInfo:firstyear=2026&license=viewerlgpl$
+ * Copyright (C) 2026 Firestorm Vulkan contributors
+ * $/LicenseInfo$
+ */
+
+#ifndef LL_LLVULKANBUFFER_H
+#define LL_LLVULKANBUFFER_H
+
+#if FIRESTORM_VULKAN_EXPERIMENTAL
+
+#include <cstdint>
+#include <vulkan/vulkan.h>
+
+class LLVulkanBuffer
+{
+public:
+    LLVulkanBuffer(VkPhysicalDevice physical_device, VkDevice device);
+    ~LLVulkanBuffer();
+
+    LLVulkanBuffer(const LLVulkanBuffer&) = delete;
+    LLVulkanBuffer& operator=(const LLVulkanBuffer&) = delete;
+
+    bool allocate(VkDeviceSize size, VkBufferUsageFlags usage,
+                  VkMemoryPropertyFlags memory_properties);
+    bool upload(const void* data, VkDeviceSize size);
+    bool uploadDeviceLocal(const void* data, VkDeviceSize size,
+                           VkBufferUsageFlags final_usage,
+                           VkCommandPool command_pool, VkQueue queue);
+    void reset();
+
+    VkBuffer handle() const { return mBuffer; }
+    VkDeviceSize size() const { return mSize; }
+    bool valid() const { return mBuffer != VK_NULL_HANDLE; }
+
+private:
+    bool selectMemoryType(std::uint32_t allowed_types,
+                          VkMemoryPropertyFlags required,
+                          std::uint32_t& selected) const;
+
+    VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
+    VkDevice mDevice = VK_NULL_HANDLE;
+    VkBuffer mBuffer = VK_NULL_HANDLE;
+    VkDeviceMemory mMemory = VK_NULL_HANDLE;
+    VkDeviceSize mSize = 0;
+};
+
+#endif // FIRESTORM_VULKAN_EXPERIMENTAL
+#endif // LL_LLVULKANBUFFER_H
diff --git a/indra/llrender/llvulkancontext.cpp b/indra/llrender/llvulkancontext.cpp
new file mode 100644
index 0000000000..2fed591f89
--- /dev/null
+++ b/indra/llrender/llvulkancontext.cpp
@@ -0,0 +1,3585 @@
+/**
+ * @file llvulkancontext.cpp
+ * @brief Experimental Vulkan instance and logical-device owner.
+ *
+ * $LicenseInfo:firstyear=2026&license=viewerlgpl$
+ * Copyright (C) 2026 Firestorm Vulkan contributors
+ * $/LicenseInfo$
+ */
+
+#include "linden_common.h"
+#include "llvulkancontext.h"
+#include "llvulkanvalidationshaders.h"
+
+#include <algorithm>
+#include <array>
+#include <deque>
+#include <limits>
+#include <mutex>
+#include <unordered_map>
+#include <unordered_set>
+#include <vector>
+
+#if FIRESTORM_VULKAN_EXPERIMENTAL
+# if LL_WINDOWS
+    static constexpr std::size_t frames_in_flight = 2;
+#  ifndef VK_USE_PLATFORM_WIN32_KHR
+#   define VK_USE_PLATFORM_WIN32_KHR
+#  endif
+# endif
+# include <vulkan/vulkan.h>
+#endif
+
+#if FIRESTORM_VULKAN_EXPERIMENTAL && LL_WINDOWS
+#include "llvulkanbuffer.h"
+#include "llrendermeshsource.h"
+#include "llglstates.h"
+#include "llglslshader.h"
+#include "llshadermgr.h"
+#include "llvertexbuffer.h"
+
+namespace
+{
+struct ValidationVertex
+{
+    float position[2];
+    float color[3];
+    float texcoord[2];
+    float normal[3];
+    float tangent[4];
+    float weights[4];
+    std::uint16_t joints[4];
+    float legacy_weight;
+};
+
+struct DrawInstanceTransforms
+{
+    std::array<float, 16> shadow{};
+    std::array<float, 16> model_view{};
+};
+
+constexpr std::array<ValidationVertex, 3> validation_vertices{{
+    {{ 0.0f, -0.62f}, {0.95f, 0.20f, 0.25f}, {0.5f, 1.f}, {0.f, 0.f, 1.f}, {1.f, 0.f, 0.f, 1.f}, {1.f, 0.f, 0.f, 0.f}, {0, 0, 0, 0}, 0.f},
+    {{ 0.62f, 0.52f}, {0.20f, 0.80f, 0.95f}, {1.f, 0.f}, {0.f, 0.f, 1.f}, {1.f, 0.f, 0.f, 1.f}, {1.f, 0.f, 0.f, 0.f}, {0, 0, 0, 0}, 0.f},
+    {{-0.62f, 0.52f}, {0.65f, 0.35f, 0.95f}, {0.f, 0.f}, {0.f, 0.f, 1.f}, {1.f, 0.f, 0.f, 1.f}, {1.f, 0.f, 0.f, 0.f}, {0, 0, 0, 0}, 0.f}
+}};
+constexpr std::array<std::uint16_t, 3> validation_indices{{0, 1, 2}};
+
+LLVulkanPrimitiveTopology translateTopology(std::uint32_t mode)
+{
+    switch (mode)
+    {
+        case LLRender::TRIANGLES: return LLVulkanPrimitiveTopology::TRIANGLE_LIST;
+        case LLRender::TRIANGLE_STRIP: return LLVulkanPrimitiveTopology::TRIANGLE_STRIP;
+        case LLRender::TRIANGLE_FAN: return LLVulkanPrimitiveTopology::TRIANGLE_FAN;
+        case LLRender::POINTS: return LLVulkanPrimitiveTopology::POINT_LIST;
+        case LLRender::LINES: return LLVulkanPrimitiveTopology::LINE_LIST;
+        case LLRender::LINE_STRIP: return LLVulkanPrimitiveTopology::LINE_STRIP;
+        case LLRender::LINE_LOOP: return LLVulkanPrimitiveTopology::UNSUPPORTED;
+        default: return LLVulkanPrimitiveTopology::UNSUPPORTED;
+    }
+}
+
+LLVulkanBlendFactor translateBlendFactor(LLRender::eBlendFactor factor)
+{
+    switch (factor)
+    {
+        case LLRender::BF_ZERO: return LLVulkanBlendFactor::ZERO;
+        case LLRender::BF_ONE: return LLVulkanBlendFactor::ONE;
+        case LLRender::BF_SOURCE_COLOR: return LLVulkanBlendFactor::SRC_COLOR;
+        case LLRender::BF_ONE_MINUS_SOURCE_COLOR: return LLVulkanBlendFactor::ONE_MINUS_SRC_COLOR;
+        case LLRender::BF_DEST_COLOR: return LLVulkanBlendFactor::DST_COLOR;
+        case LLRender::BF_ONE_MINUS_DEST_COLOR: return LLVulkanBlendFactor::ONE_MINUS_DST_COLOR;
+        case LLRender::BF_SOURCE_ALPHA: return LLVulkanBlendFactor::SRC_ALPHA;
+        case LLRender::BF_ONE_MINUS_SOURCE_ALPHA: return LLVulkanBlendFactor::ONE_MINUS_SRC_ALPHA;
+        case LLRender::BF_DEST_ALPHA: return LLVulkanBlendFactor::DST_ALPHA;
+        case LLRender::BF_ONE_MINUS_DEST_ALPHA: return LLVulkanBlendFactor::ONE_MINUS_DST_ALPHA;
+        default: return LLVulkanBlendFactor::UNSUPPORTED;
+    }
+}
+
+LLVulkanCompareOp translateCompareOp(GLenum function)
+{
+    switch (function)
+    {
+        case GL_NEVER: return LLVulkanCompareOp::NEVER;
+        case GL_LESS: return LLVulkanCompareOp::LESS;
+        case GL_EQUAL: return LLVulkanCompareOp::EQUAL;
+        case GL_LEQUAL: return LLVulkanCompareOp::LESS_OR_EQUAL;
+        case GL_GREATER: return LLVulkanCompareOp::GREATER;
+        case GL_NOTEQUAL: return LLVulkanCompareOp::NOT_EQUAL;
+        case GL_GEQUAL: return LLVulkanCompareOp::GREATER_OR_EQUAL;
+        case GL_ALWAYS: return LLVulkanCompareOp::ALWAYS;
+        default: return LLVulkanCompareOp::UNSUPPORTED;
+    }
+}
+
+LLVulkanTextureAddress translateTextureAddress(LLTexUnit::eTextureAddressMode mode)
+{
+    switch (mode)
+    {
+        case LLTexUnit::TAM_MIRROR: return LLVulkanTextureAddress::MIRRORED_REPEAT;
+        case LLTexUnit::TAM_CLAMP: return LLVulkanTextureAddress::CLAMP;
+        default: return LLVulkanTextureAddress::REPEAT;
+    }
+}
+
+LLVulkanTextureFilter translateTextureFilter(LLTexUnit::eTextureFilterOptions filter)
+{
+    switch (filter)
+    {
+        case LLTexUnit::TFO_POINT: return LLVulkanTextureFilter::NEAREST;
+        case LLTexUnit::TFO_TRILINEAR: return LLVulkanTextureFilter::TRILINEAR;
+        case LLTexUnit::TFO_ANISOTROPIC: return LLVulkanTextureFilter::ANISOTROPIC;
+        default: return LLVulkanTextureFilter::LINEAR;
+    }
+}
+
+std::uint64_t textureDescriptorKey(const LLVulkanTextureBinding& binding)
+{
+    std::uint64_t key = binding.identity;
+    key ^= static_cast<std::uint64_t>(binding.address) << 56;
+    key ^= static_cast<std::uint64_t>(binding.filter) << 59;
+    key ^= static_cast<std::uint64_t>(binding.has_mipmaps) << 63;
+    return key;
+}
+
+VkPrimitiveTopology toVkTopology(LLVulkanPrimitiveTopology topology)
+{
+    switch (topology)
+    {
+        case LLVulkanPrimitiveTopology::TRIANGLE_LIST: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
+        case LLVulkanPrimitiveTopology::TRIANGLE_STRIP: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
+        case LLVulkanPrimitiveTopology::TRIANGLE_FAN: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
+        case LLVulkanPrimitiveTopology::POINT_LIST: return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
+        case LLVulkanPrimitiveTopology::LINE_LIST: return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
+        case LLVulkanPrimitiveTopology::LINE_STRIP: return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
+        default: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
+    }
+}
+
+VkBlendFactor toVkBlendFactor(LLVulkanBlendFactor factor)
+{
+    switch (factor)
+    {
+        case LLVulkanBlendFactor::ZERO: return VK_BLEND_FACTOR_ZERO;
+        case LLVulkanBlendFactor::ONE: return VK_BLEND_FACTOR_ONE;
+        case LLVulkanBlendFactor::SRC_COLOR: return VK_BLEND_FACTOR_SRC_COLOR;
+        case LLVulkanBlendFactor::ONE_MINUS_SRC_COLOR: return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
+        case LLVulkanBlendFactor::DST_COLOR: return VK_BLEND_FACTOR_DST_COLOR;
+        case LLVulkanBlendFactor::ONE_MINUS_DST_COLOR: return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
+        case LLVulkanBlendFactor::SRC_ALPHA: return VK_BLEND_FACTOR_SRC_ALPHA;
+        case LLVulkanBlendFactor::ONE_MINUS_SRC_ALPHA: return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
+        case LLVulkanBlendFactor::DST_ALPHA: return VK_BLEND_FACTOR_DST_ALPHA;
+        case LLVulkanBlendFactor::ONE_MINUS_DST_ALPHA: return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
+        default: return VK_BLEND_FACTOR_ONE;
+    }
+}
+
+VkCompareOp toVkCompareOp(LLVulkanCompareOp operation)
+{
+    switch (operation)
+    {
+        case LLVulkanCompareOp::NEVER: return VK_COMPARE_OP_NEVER;
+        case LLVulkanCompareOp::LESS: return VK_COMPARE_OP_LESS;
+        case LLVulkanCompareOp::EQUAL: return VK_COMPARE_OP_EQUAL;
+        case LLVulkanCompareOp::LESS_OR_EQUAL: return VK_COMPARE_OP_LESS_OR_EQUAL;
+        case LLVulkanCompareOp::GREATER: return VK_COMPARE_OP_GREATER;
+        case LLVulkanCompareOp::NOT_EQUAL: return VK_COMPARE_OP_NOT_EQUAL;
+        case LLVulkanCompareOp::GREATER_OR_EQUAL: return VK_COMPARE_OP_GREATER_OR_EQUAL;
+        case LLVulkanCompareOp::ALWAYS: return VK_COMPARE_OP_ALWAYS;
+        default: return VK_COMPARE_OP_ALWAYS;
+    }
+}
+
+VkFormat selectDepthFormat(VkPhysicalDevice physical_device)
+{
+    constexpr VkFormat candidates[] = {
+        VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
+        VK_FORMAT_D24_UNORM_S8_UINT};
+    for (VkFormat format : candidates)
+    {
+        VkFormatProperties properties{};
+        vkGetPhysicalDeviceFormatProperties(physical_device, format, &properties);
+        if ((properties.optimalTilingFeatures &
+             VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0)
+            return format;
+    }
+    return VK_FORMAT_UNDEFINED;
+}
+
+bool selectDeviceMemoryType(VkPhysicalDevice physical_device,
+                            std::uint32_t allowed,
+                            VkMemoryPropertyFlags required,
+                            std::uint32_t& selected)
+{
+    VkPhysicalDeviceMemoryProperties properties{};
+    vkGetPhysicalDeviceMemoryProperties(physical_device, &properties);
+    for (std::uint32_t index = 0; index < properties.memoryTypeCount; ++index)
+    {
+        if ((allowed & (1u << index)) != 0 &&
+            (properties.memoryTypes[index].propertyFlags & required) == required)
+        {
+            selected = index;
+            return true;
+        }
+    }
+    return false;
+}
+
+struct PipelineKeyHash
+{
+    std::size_t operator()(const LLVulkanPipelineKey& key) const
+    {
+        std::size_t value = static_cast<std::size_t>(key.topology);
+        auto mix = [&value](std::size_t part) { value = value * 131u + part; };
+        mix(static_cast<std::size_t>(key.color_src));
+        mix(static_cast<std::size_t>(key.color_dst));
+        mix(static_cast<std::size_t>(key.alpha_src));
+        mix(static_cast<std::size_t>(key.alpha_dst));
+        mix(static_cast<std::size_t>(key.depth_compare));
+        mix(static_cast<std::size_t>(key.cull_mode));
+        mix(static_cast<std::size_t>(key.polygon_mode));
+        mix(static_cast<std::size_t>(key.shader_family));
+        mix(key.blend_enabled); mix(key.depth_test_enabled); mix(key.depth_write_enabled);
+        return value;
+    }
+};
+
+VkPipeline createCapturedPipeline(VkDevice device, VkRenderPass render_pass,
+                                  VkPipelineLayout layout,
+                                  VkShaderModule vertex_shader,
+                                  VkShaderModule fragment_shader,
+                                  const LLVulkanPipelineKey& key)
+{
+    const bool shadow = key.shader_family == LLVulkanShaderFamily::SHADOW_STATIC ||
+        key.shader_family == LLVulkanShaderFamily::SHADOW_ALPHA ||
+        key.shader_family == LLVulkanShaderFamily::SHADOW_SKINNED ||
+        key.shader_family == LLVulkanShaderFamily::SHADOW_AVATAR;
+    const bool alpha_shadow =
+        key.shader_family == LLVulkanShaderFamily::SHADOW_ALPHA;
+    const VkPipelineShaderStageCreateInfo stages[] = {
+        {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0,
+         VK_SHADER_STAGE_VERTEX_BIT, vertex_shader, "main", nullptr},
+        {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0,
+         VK_SHADER_STAGE_FRAGMENT_BIT, fragment_shader, "main", nullptr}
+    };
+    const VkVertexInputBindingDescription bindings[] = {
+        {0, sizeof(float) * 4, VK_VERTEX_INPUT_RATE_VERTEX},
+        {1, sizeof(std::uint32_t), VK_VERTEX_INPUT_RATE_VERTEX},
+        {2, sizeof(float) * 2, VK_VERTEX_INPUT_RATE_VERTEX},
+        {3, sizeof(float) * 4, VK_VERTEX_INPUT_RATE_VERTEX},
+        {4, sizeof(float) * 4, VK_VERTEX_INPUT_RATE_VERTEX},
+        {5, sizeof(float) * 4, VK_VERTEX_INPUT_RATE_VERTEX},
+        {6, sizeof(std::uint16_t) * 4, VK_VERTEX_INPUT_RATE_VERTEX},
+        {7, sizeof(float), VK_VERTEX_INPUT_RATE_VERTEX}
+        ,{8, sizeof(DrawInstanceTransforms), VK_VERTEX_INPUT_RATE_INSTANCE}
+    };
+    const VkVertexInputAttributeDescription attributes[] = {
+        {0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0},
+        {1, 1, VK_FORMAT_R8G8B8A8_UNORM, 0},
+        {2, 2, VK_FORMAT_R32G32_SFLOAT, 0},
+        {3, 3, VK_FORMAT_R32G32B32_SFLOAT, 0},
+        {4, 4, VK_FORMAT_R32G32B32A32_SFLOAT, 0},
+        {5, 5, VK_FORMAT_R32G32B32A32_SFLOAT, 0},
+        {6, 6, VK_FORMAT_R16G16B16A16_UINT, 0},
+        {7, 7, VK_FORMAT_R32_SFLOAT, 0}
+        ,{8, 8, VK_FORMAT_R32G32B32A32_SFLOAT, 0}
+        ,{9, 8, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 4}
+        ,{10, 8, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 8}
+        ,{11, 8, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 12}
+        ,{12, 8, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 16}
+        ,{13, 8, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 20}
+        ,{14, 8, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 24}
+        ,{15, 8, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 28}
+    };
+    const VkPipelineVertexInputStateCreateInfo vertex_input{
+        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO, nullptr, 0,
+        9, bindings, 16, attributes};
+    const VkPipelineInputAssemblyStateCreateInfo assembly{
+        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, nullptr, 0,
+        toVkTopology(key.topology), VK_FALSE};
+    const VkPipelineViewportStateCreateInfo viewport{
+        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO, nullptr, 0,
+        1, nullptr, 1, nullptr};
+    const VkPipelineRasterizationStateCreateInfo rasterization{
+        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO, nullptr, 0,
+        VK_FALSE, VK_FALSE, VK_POLYGON_MODE_FILL,
+        key.cull_mode == LLVulkanCullMode::NONE ? VK_CULL_MODE_NONE :
+            (key.cull_mode == LLVulkanCullMode::FRONT ? VK_CULL_MODE_FRONT_BIT :
+                                                       VK_CULL_MODE_BACK_BIT),
+        VK_FRONT_FACE_COUNTER_CLOCKWISE, shadow ? VK_TRUE : VK_FALSE,
+        shadow ? 1.25f : 0.f, 0.f, shadow ? 1.75f : 0.f, 1.f};
+    const VkPipelineMultisampleStateCreateInfo multisample{
+        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO, nullptr, 0,
+        VK_SAMPLE_COUNT_1_BIT, VK_FALSE, 1.f, nullptr, VK_FALSE, VK_FALSE};
+    const VkPipelineDepthStencilStateCreateInfo depth_stencil{
+        VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO, nullptr, 0,
+        shadow ? VK_TRUE : (key.depth_test_enabled ? VK_TRUE : VK_FALSE),
+        shadow ? VK_TRUE : (key.depth_write_enabled ? VK_TRUE : VK_FALSE),
+        shadow ? VK_COMPARE_OP_LESS_OR_EQUAL : toVkCompareOp(key.depth_compare),
+        VK_FALSE, VK_FALSE,
+        {}, {}, 0.f, 1.f};
+    const VkPipelineColorBlendAttachmentState attachment{
+        key.blend_enabled ? VK_TRUE : VK_FALSE,
+        toVkBlendFactor(key.color_src), toVkBlendFactor(key.color_dst), VK_BLEND_OP_ADD,
+        toVkBlendFactor(key.alpha_src), toVkBlendFactor(key.alpha_dst), VK_BLEND_OP_ADD,
+        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
+            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT};
+    const VkPipelineColorBlendStateCreateInfo blend{
+        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO, nullptr, 0,
+        VK_FALSE, VK_LOGIC_OP_COPY, shadow ? 0u : 1u,
+        shadow ? nullptr : &attachment, {0.f, 0.f, 0.f, 0.f}};
+    constexpr VkDynamicState dynamic_states[] = {
+        VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
+    const VkPipelineDynamicStateCreateInfo dynamic{
+        VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO, nullptr, 0,
+        2, dynamic_states};
+    const VkGraphicsPipelineCreateInfo info{
+        VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO, nullptr, 0,
+        shadow && !alpha_shadow ? 1u : 2u, stages,
+        &vertex_input, &assembly, nullptr, &viewport, &rasterization,
+        &multisample, &depth_stencil, &blend, &dynamic, layout, render_pass, 0,
+        VK_NULL_HANDLE, -1};
+    VkPipeline pipeline = VK_NULL_HANDLE;
+    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &info, nullptr,
+                                  &pipeline) != VK_SUCCESS)
+        return VK_NULL_HANDLE;
+    return pipeline;
+}
+
+} // namespace
+#endif
+
+bool LLVulkanPipelineKey::operator==(const LLVulkanPipelineKey& rhs) const
+{
+    return topology == rhs.topology && color_src == rhs.color_src &&
+        color_dst == rhs.color_dst && alpha_src == rhs.alpha_src &&
+        alpha_dst == rhs.alpha_dst && depth_compare == rhs.depth_compare &&
+        cull_mode == rhs.cull_mode && polygon_mode == rhs.polygon_mode &&
+        shader_family == rhs.shader_family &&
+        blend_enabled == rhs.blend_enabled &&
+        depth_test_enabled == rhs.depth_test_enabled &&
+        depth_write_enabled == rhs.depth_write_enabled;
+}
+
+struct LLVulkanContext::Impl
+{
+    bool ready = false;
+    std::string device_name;
+    std::string error;
+    unsigned swapchain_width = 0;
+    unsigned swapchain_height = 0;
+    unsigned swapchain_image_count = 0;
+    std::size_t current_frame = 0;
+
+#if FIRESTORM_VULKAN_EXPERIMENTAL
+    VkInstance instance = VK_NULL_HANDLE;
+    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
+    VkDevice device = VK_NULL_HANDLE;
+    VkQueue graphics_queue = VK_NULL_HANDLE;
+    std::uint32_t graphics_queue_family = std::numeric_limits<std::uint32_t>::max();
+    bool sampler_anisotropy = false;
+    float max_sampler_anisotropy = 1.f;
+# if LL_WINDOWS
+    struct CachedMesh
+    {
+        std::uint64_t revision = 0;
+        std::unique_ptr<LLVulkanBuffer> vertices;
+        std::unique_ptr<LLVulkanBuffer> indices;
+        std::uint32_t vertex_count = 0;
+        std::uint32_t index_count = 0;
+        std::uint32_t index_stride = 0;
+        std::uint32_t type_mask = 0;
+        std::array<std::uint32_t, LLRenderMeshSource::attribute_count> offsets{};
+    };
+    std::unordered_map<std::uint64_t, CachedMesh> mesh_cache;
+    std::deque<LLVulkanDrawRecord> draw_queue;
+    std::unordered_set<LLVulkanPipelineKey, PipelineKeyHash> pipeline_keys;
+    std::unordered_map<LLVulkanPipelineKey, VkPipeline, PipelineKeyHash> pipeline_cache;
+    std::uint64_t unsupported_draw_count = 0;
+    std::uint64_t unsupported_state_count = 0;
+    std::uint64_t executed_draw_count = 0;
+    VkSurfaceKHR surface = VK_NULL_HANDLE;
+    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
+    VkFormat swapchain_format = VK_FORMAT_UNDEFINED;
+    std::vector<VkImage> swapchain_images;
+    std::vector<VkImageView> swapchain_image_views;
+    VkFormat depth_format = VK_FORMAT_UNDEFINED;
+    std::vector<VkImage> depth_images;
+    std::vector<VkDeviceMemory> depth_memory;
+    std::vector<VkImageView> depth_image_views;
+    VkRenderPass render_pass = VK_NULL_HANDLE;
+    VkRenderPass shadow_render_pass = VK_NULL_HANDLE;
+    VkImage shadow_image = VK_NULL_HANDLE;
+    VkDeviceMemory shadow_memory = VK_NULL_HANDLE;
+    VkImageView shadow_view = VK_NULL_HANDLE;
+    VkFramebuffer shadow_framebuffer = VK_NULL_HANDLE;
+    VkSampler shadow_sampler = VK_NULL_HANDLE;
+    VkDescriptorSet shadow_descriptor = VK_NULL_HANDLE;
+    struct ProbeArray
+    {
+        VkImage image = VK_NULL_HANDLE;
+        VkDeviceMemory memory = VK_NULL_HANDLE;
+        VkImageView view = VK_NULL_HANDLE;
+        std::uint32_t mip_levels = 1;
+        std::uint32_t cube_count = 256;
+    };
+    ProbeArray radiance_probes;
+    ProbeArray irradiance_probes;
+    VkSampler probe_sampler = VK_NULL_HANDLE;
+    VkDescriptorSet radiance_probe_descriptor = VK_NULL_HANDLE;
+    VkDescriptorSet irradiance_probe_descriptor = VK_NULL_HANDLE;
+    std::unique_ptr<LLVulkanBuffer> shadow_transform_buffers[frames_in_flight];
+    std::unique_ptr<LLVulkanBuffer> probe_metadata_buffers[frames_in_flight];
+    VkDescriptorSet probe_metadata_descriptors[frames_in_flight]{};
+    std::vector<LLVulkanProbeRecord> pending_probe_metadata;
+    std::mutex pending_probe_mutex;
+    std::vector<LLVulkanProbeFace> pending_probe_faces;
+    std::mutex pending_probe_face_mutex;
+    static constexpr std::uint32_t shadow_extent = 2048;
+    VkShaderModule validation_vertex_shader = VK_NULL_HANDLE;
+    VkShaderModule validation_fragment_shader = VK_NULL_HANDLE;
+    VkPipelineLayout validation_pipeline_layout = VK_NULL_HANDLE;
+    VkPipeline validation_pipeline = VK_NULL_HANDLE;
+    std::unique_ptr<LLVulkanBuffer> validation_vertex_buffer;
+    std::unique_ptr<LLVulkanBuffer> validation_index_buffer;
+    VkDescriptorSetLayout texture_descriptor_layout = VK_NULL_HANDLE;
+    VkDescriptorSetLayout skin_descriptor_layout = VK_NULL_HANDLE;
+    VkDescriptorPool texture_descriptor_pool = VK_NULL_HANDLE;
+    struct SkinRecord
+    {
+        std::uint64_t revision = 0;
+        std::unique_ptr<LLVulkanBuffer> buffer;
+        VkDescriptorSet descriptor = VK_NULL_HANDLE;
+    };
+    std::unordered_map<std::uint64_t, SkinRecord> skin_cache;
+    std::unique_ptr<LLVulkanBuffer> identity_skin_buffer;
+    VkDescriptorSet identity_skin_descriptor = VK_NULL_HANDLE;
+    VkImage fallback_texture_image = VK_NULL_HANDLE;
+    VkDeviceMemory fallback_texture_memory = VK_NULL_HANDLE;
+    VkImageView fallback_texture_view = VK_NULL_HANDLE;
+    std::unordered_map<std::uint64_t, VkSampler> sampler_cache;
+    struct DescriptorRecord
+    {
+        VkDescriptorSet set = VK_NULL_HANDLE;
+        std::uint64_t sampler_key = 0;
+    };
+    struct TextureRecord
+    {
+        VkImage image = VK_NULL_HANDLE;
+        VkDeviceMemory memory = VK_NULL_HANDLE;
+        VkImageView view = VK_NULL_HANDLE;
+        std::uint32_t width = 0;
+        std::uint32_t height = 0;
+        std::uint32_t mip_levels = 1;
+    };
+    struct PendingTexture
+    {
+        std::uint32_t width = 0;
+        std::uint32_t height = 0;
+        std::vector<std::uint8_t> rgba;
+        bool generate_mipmaps = false;
+    };
+    std::unordered_map<std::uint64_t, DescriptorRecord> descriptor_cache;
+    std::unordered_map<std::uint64_t, TextureRecord> texture_cache;
+    std::unordered_map<std::uint64_t, PendingTexture> pending_textures;
+    std::mutex pending_texture_mutex;
+    std::vector<VkFramebuffer> framebuffers;
+    VkCommandPool command_pool = VK_NULL_HANDLE;
+    std::vector<VkCommandBuffer> command_buffers;
+    VkSemaphore image_available[frames_in_flight]{};
+    VkSemaphore render_finished[frames_in_flight]{};
+    VkFence frame_fences[frames_in_flight]{};
+# endif
+#endif
+};
+
+LLVulkanContext& LLVulkanContext::instance()
+{
+    static LLVulkanContext context;
+    return context;
+}
+
+LLVulkanContext::LLVulkanContext() : mImpl(std::make_unique<Impl>())
+{
+}
+
+LLVulkanContext::~LLVulkanContext()
+{
+    shutdown();
+}
+
+bool LLVulkanContext::initialize()
+{
+    if (mImpl->ready)
+    {
+        return true;
+    }
+    mImpl->error.clear();
+
+#if !FIRESTORM_VULKAN_EXPERIMENTAL
+    mImpl->error = "Firestorm was built without FIRESTORM_VULKAN_EXPERIMENTAL";
+    return false;
+#else
+    std::vector<const char*> instance_extensions;
+#if LL_WINDOWS
+    instance_extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
+    instance_extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
+#endif
+    const VkApplicationInfo app_info{
+        VK_STRUCTURE_TYPE_APPLICATION_INFO,
+        nullptr,
+        "Firestorm Vulkan Experimental",
+        VK_MAKE_API_VERSION(0, 0, 3, 0),
+        "Firestorm Vulkan Renderer",
+        VK_MAKE_API_VERSION(0, 0, 3, 0),
+        VK_API_VERSION_1_2
+    };
+    const VkInstanceCreateInfo instance_info{
+        VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
+        nullptr,
+        0,
+        &app_info,
+        static_cast<std::uint32_t>(instance_extensions.size()),
+        instance_extensions.empty() ? nullptr : instance_extensions.data(),
+        0,
+        nullptr
+    };
+    VkResult status = vkCreateInstance(&instance_info, nullptr, &mImpl->instance);
+    if (status != VK_SUCCESS)
+    {
+        mImpl->error = "vkCreateInstance failed with code " + std::to_string(status);
+        shutdown();
+        return false;
+    }
+
+    std::uint32_t physical_count = 0;
+    status = vkEnumeratePhysicalDevices(mImpl->instance, &physical_count, nullptr);
+    if (status != VK_SUCCESS || physical_count == 0)
+    {
+        mImpl->error = "No Vulkan physical devices were reported";
+        shutdown();
+        return false;
+    }
+    std::vector<VkPhysicalDevice> physical_devices(physical_count);
+    status = vkEnumeratePhysicalDevices(mImpl->instance, &physical_count,
+                                        physical_devices.data());
+    if (status != VK_SUCCESS)
+    {
+        mImpl->error = "vkEnumeratePhysicalDevices failed";
+        shutdown();
+        return false;
+    }
+
+    // Prefer a discrete GPU, then an integrated GPU, then any graphics-capable
+    // Vulkan device. This ordering is deterministic and will later be exposed
+    // as a user-selectable adapter preference.
+    int best_score = -1;
+    for (VkPhysicalDevice candidate : physical_devices)
+    {
+        VkPhysicalDeviceProperties properties{};
+        vkGetPhysicalDeviceProperties(candidate, &properties);
+
+        std::uint32_t queue_count = 0;
+        vkGetPhysicalDeviceQueueFamilyProperties(candidate, &queue_count, nullptr);
+        std::vector<VkQueueFamilyProperties> queues(queue_count);
+        vkGetPhysicalDeviceQueueFamilyProperties(candidate, &queue_count, queues.data());
+        for (std::uint32_t queue_index = 0; queue_index < queue_count; ++queue_index)
+        {
+            if ((queues[queue_index].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0)
+            {
+                continue;
+            }
+            int score = 1;
+            if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
+            {
+                score = 2;
+            }
+            else if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
+            {
+                score = 3;
+            }
+            if (score > best_score)
+            {
+                best_score = score;
+                mImpl->physical_device = candidate;
+                mImpl->graphics_queue_family = queue_index;
+                mImpl->device_name = properties.deviceName;
+            }
+            break;
+        }
+    }
+    if (mImpl->physical_device == VK_NULL_HANDLE)
+    {
+        mImpl->error = "No Vulkan graphics queue was found";
+        shutdown();
+        return false;
+    }
+
+    constexpr float queue_priority = 1.0f;
+    VkPhysicalDeviceFeatures supported_features{};
+    vkGetPhysicalDeviceFeatures(mImpl->physical_device, &supported_features);
+    VkPhysicalDeviceFeatures enabled_features{};
+    enabled_features.samplerAnisotropy = supported_features.samplerAnisotropy;
+    mImpl->sampler_anisotropy = supported_features.samplerAnisotropy == VK_TRUE;
+    VkPhysicalDeviceProperties selected_properties{};
+    vkGetPhysicalDeviceProperties(mImpl->physical_device, &selected_properties);
+    mImpl->max_sampler_anisotropy = mImpl->sampler_anisotropy ?
+        std::min(16.f, selected_properties.limits.maxSamplerAnisotropy) : 1.f;
+    const VkDeviceQueueCreateInfo queue_info{
+        VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
+        nullptr,
+        0,
+        mImpl->graphics_queue_family,
+        1,
+        &queue_priority
+    };
+#if LL_WINDOWS
+    constexpr const char* device_extensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
+#endif
+    const VkDeviceCreateInfo device_info{
+        VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
+        nullptr,
+        0,
+        1,
+        &queue_info,
+#if LL_WINDOWS
+        1,
+        device_extensions,
+#else
+        0,
+        nullptr,
+#endif
+        0,
+        nullptr,
+        &enabled_features
+    };
+    status = vkCreateDevice(mImpl->physical_device, &device_info, nullptr,
+                            &mImpl->device);
+    if (status != VK_SUCCESS)
+    {
+        mImpl->error = "vkCreateDevice failed with code " + std::to_string(status);
+        shutdown();
+        return false;
+    }
+    vkGetDeviceQueue(mImpl->device, mImpl->graphics_queue_family, 0,
+                     &mImpl->graphics_queue);
+    mImpl->ready = mImpl->graphics_queue != VK_NULL_HANDLE;
+    if (!mImpl->ready)
+    {
+        mImpl->error = "Vulkan graphics queue creation failed";
+        shutdown();
+    }
+    return mImpl->ready;
+#endif
+}
+
+bool LLVulkanContext::attachWindow(void* native_window, unsigned width, unsigned height)
+{
+    mImpl->error.clear();
+#if !(FIRESTORM_VULKAN_EXPERIMENTAL && LL_WINDOWS)
+    (void)native_window;
+    (void)width;
+    (void)height;
+    mImpl->error = "Win32 Vulkan presentation support is not compiled";
+    return false;
+#else
+    if (!mImpl->ready || !native_window)
+    {
+        mImpl->error = "A Vulkan device and valid Win32 window are required";
+        return false;
+    }
+    detachWindow();
+
+    const VkWin32SurfaceCreateInfoKHR surface_info{
+        VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
+        nullptr,
+        0,
+        GetModuleHandleW(nullptr),
+        static_cast<HWND>(native_window)
+    };
+    VkResult status = vkCreateWin32SurfaceKHR(mImpl->instance, &surface_info,
+                                               nullptr, &mImpl->surface);
+    if (status != VK_SUCCESS)
+    {
+        mImpl->error = "vkCreateWin32SurfaceKHR failed with code " +
+                       std::to_string(status);
+        return false;
+    }
+
+    VkBool32 presentation_supported = VK_FALSE;
+    status = vkGetPhysicalDeviceSurfaceSupportKHR(
+        mImpl->physical_device, mImpl->graphics_queue_family, mImpl->surface,
+        &presentation_supported);
+    if (status != VK_SUCCESS || presentation_supported != VK_TRUE)
+    {
+        mImpl->error = "Selected graphics queue cannot present to the Win32 surface";
+        detachWindow();
+        return false;
+    }
+
+    VkSurfaceCapabilitiesKHR capabilities{};
+    status = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
+        mImpl->physical_device, mImpl->surface, &capabilities);
+    if (status != VK_SUCCESS)
+    {
+        mImpl->error = "Unable to query Vulkan surface capabilities";
+        detachWindow();
+        return false;
+    }
+
+    std::uint32_t format_count = 0;
+    vkGetPhysicalDeviceSurfaceFormatsKHR(mImpl->physical_device, mImpl->surface,
+                                         &format_count, nullptr);
+    if (format_count == 0)
+    {
+        mImpl->error = "The Vulkan surface reported no formats";
+        detachWindow();
+        return false;
+    }
+    std::vector<VkSurfaceFormatKHR> formats(format_count);
+    vkGetPhysicalDeviceSurfaceFormatsKHR(mImpl->physical_device, mImpl->surface,
+                                         &format_count, formats.data());
+    VkSurfaceFormatKHR chosen_format = formats.front();
+    for (const VkSurfaceFormatKHR& candidate : formats)
+    {
+        if (candidate.format == VK_FORMAT_B8G8R8A8_SRGB &&
+            candidate.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
+        {
+            chosen_format = candidate;
+            break;
+        }
+    }
+
+    VkExtent2D extent = capabilities.currentExtent;
+    if (extent.width == std::numeric_limits<std::uint32_t>::max())
+    {
+        extent.width = std::max(capabilities.minImageExtent.width,
+            std::min(capabilities.maxImageExtent.width,
+                     static_cast<std::uint32_t>(width)));
+        extent.height = std::max(capabilities.minImageExtent.height,
+            std::min(capabilities.maxImageExtent.height,
+                     static_cast<std::uint32_t>(height)));
+    }
+    std::uint32_t image_count = capabilities.minImageCount + 1;
+    if (capabilities.maxImageCount > 0)
+    {
+        image_count = std::min(image_count, capabilities.maxImageCount);
+    }
+
+    VkCompositeAlphaFlagBitsKHR composite_alpha =
+        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
+    if ((capabilities.supportedCompositeAlpha & composite_alpha) == 0)
+    {
+        constexpr VkCompositeAlphaFlagBitsKHR candidates[] = {
+            VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
+            VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
+            VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR
+        };
+        for (VkCompositeAlphaFlagBitsKHR candidate : candidates)
+        {
+            if ((capabilities.supportedCompositeAlpha & candidate) != 0)
+            {
+                composite_alpha = candidate;
+                break;
+            }
+        }
+    }
+
+    const VkSwapchainCreateInfoKHR swapchain_info{
+        VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
+        nullptr,
+        0,
+        mImpl->surface,
+        image_count,
+        chosen_format.format,
+        chosen_format.colorSpace,
+        extent,
+        1,
+        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
+        VK_SHARING_MODE_EXCLUSIVE,
+        0,
+        nullptr,
+        capabilities.currentTransform,
+        composite_alpha,
+        VK_PRESENT_MODE_FIFO_KHR,
+        VK_TRUE,
+        VK_NULL_HANDLE
+    };
+    status = vkCreateSwapchainKHR(mImpl->device, &swapchain_info, nullptr,
+                                  &mImpl->swapchain);
+    if (status != VK_SUCCESS)
+    {
+        mImpl->error = "vkCreateSwapchainKHR failed with code " +
+                       std::to_string(status);
+        detachWindow();
+        return false;
+    }
+    mImpl->swapchain_format = chosen_format.format;
+    vkGetSwapchainImagesKHR(mImpl->device, mImpl->swapchain,
+                            &mImpl->swapchain_image_count, nullptr);
+    mImpl->swapchain_images.resize(mImpl->swapchain_image_count);
+    status = vkGetSwapchainImagesKHR(mImpl->device, mImpl->swapchain,
+                                     &mImpl->swapchain_image_count,
+                                     mImpl->swapchain_images.data());
+    if (status != VK_SUCCESS)
+    {
+        mImpl->error = "Unable to retrieve Vulkan swapchain images";
+        detachWindow();
+        return false;
+    }
+
+    mImpl->swapchain_image_views.reserve(mImpl->swapchain_images.size());
+    for (VkImage image : mImpl->swapchain_images)
+    {
+        const VkImageViewCreateInfo view_info{
+            VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
+            nullptr,
+            0,
+            image,
+            VK_IMAGE_VIEW_TYPE_2D,
+            chosen_format.format,
+            {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
+             VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY},
+            {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}
+        };
+        VkImageView view = VK_NULL_HANDLE;
+        status = vkCreateImageView(mImpl->device, &view_info, nullptr, &view);
+        if (status != VK_SUCCESS)
+        {
+            mImpl->error = "vkCreateImageView failed with code " +
+                           std::to_string(status);
+            detachWindow();
+            return false;
+        }
+        mImpl->swapchain_image_views.push_back(view);
+    }
+
+    mImpl->depth_format = selectDepthFormat(mImpl->physical_device);
+    if (mImpl->depth_format == VK_FORMAT_UNDEFINED)
+    {
+        mImpl->error = "No supported Vulkan depth attachment format";
+        detachWindow();
+        return false;
+    }
+    for (std::size_t index = 0; index < mImpl->swapchain_images.size(); ++index)
+    {
+        const VkImageCreateInfo image_info{
+            VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, nullptr, 0,
+            VK_IMAGE_TYPE_2D, mImpl->depth_format,
+            {extent.width, extent.height, 1}, 1, 1, VK_SAMPLE_COUNT_1_BIT,
+            VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
+            VK_SHARING_MODE_EXCLUSIVE, 0, nullptr,
+            VK_IMAGE_LAYOUT_UNDEFINED};
+        VkImage image = VK_NULL_HANDLE;
+        if (vkCreateImage(mImpl->device, &image_info, nullptr, &image) != VK_SUCCESS)
+        {
+            mImpl->error = "Unable to create Vulkan depth image";
+            detachWindow();
+            return false;
+        }
+        mImpl->depth_images.push_back(image);
+        VkMemoryRequirements requirements{};
+        vkGetImageMemoryRequirements(mImpl->device, image, &requirements);
+        std::uint32_t memory_type = 0;
+        if (!selectDeviceMemoryType(mImpl->physical_device,
+                                    requirements.memoryTypeBits,
+                                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
+                                    memory_type))
+        {
+            mImpl->error = "No device-local memory type for Vulkan depth image";
+            detachWindow();
+            return false;
+        }
+        const VkMemoryAllocateInfo allocation_info{
+            VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, nullptr,
+            requirements.size, memory_type};
+        VkDeviceMemory memory = VK_NULL_HANDLE;
+        if (vkAllocateMemory(mImpl->device, &allocation_info, nullptr,
+                             &memory) != VK_SUCCESS ||
+            vkBindImageMemory(mImpl->device, image, memory, 0) != VK_SUCCESS)
+        {
+            if (memory != VK_NULL_HANDLE)
+                vkFreeMemory(mImpl->device, memory, nullptr);
+            mImpl->error = "Unable to allocate Vulkan depth memory";
+            detachWindow();
+            return false;
+        }
+        mImpl->depth_memory.push_back(memory);
+        const VkImageViewCreateInfo depth_view_info{
+            VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, nullptr, 0, image,
+            VK_IMAGE_VIEW_TYPE_2D, mImpl->depth_format,
+            {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
+             VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY},
+            {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1}};
+        VkImageView depth_view = VK_NULL_HANDLE;
+        if (vkCreateImageView(mImpl->device, &depth_view_info, nullptr,
+                              &depth_view) != VK_SUCCESS)
+        {
+            mImpl->error = "Unable to create Vulkan depth image view";
+            detachWindow();
+            return false;
+        }
+        mImpl->depth_image_views.push_back(depth_view);
+    }
+
+    const VkAttachmentDescription color_attachment{
+        0,
+        chosen_format.format,
+        VK_SAMPLE_COUNT_1_BIT,
+        VK_ATTACHMENT_LOAD_OP_CLEAR,
+        VK_ATTACHMENT_STORE_OP_STORE,
+        VK_ATTACHMENT_LOAD_OP_DONT_CARE,
+        VK_ATTACHMENT_STORE_OP_DONT_CARE,
+        VK_IMAGE_LAYOUT_UNDEFINED,
+        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
+    };
+    const VkAttachmentDescription depth_attachment{
+        0, mImpl->depth_format, VK_SAMPLE_COUNT_1_BIT,
+        VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_DONT_CARE,
+        VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
+        VK_IMAGE_LAYOUT_UNDEFINED,
+        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
+    const VkAttachmentDescription attachments[] = {
+        color_attachment, depth_attachment};
+    const VkAttachmentReference color_reference{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
+    const VkAttachmentReference depth_reference{
+        1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
+    const VkSubpassDescription subpass{
+        0,
+        VK_PIPELINE_BIND_POINT_GRAPHICS,
+        0,
+        nullptr,
+        1,
+        &color_reference,
+        nullptr,
+        &depth_reference,
+        0,
+        nullptr
+    };
+    const VkSubpassDependency dependency{
+        VK_SUBPASS_EXTERNAL,
+        0,
+        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
+            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
+        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
+            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
+        0,
+        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
+            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
+        0
+    };
+    const VkRenderPassCreateInfo render_pass_info{
+        VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
+        nullptr,
+        0,
+        2,
+        attachments,
+        1,
+        &subpass,
+        1,
+        &dependency
+    };
+    status = vkCreateRenderPass(mImpl->device, &render_pass_info, nullptr,
+                                &mImpl->render_pass);
+    if (status != VK_SUCCESS)
+    {
+        mImpl->error = "vkCreateRenderPass failed with code " +
+                       std::to_string(status);
+        detachWindow();
+        return false;
+    }
+
+    const VkAttachmentDescription shadow_attachment{
+        0, mImpl->depth_format, VK_SAMPLE_COUNT_1_BIT,
+        VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
+        VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
+        VK_IMAGE_LAYOUT_UNDEFINED,
+        VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL};
+    const VkAttachmentReference shadow_reference{
+        0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
+    const VkSubpassDescription shadow_subpass{
+        0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr,
+        nullptr, &shadow_reference, 0, nullptr};
+    const VkSubpassDependency shadow_dependencies[] = {
+        {VK_SUBPASS_EXTERNAL, 0,
+         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
+         VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
+         VK_ACCESS_SHADER_READ_BIT,
+         VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
+         VK_DEPENDENCY_BY_REGION_BIT},
+        {0, VK_SUBPASS_EXTERNAL,
+         VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
+         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
+         VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
+         VK_ACCESS_SHADER_READ_BIT,
+         VK_DEPENDENCY_BY_REGION_BIT}};
+    const VkRenderPassCreateInfo shadow_pass_info{
+        VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0,
+        1, &shadow_attachment, 1, &shadow_subpass, 2, shadow_dependencies};
+    if (vkCreateRenderPass(mImpl->device, &shadow_pass_info, nullptr,
+                           &mImpl->shadow_render_pass) != VK_SUCCESS)
+    {
+        mImpl->error = "Unable to create Vulkan shadow render pass";
+        detachWindow();
+        return false;
+    }
+    const VkImageCreateInfo shadow_image_info{
+        VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, nullptr, 0, VK_IMAGE_TYPE_2D,
+        mImpl->depth_format,
+        {Impl::shadow_extent, Impl::shadow_extent, 1}, 1, 1,
+        VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL,
+        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
+        VK_SHARING_MODE_EXCLUSIVE, 0, nullptr, VK_IMAGE_LAYOUT_UNDEFINED};
+    if (vkCreateImage(mImpl->device, &shadow_image_info, nullptr,
+                      &mImpl->shadow_image) != VK_SUCCESS)
+    {
+        mImpl->error = "Unable to create Vulkan shadow depth image";
+        detachWindow();
+        return false;
+    }
+    VkMemoryRequirements shadow_requirements{};
+    vkGetImageMemoryRequirements(mImpl->device, mImpl->shadow_image,
+                                 &shadow_requirements);
+    std::uint32_t shadow_memory_type = 0;
+    if (!selectDeviceMemoryType(mImpl->physical_device,
+                                shadow_requirements.memoryTypeBits,
+                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
+                                shadow_memory_type))
+    {
+        mImpl->error = "No device-local memory type for Vulkan shadow image";
+        detachWindow();
+        return false;
+    }
+    const VkMemoryAllocateInfo shadow_allocation{
+        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, nullptr,
+        shadow_requirements.size, shadow_memory_type};
+    if (vkAllocateMemory(mImpl->device, &shadow_allocation, nullptr,
+                         &mImpl->shadow_memory) != VK_SUCCESS ||
+        vkBindImageMemory(mImpl->device, mImpl->shadow_image,
+                          mImpl->shadow_memory, 0) != VK_SUCCESS)
+    {
+        mImpl->error = "Unable to allocate Vulkan shadow depth memory";
+        detachWindow();
+        return false;
+    }
+    const VkImageViewCreateInfo shadow_view_info{
+        VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, nullptr, 0,
+        mImpl->shadow_image, VK_IMAGE_VIEW_TYPE_2D, mImpl->depth_format,
+        {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
+         VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY},
+        {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1}};
+    if (vkCreateImageView(mImpl->device, &shadow_view_info, nullptr,
+                          &mImpl->shadow_view) != VK_SUCCESS)
+    {
+        mImpl->error = "Unable to create Vulkan shadow depth view";
+        detachWindow();
+        return false;
+    }
+    const VkFramebufferCreateInfo shadow_framebuffer_info{
+        VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0,
+        mImpl->shadow_render_pass, 1, &mImpl->shadow_view,
+        Impl::shadow_extent, Impl::shadow_extent, 1};
+    if (vkCreateFramebuffer(mImpl->device, &shadow_framebuffer_info, nullptr,
+                            &mImpl->shadow_framebuffer) != VK_SUCCESS)
+    {
+        mImpl->error = "Unable to create Vulkan shadow framebuffer";
+        detachWindow();
+        return false;
+    }
+
+    const VkShaderModuleCreateInfo vertex_shader_info{
+        VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, nullptr, 0,
+        sizeof(LLVulkanValidationShaders::validation_vertex_spirv),
+        LLVulkanValidationShaders::validation_vertex_spirv};
+    status = vkCreateShaderModule(mImpl->device, &vertex_shader_info, nullptr,
+                                  &mImpl->validation_vertex_shader);
+    if (status != VK_SUCCESS)
+    {
+        mImpl->error = "Unable to create validation vertex shader module";
+        detachWindow();
+        return false;
+    }
+    const VkShaderModuleCreateInfo fragment_shader_info{
+        VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, nullptr, 0,
+        sizeof(LLVulkanValidationShaders::validation_fragment_spirv),
+        LLVulkanValidationShaders::validation_fragment_spirv};
+    status = vkCreateShaderModule(mImpl->device, &fragment_shader_info, nullptr,
+                                  &mImpl->validation_fragment_shader);
+    if (status != VK_SUCCESS)
+    {
+        mImpl->error = "Unable to create validation fragment shader module";
+        detachWindow();
+        return false;
+    }
+
+    const VkPipelineShaderStageCreateInfo shader_stages[] = {
+        {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0,
+         VK_SHADER_STAGE_VERTEX_BIT, mImpl->validation_vertex_shader, "main", nullptr},
+        {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0,
+         VK_SHADER_STAGE_FRAGMENT_BIT, mImpl->validation_fragment_shader, "main", nullptr}
+    };
+    const VkVertexInputBindingDescription vertex_bindings[] = {
+        {0, sizeof(ValidationVertex), VK_VERTEX_INPUT_RATE_VERTEX},
+        {1, sizeof(DrawInstanceTransforms), VK_VERTEX_INPUT_RATE_INSTANCE}};
+    const VkVertexInputAttributeDescription vertex_attributes[] = {
+        {0, 0, VK_FORMAT_R32G32_SFLOAT, 0},
+        {1, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 2},
+        {2, 0, VK_FORMAT_R32G32_SFLOAT, sizeof(float) * 5},
+        {3, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 7},
+        {4, 0, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 10},
+        {5, 0, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 14},
+        {6, 0, VK_FORMAT_R16G16B16A16_UINT, sizeof(float) * 18},
+        {7, 0, VK_FORMAT_R32_SFLOAT, sizeof(float) * 20},
+        {8, 1, VK_FORMAT_R32G32B32A32_SFLOAT, 0},
+        {9, 1, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 4},
+        {10, 1, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 8},
+        {11, 1, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 12}
+        ,{12, 1, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 16}
+        ,{13, 1, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 20}
+        ,{14, 1, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 24}
+        ,{15, 1, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 28}
+    };
+    const VkPipelineVertexInputStateCreateInfo vertex_input{
+        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
+        nullptr, 0, 2, vertex_bindings, 16, vertex_attributes};
+    const VkPipelineInputAssemblyStateCreateInfo input_assembly{
+        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
+        nullptr, 0, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE};
+    const VkPipelineViewportStateCreateInfo viewport_state{
+        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
+        nullptr, 0, 1, nullptr, 1, nullptr};
+    const VkPipelineRasterizationStateCreateInfo rasterization{
+        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
+        nullptr, 0, VK_FALSE, VK_FALSE, VK_POLYGON_MODE_FILL,
+        VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE,
+        VK_FALSE, 0.f, 0.f, 0.f, 1.f};
+    const VkPipelineMultisampleStateCreateInfo multisample{
+        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
+        nullptr, 0, VK_SAMPLE_COUNT_1_BIT, VK_FALSE, 1.f,
+        nullptr, VK_FALSE, VK_FALSE};
+    const VkPipelineDepthStencilStateCreateInfo validation_depth{
+        VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
+        nullptr, 0, VK_FALSE, VK_FALSE, VK_COMPARE_OP_ALWAYS,
+        VK_FALSE, VK_FALSE, {}, {}, 0.f, 1.f};
+    const VkPipelineColorBlendAttachmentState blend_attachment{
+        VK_FALSE,
+        VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD,
+        VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD,
+        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
+        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT};
+    const VkPipelineColorBlendStateCreateInfo color_blend{
+        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
+        nullptr, 0, VK_FALSE, VK_LOGIC_OP_COPY, 1, &blend_attachment,
+        {0.f, 0.f, 0.f, 0.f}};
+    constexpr VkDynamicState dynamic_states[] = {
+        VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
+    const VkPipelineDynamicStateCreateInfo dynamic_state{
+        VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
+        nullptr, 0, 2, dynamic_states};
+    const VkPushConstantRange transform_push_constant{
+        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
+        0, sizeof(float) * 32};
+    const VkDescriptorSetLayoutBinding texture_binding{
+        0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
+        VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
+    const VkDescriptorSetLayoutCreateInfo descriptor_layout_info{
+        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, nullptr, 0,
+        1, &texture_binding};
+    status = vkCreateDescriptorSetLayout(
+        mImpl->device, &descriptor_layout_info, nullptr,
+        &mImpl->texture_descriptor_layout);
+    if (status != VK_SUCCESS)
+    {
+        mImpl->error = "Unable to create Vulkan texture descriptor layout";
+        detachWindow();
+        return false;
+    }
+    const VkDescriptorSetLayoutBinding skin_binding{
+        0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1,
+        VK_SHADER_STAGE_VERTEX_BIT, nullptr};
+    const VkDescriptorSetLayoutCreateInfo skin_layout_info{
+        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, nullptr, 0,
+        1, &skin_binding};
+    status = vkCreateDescriptorSetLayout(mImpl->device, &skin_layout_info,
+                                         nullptr,
+                                         &mImpl->skin_descriptor_layout);
+    if (status != VK_SUCCESS)
+    {
+        mImpl->error = "Unable to create Vulkan skin descriptor layout";
+        detachWindow();
+        return false;
+    }
+    const std::array<VkDescriptorSetLayout, 10> material_set_layouts{{
+        mImpl->texture_descriptor_layout,
+        mImpl->texture_descriptor_layout,
+        mImpl->texture_descriptor_layout,
+        mImpl->texture_descriptor_layout,
+        mImpl->texture_descriptor_layout,
+        mImpl->skin_descriptor_layout,
+        mImpl->texture_descriptor_layout,
+        mImpl->texture_descriptor_layout,
+        mImpl->texture_descriptor_layout,
+        mImpl->skin_descriptor_layout}};
+    const VkPipelineLayoutCreateInfo pipeline_layout_info{
+        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, nullptr, 0,
+        static_cast<std::uint32_t>(material_set_layouts.size()),
+        material_set_layouts.data(),
+        1, &transform_push_constant};
+    status = vkCreatePipelineLayout(mImpl->device, &pipeline_layout_info, nullptr,
+                                    &mImpl->validation_pipeline_layout);
+    if (status != VK_SUCCESS)
+    {
+        mImpl->error = "Unable to create validation pipeline layout";
+        detachWindow();
+        return false;
+    }
+    const VkGraphicsPipelineCreateInfo pipeline_info{
+        VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
+        nullptr, 0, 2, shader_stages, &vertex_input, &input_assembly, nullptr,
+        &viewport_state, &rasterization, &multisample, &validation_depth,
+        &color_blend,
+        &dynamic_state, mImpl->validation_pipeline_layout, mImpl->render_pass,
+        0, VK_NULL_HANDLE, -1};
+    status = vkCreateGraphicsPipelines(mImpl->device, VK_NULL_HANDLE, 1,
+                                       &pipeline_info, nullptr,
+                                       &mImpl->validation_pipeline);
+    if (status != VK_SUCCESS)
+    {
+        mImpl->error = "vkCreateGraphicsPipelines failed with code " +
+                       std::to_string(status);
+        detachWindow();
+        return false;
+    }
+
+    mImpl->framebuffers.reserve(mImpl->swapchain_image_views.size());
+    for (std::size_t index = 0;
+         index < mImpl->swapchain_image_views.size(); ++index)
+    {
+        const VkImageView framebuffer_attachments[] = {
+            mImpl->swapchain_image_views[index],
+            mImpl->depth_image_views[index]};
+        const VkFramebufferCreateInfo framebuffer_info{
+            VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
+            nullptr,
+            0,
+            mImpl->render_pass,
+            2,
+            framebuffer_attachments,
+            extent.width,
+            extent.height,
+            1
+        };
+        VkFramebuffer framebuffer = VK_NULL_HANDLE;
+        status = vkCreateFramebuffer(mImpl->device, &framebuffer_info, nullptr,
+                                     &framebuffer);
+        if (status != VK_SUCCESS)
+        {
+            mImpl->error = "vkCreateFramebuffer failed with code " +
+                           std::to_string(status);
+            detachWindow();
+            return false;
+        }
+        mImpl->framebuffers.push_back(framebuffer);
+    }
+
+    const VkCommandPoolCreateInfo command_pool_info{
+        VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
+        nullptr,
+        VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
+        mImpl->graphics_queue_family
+    };
+    status = vkCreateCommandPool(mImpl->device, &command_pool_info, nullptr,
+                                 &mImpl->command_pool);
+    if (status != VK_SUCCESS)
+    {
+        mImpl->error = "vkCreateCommandPool failed with code " +
+                       std::to_string(status);
+        detachWindow();
+        return false;
+    }
+
+    const VkDescriptorPoolSize descriptor_pool_sizes[] = {
+        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1027},
+        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 67}};
+    const VkDescriptorPoolCreateInfo descriptor_pool_info{
+        VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO, nullptr, 0,
+        1094, 2, descriptor_pool_sizes};
+    if (vkCreateDescriptorPool(mImpl->device, &descriptor_pool_info, nullptr,
+                               &mImpl->texture_descriptor_pool) != VK_SUCCESS)
+    {
+        mImpl->error = "Unable to create Vulkan texture descriptor pool";
+        detachWindow();
+        return false;
+    }
+
+    const VkSamplerCreateInfo shadow_sampler_info{
+        VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO, nullptr, 0,
+        VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST,
+        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
+        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
+        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, 0.f, VK_FALSE, 1.f,
+        VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL, 0.f, 0.f,
+        VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE, VK_FALSE};
+    if (vkCreateSampler(mImpl->device, &shadow_sampler_info, nullptr,
+                        &mImpl->shadow_sampler) != VK_SUCCESS)
+    {
+        mImpl->error = "Unable to create Vulkan shadow comparison sampler";
+        detachWindow();
+        return false;
+    }
+    const VkDescriptorSetAllocateInfo shadow_descriptor_allocate{
+        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, nullptr,
+        mImpl->texture_descriptor_pool, 1,
+        &mImpl->texture_descriptor_layout};
+    if (vkAllocateDescriptorSets(mImpl->device, &shadow_descriptor_allocate,
+                                 &mImpl->shadow_descriptor) != VK_SUCCESS)
+    {
+        mImpl->error = "Unable to allocate Vulkan shadow descriptor";
+        detachWindow();
+        return false;
+    }
+    const VkDescriptorImageInfo shadow_image_descriptor{
+        mImpl->shadow_sampler, mImpl->shadow_view,
+        VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL};
+    const VkWriteDescriptorSet shadow_descriptor_write{
+        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr,
+        mImpl->shadow_descriptor, 0, 0, 1,
+        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
+        &shadow_image_descriptor, nullptr, nullptr};
+    vkUpdateDescriptorSets(mImpl->device, 1, &shadow_descriptor_write,
+                           0, nullptr);
+
+    VkFormat probe_format = VK_FORMAT_R16G16B16A16_SFLOAT;
+    VkFormatProperties probe_properties{};
+    vkGetPhysicalDeviceFormatProperties(mImpl->physical_device, probe_format,
+                                         &probe_properties);
+    const VkFormatFeatureFlags required_probe_features =
+        VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT |
+        VK_FORMAT_FEATURE_TRANSFER_DST_BIT;
+    if ((probe_properties.optimalTilingFeatures & required_probe_features) !=
+        required_probe_features)
+    {
+        mImpl->error = "RGBA16F reflection probes are unsupported by this GPU";
+        detachWindow();
+        return false;
+    }
+
+    auto create_probe_array = [this, probe_format](
+        std::uint32_t size, std::uint32_t mip_levels,
+        Impl::ProbeArray& probe) -> bool
+    {
+        probe.mip_levels = mip_levels;
+        const VkImageCreateInfo image_info{
+            VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, nullptr,
+            VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT, VK_IMAGE_TYPE_2D,
+            probe_format, {size, size, 1}, mip_levels, probe.cube_count * 6,
+            VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL,
+            VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
+            VK_SHARING_MODE_EXCLUSIVE, 0, nullptr, VK_IMAGE_LAYOUT_UNDEFINED};
+        if (vkCreateImage(mImpl->device, &image_info, nullptr,
+                          &probe.image) != VK_SUCCESS)
+            return false;
+        VkMemoryRequirements requirements{};
+        vkGetImageMemoryRequirements(mImpl->device, probe.image, &requirements);
+        std::uint32_t memory_type = 0;
+        if (!selectDeviceMemoryType(mImpl->physical_device,
+                                    requirements.memoryTypeBits,
+                                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
+                                    memory_type))
+            return false;
+        const VkMemoryAllocateInfo allocation{
+            VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, nullptr,
+            requirements.size, memory_type};
+        if (vkAllocateMemory(mImpl->device, &allocation, nullptr,
+                             &probe.memory) != VK_SUCCESS ||
+            vkBindImageMemory(mImpl->device, probe.image,
+                              probe.memory, 0) != VK_SUCCESS)
+            return false;
+        const VkImageViewCreateInfo view_info{
+            VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, nullptr, 0,
+            probe.image, VK_IMAGE_VIEW_TYPE_CUBE_ARRAY, probe_format,
+            {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
+             VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY},
+            {VK_IMAGE_ASPECT_COLOR_BIT, 0, mip_levels, 0,
+             probe.cube_count * 6}};
+        return vkCreateImageView(mImpl->device, &view_info, nullptr,
+                                 &probe.view) == VK_SUCCESS;
+    };
+    if (!create_probe_array(128, 8, mImpl->radiance_probes) ||
+        !create_probe_array(32, 1, mImpl->irradiance_probes))
+    {
+        mImpl->error = "Unable to create Vulkan fallback probe arrays";
+        detachWindow();
+        return false;
+    }
+
+    VkCommandBuffer probe_upload = VK_NULL_HANDLE;
+    const VkCommandBufferAllocateInfo probe_upload_allocate{
+        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, nullptr,
+        mImpl->command_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1};
+    VkResult probe_status = vkAllocateCommandBuffers(
+        mImpl->device, &probe_upload_allocate, &probe_upload);
+    const VkCommandBufferBeginInfo probe_upload_begin{
+        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr,
+        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr};
+    if (probe_status == VK_SUCCESS)
+        probe_status = vkBeginCommandBuffer(probe_upload, &probe_upload_begin);
+    auto initialize_probe = [probe_upload](const Impl::ProbeArray& probe,
+                                           const VkClearColorValue& color)
+    {
+        const VkImageSubresourceRange range{
+            VK_IMAGE_ASPECT_COLOR_BIT, 0, probe.mip_levels, 0,
+            probe.cube_count * 6};
+        const VkImageMemoryBarrier to_transfer{
+            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, nullptr, 0,
+            VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
+            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
+            VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
+            probe.image, range};
+        vkCmdPipelineBarrier(probe_upload, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
+                             VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
+                             0, nullptr, 0, nullptr, 1, &to_transfer);
+        vkCmdClearColorImage(probe_upload, probe.image,
+                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
+                             &color, 1, &range);
+        const VkImageMemoryBarrier to_sample{
+            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, nullptr,
+            VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
+            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
+            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
+            VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
+            probe.image, range};
+        vkCmdPipelineBarrier(probe_upload, VK_PIPELINE_STAGE_TRANSFER_BIT,
+                             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
+                             0, nullptr, 0, nullptr, 1, &to_sample);
+    };
+    if (probe_status == VK_SUCCESS)
+    {
+        const VkClearColorValue radiance_clear{{0.08f, 0.12f, 0.18f, 1.f}};
+        const VkClearColorValue irradiance_clear{{0.18f, 0.20f, 0.24f, 1.f}};
+        initialize_probe(mImpl->radiance_probes, radiance_clear);
+        initialize_probe(mImpl->irradiance_probes, irradiance_clear);
+        probe_status = vkEndCommandBuffer(probe_upload);
+    }
+    if (probe_status == VK_SUCCESS)
+    {
+        const VkSubmitInfo submit{
+            VK_STRUCTURE_TYPE_SUBMIT_INFO, nullptr, 0, nullptr, nullptr,
+            1, &probe_upload, 0, nullptr};
+        probe_status = vkQueueSubmit(mImpl->graphics_queue, 1, &submit,
+                                     VK_NULL_HANDLE);
+    }
+    if (probe_status == VK_SUCCESS)
+        probe_status = vkQueueWaitIdle(mImpl->graphics_queue);
+    if (probe_upload != VK_NULL_HANDLE)
+        vkFreeCommandBuffers(mImpl->device, mImpl->command_pool, 1,
+                             &probe_upload);
+    if (probe_status != VK_SUCCESS)
+    {
+        mImpl->error = "Unable to initialize Vulkan fallback probe arrays";
+        detachWindow();
+        return false;
+    }
+
+    const VkSamplerCreateInfo probe_sampler_info{
+        VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO, nullptr, 0,
+        VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR,
+        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
+        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
+        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, 0.f, VK_FALSE, 1.f,
+        VK_FALSE, VK_COMPARE_OP_ALWAYS, 0.f, 7.f,
+        VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK, VK_FALSE};
+    if (vkCreateSampler(mImpl->device, &probe_sampler_info, nullptr,
+                        &mImpl->probe_sampler) != VK_SUCCESS)
+    {
+        mImpl->error = "Unable to create Vulkan probe sampler";
+        detachWindow();
+        return false;
+    }
+    const VkDescriptorSetLayout probe_layouts[] = {
+        mImpl->texture_descriptor_layout, mImpl->texture_descriptor_layout};
+    const VkDescriptorSetAllocateInfo probe_descriptor_allocate{
+        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, nullptr,
+        mImpl->texture_descriptor_pool, 2, probe_layouts};
+    VkDescriptorSet probe_sets[2]{};
+    if (vkAllocateDescriptorSets(mImpl->device, &probe_descriptor_allocate,
+                                 probe_sets) != VK_SUCCESS)
+    {
+        mImpl->error = "Unable to allocate Vulkan probe descriptors";
+        detachWindow();
+        return false;
+    }
+    mImpl->radiance_probe_descriptor = probe_sets[0];
+    mImpl->irradiance_probe_descriptor = probe_sets[1];
+    const VkDescriptorImageInfo probe_images[] = {
+        {mImpl->probe_sampler, mImpl->radiance_probes.view,
+         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
+        {mImpl->probe_sampler, mImpl->irradiance_probes.view,
+         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL}};
+    const VkWriteDescriptorSet probe_writes[] = {
+        {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, probe_sets[0],
+         0, 0, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
+         &probe_images[0], nullptr, nullptr},
+        {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, probe_sets[1],
+         0, 0, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
+         &probe_images[1], nullptr, nullptr}};
+    vkUpdateDescriptorSets(mImpl->device, 2, probe_writes, 0, nullptr);
+    const VkDescriptorSetLayout metadata_layouts[Impl::frames_in_flight] = {
+        mImpl->skin_descriptor_layout, mImpl->skin_descriptor_layout};
+    const VkDescriptorSetAllocateInfo metadata_allocate{
+        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, nullptr,
+        mImpl->texture_descriptor_pool,
+        static_cast<std::uint32_t>(Impl::frames_in_flight), metadata_layouts};
+    if (vkAllocateDescriptorSets(mImpl->device, &metadata_allocate,
+                                 mImpl->probe_metadata_descriptors) != VK_SUCCESS)
+    {
+        mImpl->error = "Unable to allocate Vulkan probe metadata descriptors";
+        detachWindow();
+        return false;
+    }
+
+    constexpr std::array<float, 12> identity_skin{{
+        1.f, 0.f, 0.f, 0.f,
+        0.f, 1.f, 0.f, 0.f,
+        0.f, 0.f, 1.f, 0.f}};
+    mImpl->identity_skin_buffer = std::make_unique<LLVulkanBuffer>(
+        mImpl->physical_device, mImpl->device);
+    if (!mImpl->identity_skin_buffer->uploadDeviceLocal(
+            identity_skin.data(), sizeof(identity_skin),
+            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
+            mImpl->command_pool, mImpl->graphics_queue))
+    {
+        mImpl->error = "Unable to upload Vulkan identity skin palette";
+        detachWindow();
+        return false;
+    }
+    const VkDescriptorSetAllocateInfo skin_allocate{
+        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, nullptr,
+        mImpl->texture_descriptor_pool, 1, &mImpl->skin_descriptor_layout};
+    if (vkAllocateDescriptorSets(mImpl->device, &skin_allocate,
+                                 &mImpl->identity_skin_descriptor) != VK_SUCCESS)
+    {
+        mImpl->error = "Unable to allocate Vulkan identity skin descriptor";
+        detachWindow();
+        return false;
+    }
+    const VkDescriptorBufferInfo identity_skin_info{
+        mImpl->identity_skin_buffer->handle(), 0, sizeof(identity_skin)};
+    const VkWriteDescriptorSet identity_skin_write{
+        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr,
+        mImpl->identity_skin_descriptor, 0, 0, 1,
+        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, nullptr,
+        &identity_skin_info, nullptr};
+    vkUpdateDescriptorSets(mImpl->device, 1, &identity_skin_write, 0, nullptr);
+
+    const VkImageCreateInfo fallback_image_info{
+        VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, nullptr, 0, VK_IMAGE_TYPE_2D,
+        VK_FORMAT_R8G8B8A8_UNORM, {1, 1, 1}, 1, 1,
+        VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL,
+        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
+        VK_SHARING_MODE_EXCLUSIVE, 0, nullptr, VK_IMAGE_LAYOUT_UNDEFINED};
+    if (vkCreateImage(mImpl->device, &fallback_image_info, nullptr,
+                      &mImpl->fallback_texture_image) != VK_SUCCESS)
+    {
+        mImpl->error = "Unable to create Vulkan fallback texture";
+        detachWindow();
+        return false;
+    }
+    VkMemoryRequirements fallback_requirements{};
+    vkGetImageMemoryRequirements(mImpl->device, mImpl->fallback_texture_image,
+                                 &fallback_requirements);
+    std::uint32_t fallback_memory_type = 0;
+    if (!selectDeviceMemoryType(mImpl->physical_device,
+                                fallback_requirements.memoryTypeBits,
+                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
+                                fallback_memory_type))
+    {
+        mImpl->error = "No memory type for Vulkan fallback texture";
+        detachWindow();
+        return false;
+    }
+    const VkMemoryAllocateInfo fallback_allocation{
+        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, nullptr,
+        fallback_requirements.size, fallback_memory_type};
+    if (vkAllocateMemory(mImpl->device, &fallback_allocation, nullptr,
+                         &mImpl->fallback_texture_memory) != VK_SUCCESS ||
+        vkBindImageMemory(mImpl->device, mImpl->fallback_texture_image,
+                          mImpl->fallback_texture_memory, 0) != VK_SUCCESS)
+    {
+        mImpl->error = "Unable to allocate Vulkan fallback texture";
+        detachWindow();
+        return false;
+    }
+    const VkImageViewCreateInfo fallback_view_info{
+        VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, nullptr, 0,
+        mImpl->fallback_texture_image, VK_IMAGE_VIEW_TYPE_2D,
+        VK_FORMAT_R8G8B8A8_UNORM,
+        {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
+         VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY},
+        {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}};
+    if (vkCreateImageView(mImpl->device, &fallback_view_info, nullptr,
+                          &mImpl->fallback_texture_view) != VK_SUCCESS)
+    {
+        mImpl->error = "Unable to create Vulkan fallback texture view";
+        detachWindow();
+        return false;
+    }
+
+    const std::uint32_t white_pixel = 0xffffffffu;
+    LLVulkanBuffer texture_staging(mImpl->physical_device, mImpl->device);
+    if (!texture_staging.allocate(sizeof(white_pixel),
+                                  VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
+                                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
+                                      VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) ||
+        !texture_staging.upload(&white_pixel, sizeof(white_pixel)))
+    {
+        mImpl->error = "Unable to stage Vulkan fallback texture";
+        detachWindow();
+        return false;
+    }
+    VkCommandBuffer upload_command = VK_NULL_HANDLE;
+    const VkCommandBufferAllocateInfo upload_allocate_info{
+        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, nullptr,
+        mImpl->command_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1};
+    status = vkAllocateCommandBuffers(mImpl->device, &upload_allocate_info,
+                                      &upload_command);
+    const VkCommandBufferBeginInfo upload_begin_info{
+        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr,
+        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr};
+    if (status == VK_SUCCESS)
+        status = vkBeginCommandBuffer(upload_command, &upload_begin_info);
+    if (status == VK_SUCCESS)
+    {
+        const VkImageMemoryBarrier to_transfer{
+            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, nullptr, 0,
+            VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
+            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_QUEUE_FAMILY_IGNORED,
+            VK_QUEUE_FAMILY_IGNORED, mImpl->fallback_texture_image,
+            {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}};
+        vkCmdPipelineBarrier(upload_command, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
+                             VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr,
+                             0, nullptr, 1, &to_transfer);
+        const VkBufferImageCopy copy{
+            0, 0, 0, {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
+            {0, 0, 0}, {1, 1, 1}};
+        vkCmdCopyBufferToImage(upload_command, texture_staging.handle(),
+                               mImpl->fallback_texture_image,
+                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);
+        const VkImageMemoryBarrier to_shader_read{
+            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, nullptr,
+            VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
+            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
+            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
+            VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
+            mImpl->fallback_texture_image,
+            {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}};
+        vkCmdPipelineBarrier(upload_command, VK_PIPELINE_STAGE_TRANSFER_BIT,
+                             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
+                             0, nullptr, 0, nullptr, 1, &to_shader_read);
+        status = vkEndCommandBuffer(upload_command);
+    }
+    if (status == VK_SUCCESS)
+    {
+        const VkSubmitInfo upload_submit{
+            VK_STRUCTURE_TYPE_SUBMIT_INFO, nullptr, 0, nullptr, nullptr,
+            1, &upload_command, 0, nullptr};
+        status = vkQueueSubmit(mImpl->graphics_queue, 1, &upload_submit,
+                               VK_NULL_HANDLE);
+    }
+    if (status == VK_SUCCESS)
+        status = vkQueueWaitIdle(mImpl->graphics_queue);
+    if (upload_command != VK_NULL_HANDLE)
+        vkFreeCommandBuffers(mImpl->device, mImpl->command_pool, 1,
+                             &upload_command);
+    if (status != VK_SUCCESS)
+    {
+        mImpl->error = "Unable to upload Vulkan fallback texture";
+        detachWindow();
+        return false;
+    }
+
+    const LLVulkanTextureBinding default_binding{};
+    const std::uint64_t default_sampler_key =
+        static_cast<std::uint64_t>(default_binding.filter) << 1;
+    const VkSamplerCreateInfo default_sampler_info{
+        VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO, nullptr, 0,
+        VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST,
+        VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT,
+        VK_SAMPLER_ADDRESS_MODE_REPEAT, 0.f, VK_FALSE, 1.f, VK_FALSE,
+        VK_COMPARE_OP_ALWAYS, 0.f, 0.f,
+        VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK, VK_FALSE};
+    VkSampler default_sampler = VK_NULL_HANDLE;
+    if (vkCreateSampler(mImpl->device, &default_sampler_info, nullptr,
+                        &default_sampler) != VK_SUCCESS)
+    {
+        mImpl->error = "Unable to create Vulkan fallback sampler";
+        detachWindow();
+        return false;
+    }
+    mImpl->sampler_cache.emplace(default_sampler_key, default_sampler);
+    VkDescriptorSet default_descriptor = VK_NULL_HANDLE;
+    const VkDescriptorSetAllocateInfo default_descriptor_allocate{
+        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, nullptr,
+        mImpl->texture_descriptor_pool, 1, &mImpl->texture_descriptor_layout};
+    if (vkAllocateDescriptorSets(mImpl->device, &default_descriptor_allocate,
+                                 &default_descriptor) != VK_SUCCESS)
+    {
+        mImpl->error = "Unable to allocate Vulkan fallback descriptor";
+        detachWindow();
+        return false;
+    }
+    const VkDescriptorImageInfo default_image_info{
+        default_sampler, mImpl->fallback_texture_view,
+        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
+    const VkWriteDescriptorSet default_write{
+        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, default_descriptor,
+        0, 0, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
+        &default_image_info, nullptr, nullptr};
+    vkUpdateDescriptorSets(mImpl->device, 1, &default_write, 0, nullptr);
+    mImpl->descriptor_cache.emplace(
+        textureDescriptorKey(default_binding),
+        Impl::DescriptorRecord{default_descriptor, default_sampler_key});
+
+    mImpl->command_buffers.resize(mImpl->framebuffers.size());
+    const VkCommandBufferAllocateInfo command_buffer_info{
+        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
+        nullptr,
+        mImpl->command_pool,
+        VK_COMMAND_BUFFER_LEVEL_PRIMARY,
+        static_cast<std::uint32_t>(mImpl->command_buffers.size())
+    };
+    status = vkAllocateCommandBuffers(mImpl->device, &command_buffer_info,
+                                      mImpl->command_buffers.data());
+    if (status != VK_SUCCESS)
+    {
+        mImpl->error = "vkAllocateCommandBuffers failed with code " +
+                       std::to_string(status);
+        detachWindow();
+        return false;
+    }
+
+    const VkDeviceSize vertex_bytes = sizeof(validation_vertices);
+    const VkDeviceSize index_bytes = sizeof(validation_indices);
+    mImpl->validation_vertex_buffer =
+        std::make_unique<LLVulkanBuffer>(mImpl->physical_device, mImpl->device);
+    mImpl->validation_index_buffer =
+        std::make_unique<LLVulkanBuffer>(mImpl->physical_device, mImpl->device);
+    if (!mImpl->validation_vertex_buffer->uploadDeviceLocal(
+            validation_vertices.data(), vertex_bytes,
+            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
+            mImpl->command_pool, mImpl->graphics_queue) ||
+        !mImpl->validation_index_buffer->uploadDeviceLocal(
+            validation_indices.data(), index_bytes,
+            VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
+            mImpl->command_pool, mImpl->graphics_queue))
+    {
+        mImpl->error = "Unable to upload Vulkan validation mesh";
+        detachWindow();
+        return false;
+    }
+
+    const VkSemaphoreCreateInfo semaphore_info{
+        VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, nullptr, 0};
+    const VkFenceCreateInfo fence_info{
+        VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, nullptr, VK_FENCE_CREATE_SIGNALED_BIT};
+    for (std::size_t frame = 0; frame < Impl::frames_in_flight; ++frame)
+    {
+        if (vkCreateSemaphore(mImpl->device, &semaphore_info, nullptr,
+                              &mImpl->image_available[frame]) != VK_SUCCESS ||
+            vkCreateSemaphore(mImpl->device, &semaphore_info, nullptr,
+                              &mImpl->render_finished[frame]) != VK_SUCCESS ||
+            vkCreateFence(mImpl->device, &fence_info, nullptr,
+                          &mImpl->frame_fences[frame]) != VK_SUCCESS)
+        {
+            mImpl->error = "Unable to create Vulkan frame synchronization objects";
+            detachWindow();
+            return false;
+        }
+    }
+    mImpl->swapchain_width = extent.width;
+    mImpl->swapchain_height = extent.height;
+    return true;
+#endif
+}
+
+void LLVulkanContext::detachWindow()
+{
+#if FIRESTORM_VULKAN_EXPERIMENTAL && LL_WINDOWS
+    if (mImpl->device != VK_NULL_HANDLE)
+    {
+        vkDeviceWaitIdle(mImpl->device);
+    }
+    mImpl->mesh_cache.clear();
+    mImpl->draw_queue.clear();
+    mImpl->pipeline_keys.clear();
+    for (const auto& entry : mImpl->pipeline_cache)
+        vkDestroyPipeline(mImpl->device, entry.second, nullptr);
+    mImpl->pipeline_cache.clear();
+    for (std::size_t frame = 0; frame < Impl::frames_in_flight; ++frame)
+    {
+        if (mImpl->frame_fences[frame] != VK_NULL_HANDLE)
+        {
+            vkDestroyFence(mImpl->device, mImpl->frame_fences[frame], nullptr);
+            mImpl->frame_fences[frame] = VK_NULL_HANDLE;
+        }
+        if (mImpl->render_finished[frame] != VK_NULL_HANDLE)
+        {
+            vkDestroySemaphore(mImpl->device, mImpl->render_finished[frame], nullptr);
+            mImpl->render_finished[frame] = VK_NULL_HANDLE;
+        }
+        if (mImpl->image_available[frame] != VK_NULL_HANDLE)
+        {
+            vkDestroySemaphore(mImpl->device, mImpl->image_available[frame], nullptr);
+            mImpl->image_available[frame] = VK_NULL_HANDLE;
+        }
+    }
+    mImpl->descriptor_cache.clear();
+    mImpl->skin_cache.clear();
+    mImpl->identity_skin_buffer.reset();
+    mImpl->identity_skin_descriptor = VK_NULL_HANDLE;
+    for (auto& buffer : mImpl->shadow_transform_buffers)
+        buffer.reset();
+    for (auto& buffer : mImpl->probe_metadata_buffers)
+        buffer.reset();
+    for (auto& descriptor : mImpl->probe_metadata_descriptors)
+        descriptor = VK_NULL_HANDLE;
+    {
+        std::lock_guard<std::mutex> lock(mImpl->pending_probe_mutex);
+        mImpl->pending_probe_metadata.clear();
+    }
+    {
+        std::lock_guard<std::mutex> lock(mImpl->pending_probe_face_mutex);
+        mImpl->pending_probe_faces.clear();
+    }
+    mImpl->shadow_descriptor = VK_NULL_HANDLE;
+    mImpl->radiance_probe_descriptor = VK_NULL_HANDLE;
+    mImpl->irradiance_probe_descriptor = VK_NULL_HANDLE;
+    if (mImpl->texture_descriptor_pool != VK_NULL_HANDLE)
+    {
+        vkDestroyDescriptorPool(mImpl->device,
+                                mImpl->texture_descriptor_pool, nullptr);
+        mImpl->texture_descriptor_pool = VK_NULL_HANDLE;
+    }
+    for (const auto& entry : mImpl->sampler_cache)
+        vkDestroySampler(mImpl->device, entry.second, nullptr);
+    mImpl->sampler_cache.clear();
+    if (mImpl->shadow_sampler != VK_NULL_HANDLE)
+    {
+        vkDestroySampler(mImpl->device, mImpl->shadow_sampler, nullptr);
+        mImpl->shadow_sampler = VK_NULL_HANDLE;
+    }
+    if (mImpl->probe_sampler != VK_NULL_HANDLE)
+    {
+        vkDestroySampler(mImpl->device, mImpl->probe_sampler, nullptr);
+        mImpl->probe_sampler = VK_NULL_HANDLE;
+    }
+    auto destroy_probe = [this](Impl::ProbeArray& probe)
+    {
+        if (probe.view != VK_NULL_HANDLE)
+            vkDestroyImageView(mImpl->device, probe.view, nullptr);
+        if (probe.image != VK_NULL_HANDLE)
+            vkDestroyImage(mImpl->device, probe.image, nullptr);
+        if (probe.memory != VK_NULL_HANDLE)
+            vkFreeMemory(mImpl->device, probe.memory, nullptr);
+        probe = {};
+    };
+    destroy_probe(mImpl->radiance_probes);
+    destroy_probe(mImpl->irradiance_probes);
+    for (const auto& entry : mImpl->texture_cache)
+    {
+        vkDestroyImageView(mImpl->device, entry.second.view, nullptr);
+        vkDestroyImage(mImpl->device, entry.second.image, nullptr);
+        vkFreeMemory(mImpl->device, entry.second.memory, nullptr);
+    }
+    mImpl->texture_cache.clear();
+    {
+        std::lock_guard<std::mutex> lock(mImpl->pending_texture_mutex);
+        mImpl->pending_textures.clear();
+    }
+    if (mImpl->fallback_texture_view != VK_NULL_HANDLE)
+    {
+        vkDestroyImageView(mImpl->device, mImpl->fallback_texture_view, nullptr);
+        mImpl->fallback_texture_view = VK_NULL_HANDLE;
+    }
+    if (mImpl->fallback_texture_image != VK_NULL_HANDLE)
+    {
+        vkDestroyImage(mImpl->device, mImpl->fallback_texture_image, nullptr);
+        mImpl->fallback_texture_image = VK_NULL_HANDLE;
+    }
+    if (mImpl->fallback_texture_memory != VK_NULL_HANDLE)
+    {
+        vkFreeMemory(mImpl->device, mImpl->fallback_texture_memory, nullptr);
+        mImpl->fallback_texture_memory = VK_NULL_HANDLE;
+    }
+    if (mImpl->command_pool != VK_NULL_HANDLE)
+    {
+        vkDestroyCommandPool(mImpl->device, mImpl->command_pool, nullptr);
+        mImpl->command_pool = VK_NULL_HANDLE;
+    }
+    mImpl->command_buffers.clear();
+    mImpl->validation_index_buffer.reset();
+    mImpl->validation_vertex_buffer.reset();
+    for (VkFramebuffer framebuffer : mImpl->framebuffers)
+    {
+        vkDestroyFramebuffer(mImpl->device, framebuffer, nullptr);
+    }
+    mImpl->framebuffers.clear();
+    if (mImpl->shadow_framebuffer != VK_NULL_HANDLE)
+    {
+        vkDestroyFramebuffer(mImpl->device, mImpl->shadow_framebuffer, nullptr);
+        mImpl->shadow_framebuffer = VK_NULL_HANDLE;
+    }
+    if (mImpl->shadow_view != VK_NULL_HANDLE)
+    {
+        vkDestroyImageView(mImpl->device, mImpl->shadow_view, nullptr);
+        mImpl->shadow_view = VK_NULL_HANDLE;
+    }
+    if (mImpl->shadow_image != VK_NULL_HANDLE)
+    {
+        vkDestroyImage(mImpl->device, mImpl->shadow_image, nullptr);
+        mImpl->shadow_image = VK_NULL_HANDLE;
+    }
+    if (mImpl->shadow_memory != VK_NULL_HANDLE)
+    {
+        vkFreeMemory(mImpl->device, mImpl->shadow_memory, nullptr);
+        mImpl->shadow_memory = VK_NULL_HANDLE;
+    }
+    for (VkImageView view : mImpl->depth_image_views)
+        vkDestroyImageView(mImpl->device, view, nullptr);
+    mImpl->depth_image_views.clear();
+    for (VkImage image : mImpl->depth_images)
+        vkDestroyImage(mImpl->device, image, nullptr);
+    mImpl->depth_images.clear();
+    for (VkDeviceMemory memory : mImpl->depth_memory)
+        vkFreeMemory(mImpl->device, memory, nullptr);
+    mImpl->depth_memory.clear();
+    mImpl->depth_format = VK_FORMAT_UNDEFINED;
+    if (mImpl->validation_pipeline != VK_NULL_HANDLE)
+    {
+        vkDestroyPipeline(mImpl->device, mImpl->validation_pipeline, nullptr);
+        mImpl->validation_pipeline = VK_NULL_HANDLE;
+    }
+    if (mImpl->validation_pipeline_layout != VK_NULL_HANDLE)
+    {
+        vkDestroyPipelineLayout(mImpl->device,
+                                mImpl->validation_pipeline_layout, nullptr);
+        mImpl->validation_pipeline_layout = VK_NULL_HANDLE;
+    }
+    if (mImpl->texture_descriptor_layout != VK_NULL_HANDLE)
+    {
+        vkDestroyDescriptorSetLayout(mImpl->device,
+                                     mImpl->texture_descriptor_layout, nullptr);
+        mImpl->texture_descriptor_layout = VK_NULL_HANDLE;
+    }
+    if (mImpl->skin_descriptor_layout != VK_NULL_HANDLE)
+    {
+        vkDestroyDescriptorSetLayout(mImpl->device,
+                                     mImpl->skin_descriptor_layout, nullptr);
+        mImpl->skin_descriptor_layout = VK_NULL_HANDLE;
+    }
+    if (mImpl->validation_fragment_shader != VK_NULL_HANDLE)
+    {
+        vkDestroyShaderModule(mImpl->device,
+                              mImpl->validation_fragment_shader, nullptr);
+        mImpl->validation_fragment_shader = VK_NULL_HANDLE;
+    }
+    if (mImpl->validation_vertex_shader != VK_NULL_HANDLE)
+    {
+        vkDestroyShaderModule(mImpl->device,
+                              mImpl->validation_vertex_shader, nullptr);
+        mImpl->validation_vertex_shader = VK_NULL_HANDLE;
+    }
+    if (mImpl->render_pass != VK_NULL_HANDLE)
+    {
+        vkDestroyRenderPass(mImpl->device, mImpl->render_pass, nullptr);
+        mImpl->render_pass = VK_NULL_HANDLE;
+    }
+    if (mImpl->shadow_render_pass != VK_NULL_HANDLE)
+    {
+        vkDestroyRenderPass(mImpl->device, mImpl->shadow_render_pass, nullptr);
+        mImpl->shadow_render_pass = VK_NULL_HANDLE;
+    }
+    for (VkImageView view : mImpl->swapchain_image_views)
+    {
+        vkDestroyImageView(mImpl->device, view, nullptr);
+    }
+    mImpl->swapchain_image_views.clear();
+    mImpl->swapchain_images.clear();
+    if (mImpl->swapchain != VK_NULL_HANDLE)
+    {
+        vkDestroySwapchainKHR(mImpl->device, mImpl->swapchain, nullptr);
+        mImpl->swapchain = VK_NULL_HANDLE;
+    }
+    if (mImpl->surface != VK_NULL_HANDLE)
+    {
+        vkDestroySurfaceKHR(mImpl->instance, mImpl->surface, nullptr);
+        mImpl->surface = VK_NULL_HANDLE;
+    }
+#endif
+    mImpl->swapchain_width = 0;
+    mImpl->swapchain_height = 0;
+    mImpl->swapchain_image_count = 0;
+}
+
+void LLVulkanContext::shutdown()
+{
+    detachWindow();
+#if FIRESTORM_VULKAN_EXPERIMENTAL
+    if (mImpl->device != VK_NULL_HANDLE)
+    {
+        vkDeviceWaitIdle(mImpl->device);
+        vkDestroyDevice(mImpl->device, nullptr);
+        mImpl->device = VK_NULL_HANDLE;
+    }
+    if (mImpl->instance != VK_NULL_HANDLE)
+    {
+        vkDestroyInstance(mImpl->instance, nullptr);
+        mImpl->instance = VK_NULL_HANDLE;
+    }
+    mImpl->physical_device = VK_NULL_HANDLE;
+    mImpl->graphics_queue = VK_NULL_HANDLE;
+#endif
+    mImpl->ready = false;
+}
+
+bool LLVulkanContext::ready() const { return mImpl->ready; }
+bool LLVulkanContext::presentationReady() const
+{
+#if FIRESTORM_VULKAN_EXPERIMENTAL && LL_WINDOWS
+    return mImpl->swapchain != VK_NULL_HANDLE;
+#else
+    return false;
+#endif
+}
+
+void LLVulkanContext::observeTextureUpload(std::uint64_t identity,
+                                           std::uint32_t width,
+                                           std::uint32_t height,
+                                           std::uint32_t components,
+                                           const void* pixels,
+                                           std::size_t byte_count,
+                                           bool generate_mipmaps)
+{
+#if FIRESTORM_VULKAN_EXPERIMENTAL && LL_WINDOWS
+    if (!pixels || identity == 0 || width == 0 || height == 0 ||
+        components < 1 || components > 4)
+        return;
+    const std::size_t pixel_count = static_cast<std::size_t>(width) * height;
+    if (pixel_count > 2048u * 2048u || byte_count < pixel_count * components)
+        return;
+    Impl::PendingTexture pending;
+    pending.width = width;
+    pending.height = height;
+    pending.generate_mipmaps = generate_mipmaps;
+    pending.rgba.resize(pixel_count * 4);
+    const auto* source = static_cast<const std::uint8_t*>(pixels);
+    for (std::size_t pixel = 0; pixel < pixel_count; ++pixel)
+    {
+        const std::uint8_t* input = source + pixel * components;
+        std::uint8_t* output = pending.rgba.data() + pixel * 4;
+        if (components < 3)
+            output[0] = output[1] = output[2] = input[0];
+        else
+        {
+            output[0] = input[0]; output[1] = input[1]; output[2] = input[2];
+        }
+        output[3] = components == 2 ? input[1] :
+                    (components == 4 ? input[3] : 255);
+    }
+    std::lock_guard<std::mutex> lock(mImpl->pending_texture_mutex);
+    if (mImpl->pending_textures.size() >= 64 &&
+        mImpl->pending_textures.find(identity) == mImpl->pending_textures.end())
+        mImpl->pending_textures.erase(mImpl->pending_textures.begin());
+    mImpl->pending_textures.insert_or_assign(identity, std::move(pending));
+#else
+    (void)identity; (void)width; (void)height; (void)components;
+    (void)pixels; (void)byte_count; (void)generate_mipmaps;
+#endif
+}
+
+bool LLVulkanContext::observeDraw(const LLVertexBuffer& source,
+                                  std::uint32_t mode,
+                                  std::uint32_t first,
+                                  std::uint32_t count,
+                                  bool indexed)
+{
+#if !(FIRESTORM_VULKAN_EXPERIMENTAL && LL_WINDOWS)
+    (void)source;
+    (void)mode;
+    (void)first;
+    (void)count;
+    (void)indexed;
+    return false;
+#else
+    if (!presentationReady())
+        return false;
+
+    const LLVulkanPrimitiveTopology topology = translateTopology(mode);
+    if (topology == LLVulkanPrimitiveTopology::UNSUPPORTED)
+    {
+        ++mImpl->unsupported_draw_count;
+        return false;
+    }
+
+    const std::uint64_t identity = source.getBackendIdentity();
+    const std::uint64_t revision = source.getDataRevision();
+    auto existing = mImpl->mesh_cache.find(identity);
+    if (existing == mImpl->mesh_cache.end() ||
+        existing->second.revision != revision)
+    {
+        LLRenderMeshSource mesh = LLRenderMeshSource::fromVertexBuffer(source);
+        if (!mesh.valid())
+            return false;
+
+        Impl::CachedMesh cached;
+        cached.revision = revision;
+        cached.vertex_count = mesh.vertex_count;
+        cached.index_count = mesh.index_count;
+        cached.index_stride = mesh.index_stride;
+        cached.type_mask = mesh.type_mask;
+        cached.offsets = mesh.attribute_offsets;
+        cached.vertices =
+            std::make_unique<LLVulkanBuffer>(mImpl->physical_device, mImpl->device);
+        if (!cached.vertices->uploadDeviceLocal(
+                mesh.vertex_data.data(), mesh.vertex_data.size(),
+                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
+                mImpl->command_pool, mImpl->graphics_queue))
+            return false;
+
+        if (!mesh.index_data.empty())
+        {
+            cached.indices = std::make_unique<LLVulkanBuffer>(
+                mImpl->physical_device, mImpl->device);
+            if (!cached.indices->uploadDeviceLocal(
+                    mesh.index_data.data(), mesh.index_data.size(),
+                    VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
+                    mImpl->command_pool, mImpl->graphics_queue))
+                return false;
+        }
+
+        if (mImpl->mesh_cache.size() >= 512 &&
+            existing == mImpl->mesh_cache.end())
+            mImpl->mesh_cache.erase(mImpl->mesh_cache.begin());
+        mImpl->mesh_cache.insert_or_assign(identity, std::move(cached));
+    }
+
+    if (mImpl->draw_queue.size() >= 8192)
+        mImpl->draw_queue.pop_front();
+    LLVulkanDrawRecord record;
+    record.mesh_identity = identity;
+    record.mesh_revision = revision;
+    record.topology = topology;
+    const glm::mat4 model_view_projection =
+        gGL.getProjectionMatrix() * gGL.getModelviewMatrix();
+    std::copy(&model_view_projection[0][0],
+              &model_view_projection[0][0] + record.model_view_projection.size(),
+              record.model_view_projection.begin());
+    const glm::mat4 model_view = gGL.getModelviewMatrix();
+    std::copy(&model_view[0][0], &model_view[0][0] + record.model_view.size(),
+              record.model_view.begin());
+    const glm::mat3 normal_matrix = glm::transpose(glm::inverse(
+        glm::mat3(gGL.getModelviewMatrix())));
+    for (std::size_t column = 0; column < 3; ++column)
+    {
+        record.normal_matrix[column * 4] = normal_matrix[column][0];
+        record.normal_matrix[column * 4 + 1] = normal_matrix[column][1];
+        record.normal_matrix[column * 4 + 2] = normal_matrix[column][2];
+        record.normal_matrix[column * 4 + 3] = 0.f;
+    }
+    auto capture_texture = [](LLTexUnit* unit)
+    {
+        LLVulkanTextureBinding binding;
+        binding.identity = unit->getCurrTexture();
+        binding.address = translateTextureAddress(unit->getTextureAddressMode());
+        binding.filter = translateTextureFilter(unit->getTextureFilterOption());
+        binding.has_mipmaps = unit->hasMipMaps();
+        return binding;
+    };
+    LLGLSLShader* shader = LLGLSLShader::sCurBoundShaderPtr;
+    auto texture_channel = [shader](S32 uniform, S32 fallback)
+    {
+        if (!shader)
+            return fallback;
+        const S32 channel = shader->getTextureChannel(uniform);
+        return channel >= 0 && channel < static_cast<S32>(LL_NUM_TEXTURE_LAYERS) ?
+            channel : fallback;
+    };
+    record.sky = shader && shader->mFeatures.isSky;
+    record.cloud = shader && shader->mFeatures.isCloud;
+    record.sun = shader && shader->mFeatures.isSun;
+    record.moon = shader && shader->mFeatures.isMoon;
+    const S32 primary_texture_uniform = record.cloud ?
+        LLShaderMgr::CLOUD_NOISE_MAP : LLShaderMgr::DIFFUSE_MAP;
+    record.texture = capture_texture(gGL.getTexUnit(
+        texture_channel(primary_texture_uniform, 0)));
+    record.pbr = shader && shader->mFeatures.mGLTF;
+    record.normal_texture = capture_texture(gGL.getTexUnit(texture_channel(
+        record.pbr ? LLShaderMgr::NORMAL_MAP : LLShaderMgr::BUMP_MAP, 1)));
+    record.specular_texture = capture_texture(gGL.getTexUnit(texture_channel(
+        record.pbr ? LLShaderMgr::METALLIC_ROUGHNESS_MAP :
+                     LLShaderMgr::SPECULAR_MAP, 2)));
+    record.occlusion_texture = capture_texture(gGL.getTexUnit(texture_channel(
+        LLShaderMgr::OCCLUSION_MAP, 3)));
+    record.emissive_texture = capture_texture(gGL.getTexUnit(texture_channel(
+        LLShaderMgr::EMISSIVE_MAP, 4)));
+    if (shader)
+    {
+        record.alpha_cutoff = std::max(0.f, shader->getMinimumAlpha());
+        record.alpha_mask = shader->mFeatures.hasAlphaMask ||
+                            record.alpha_cutoff > 0.f;
+        record.lighting = (shader->mFeatures.calculatesLighting ||
+                           shader->mFeatures.hasLighting) &&
+                          (source.getTypeMask() & LLVertexBuffer::MAP_NORMAL) != 0;
+        record.srgb = shader->mFeatures.hasSrgb;
+        record.metallic_factor = std::clamp(shader->getMetallicFactor(), 0.f, 1.f);
+        record.roughness_factor = std::clamp(shader->getRoughnessFactor(), 0.04f, 1.f);
+        record.emissive_strength = std::max(0.f, shader->getEmissiveStrength());
+        record.water = shader->mFeatures.isWater;
+        record.underwater = shader->mFeatures.isUnderwater;
+        record.shadow_pass = shader->mFeatures.isShadow;
+        record.reflection_probes = shader->mFeatures.hasReflectionProbes;
+        const std::uint32_t skin_attributes = LLVertexBuffer::MAP_WEIGHT4 |
+                                              LLVertexBuffer::MAP_JOINT;
+        record.legacy_avatar = shader->mFeatures.hasSkinning &&
+            (source.getTypeMask() & LLVertexBuffer::MAP_WEIGHT) != 0 &&
+            shader->isLegacyAvatarMatrixPalette() &&
+            shader->getAvatarMatrixPalette().size() >= 45 * 4;
+        record.skinned = (shader->mFeatures.hasObjectSkinning &&
+            (source.getTypeMask() & skin_attributes) == skin_attributes &&
+            !shader->isLegacyAvatarMatrixPalette() &&
+            !shader->getAvatarMatrixPalette().empty()) || record.legacy_avatar;
+        if (record.legacy_avatar)
+        {
+            const glm::mat4 projection = gGL.getProjectionMatrix();
+            std::copy(&projection[0][0], &projection[0][0] + 16,
+                      record.model_view_projection.begin());
+            record.normal_matrix = {{
+                1.f, 0.f, 0.f, 0.f,
+                0.f, 1.f, 0.f, 0.f,
+                0.f, 0.f, 1.f, 0.f}};
+        }
+        if (record.skinned)
+        {
+            record.skin_revision = shader->getAvatarMatrixPaletteRevision();
+            record.skin_identity =
+                static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(shader)) ^
+                (record.skin_revision * 0x9e3779b97f4a7c15ULL);
+            auto skin = mImpl->skin_cache.find(record.skin_identity);
+            if (skin == mImpl->skin_cache.end())
+            {
+                Impl::SkinRecord cached_skin;
+                cached_skin.revision = record.skin_revision;
+                cached_skin.buffer = std::make_unique<LLVulkanBuffer>(
+                    mImpl->physical_device, mImpl->device);
+                const auto& palette = shader->getAvatarMatrixPalette();
+                const VkDeviceSize palette_bytes = palette.size() * sizeof(F32);
+                if (!cached_skin.buffer->uploadDeviceLocal(
+                        palette.data(), palette_bytes,
+                        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
+                        mImpl->command_pool, mImpl->graphics_queue))
+                    return false;
+                const VkDescriptorSetAllocateInfo allocate{
+                    VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, nullptr,
+                    mImpl->texture_descriptor_pool, 1,
+                    &mImpl->skin_descriptor_layout};
+                if (vkAllocateDescriptorSets(mImpl->device, &allocate,
+                                             &cached_skin.descriptor) != VK_SUCCESS)
+                    return false;
+                const VkDescriptorBufferInfo buffer_info{
+                    cached_skin.buffer->handle(), 0, palette_bytes};
+                const VkWriteDescriptorSet write{
+                    VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr,
+                    cached_skin.descriptor, 0, 0, 1,
+                    VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, nullptr,
+                    &buffer_info, nullptr};
+                vkUpdateDescriptorSets(mImpl->device, 1, &write, 0, nullptr);
+                if (mImpl->skin_cache.size() >= 64)
+                    mImpl->skin_cache.erase(mImpl->skin_cache.begin());
+                mImpl->skin_cache.insert_or_assign(record.skin_identity,
+                                                    std::move(cached_skin));
+            }
+        }
+    }
+    record.normal_matrix[3] = record.metallic_factor;
+    record.normal_matrix[7] = record.roughness_factor;
+    record.normal_matrix[11] = record.emissive_strength;
+
+    const bool material_shader = shader &&
+        (shader->mFeatures.isSpecular || shader->mFeatures.mGLTF);
+    record.normal_mapped = (record.water ||
+        (record.lighting && material_shader)) &&
+        record.normal_texture.identity != 0 &&
+        ((source.getTypeMask() & LLVertexBuffer::MAP_TANGENT) != 0 ||
+         record.water);
+    record.specular_mapped = record.lighting && material_shader &&
+        record.specular_texture.identity != 0;
+    record.occlusion_mapped = record.pbr &&
+        record.occlusion_texture.identity != 0;
+    record.emissive_mapped = record.pbr &&
+        record.emissive_texture.identity != 0 &&
+        record.emissive_strength > 0.f;
+
+    auto ensure_descriptor = [this](const LLVulkanTextureBinding& binding)
+    {
+        const std::uint64_t sampler_key =
+            (static_cast<std::uint64_t>(binding.address) << 8) |
+            (static_cast<std::uint64_t>(binding.filter) << 1) |
+            static_cast<std::uint64_t>(binding.has_mipmaps);
+        auto sampler_entry = mImpl->sampler_cache.find(sampler_key);
+        if (sampler_entry == mImpl->sampler_cache.end())
+        {
+        const VkFilter filter = binding.filter ==
+            LLVulkanTextureFilter::NEAREST ? VK_FILTER_NEAREST : VK_FILTER_LINEAR;
+        const VkSamplerMipmapMode mipmap_mode = binding.filter ==
+            LLVulkanTextureFilter::TRILINEAR || binding.filter ==
+            LLVulkanTextureFilter::ANISOTROPIC ? VK_SAMPLER_MIPMAP_MODE_LINEAR :
+                                                VK_SAMPLER_MIPMAP_MODE_NEAREST;
+        VkSamplerAddressMode address_mode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
+        if (binding.address == LLVulkanTextureAddress::MIRRORED_REPEAT)
+            address_mode = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
+        else if (binding.address == LLVulkanTextureAddress::CLAMP)
+            address_mode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
+        const bool use_anisotropy = binding.filter ==
+            LLVulkanTextureFilter::ANISOTROPIC && mImpl->sampler_anisotropy;
+        const VkSamplerCreateInfo sampler_info{
+            VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO, nullptr, 0,
+            filter, filter, mipmap_mode, address_mode, address_mode,
+            address_mode, 0.f,
+            use_anisotropy ? VK_TRUE : VK_FALSE,
+            use_anisotropy ? mImpl->max_sampler_anisotropy : 1.f, VK_FALSE,
+            VK_COMPARE_OP_ALWAYS, 0.f,
+            binding.has_mipmaps ? VK_LOD_CLAMP_NONE : 0.f,
+            VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK, VK_FALSE};
+        VkSampler sampler = VK_NULL_HANDLE;
+        if (vkCreateSampler(mImpl->device, &sampler_info, nullptr,
+                            &sampler) == VK_SUCCESS)
+            sampler_entry = mImpl->sampler_cache.emplace(sampler_key,
+                                                         sampler).first;
+        }
+    const std::uint64_t descriptor_key = textureDescriptorKey(binding);
+    if (sampler_entry != mImpl->sampler_cache.end() &&
+        mImpl->descriptor_cache.find(descriptor_key) ==
+            mImpl->descriptor_cache.end() &&
+        mImpl->descriptor_cache.size() < 1024)
+    {
+        VkDescriptorSet descriptor = VK_NULL_HANDLE;
+        const VkDescriptorSetAllocateInfo descriptor_allocate{
+            VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, nullptr,
+            mImpl->texture_descriptor_pool, 1,
+            &mImpl->texture_descriptor_layout};
+        if (vkAllocateDescriptorSets(mImpl->device, &descriptor_allocate,
+                                     &descriptor) == VK_SUCCESS)
+        {
+            VkImageView sampled_view = mImpl->fallback_texture_view;
+            const auto texture = mImpl->texture_cache.find(
+                binding.identity);
+            if (texture != mImpl->texture_cache.end())
+                sampled_view = texture->second.view;
+            const VkDescriptorImageInfo image_info{
+                sampler_entry->second, sampled_view,
+                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
+            const VkWriteDescriptorSet write{
+                VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, descriptor,
+                0, 0, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
+                &image_info, nullptr, nullptr};
+            vkUpdateDescriptorSets(mImpl->device, 1, &write, 0, nullptr);
+            mImpl->descriptor_cache.emplace(
+                descriptor_key,
+                Impl::DescriptorRecord{descriptor, sampler_key});
+        }
+    }
+    };
+    ensure_descriptor(record.texture);
+    ensure_descriptor(record.normal_texture);
+    ensure_descriptor(record.specular_texture);
+    ensure_descriptor(record.occlusion_texture);
+    ensure_descriptor(record.emissive_texture);
+    record.pipeline.topology = topology;
+    record.pipeline.blend_enabled = LLGLState::isEnabled(GL_BLEND);
+    record.pipeline.color_src = translateBlendFactor(gGL.getBlendColorSource());
+    record.pipeline.color_dst = translateBlendFactor(gGL.getBlendColorDest());
+    record.pipeline.alpha_src = translateBlendFactor(gGL.getBlendAlphaSource());
+    record.pipeline.alpha_dst = translateBlendFactor(gGL.getBlendAlphaDest());
+    record.pipeline.depth_test_enabled = LLGLDepthTest::depthEnabled();
+    record.pipeline.depth_write_enabled = LLGLDepthTest::depthWriteEnabled();
+    record.pipeline.depth_compare = translateCompareOp(LLGLDepthTest::depthFunction());
+    record.pipeline.cull_mode = LLGLState::isEnabled(GL_CULL_FACE) ?
+        LLVulkanCullMode::BACK : LLVulkanCullMode::NONE;
+    record.pipeline.polygon_mode = LLVulkanPolygonMode::FILL;
+    if (record.shadow_pass)
+    {
+        if (record.legacy_avatar)
+            record.pipeline.shader_family = LLVulkanShaderFamily::SHADOW_AVATAR;
+        else if (record.skinned)
+            record.pipeline.shader_family = LLVulkanShaderFamily::SHADOW_SKINNED;
+        else if (record.alpha_mask || record.pipeline.blend_enabled)
+            record.pipeline.shader_family = LLVulkanShaderFamily::SHADOW_ALPHA;
+        else
+            record.pipeline.shader_family = LLVulkanShaderFamily::SHADOW_STATIC;
+    }
+    else if (record.reflection_probes && record.pbr)
+        record.pipeline.shader_family = record.alpha_mask ?
+            LLVulkanShaderFamily::PROBE_MATERIAL_ALPHA :
+            LLVulkanShaderFamily::PROBE_MATERIAL;
+    else if (record.sky)
+        record.pipeline.shader_family = LLVulkanShaderFamily::SKY_DOME;
+    else if (record.cloud)
+        record.pipeline.shader_family = LLVulkanShaderFamily::SKY_CLOUDS;
+    else if (record.sun)
+        record.pipeline.shader_family = LLVulkanShaderFamily::SKY_SUN;
+    else if (record.moon)
+        record.pipeline.shader_family = LLVulkanShaderFamily::SKY_MOON;
+    else if (record.water)
+        record.pipeline.shader_family = record.underwater ?
+            LLVulkanShaderFamily::UNDERWATER_SURFACE :
+            LLVulkanShaderFamily::WATER_SURFACE;
+    else if (record.legacy_avatar)
+        record.pipeline.shader_family = record.alpha_mask ?
+            LLVulkanShaderFamily::AVATAR_BODY_ALPHA_MASK :
+            LLVulkanShaderFamily::AVATAR_BODY;
+    else if (record.skinned)
+        record.pipeline.shader_family = record.alpha_mask ?
+            LLVulkanShaderFamily::SKINNED_ALPHA_MASK :
+            LLVulkanShaderFamily::SKINNED_TEXTURED;
+    else if (record.normal_mapped || record.specular_mapped)
+        record.pipeline.shader_family = record.alpha_mask ?
+            LLVulkanShaderFamily::MATERIAL_ALPHA_MASK :
+            LLVulkanShaderFamily::MATERIAL_TEXTURED;
+    else if (record.lighting)
+        record.pipeline.shader_family = record.alpha_mask ?
+            LLVulkanShaderFamily::LIT_ALPHA_MASK :
+            LLVulkanShaderFamily::LIT_TEXTURED;
+    else
+        record.pipeline.shader_family = record.alpha_mask ?
+            LLVulkanShaderFamily::ALPHA_MASK :
+            LLVulkanShaderFamily::UNLIT_TEXTURED;
+    if (record.pipeline.color_src == LLVulkanBlendFactor::UNSUPPORTED ||
+        record.pipeline.color_dst == LLVulkanBlendFactor::UNSUPPORTED ||
+        record.pipeline.alpha_src == LLVulkanBlendFactor::UNSUPPORTED ||
+        record.pipeline.alpha_dst == LLVulkanBlendFactor::UNSUPPORTED ||
+        record.pipeline.depth_compare == LLVulkanCompareOp::UNSUPPORTED)
+    {
+        ++mImpl->unsupported_state_count;
+        return false;
+    }
+    if (mImpl->pipeline_keys.size() >= 512 &&
+        mImpl->pipeline_keys.find(record.pipeline) == mImpl->pipeline_keys.end())
+        mImpl->pipeline_keys.erase(mImpl->pipeline_keys.begin());
+    mImpl->pipeline_keys.insert(record.pipeline);
+    record.indexed = indexed;
+    if (indexed)
+    {
+        record.first_index = first;
+        record.index_count = count;
+    }
+    else
+    {
+        record.first_vertex = first;
+        record.vertex_count = count;
+    }
+    mImpl->draw_queue.push_back(record);
+    return true;
+#endif
+}
+
+std::size_t LLVulkanContext::cachedMeshCount() const
+{
+#if FIRESTORM_VULKAN_EXPERIMENTAL && LL_WINDOWS
+    return mImpl->mesh_cache.size();
+#else
+    return 0;
+#endif
+}
+
+std::size_t LLVulkanContext::queuedDrawCount() const
+{
+#if FIRESTORM_VULKAN_EXPERIMENTAL && LL_WINDOWS
+    return mImpl->draw_queue.size();
+#else
+    return 0;
+#endif
+}
+
+std::size_t LLVulkanContext::pipelineKeyCount() const
+{
+#if FIRESTORM_VULKAN_EXPERIMENTAL && LL_WINDOWS
+    return mImpl->pipeline_keys.size();
+#else
+    return 0;
+#endif
+}
+
+std::size_t LLVulkanContext::pipelineVariantCount() const
+{
+#if FIRESTORM_VULKAN_EXPERIMENTAL && LL_WINDOWS
+    return mImpl->pipeline_cache.size();
+#else
+    return 0;
+#endif
+}
+
+std::uint64_t LLVulkanContext::unsupportedStateCount() const
+{
+#if FIRESTORM_VULKAN_EXPERIMENTAL && LL_WINDOWS
+    return mImpl->unsupported_state_count;
+#else
+    return 0;
+#endif
+}
+
+std::uint64_t LLVulkanContext::executedDrawCount() const
+{
+#if FIRESTORM_VULKAN_EXPERIMENTAL && LL_WINDOWS
+    return mImpl->executed_draw_count;
+#else
+    return 0;
+#endif
+}
+
+std::size_t LLVulkanContext::descriptorCount() const
+{
+#if FIRESTORM_VULKAN_EXPERIMENTAL && LL_WINDOWS
+    return mImpl->descriptor_cache.size();
+#else
+    return 0;
+#endif
+}
+
+std::size_t LLVulkanContext::cachedTextureCount() const
+{
+#if FIRESTORM_VULKAN_EXPERIMENTAL && LL_WINDOWS
+    return mImpl->texture_cache.size();
+#else
+    return 0;
+#endif
+}
+
+std::uint64_t LLVulkanContext::unsupportedDrawCount() const
+{
+#if FIRESTORM_VULKAN_EXPERIMENTAL && LL_WINDOWS
+    return mImpl->unsupported_draw_count;
+#else
+    return 0;
+#endif
+}
+
+void LLVulkanContext::clearQueuedDraws()
+{
+#if FIRESTORM_VULKAN_EXPERIMENTAL && LL_WINDOWS
+    mImpl->draw_queue.clear();
+#endif
+}
+
+LLVulkanFrameResult LLVulkanContext::drawValidationFrame(float red,
+                                                         float green,
+                                                         float blue)
+{
+#if !(FIRESTORM_VULKAN_EXPERIMENTAL && LL_WINDOWS)
+    (void)red;
+    (void)green;
+    (void)blue;
+    mImpl->error = "Win32 Vulkan presentation support is not compiled";
+    return LLVulkanFrameResult::NOT_READY;
+#else
+    if (!presentationReady())
+    {
+        mImpl->error = "Vulkan swapchain is not ready";
+        return LLVulkanFrameResult::NOT_READY;
+    }
+
+    std::vector<LLVulkanProbeFace> pending_probe_faces;
+    {
+        std::lock_guard<std::mutex> lock(mImpl->pending_probe_face_mutex);
+        pending_probe_faces.swap(mImpl->pending_probe_faces);
+    }
+    if (!pending_probe_faces.empty())
+    {
+        vkDeviceWaitIdle(mImpl->device);
+        VkCommandBuffer upload = VK_NULL_HANDLE;
+        const VkCommandBufferAllocateInfo allocate_info{
+            VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, nullptr,
+            mImpl->command_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1};
+        VkResult upload_status = vkAllocateCommandBuffers(
+            mImpl->device, &allocate_info, &upload);
+        const VkCommandBufferBeginInfo begin_info{
+            VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr,
+            VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr};
+        if (upload_status == VK_SUCCESS)
+            upload_status = vkBeginCommandBuffer(upload, &begin_info);
+        std::vector<std::unique_ptr<LLVulkanBuffer>> staging_buffers;
+        for (const LLVulkanProbeFace& face : pending_probe_faces)
+        {
+            Impl::ProbeArray& target = face.irradiance ?
+                mImpl->irradiance_probes : mImpl->radiance_probes;
+            if (upload_status != VK_SUCCESS || face.face >= 6 ||
+                face.cube_index >= target.cube_count ||
+                face.mip_level >= target.mip_levels || face.width == 0 ||
+                face.rgba16f.size() !=
+                    static_cast<std::size_t>(face.width) * face.width * 4)
+                continue;
+            auto staging = std::make_unique<LLVulkanBuffer>(
+                mImpl->physical_device, mImpl->device);
+            const VkDeviceSize byte_count =
+                face.rgba16f.size() * sizeof(face.rgba16f.front());
+            if (!staging->allocate(byte_count, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
+                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
+                                       VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) ||
+                !staging->upload(face.rgba16f.data(), byte_count))
+                continue;
+            const std::uint32_t array_layer = face.cube_index * 6 + face.face;
+            const VkImageMemoryBarrier to_transfer{
+                VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, nullptr,
+                VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_TRANSFER_WRITE_BIT,
+                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
+                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
+                VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
+                target.image,
+                {VK_IMAGE_ASPECT_COLOR_BIT, face.mip_level, 1,
+                 array_layer, 1}};
+            vkCmdPipelineBarrier(upload, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
+                                 VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr,
+                                 0, nullptr, 1, &to_transfer);
+            const VkBufferImageCopy copy{
+                0, 0, 0,
+                {VK_IMAGE_ASPECT_COLOR_BIT, face.mip_level,
+                 array_layer, 1},
+                {0, 0, 0}, {face.width, face.width, 1}};
+            vkCmdCopyBufferToImage(upload, staging->handle(), target.image,
+                                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
+                                   1, &copy);
+            const VkImageMemoryBarrier to_read{
+                VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, nullptr,
+                VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
+                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
+                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
+                VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
+                target.image,
+                {VK_IMAGE_ASPECT_COLOR_BIT, face.mip_level, 1,
+                 array_layer, 1}};
+            vkCmdPipelineBarrier(upload, VK_PIPELINE_STAGE_TRANSFER_BIT,
+                                 VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
+                                 0, nullptr, 0, nullptr, 1, &to_read);
+            staging_buffers.push_back(std::move(staging));
+        }
+        if (upload_status == VK_SUCCESS)
+            upload_status = vkEndCommandBuffer(upload);
+        if (upload_status == VK_SUCCESS)
+        {
+            const VkSubmitInfo submit{
+                VK_STRUCTURE_TYPE_SUBMIT_INFO, nullptr, 0, nullptr, nullptr,
+                1, &upload, 0, nullptr};
+            upload_status = vkQueueSubmit(mImpl->graphics_queue, 1, &submit,
+                                          VK_NULL_HANDLE);
+        }
+        if (upload_status == VK_SUCCESS)
+            upload_status = vkQueueWaitIdle(mImpl->graphics_queue);
+        if (upload != VK_NULL_HANDLE)
+            vkFreeCommandBuffers(mImpl->device, mImpl->command_pool, 1, &upload);
+        if (upload_status != VK_SUCCESS)
+        {
+            mImpl->error = "Unable to mirror Firestorm reflection probes";
+            return LLVulkanFrameResult::ERROR;
+        }
+    }
+
+    std::unordered_map<std::uint64_t, Impl::PendingTexture> pending_textures;
+    {
+        std::lock_guard<std::mutex> lock(mImpl->pending_texture_mutex);
+        pending_textures.swap(mImpl->pending_textures);
+    }
+    if (!pending_textures.empty())
+        vkDeviceWaitIdle(mImpl->device);
+    for (auto& pending_entry : pending_textures)
+    {
+        const std::uint64_t identity = pending_entry.first;
+        Impl::PendingTexture& pending = pending_entry.second;
+        if (mImpl->texture_cache.size() >= 1024 &&
+            mImpl->texture_cache.find(identity) == mImpl->texture_cache.end())
+            continue;
+        Impl::TextureRecord texture;
+        texture.width = pending.width;
+        texture.height = pending.height;
+        VkFormatProperties rgba8_properties{};
+        vkGetPhysicalDeviceFormatProperties(mImpl->physical_device,
+                                             VK_FORMAT_R8G8B8A8_UNORM,
+                                             &rgba8_properties);
+        const bool can_generate_mipmaps = pending.generate_mipmaps &&
+            (rgba8_properties.optimalTilingFeatures &
+             VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT) != 0;
+        if (can_generate_mipmaps)
+        {
+            std::uint32_t dimension = std::max(pending.width, pending.height);
+            while (dimension > 1)
+            {
+                dimension >>= 1;
+                ++texture.mip_levels;
+            }
+        }
+        const VkImageCreateInfo image_info{
+            VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, nullptr, 0,
+            VK_IMAGE_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM,
+            {pending.width, pending.height, 1}, texture.mip_levels, 1,
+            VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL,
+            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
+                VK_IMAGE_USAGE_SAMPLED_BIT,
+            VK_SHARING_MODE_EXCLUSIVE, 0, nullptr,
+            VK_IMAGE_LAYOUT_UNDEFINED};
+        if (vkCreateImage(mImpl->device, &image_info, nullptr,
+                          &texture.image) != VK_SUCCESS)
+            continue;
+        VkMemoryRequirements requirements{};
+        vkGetImageMemoryRequirements(mImpl->device, texture.image,
+                                     &requirements);
+        std::uint32_t memory_type = 0;
+        if (!selectDeviceMemoryType(mImpl->physical_device,
+                                    requirements.memoryTypeBits,
+                                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
+                                    memory_type))
+        {
+            vkDestroyImage(mImpl->device, texture.image, nullptr);
+            continue;
+        }
+        const VkMemoryAllocateInfo allocation{
+            VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, nullptr,
+            requirements.size, memory_type};
+        if (vkAllocateMemory(mImpl->device, &allocation, nullptr,
+                             &texture.memory) != VK_SUCCESS ||
+            vkBindImageMemory(mImpl->device, texture.image,
+                              texture.memory, 0) != VK_SUCCESS)
+        {
+            if (texture.memory != VK_NULL_HANDLE)
+                vkFreeMemory(mImpl->device, texture.memory, nullptr);
+            vkDestroyImage(mImpl->device, texture.image, nullptr);
+            continue;
+        }
+        const VkImageViewCreateInfo view_info{
+            VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, nullptr, 0,
+            texture.image, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM,
+            {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
+             VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY},
+            {VK_IMAGE_ASPECT_COLOR_BIT, 0, texture.mip_levels, 0, 1}};
+        if (vkCreateImageView(mImpl->device, &view_info, nullptr,
+                              &texture.view) != VK_SUCCESS)
+        {
+            vkDestroyImage(mImpl->device, texture.image, nullptr);
+            vkFreeMemory(mImpl->device, texture.memory, nullptr);
+            continue;
+        }
+
+        LLVulkanBuffer staging(mImpl->physical_device, mImpl->device);
+        if (!staging.allocate(pending.rgba.size(),
+                              VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
+                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
+                                  VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) ||
+            !staging.upload(pending.rgba.data(), pending.rgba.size()))
+        {
+            vkDestroyImageView(mImpl->device, texture.view, nullptr);
+            vkDestroyImage(mImpl->device, texture.image, nullptr);
+            vkFreeMemory(mImpl->device, texture.memory, nullptr);
+            continue;
+        }
+        VkCommandBuffer upload = VK_NULL_HANDLE;
+        const VkCommandBufferAllocateInfo allocate_info{
+            VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, nullptr,
+            mImpl->command_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1};
+        VkResult upload_status = vkAllocateCommandBuffers(
+            mImpl->device, &allocate_info, &upload);
+        const VkCommandBufferBeginInfo begin_info{
+            VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr,
+            VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr};
+        if (upload_status == VK_SUCCESS)
+            upload_status = vkBeginCommandBuffer(upload, &begin_info);
+        if (upload_status == VK_SUCCESS)
+        {
+            const VkImageMemoryBarrier to_transfer{
+                VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, nullptr, 0,
+                VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
+                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
+                VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
+                texture.image,
+                {VK_IMAGE_ASPECT_COLOR_BIT, 0, texture.mip_levels, 0, 1}};
+            vkCmdPipelineBarrier(upload, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
+                                 VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr,
+                                 0, nullptr, 1, &to_transfer);
+            const VkBufferImageCopy copy{
+                0, 0, 0, {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
+                {0, 0, 0}, {pending.width, pending.height, 1}};
+            vkCmdCopyBufferToImage(upload, staging.handle(), texture.image,
+                                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
+                                   1, &copy);
+            std::int32_t mip_width = static_cast<std::int32_t>(pending.width);
+            std::int32_t mip_height = static_cast<std::int32_t>(pending.height);
+            for (std::uint32_t level = 1;
+                 level < texture.mip_levels; ++level)
+            {
+                VkImageMemoryBarrier to_source{
+                    VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, nullptr,
+                    VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT,
+                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
+                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
+                    VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
+                    texture.image,
+                    {VK_IMAGE_ASPECT_COLOR_BIT, level - 1, 1, 0, 1}};
+                vkCmdPipelineBarrier(upload, VK_PIPELINE_STAGE_TRANSFER_BIT,
+                                     VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
+                                     0, nullptr, 0, nullptr, 1, &to_source);
+                const std::int32_t next_width = std::max(1, mip_width / 2);
+                const std::int32_t next_height = std::max(1, mip_height / 2);
+                const VkImageBlit blit{
+                    {VK_IMAGE_ASPECT_COLOR_BIT, level - 1, 0, 1},
+                    {{0, 0, 0}, {mip_width, mip_height, 1}},
+                    {VK_IMAGE_ASPECT_COLOR_BIT, level, 0, 1},
+                    {{0, 0, 0}, {next_width, next_height, 1}}};
+                vkCmdBlitImage(upload, texture.image,
+                               VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
+                               texture.image,
+                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
+                               1, &blit, VK_FILTER_LINEAR);
+                VkImageMemoryBarrier source_to_read{
+                    VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, nullptr,
+                    VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_SHADER_READ_BIT,
+                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
+                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
+                    VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
+                    texture.image,
+                    {VK_IMAGE_ASPECT_COLOR_BIT, level - 1, 1, 0, 1}};
+                vkCmdPipelineBarrier(upload, VK_PIPELINE_STAGE_TRANSFER_BIT,
+                                     VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
+                                     0, nullptr, 0, nullptr, 1,
+                                     &source_to_read);
+                mip_width = next_width;
+                mip_height = next_height;
+            }
+            const VkImageMemoryBarrier last_level_to_read{
+                VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, nullptr,
+                VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
+                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
+                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
+                VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
+                texture.image,
+                {VK_IMAGE_ASPECT_COLOR_BIT, texture.mip_levels - 1, 1, 0, 1}};
+            vkCmdPipelineBarrier(upload, VK_PIPELINE_STAGE_TRANSFER_BIT,
+                                 VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
+                                 0, nullptr, 0, nullptr, 1,
+                                 &last_level_to_read);
+            upload_status = vkEndCommandBuffer(upload);
+        }
+        if (upload_status == VK_SUCCESS)
+        {
+            const VkSubmitInfo submit{
+                VK_STRUCTURE_TYPE_SUBMIT_INFO, nullptr, 0, nullptr, nullptr,
+                1, &upload, 0, nullptr};
+            upload_status = vkQueueSubmit(mImpl->graphics_queue, 1,
+                                          &submit, VK_NULL_HANDLE);
+        }
+        if (upload_status == VK_SUCCESS)
+            upload_status = vkQueueWaitIdle(mImpl->graphics_queue);
+        if (upload != VK_NULL_HANDLE)
+            vkFreeCommandBuffers(mImpl->device, mImpl->command_pool, 1, &upload);
+        if (upload_status != VK_SUCCESS)
+        {
+            vkDestroyImageView(mImpl->device, texture.view, nullptr);
+            vkDestroyImage(mImpl->device, texture.image, nullptr);
+            vkFreeMemory(mImpl->device, texture.memory, nullptr);
+            continue;
+        }
+        auto old = mImpl->texture_cache.find(identity);
+        for (auto& descriptor_entry : mImpl->descriptor_cache)
+        {
+            if (static_cast<std::uint32_t>(descriptor_entry.first) !=
+                static_cast<std::uint32_t>(identity))
+                continue;
+            const auto sampler = mImpl->sampler_cache.find(
+                descriptor_entry.second.sampler_key);
+            if (sampler == mImpl->sampler_cache.end())
+                continue;
+            const VkDescriptorImageInfo descriptor_image{
+                sampler->second, texture.view,
+                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
+            const VkWriteDescriptorSet write{
+                VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr,
+                descriptor_entry.second.set, 0, 0, 1,
+                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
+                &descriptor_image, nullptr, nullptr};
+            vkUpdateDescriptorSets(mImpl->device, 1, &write, 0, nullptr);
+        }
+        if (old != mImpl->texture_cache.end())
+        {
+            vkDestroyImageView(mImpl->device, old->second.view, nullptr);
+            vkDestroyImage(mImpl->device, old->second.image, nullptr);
+            vkFreeMemory(mImpl->device, old->second.memory, nullptr);
+        }
+        mImpl->texture_cache.insert_or_assign(identity, texture);
+    }
+
+    const std::size_t frame = mImpl->current_frame;
+    VkResult status = vkWaitForFences(mImpl->device, 1,
+                                      &mImpl->frame_fences[frame], VK_TRUE,
+                                      std::numeric_limits<std::uint64_t>::max());
+    if (status != VK_SUCCESS)
+    {
+        mImpl->error = "vkWaitForFences failed with code " +
+                       std::to_string(status);
+        return LLVulkanFrameResult::ERROR;
+    }
+
+    constexpr std::array<float, 16> identity_shadow_transform{{
+        1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f,
+        0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f}};
+    std::unordered_map<std::uint64_t, std::array<float, 16>> caster_transforms;
+    for (const LLVulkanDrawRecord& draw : mImpl->draw_queue)
+        if (draw.shadow_pass)
+            caster_transforms.insert_or_assign(draw.mesh_identity,
+                                                draw.model_view_projection);
+    std::vector<DrawInstanceTransforms> shadow_transforms{{
+        identity_shadow_transform, identity_shadow_transform}};
+    std::unordered_map<std::uint64_t, std::uint32_t> shadow_transform_indices;
+    for (const LLVulkanDrawRecord& draw : mImpl->draw_queue)
+    {
+        if (draw.shadow_pass ||
+            shadow_transform_indices.find(draw.mesh_identity) !=
+                shadow_transform_indices.end())
+            continue;
+        shadow_transform_indices.emplace(
+            draw.mesh_identity,
+            static_cast<std::uint32_t>(shadow_transforms.size()));
+        const auto caster = caster_transforms.find(draw.mesh_identity);
+        shadow_transforms.push_back({
+            caster != caster_transforms.end() ? caster->second :
+                                               identity_shadow_transform,
+            draw.model_view});
+    }
+    mImpl->shadow_transform_buffers[frame] = std::make_unique<LLVulkanBuffer>(
+        mImpl->physical_device, mImpl->device);
+    if (!mImpl->shadow_transform_buffers[frame]->uploadDeviceLocal(
+            shadow_transforms.data(),
+            shadow_transforms.size() * sizeof(shadow_transforms.front()),
+            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
+            mImpl->command_pool, mImpl->graphics_queue))
+    {
+        mImpl->error = "Unable to upload Vulkan shadow transforms";
+        return LLVulkanFrameResult::ERROR;
+    }
+
+    std::vector<LLVulkanProbeRecord> probe_metadata;
+    {
+        std::lock_guard<std::mutex> lock(mImpl->pending_probe_mutex);
+        probe_metadata = mImpl->pending_probe_metadata;
+    }
+    if (probe_metadata.empty())
+    {
+        LLVulkanProbeRecord fallback_probe;
+        fallback_probe.box = identity_shadow_transform;
+        fallback_probe.sphere = {{0.f, 0.f, 0.f, 100000.f}};
+        fallback_probe.parameters = {{1.f, 1.f, 1.f, -100000.f}};
+        fallback_probe.indices = {{0, -1, 0, 1}};
+        probe_metadata.push_back(fallback_probe);
+    }
+    if (probe_metadata.size() > 256)
+        probe_metadata.resize(256);
+    mImpl->probe_metadata_buffers[frame] = std::make_unique<LLVulkanBuffer>(
+        mImpl->physical_device, mImpl->device);
+    const VkDeviceSize metadata_bytes =
+        probe_metadata.size() * sizeof(probe_metadata.front());
+    if (!mImpl->probe_metadata_buffers[frame]->uploadDeviceLocal(
+            probe_metadata.data(), metadata_bytes,
+            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
+            mImpl->command_pool, mImpl->graphics_queue))
+    {
+        mImpl->error = "Unable to upload Vulkan probe metadata";
+        return LLVulkanFrameResult::ERROR;
+    }
+    const VkDescriptorBufferInfo metadata_info{
+        mImpl->probe_metadata_buffers[frame]->handle(), 0, metadata_bytes};
+    const VkWriteDescriptorSet metadata_write{
+        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr,
+        mImpl->probe_metadata_descriptors[frame], 0, 0, 1,
+        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, nullptr, &metadata_info, nullptr};
+    vkUpdateDescriptorSets(mImpl->device, 1, &metadata_write, 0, nullptr);
+
+    std::uint32_t image_index = 0;
+    status = vkAcquireNextImageKHR(
+        mImpl->device, mImpl->swapchain,
+        std::numeric_limits<std::uint64_t>::max(),
+        mImpl->image_available[frame], VK_NULL_HANDLE, &image_index);
+    if (status == VK_ERROR_OUT_OF_DATE_KHR)
+    {
+        return LLVulkanFrameResult::SWAPCHAIN_OUT_OF_DATE;
+    }
+    const bool acquired_suboptimal = status == VK_SUBOPTIMAL_KHR;
+    if (status != VK_SUCCESS && !acquired_suboptimal)
+    {
+        mImpl->error = "vkAcquireNextImageKHR failed with code " +
+                       std::to_string(status);
+        return LLVulkanFrameResult::ERROR;
+    }
+
+    VkCommandBuffer command_buffer = mImpl->command_buffers[image_index];
+    vkResetCommandBuffer(command_buffer, 0);
+    const VkCommandBufferBeginInfo begin_info{
+        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
+        nullptr,
+        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
+        nullptr
+    };
+    status = vkBeginCommandBuffer(command_buffer, &begin_info);
+    if (status != VK_SUCCESS)
+    {
+        mImpl->error = "vkBeginCommandBuffer failed with code " +
+                       std::to_string(status);
+        return LLVulkanFrameResult::ERROR;
+    }
+
+    VkClearValue shadow_clear{};
+    shadow_clear.depthStencil = {1.0f, 0};
+    const VkRenderPassBeginInfo shadow_begin{
+        VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO, nullptr,
+        mImpl->shadow_render_pass, mImpl->shadow_framebuffer,
+        {{0, 0}, {Impl::shadow_extent, Impl::shadow_extent}},
+        1, &shadow_clear};
+    vkCmdBeginRenderPass(command_buffer, &shadow_begin,
+                         VK_SUBPASS_CONTENTS_INLINE);
+    const VkViewport shadow_viewport{
+        0.f, 0.f, static_cast<float>(Impl::shadow_extent),
+        static_cast<float>(Impl::shadow_extent), 0.f, 1.f};
+    const VkRect2D shadow_scissor{{0, 0},
+        {Impl::shadow_extent, Impl::shadow_extent}};
+    vkCmdSetViewport(command_buffer, 0, 1, &shadow_viewport);
+    vkCmdSetScissor(command_buffer, 0, 1, &shadow_scissor);
+    for (const LLVulkanDrawRecord& draw : mImpl->draw_queue)
+    {
+        if (!draw.shadow_pass)
+            continue;
+        auto mesh_entry = mImpl->mesh_cache.find(draw.mesh_identity);
+        if (mesh_entry == mImpl->mesh_cache.end() ||
+            mesh_entry->second.revision != draw.mesh_revision)
+            continue;
+        Impl::CachedMesh& mesh = mesh_entry->second;
+        if ((draw.indexed &&
+             (!mesh.indices || draw.first_index > mesh.index_count ||
+              draw.index_count > mesh.index_count - draw.first_index)) ||
+            (!draw.indexed &&
+             (draw.first_vertex > mesh.vertex_count ||
+              draw.vertex_count > mesh.vertex_count - draw.first_vertex)))
+            continue;
+
+        auto pipeline_entry = mImpl->pipeline_cache.find(draw.pipeline);
+        if (pipeline_entry == mImpl->pipeline_cache.end())
+        {
+            VkPipeline pipeline = createCapturedPipeline(
+                mImpl->device, mImpl->shadow_render_pass,
+                mImpl->validation_pipeline_layout,
+                mImpl->validation_vertex_shader,
+                mImpl->validation_fragment_shader, draw.pipeline);
+            if (pipeline == VK_NULL_HANDLE)
+            {
+                ++mImpl->unsupported_state_count;
+                continue;
+            }
+            if (mImpl->pipeline_cache.size() >= 512)
+            {
+                auto evicted = mImpl->pipeline_cache.begin();
+                vkDestroyPipeline(mImpl->device, evicted->second, nullptr);
+                mImpl->pipeline_cache.erase(evicted);
+            }
+            pipeline_entry = mImpl->pipeline_cache.emplace(draw.pipeline,
+                                                            pipeline).first;
+        }
+        vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
+                          pipeline_entry->second);
+        vkCmdPushConstants(command_buffer, mImpl->validation_pipeline_layout,
+                           VK_SHADER_STAGE_VERTEX_BIT, 0,
+                           sizeof(draw.model_view_projection),
+                           draw.model_view_projection.data());
+        vkCmdPushConstants(command_buffer, mImpl->validation_pipeline_layout,
+                           VK_SHADER_STAGE_VERTEX_BIT, sizeof(float) * 16,
+                           sizeof(draw.normal_matrix), draw.normal_matrix.data());
+        const std::array<float, 4> shadow_material{{
+            draw.pipeline.shader_family == LLVulkanShaderFamily::SHADOW_ALPHA ?
+                std::max(draw.alpha_cutoff, 0.01f) : 0.f, 0.f,
+            draw.srgb ? 1.f : 0.f,
+            static_cast<float>((draw.skinned && !draw.legacy_avatar ? 32 : 0) |
+                               (draw.legacy_avatar ? 64 : 0) | 8192)}};
+        vkCmdPushConstants(command_buffer, mImpl->validation_pipeline_layout,
+                           VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
+                           sizeof(float) * 28, sizeof(shadow_material),
+                           shadow_material.data());
+
+        const auto fallback = mImpl->descriptor_cache.find(
+            textureDescriptorKey(LLVulkanTextureBinding{}));
+        if (fallback == mImpl->descriptor_cache.end())
+            continue;
+        VkDescriptorSet sets[6] = {
+            fallback->second.set, fallback->second.set, fallback->second.set,
+            fallback->second.set, fallback->second.set,
+            mImpl->identity_skin_descriptor};
+        const auto diffuse = mImpl->descriptor_cache.find(
+            textureDescriptorKey(draw.texture));
+        if (diffuse != mImpl->descriptor_cache.end())
+            sets[0] = diffuse->second.set;
+        if (draw.skinned)
+        {
+            const auto skin = mImpl->skin_cache.find(draw.skin_identity);
+            if (skin == mImpl->skin_cache.end() ||
+                skin->second.revision != draw.skin_revision)
+                continue;
+            sets[5] = skin->second.descriptor;
+        }
+        vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
+                                mImpl->validation_pipeline_layout, 0, 6,
+                                sets, 0, nullptr);
+        vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
+                                mImpl->validation_pipeline_layout, 6, 1,
+                                &mImpl->shadow_descriptor, 0, nullptr);
+        const VkDescriptorSet probe_sets[] = {
+            mImpl->radiance_probe_descriptor,
+            mImpl->irradiance_probe_descriptor};
+        vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
+                                mImpl->validation_pipeline_layout, 7, 2,
+                                probe_sets, 0, nullptr);
+        vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
+                                mImpl->validation_pipeline_layout, 9, 1,
+                                &mImpl->probe_metadata_descriptors[frame],
+                                0, nullptr);
+
+        const VkBuffer attribute_buffers[8] = {
+            mesh.vertices->handle(), mesh.vertices->handle(),
+            mesh.vertices->handle(), mesh.vertices->handle(),
+            mesh.vertices->handle(), mesh.vertices->handle(),
+            mesh.vertices->handle(), mesh.vertices->handle()};
+        auto offset_or_vertex = [&mesh](std::uint32_t mask, std::size_t type)
+        {
+            return static_cast<VkDeviceSize>((mesh.type_mask & mask) != 0 ?
+                mesh.offsets[type] : mesh.offsets[LLVertexBuffer::TYPE_VERTEX]);
+        };
+        const VkDeviceSize attribute_offsets[8] = {
+            mesh.offsets[LLVertexBuffer::TYPE_VERTEX],
+            offset_or_vertex(LLVertexBuffer::MAP_COLOR, LLVertexBuffer::TYPE_COLOR),
+            offset_or_vertex(LLVertexBuffer::MAP_TEXCOORD0, LLVertexBuffer::TYPE_TEXCOORD0),
+            offset_or_vertex(LLVertexBuffer::MAP_NORMAL, LLVertexBuffer::TYPE_NORMAL),
+            offset_or_vertex(LLVertexBuffer::MAP_TANGENT, LLVertexBuffer::TYPE_TANGENT),
+            offset_or_vertex(LLVertexBuffer::MAP_WEIGHT4, LLVertexBuffer::TYPE_WEIGHT4),
+            offset_or_vertex(LLVertexBuffer::MAP_JOINT, LLVertexBuffer::TYPE_JOINT),
+            offset_or_vertex(LLVertexBuffer::MAP_WEIGHT, LLVertexBuffer::TYPE_WEIGHT)};
+        vkCmdBindVertexBuffers(command_buffer, 0, 8, attribute_buffers,
+                               attribute_offsets);
+        const VkBuffer shadow_transform_buffer =
+            mImpl->shadow_transform_buffers[frame]->handle();
+        constexpr VkDeviceSize identity_shadow_offset = 0;
+        vkCmdBindVertexBuffers(command_buffer, 8, 1,
+                               &shadow_transform_buffer,
+                               &identity_shadow_offset);
+        if (draw.indexed)
+        {
+            vkCmdBindIndexBuffer(command_buffer, mesh.indices->handle(), 0,
+                mesh.index_stride == 4 ? VK_INDEX_TYPE_UINT32 :
+                                         VK_INDEX_TYPE_UINT16);
+            vkCmdDrawIndexed(command_buffer, draw.index_count, 1,
+                             draw.first_index, 0, 0);
+        }
+        else
+        {
+            vkCmdDraw(command_buffer, draw.vertex_count, 1,
+                      draw.first_vertex, 0);
+        }
+    }
+    vkCmdEndRenderPass(command_buffer);
+
+    VkClearValue clear_values[2]{};
+    clear_values[0].color = {{red, green, blue, 1.0f}};
+    clear_values[1].depthStencil = {1.0f, 0};
+    const VkRenderPassBeginInfo render_pass_info{
+        VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
+        nullptr,
+        mImpl->render_pass,
+        mImpl->framebuffers[image_index],
+        {{0, 0}, {mImpl->swapchain_width, mImpl->swapchain_height}},
+        2,
+        clear_values
+    };
+    vkCmdBeginRenderPass(command_buffer, &render_pass_info,
+                         VK_SUBPASS_CONTENTS_INLINE);
+    const VkViewport viewport{
+        0.f, 0.f,
+        static_cast<float>(mImpl->swapchain_width),
+        static_cast<float>(mImpl->swapchain_height),
+        0.f, 1.f};
+    const VkRect2D scissor{{0, 0},
+                           {mImpl->swapchain_width, mImpl->swapchain_height}};
+    vkCmdSetViewport(command_buffer, 0, 1, &viewport);
+    vkCmdSetScissor(command_buffer, 0, 1, &scissor);
+    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
+                      mImpl->validation_pipeline);
+    constexpr std::array<float, 16> identity_transform{{
+        1.f, 0.f, 0.f, 0.f,
+        0.f, 1.f, 0.f, 0.f,
+        0.f, 0.f, 1.f, 0.f,
+        0.f, 0.f, 0.f, 1.f}};
+    vkCmdPushConstants(command_buffer, mImpl->validation_pipeline_layout,
+                       VK_SHADER_STAGE_VERTEX_BIT, 0,
+                       sizeof(identity_transform), identity_transform.data());
+    constexpr std::array<float, 12> identity_normal{{
+        1.f, 0.f, 0.f, 0.f,
+        0.f, 1.f, 0.f, 0.f,
+        0.f, 0.f, 1.f, 0.f}};
+    vkCmdPushConstants(command_buffer, mImpl->validation_pipeline_layout,
+                       VK_SHADER_STAGE_VERTEX_BIT, sizeof(float) * 16,
+                       sizeof(identity_normal), identity_normal.data());
+    constexpr std::array<float, 4> validation_material{{0.f, 0.f, 0.f, 0.f}};
+    vkCmdPushConstants(command_buffer, mImpl->validation_pipeline_layout,
+                       VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
+                       sizeof(float) * 28,
+                       sizeof(validation_material), validation_material.data());
+    const LLVulkanTextureBinding validation_texture{};
+    const auto validation_descriptor = mImpl->descriptor_cache.find(
+        textureDescriptorKey(validation_texture));
+    if (validation_descriptor != mImpl->descriptor_cache.end())
+    {
+        const VkDescriptorSet validation_sets[] = {
+            validation_descriptor->second.set,
+            validation_descriptor->second.set,
+            validation_descriptor->second.set,
+            validation_descriptor->second.set,
+            validation_descriptor->second.set,
+            mImpl->identity_skin_descriptor,
+            mImpl->shadow_descriptor,
+            mImpl->radiance_probe_descriptor,
+            mImpl->irradiance_probe_descriptor,
+            mImpl->probe_metadata_descriptors[frame]};
+        vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
+                                mImpl->validation_pipeline_layout, 0, 10,
+                                validation_sets,
+                                0, nullptr);
+    }
+    constexpr VkDeviceSize vertex_offset = 0;
+    const VkBuffer vertex_buffer = mImpl->validation_vertex_buffer->handle();
+    vkCmdBindVertexBuffers(command_buffer, 0, 1,
+                           &vertex_buffer, &vertex_offset);
+    const VkBuffer validation_shadow_buffer =
+        mImpl->shadow_transform_buffers[frame]->handle();
+    vkCmdBindVertexBuffers(command_buffer, 1, 1,
+                           &validation_shadow_buffer, &vertex_offset);
+    vkCmdBindIndexBuffer(command_buffer,
+                         mImpl->validation_index_buffer->handle(), 0,
+                         VK_INDEX_TYPE_UINT16);
+    vkCmdDrawIndexed(command_buffer,
+                     static_cast<std::uint32_t>(validation_indices.size()),
+                     1, 0, 0, 0);
+    for (const LLVulkanDrawRecord& draw : mImpl->draw_queue)
+    {
+        auto mesh_entry = mImpl->mesh_cache.find(draw.mesh_identity);
+        if (mesh_entry == mImpl->mesh_cache.end() ||
+            mesh_entry->second.revision != draw.mesh_revision)
+            continue;
+        Impl::CachedMesh& mesh = mesh_entry->second;
+        // Shadow-only casters were replayed into the depth map above and must
+        // never leak into the swapchain color pass.
+        if (draw.shadow_pass)
+            continue;
+        if (draw.indexed)
+        {
+            if (!mesh.indices || draw.first_index > mesh.index_count ||
+                draw.index_count > mesh.index_count - draw.first_index)
+                continue;
+        }
+        else if (draw.first_vertex > mesh.vertex_count ||
+                 draw.vertex_count > mesh.vertex_count - draw.first_vertex)
+        {
+            continue;
+        }
+
+        auto pipeline_entry = mImpl->pipeline_cache.find(draw.pipeline);
+        if (pipeline_entry == mImpl->pipeline_cache.end())
+        {
+            VkPipeline pipeline = createCapturedPipeline(
+                mImpl->device, mImpl->render_pass,
+                mImpl->validation_pipeline_layout,
+                mImpl->validation_vertex_shader,
+                mImpl->validation_fragment_shader, draw.pipeline);
+            if (pipeline == VK_NULL_HANDLE)
+            {
+                ++mImpl->unsupported_state_count;
+                continue;
+            }
+            if (mImpl->pipeline_cache.size() >= 512)
+            {
+                auto evicted = mImpl->pipeline_cache.begin();
+                vkDestroyPipeline(mImpl->device, evicted->second, nullptr);
+                mImpl->pipeline_cache.erase(evicted);
+            }
+            pipeline_entry = mImpl->pipeline_cache.emplace(draw.pipeline,
+                                                            pipeline).first;
+        }
+
+        vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
+                          pipeline_entry->second);
+        vkCmdPushConstants(command_buffer, mImpl->validation_pipeline_layout,
+                           VK_SHADER_STAGE_VERTEX_BIT, 0,
+                           sizeof(draw.model_view_projection),
+                           draw.model_view_projection.data());
+        vkCmdPushConstants(command_buffer, mImpl->validation_pipeline_layout,
+                           VK_SHADER_STAGE_VERTEX_BIT, sizeof(float) * 16,
+                           sizeof(draw.normal_matrix), draw.normal_matrix.data());
+        const auto shadow_transform =
+            shadow_transform_indices.find(draw.mesh_identity);
+        const bool receives_shadow =
+            shadow_transform != shadow_transform_indices.end() &&
+            caster_transforms.find(draw.mesh_identity) != caster_transforms.end() &&
+            draw.lighting;
+        const std::array<float, 4> material{{
+            draw.alpha_mask ? draw.alpha_cutoff : 0.f,
+            draw.lighting ? 1.f : 0.f,
+            draw.srgb ? 1.f : 0.f,
+            static_cast<float>((draw.normal_mapped ? 1 : 0) |
+                               (draw.specular_mapped ? 2 : 0) |
+                               (draw.pbr ? 4 : 0) |
+                               (draw.occlusion_mapped ? 8 : 0) |
+                               (draw.emissive_mapped ? 16 : 0) |
+                               (draw.skinned && !draw.legacy_avatar ? 32 : 0) |
+                               (draw.legacy_avatar ? 64 : 0) |
+                               (draw.water ? 128 : 0) |
+                               (draw.underwater ? 256 : 0) |
+                               (draw.sky ? 512 : 0) |
+                               (draw.cloud ? 1024 : 0) |
+                               (draw.sun ? 2048 : 0) |
+                               (draw.moon ? 4096 : 0) |
+                               (receives_shadow ? 16384 : 0) |
+                               (draw.reflection_probes ? 32768 : 0))}};
+        vkCmdPushConstants(command_buffer, mImpl->validation_pipeline_layout,
+                           VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
+                           sizeof(float) * 28,
+                           sizeof(material), material.data());
+        const auto fallback_descriptor = mImpl->descriptor_cache.find(
+            textureDescriptorKey(LLVulkanTextureBinding{}));
+        if (fallback_descriptor != mImpl->descriptor_cache.end())
+        {
+            VkDescriptorSet material_sets[5] = {
+                fallback_descriptor->second.set,
+                fallback_descriptor->second.set,
+                fallback_descriptor->second.set,
+                fallback_descriptor->second.set,
+                fallback_descriptor->second.set};
+            const LLVulkanTextureBinding bindings[] = {
+                draw.texture, draw.normal_texture, draw.specular_texture,
+                draw.occlusion_texture, draw.emissive_texture};
+            for (std::size_t set = 0; set < 5; ++set)
+            {
+                const auto descriptor = mImpl->descriptor_cache.find(
+                    textureDescriptorKey(bindings[set]));
+                if (descriptor != mImpl->descriptor_cache.end())
+                    material_sets[set] = descriptor->second.set;
+            }
+            vkCmdBindDescriptorSets(command_buffer,
+                                    VK_PIPELINE_BIND_POINT_GRAPHICS,
+                                    mImpl->validation_pipeline_layout,
+                                    0, 5, material_sets, 0, nullptr);
+        }
+        VkDescriptorSet skin_descriptor = mImpl->identity_skin_descriptor;
+        if (draw.skinned)
+        {
+            const auto skin = mImpl->skin_cache.find(draw.skin_identity);
+            if (skin == mImpl->skin_cache.end() ||
+                skin->second.revision != draw.skin_revision)
+                continue;
+            skin_descriptor = skin->second.descriptor;
+        }
+        vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
+                                mImpl->validation_pipeline_layout, 5, 1,
+                                &skin_descriptor, 0, nullptr);
+        vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
+                                mImpl->validation_pipeline_layout, 6, 1,
+                                &mImpl->shadow_descriptor, 0, nullptr);
+        const VkDescriptorSet receiver_probe_sets[] = {
+            mImpl->radiance_probe_descriptor,
+            mImpl->irradiance_probe_descriptor};
+        vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
+                                mImpl->validation_pipeline_layout, 7, 2,
+                                receiver_probe_sets, 0, nullptr);
+        vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
+                                mImpl->validation_pipeline_layout, 9, 1,
+                                &mImpl->probe_metadata_descriptors[frame],
+                                0, nullptr);
+        const VkBuffer attribute_buffers[] = {
+            mesh.vertices->handle(), mesh.vertices->handle(),
+            mesh.vertices->handle(), mesh.vertices->handle(),
+            mesh.vertices->handle(), mesh.vertices->handle(),
+            mesh.vertices->handle(), mesh.vertices->handle()};
+        const bool has_color = (mesh.type_mask & LLVertexBuffer::MAP_COLOR) != 0;
+        const bool has_texcoord =
+            (mesh.type_mask & LLVertexBuffer::MAP_TEXCOORD0) != 0;
+        const bool has_normal =
+            (mesh.type_mask & LLVertexBuffer::MAP_NORMAL) != 0;
+        const bool has_tangent =
+            (mesh.type_mask & LLVertexBuffer::MAP_TANGENT) != 0;
+        const bool has_weights =
+            (mesh.type_mask & LLVertexBuffer::MAP_WEIGHT4) != 0;
+        const bool has_joints =
+            (mesh.type_mask & LLVertexBuffer::MAP_JOINT) != 0;
+        const bool has_legacy_weight =
+            (mesh.type_mask & LLVertexBuffer::MAP_WEIGHT) != 0;
+        const VkDeviceSize attribute_offsets[] = {
+            mesh.offsets[LLVertexBuffer::TYPE_VERTEX],
+            has_color ? mesh.offsets[LLVertexBuffer::TYPE_COLOR]
+                      : mesh.offsets[LLVertexBuffer::TYPE_VERTEX],
+            has_texcoord ? mesh.offsets[LLVertexBuffer::TYPE_TEXCOORD0]
+                         : mesh.offsets[LLVertexBuffer::TYPE_VERTEX],
+            has_normal ? mesh.offsets[LLVertexBuffer::TYPE_NORMAL]
+                       : mesh.offsets[LLVertexBuffer::TYPE_VERTEX],
+            has_tangent ? mesh.offsets[LLVertexBuffer::TYPE_TANGENT]
+                        : mesh.offsets[LLVertexBuffer::TYPE_VERTEX],
+            has_weights ? mesh.offsets[LLVertexBuffer::TYPE_WEIGHT4]
+                        : mesh.offsets[LLVertexBuffer::TYPE_VERTEX],
+            has_joints ? mesh.offsets[LLVertexBuffer::TYPE_JOINT]
+                       : mesh.offsets[LLVertexBuffer::TYPE_VERTEX],
+            has_legacy_weight ? mesh.offsets[LLVertexBuffer::TYPE_WEIGHT]
+                              : mesh.offsets[LLVertexBuffer::TYPE_VERTEX]};
+        vkCmdBindVertexBuffers(command_buffer, 0, 8, attribute_buffers,
+                               attribute_offsets);
+        const VkBuffer receiver_shadow_buffer =
+            mImpl->shadow_transform_buffers[frame]->handle();
+        const VkDeviceSize receiver_shadow_offset =
+            shadow_transform != shadow_transform_indices.end() ?
+            static_cast<VkDeviceSize>(shadow_transform->second) *
+                sizeof(DrawInstanceTransforms) : 0;
+        vkCmdBindVertexBuffers(command_buffer, 8, 1,
+                               &receiver_shadow_buffer,
+                               &receiver_shadow_offset);
+        if (draw.indexed)
+        {
+            vkCmdBindIndexBuffer(command_buffer, mesh.indices->handle(), 0,
+                                 mesh.index_stride == 4 ? VK_INDEX_TYPE_UINT32
+                                                        : VK_INDEX_TYPE_UINT16);
+            vkCmdDrawIndexed(command_buffer, draw.index_count, 1,
+                             draw.first_index, 0, 0);
+        }
+        else
+        {
+            vkCmdDraw(command_buffer, draw.vertex_count, 1,
+                      draw.first_vertex, 0);
+        }
+        ++mImpl->executed_draw_count;
+    }
+    mImpl->draw_queue.clear();
+    vkCmdEndRenderPass(command_buffer);
+    status = vkEndCommandBuffer(command_buffer);
+    if (status != VK_SUCCESS)
+    {
+        mImpl->error = "vkEndCommandBuffer failed with code " +
+                       std::to_string(status);
+        return LLVulkanFrameResult::ERROR;
+    }
+
+    constexpr VkPipelineStageFlags wait_stage =
+        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
+    const VkSubmitInfo submit_info{
+        VK_STRUCTURE_TYPE_SUBMIT_INFO,
+        nullptr,
+        1,
+        &mImpl->image_available[frame],
+        &wait_stage,
+        1,
+        &command_buffer,
+        1,
+        &mImpl->render_finished[frame]
+    };
+    status = vkResetFences(mImpl->device, 1, &mImpl->frame_fences[frame]);
+    if (status != VK_SUCCESS)
+    {
+        mImpl->error = "vkResetFences failed with code " +
+                       std::to_string(status);
+        return LLVulkanFrameResult::ERROR;
+    }
+    status = vkQueueSubmit(mImpl->graphics_queue, 1, &submit_info,
+                           mImpl->frame_fences[frame]);
+    if (status != VK_SUCCESS)
+    {
+        mImpl->error = "vkQueueSubmit failed with code " +
+                       std::to_string(status);
+        return LLVulkanFrameResult::ERROR;
+    }
+
+    const VkPresentInfoKHR present_info{
+        VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
+        nullptr,
+        1,
+        &mImpl->render_finished[frame],
+        1,
+        &mImpl->swapchain,
+        &image_index,
+        nullptr
+    };
+    status = vkQueuePresentKHR(mImpl->graphics_queue, &present_info);
+    mImpl->current_frame = (frame + 1) % Impl::frames_in_flight;
+    if (status == VK_ERROR_OUT_OF_DATE_KHR || status == VK_SUBOPTIMAL_KHR ||
+        acquired_suboptimal)
+    {
+        return LLVulkanFrameResult::SWAPCHAIN_OUT_OF_DATE;
+    }
+    if (status != VK_SUCCESS)
+    {
+        mImpl->error = "vkQueuePresentKHR failed with code " +
+                       std::to_string(status);
+        return LLVulkanFrameResult::ERROR;
+    }
+    return LLVulkanFrameResult::PRESENTED;
+#endif
+}
+
+void LLVulkanContext::queueProbeMetadata(
+    const std::vector<LLVulkanProbeRecord>& probes)
+{
+#if FIRESTORM_VULKAN_EXPERIMENTAL && LL_WINDOWS
+    std::lock_guard<std::mutex> lock(mImpl->pending_probe_mutex);
+    mImpl->pending_probe_metadata = probes;
+#else
+    (void)probes;
+#endif
+}
+
+void LLVulkanContext::queueProbeFaces(std::vector<LLVulkanProbeFace> faces)
+{
+#if FIRESTORM_VULKAN_EXPERIMENTAL && LL_WINDOWS
+    constexpr std::size_t max_pending_probe_faces = 512;
+    std::lock_guard<std::mutex> lock(mImpl->pending_probe_face_mutex);
+    for (LLVulkanProbeFace& face : faces)
+    {
+        auto existing = std::find_if(
+            mImpl->pending_probe_faces.begin(), mImpl->pending_probe_faces.end(),
+            [&face](const LLVulkanProbeFace& queued)
+            {
+                return queued.cube_index == face.cube_index &&
+                    queued.face == face.face &&
+                    queued.mip_level == face.mip_level &&
+                    queued.irradiance == face.irradiance;
+            });
+        if (existing != mImpl->pending_probe_faces.end())
+            *existing = std::move(face);
+        else
+            mImpl->pending_probe_faces.push_back(std::move(face));
+        if (mImpl->pending_probe_faces.size() > max_pending_probe_faces)
+            mImpl->pending_probe_faces.erase(
+                mImpl->pending_probe_faces.begin());
+    }
+#else
+    (void)faces;
+#endif
+}
+
+const std::string& LLVulkanContext::deviceName() const { return mImpl->device_name; }
+const std::string& LLVulkanContext::error() const { return mImpl->error; }
+unsigned LLVulkanContext::swapchainWidth() const { return mImpl->swapchain_width; }
+unsigned LLVulkanContext::swapchainHeight() const { return mImpl->swapchain_height; }
+unsigned LLVulkanContext::swapchainImageCount() const { return mImpl->swapchain_image_count; }
+const char* LLVulkanContext::frameResultName(LLVulkanFrameResult result) const
+{
+    switch (result)
+    {
+        case LLVulkanFrameResult::PRESENTED: return "presented";
+        case LLVulkanFrameResult::NOT_READY: return "not-ready";
+        case LLVulkanFrameResult::SWAPCHAIN_OUT_OF_DATE: return "out-of-date";
+        case LLVulkanFrameResult::ERROR: return "error";
+    }
+    return "unknown";
+}
diff --git a/indra/llrender/llvulkancontext.h b/indra/llrender/llvulkancontext.h
new file mode 100644
index 0000000000..67cc228686
--- /dev/null
+++ b/indra/llrender/llvulkancontext.h
@@ -0,0 +1,208 @@
+/**
+ * @file llvulkancontext.h
+ * @brief Experimental Vulkan instance and logical-device owner.
+ *
+ * $LicenseInfo:firstyear=2026&license=viewerlgpl$
+ * Copyright (C) 2026 Firestorm Vulkan contributors
+ * $/LicenseInfo$
+ */
+
+#ifndef LL_LLVULKANCONTEXT_H
+#define LL_LLVULKANCONTEXT_H
+
+#include <cstddef>
+#include <array>
+#include <cstdint>
+#include <memory>
+#include <string>
+#include <vector>
+
+class LLVertexBuffer;
+
+enum class LLVulkanFrameResult
+{
+    PRESENTED,
+    NOT_READY,
+    SWAPCHAIN_OUT_OF_DATE,
+    ERROR
+};
+
+enum class LLVulkanPrimitiveTopology
+{
+    TRIANGLE_LIST,
+    TRIANGLE_STRIP,
+    TRIANGLE_FAN,
+    POINT_LIST,
+    LINE_LIST,
+    LINE_STRIP,
+    UNSUPPORTED
+};
+
+enum class LLVulkanBlendFactor : std::uint8_t
+{
+    ZERO, ONE, SRC_COLOR, ONE_MINUS_SRC_COLOR, DST_COLOR,
+    ONE_MINUS_DST_COLOR, SRC_ALPHA, ONE_MINUS_SRC_ALPHA,
+    DST_ALPHA, ONE_MINUS_DST_ALPHA, UNSUPPORTED
+};
+
+enum class LLVulkanCompareOp : std::uint8_t
+{
+    NEVER, LESS, EQUAL, LESS_OR_EQUAL, GREATER, NOT_EQUAL,
+    GREATER_OR_EQUAL, ALWAYS, UNSUPPORTED
+};
+
+enum class LLVulkanCullMode : std::uint8_t { NONE, FRONT, BACK };
+enum class LLVulkanPolygonMode : std::uint8_t { FILL, LINE };
+enum class LLVulkanTextureAddress : std::uint8_t { REPEAT, MIRRORED_REPEAT, CLAMP };
+enum class LLVulkanTextureFilter : std::uint8_t { NEAREST, LINEAR, TRILINEAR, ANISOTROPIC };
+enum class LLVulkanShaderFamily : std::uint8_t
+{
+    UNLIT_TEXTURED, ALPHA_MASK, LIT_TEXTURED, LIT_ALPHA_MASK,
+    MATERIAL_TEXTURED, MATERIAL_ALPHA_MASK,
+    SKINNED_TEXTURED, SKINNED_ALPHA_MASK,
+    AVATAR_BODY, AVATAR_BODY_ALPHA_MASK,
+    WATER_SURFACE, UNDERWATER_SURFACE,
+    SKY_DOME, SKY_CLOUDS, SKY_SUN, SKY_MOON,
+    SHADOW_STATIC, SHADOW_ALPHA, SHADOW_SKINNED, SHADOW_AVATAR,
+    PROBE_MATERIAL, PROBE_MATERIAL_ALPHA
+};
+
+struct LLVulkanTextureBinding
+{
+    std::uint64_t identity = 0;
+    LLVulkanTextureAddress address = LLVulkanTextureAddress::REPEAT;
+    LLVulkanTextureFilter filter = LLVulkanTextureFilter::LINEAR;
+    bool has_mipmaps = false;
+};
+
+struct LLVulkanPipelineKey
+{
+    LLVulkanPrimitiveTopology topology = LLVulkanPrimitiveTopology::UNSUPPORTED;
+    LLVulkanBlendFactor color_src = LLVulkanBlendFactor::ONE;
+    LLVulkanBlendFactor color_dst = LLVulkanBlendFactor::ZERO;
+    LLVulkanBlendFactor alpha_src = LLVulkanBlendFactor::ONE;
+    LLVulkanBlendFactor alpha_dst = LLVulkanBlendFactor::ZERO;
+    LLVulkanCompareOp depth_compare = LLVulkanCompareOp::LESS_OR_EQUAL;
+    LLVulkanCullMode cull_mode = LLVulkanCullMode::BACK;
+    LLVulkanPolygonMode polygon_mode = LLVulkanPolygonMode::FILL;
+    LLVulkanShaderFamily shader_family = LLVulkanShaderFamily::UNLIT_TEXTURED;
+    bool blend_enabled = false;
+    bool depth_test_enabled = true;
+    bool depth_write_enabled = true;
+
+    bool operator==(const LLVulkanPipelineKey& rhs) const;
+};
+
+struct LLVulkanDrawRecord
+{
+    std::uint64_t mesh_identity = 0;
+    std::uint64_t mesh_revision = 0;
+    std::uint64_t skin_identity = 0;
+    std::uint64_t skin_revision = 0;
+    LLVulkanPrimitiveTopology topology = LLVulkanPrimitiveTopology::UNSUPPORTED;
+    std::uint32_t first_vertex = 0;
+    std::uint32_t vertex_count = 0;
+    std::uint32_t first_index = 0;
+    std::uint32_t index_count = 0;
+    bool indexed = false;
+    LLVulkanPipelineKey pipeline;
+    std::array<float, 16> model_view_projection{};
+    std::array<float, 16> model_view{};
+    std::array<float, 12> normal_matrix{};
+    LLVulkanTextureBinding texture;
+    LLVulkanTextureBinding normal_texture;
+    LLVulkanTextureBinding specular_texture;
+    LLVulkanTextureBinding occlusion_texture;
+    LLVulkanTextureBinding emissive_texture;
+    float alpha_cutoff = 0.f;
+    bool alpha_mask = false;
+    bool lighting = false;
+    bool srgb = false;
+    bool normal_mapped = false;
+    bool specular_mapped = false;
+    bool pbr = false;
+    bool occlusion_mapped = false;
+    bool emissive_mapped = false;
+    bool skinned = false;
+    bool legacy_avatar = false;
+    bool water = false;
+    bool underwater = false;
+    bool sky = false;
+    bool cloud = false;
+    bool sun = false;
+    bool moon = false;
+    bool shadow_pass = false;
+    bool reflection_probes = false;
+    float metallic_factor = 1.f;
+    float roughness_factor = 1.f;
+    float emissive_strength = 0.f;
+};
+
+struct LLVulkanProbeRecord
+{
+    std::array<float, 16> box{};
+    std::array<float, 4> sphere{};
+    std::array<float, 4> parameters{};
+    std::array<std::int32_t, 4> indices{};
+};
+
+struct LLVulkanProbeFace
+{
+    std::uint32_t cube_index = 0;
+    std::uint32_t face = 0;
+    std::uint32_t mip_level = 0;
+    std::uint32_t width = 0;
+    bool irradiance = false;
+    std::vector<std::uint16_t> rgba16f;
+};
+
+class LLVulkanContext
+{
+public:
+    static LLVulkanContext& instance();
+
+    bool initialize();
+    bool attachWindow(void* native_window, unsigned width, unsigned height);
+    void detachWindow();
+    void shutdown();
+    bool ready() const;
+    bool presentationReady() const;
+    LLVulkanFrameResult drawValidationFrame(float red, float green, float blue);
+    bool observeDraw(const LLVertexBuffer& source, std::uint32_t mode,
+                     std::uint32_t first, std::uint32_t count,
+                     bool indexed);
+    void observeTextureUpload(std::uint64_t identity, std::uint32_t width,
+                              std::uint32_t height, std::uint32_t components,
+                              const void* pixels, std::size_t byte_count,
+                              bool generate_mipmaps);
+    void queueProbeMetadata(const std::vector<LLVulkanProbeRecord>& probes);
+    void queueProbeFaces(std::vector<LLVulkanProbeFace> faces);
+    std::size_t cachedMeshCount() const;
+    std::size_t queuedDrawCount() const;
+    std::size_t pipelineKeyCount() const;
+    std::size_t pipelineVariantCount() const;
+    std::uint64_t unsupportedStateCount() const;
+    std::uint64_t executedDrawCount() const;
+    std::size_t descriptorCount() const;
+    std::size_t cachedTextureCount() const;
+    std::uint64_t unsupportedDrawCount() const;
+    void clearQueuedDraws();
+    const std::string& deviceName() const;
+    const std::string& error() const;
+    unsigned swapchainWidth() const;
+    unsigned swapchainHeight() const;
+    unsigned swapchainImageCount() const;
+    const char* frameResultName(LLVulkanFrameResult result) const;
+
+    LLVulkanContext(const LLVulkanContext&) = delete;
+    LLVulkanContext& operator=(const LLVulkanContext&) = delete;
+
+private:
+    LLVulkanContext();
+    ~LLVulkanContext();
+
+    struct Impl;
+    std::unique_ptr<Impl> mImpl;
+};
+
+#endif
diff --git a/indra/llrender/llvulkanprobe.cpp b/indra/llrender/llvulkanprobe.cpp
new file mode 100644
index 0000000000..b7778787b3
--- /dev/null
+++ b/indra/llrender/llvulkanprobe.cpp
@@ -0,0 +1,107 @@
+/**
+ * @file llvulkanprobe.cpp
+ * @brief Dependency-free Vulkan loader capability probe.
+ *
+ * $LicenseInfo:firstyear=2026&license=viewerlgpl$
+ * Copyright (C) 2026 Firestorm Vulkan contributors
+ * $/LicenseInfo$
+ */
+
+#include "linden_common.h"
+#include "llvulkanprobe.h"
+
+#include <sstream>
+
+#if LL_WINDOWS
+# define WIN32_LEAN_AND_MEAN
+# include <windows.h>
+#elif LL_LINUX
+# include <dlfcn.h>
+#endif
+
+namespace
+{
+using VkResult = std::int32_t;
+using PFN_vkEnumerateInstanceVersion = VkResult (*)(std::uint32_t*);
+constexpr VkResult VK_SUCCESS_VALUE = 0;
+constexpr std::uint32_t VK_API_VERSION_1_0_VALUE = (1u << 22);
+}
+
+unsigned LLVulkanProbeResult::major() const
+{
+    return api_version >> 22;
+}
+
+unsigned LLVulkanProbeResult::minor() const
+{
+    return (api_version >> 12) & 0x3ffu;
+}
+
+unsigned LLVulkanProbeResult::patch() const
+{
+    return api_version & 0xfffu;
+}
+
+std::string LLVulkanProbeResult::versionString() const
+{
+    if (!loader_available)
+    {
+        return "unavailable";
+    }
+    std::ostringstream out;
+    out << major() << '.' << minor() << '.' << patch();
+    return out.str();
+}
+
+LLVulkanProbeResult LLVulkanProbe::run()
+{
+    LLVulkanProbeResult result;
+
+#if LL_WINDOWS
+    result.loader_name = "vulkan-1.dll";
+    HMODULE library = LoadLibraryA(result.loader_name.c_str());
+    if (!library)
+    {
+        result.error = "Vulkan loader was not found";
+        return result;
+    }
+    result.loader_available = true;
+    auto enumerate_version = reinterpret_cast<PFN_vkEnumerateInstanceVersion>(
+        GetProcAddress(library, "vkEnumerateInstanceVersion"));
+#elif LL_LINUX
+    result.loader_name = "libvulkan.so.1";
+    void* library = dlopen(result.loader_name.c_str(), RTLD_NOW | RTLD_LOCAL);
+    if (!library)
+    {
+        const char* detail = dlerror();
+        result.error = detail ? detail : "Vulkan loader was not found";
+        return result;
+    }
+    result.loader_available = true;
+    auto enumerate_version = reinterpret_cast<PFN_vkEnumerateInstanceVersion>(
+        dlsym(library, "vkEnumerateInstanceVersion"));
+#else
+    result.error = "Vulkan probe is not implemented for this platform";
+    return result;
+#endif
+
+    // Vulkan 1.0 loaders predate vkEnumerateInstanceVersion. Loader presence is
+    // still valid; report 1.0 rather than rejecting it.
+    result.api_version = VK_API_VERSION_1_0_VALUE;
+    if (enumerate_version)
+    {
+        result.enumerate_version_available = true;
+        if (enumerate_version(&result.api_version) != VK_SUCCESS_VALUE)
+        {
+            result.error = "vkEnumerateInstanceVersion failed";
+            result.api_version = 0;
+        }
+    }
+
+#if LL_WINDOWS
+    FreeLibrary(library);
+#elif LL_LINUX
+    dlclose(library);
+#endif
+    return result;
+}
diff --git a/indra/llrender/llvulkanprobe.h b/indra/llrender/llvulkanprobe.h
new file mode 100644
index 0000000000..38bf54f0b5
--- /dev/null
+++ b/indra/llrender/llvulkanprobe.h
@@ -0,0 +1,39 @@
+/**
+ * @file llvulkanprobe.h
+ * @brief Dependency-free Vulkan loader capability probe.
+ *
+ * $LicenseInfo:firstyear=2026&license=viewerlgpl$
+ * Copyright (C) 2026 Firestorm Vulkan contributors
+ * $/LicenseInfo$
+ */
+
+#ifndef LL_LLVULKANPROBE_H
+#define LL_LLVULKANPROBE_H
+
+#include <cstdint>
+#include <string>
+
+struct LLVulkanProbeResult
+{
+    bool loader_available = false;
+    bool enumerate_version_available = false;
+    std::uint32_t api_version = 0;
+    std::string loader_name;
+    std::string error;
+
+    unsigned major() const;
+    unsigned minor() const;
+    unsigned patch() const;
+    std::string versionString() const;
+};
+
+class LLVulkanProbe
+{
+public:
+    // Loads the platform Vulkan loader only long enough to query its supported
+    // instance API version. No instance, device, window or rendering state is
+    // created by this diagnostic probe.
+    static LLVulkanProbeResult run();
+};
+
+#endif
diff --git a/indra/llrender/llvulkanvalidationshaders.h b/indra/llrender/llvulkanvalidationshaders.h
new file mode 100644
index 0000000000..9d9599961c
--- /dev/null
+++ b/indra/llrender/llvulkanvalidationshaders.h
@@ -0,0 +1,8778 @@
+// Generated SPIR-V for the Vulkan bootstrap validation pipeline.
+// Source GLSL lives in shaders/vulkan. Regenerate both arrays together.
+#ifndef LL_LLVULKANVALIDATIONSHADERS_H
+#define LL_LLVULKANVALIDATIONSHADERS_H
+
+#include <cstdint>
+
+namespace LLVulkanValidationShaders
+{
+inline constexpr std::uint32_t validation_vertex_spirv[] = {
+    0x07230203u,
+    0x00010000u,
+    0x00080008u,
+    0x0000020bu,
+    0x00000000u,
+    0x00020011u,
+    0x00000001u,
+    0x0006000bu,
+    0x00000001u,
+    0x4c534c47u,
+    0x6474732eu,
+    0x3035342eu,
+    0x00000000u,
+    0x0003000eu,
+    0x00000000u,
+    0x00000001u,
+    0x001c000fu,
+    0x00000000u,
+    0x00000004u,
+    0x6e69616du,
+    0x00000000u,
+    0x00000038u,
+    0x00000040u,
+    0x00000096u,
+    0x0000011bu,
+    0x00000130u,
+    0x00000145u,
+    0x00000164u,
+    0x00000166u,
+    0x00000168u,
+    0x0000016au,
+    0x0000018au,
+    0x0000019cu,
+    0x0000019eu,
+    0x000001a0u,
+    0x000001a2u,
+    0x000001bau,
+    0x000001c6u,
+    0x000001d6u,
+    0x000001d7u,
+    0x000001dcu,
+    0x000001deu,
+    0x000001fbu,
+    0x00000200u,
+    0x00030003u,
+    0x00000002u,
+    0x000001c2u,
+    0x00040005u,
+    0x00000004u,
+    0x6e69616du,
+    0x00000000u,
+    0x00070005u,
+    0x0000000bu,
+    0x6e696b73u,
+    0x736f705fu,
+    0x6f697469u,
+    0x6676286eu,
+    0x00003b33u,
+    0x00040005u,
+    0x0000000au,
+    0x756c6176u,
+    0x00000065u,
+    0x00070005u,
+    0x0000000eu,
+    0x6e696b73u,
+    0x7269645fu,
+    0x69746365u,
+    0x76286e6fu,
+    0x003b3366u,
+    0x00040005u,
+    0x0000000du,
+    0x756c6176u,
+    0x00000065u,
+    0x00080005u,
+    0x00000013u,
+    0x6167656cu,
+    0x735f7963u,
+    0x5f6e696bu,
+    0x6e617274u,
+    0x726f6673u,
+    0x0000286du,
+    0x00090005u,
+    0x00000016u,
+    0x6167656cu,
+    0x735f7963u,
+    0x5f6e696bu,
+    0x69736f70u,
+    0x6e6f6974u,
+    0x33667628u,
+    0x0000003bu,
+    0x00040005u,
+    0x00000015u,
+    0x756c6176u,
+    0x00000065u,
+    0x00090005u,
+    0x00000019u,
+    0x6167656cu,
+    0x735f7963u,
+    0x5f6e696bu,
+    0x65726964u,
+    0x6f697463u,
+    0x6676286eu,
+    0x00003b33u,
+    0x00040005u,
+    0x00000018u,
+    0x756c6176u,
+    0x00000065u,
+    0x00040005u,
+    0x0000001cu,
+    0x72756f73u,
+    0x00006563u,
+    0x00040005u,
+    0x00000023u,
+    0x75736572u,
+    0x0000746cu,
+    0x00050005u,
+    0x00000028u,
+    0x6c666e69u,
+    0x636e6575u,
+    0x00000065u,
+    0x00030005u,
+    0x00000035u,
+    0x00776f72u,
+    0x00040005u,
+    0x00000038u,
+    0x6e696f6au,
+    0x00007374u,
+    0x00040005u,
+    0x00000040u,
+    0x67696577u,
+    0x00737468u,
+    0x00050005u,
+    0x00000046u,
+    0x6e696b53u,
+    0x656c6150u,
+    0x00657474u,
+    0x00050006u,
+    0x00000046u,
+    0x00000000u,
+    0x73776f72u,
+    0x00000000u,
+    0x00040005u,
+    0x00000048u,
+    0x6e696b73u,
+    0x00000000u,
+    0x00040005u,
+    0x00000067u,
+    0x75736572u,
+    0x0000746cu,
+    0x00050005u,
+    0x00000068u,
+    0x6c666e69u,
+    0x636e6575u,
+    0x00000065u,
+    0x00030005u,
+    0x00000070u,
+    0x00776f72u,
+    0x00040005u,
+    0x00000095u,
+    0x6e696f6au,
+    0x00000074u,
+    0x00060005u,
+    0x00000096u,
+    0x6167656cu,
+    0x775f7963u,
+    0x68676965u,
+    0x00000074u,
+    0x00040005u,
+    0x0000009bu,
+    0x6e656c62u,
+    0x00000064u,
+    0x00050005u,
+    0x000000d5u,
+    0x6e617274u,
+    0x726f6673u,
+    0x0000006du,
+    0x00040005u,
+    0x000000d7u,
+    0x72756f73u,
+    0x00006563u,
+    0x00050005u,
+    0x000000edu,
+    0x6e617274u,
+    0x726f6673u,
+    0x0000006du,
+    0x00040005u,
+    0x00000102u,
+    0x6e696b73u,
+    0x0064656eu,
+    0x00050005u,
+    0x00000104u,
+    0x6e617254u,
+    0x726f6673u,
+    0x0000006du,
+    0x00090006u,
+    0x00000104u,
+    0x00000000u,
+    0x65646f6du,
+    0x69765f6cu,
+    0x705f7765u,
+    0x656a6f72u,
+    0x6f697463u,
+    0x0000006eu,
+    0x00070006u,
+    0x00000104u,
+    0x00000001u,
+    0x6d726f6eu,
+    0x635f6c61u,
+    0x6d756c6fu,
+    0x0000306eu,
+    0x00070006u,
+    0x00000104u,
+    0x00000002u,
+    0x6d726f6eu,
+    0x635f6c61u,
+    0x6d756c6fu,
+    0x0000316eu,
+    0x00070006u,
+    0x00000104u,
+    0x00000003u,
+    0x6d726f6eu,
+    0x635f6c61u,
+    0x6d756c6fu,
+    0x0000326eu,
+    0x00060006u,
+    0x00000104u,
+    0x00000004u,
+    0x6574616du,
+    0x6c616972u,
+    0x00000000u,
+    0x00050005u,
+    0x00000106u,
+    0x6e617274u,
+    0x726f6673u,
+    0x0000006du,
+    0x00060005u,
+    0x0000010eu,
+    0x6167656cu,
+    0x615f7963u,
+    0x61746176u,
+    0x00000072u,
+    0x00060005u,
+    0x00000115u,
+    0x77617264u,
+    0x736f705fu,
+    0x6f697469u,
+    0x0000006eu,
+    0x00050005u,
+    0x0000011bu,
+    0x69736f70u,
+    0x6e6f6974u,
+    0x00000000u,
+    0x00040005u,
+    0x0000011cu,
+    0x61726170u,
+    0x0000006du,
+    0x00040005u,
+    0x00000124u,
+    0x61726170u,
+    0x0000006du,
+    0x00050005u,
+    0x0000012bu,
+    0x77617264u,
+    0x726f6e5fu,
+    0x006c616du,
+    0x00040005u,
+    0x00000130u,
+    0x6d726f6eu,
+    0x00006c61u,
+    0x00040005u,
+    0x00000131u,
+    0x61726170u,
+    0x0000006du,
+    0x00040005u,
+    0x00000139u,
+    0x61726170u,
+    0x0000006du,
+    0x00060005u,
+    0x00000140u,
+    0x77617264u,
+    0x6e61745fu,
+    0x746e6567u,
+    0x00000000u,
+    0x00040005u,
+    0x00000145u,
+    0x676e6174u,
+    0x00746e65u,
+    0x00040005u,
+    0x00000146u,
+    0x61726170u,
+    0x0000006du,
+    0x00040005u,
+    0x0000014fu,
+    0x61726170u,
+    0x0000006du,
+    0x00040005u,
+    0x00000158u,
+    0x70696c63u,
+    0x00000000u,
+    0x00060005u,
+    0x00000163u,
+    0x64616873u,
+    0x6d5f776fu,
+    0x69727461u,
+    0x00000078u,
+    0x00060005u,
+    0x00000164u,
+    0x64616873u,
+    0x635f776fu,
+    0x6d756c6fu,
+    0x0000306eu,
+    0x00060005u,
+    0x00000166u,
+    0x64616873u,
+    0x635f776fu,
+    0x6d756c6fu,
+    0x0000316eu,
+    0x00060005u,
+    0x00000168u,
+    0x64616873u,
+    0x635f776fu,
+    0x6d756c6fu,
+    0x0000326eu,
+    0x00060005u,
+    0x0000016au,
+    0x64616873u,
+    0x635f776fu,
+    0x6d756c6fu,
+    0x0000336eu,
+    0x00050005u,
+    0x00000181u,
+    0x64616873u,
+    0x635f776fu,
+    0x0070696cu,
+    0x00070005u,
+    0x0000018au,
+    0x74726576u,
+    0x735f7865u,
+    0x6f646168u,
+    0x6f635f77u,
+    0x0064726fu,
+    0x00070005u,
+    0x0000019bu,
+    0x65646f6du,
+    0x69765f6cu,
+    0x6d5f7765u,
+    0x69727461u,
+    0x00000078u,
+    0x00070005u,
+    0x0000019cu,
+    0x65646f6du,
+    0x69765f6cu,
+    0x635f7765u,
+    0x6d756c6fu,
+    0x0000306eu,
+    0x00070005u,
+    0x0000019eu,
+    0x65646f6du,
+    0x69765f6cu,
+    0x635f7765u,
+    0x6d756c6fu,
+    0x0000316eu,
+    0x00070005u,
+    0x000001a0u,
+    0x65646f6du,
+    0x69765f6cu,
+    0x635f7765u,
+    0x6d756c6fu,
+    0x0000326eu,
+    0x00070005u,
+    0x000001a2u,
+    0x65646f6du,
+    0x69765f6cu,
+    0x635f7765u,
+    0x6d756c6fu,
+    0x0000336eu,
+    0x00080005u,
+    0x000001bau,
+    0x74726576u,
+    0x765f7865u,
+    0x5f776569u,
+    0x69736f70u,
+    0x6e6f6974u,
+    0x00000000u,
+    0x00060005u,
+    0x000001c4u,
+    0x505f6c67u,
+    0x65567265u,
+    0x78657472u,
+    0x00000000u,
+    0x00060006u,
+    0x000001c4u,
+    0x00000000u,
+    0x505f6c67u,
+    0x7469736fu,
+    0x006e6f69u,
+    0x00070006u,
+    0x000001c4u,
+    0x00000001u,
+    0x505f6c67u,
+    0x746e696fu,
+    0x657a6953u,
+    0x00000000u,
+    0x00070006u,
+    0x000001c4u,
+    0x00000002u,
+    0x435f6c67u,
+    0x4470696cu,
+    0x61747369u,
+    0x0065636eu,
+    0x00070006u,
+    0x000001c4u,
+    0x00000003u,
+    0x435f6c67u,
+    0x446c6c75u,
+    0x61747369u,
+    0x0065636eu,
+    0x00030005u,
+    0x000001c6u,
+    0x00000000u,
+    0x00060005u,
+    0x000001d6u,
+    0x74726576u,
+    0x635f7865u,
+    0x726f6c6fu,
+    0x00000000u,
+    0x00040005u,
+    0x000001d7u,
+    0x6f6c6f63u,
+    0x00000072u,
+    0x00060005u,
+    0x000001dcu,
+    0x74726576u,
+    0x745f7865u,
+    0x6f637865u,
+    0x0064726fu,
+    0x00050005u,
+    0x000001deu,
+    0x63786574u,
+    0x64726f6fu,
+    0x00000000u,
+    0x00060005u,
+    0x000001e2u,
+    0x6d726f6eu,
+    0x6d5f6c61u,
+    0x69727461u,
+    0x00000078u,
+    0x00060005u,
+    0x000001fbu,
+    0x74726576u,
+    0x6e5f7865u,
+    0x616d726fu,
+    0x0000006cu,
+    0x00060005u,
+    0x00000200u,
+    0x74726576u,
+    0x745f7865u,
+    0x65676e61u,
+    0x0000746eu,
+    0x00040047u,
+    0x00000038u,
+    0x0000001eu,
+    0x00000006u,
+    0x00040047u,
+    0x00000040u,
+    0x0000001eu,
+    0x00000005u,
+    0x00040047u,
+    0x00000045u,
+    0x00000006u,
+    0x00000010u,
+    0x00040048u,
+    0x00000046u,
+    0x00000000u,
+    0x00000018u,
+    0x00050048u,
+    0x00000046u,
+    0x00000000u,
+    0x00000023u,
+    0x00000000u,
+    0x00030047u,
+    0x00000046u,
+    0x00000003u,
+    0x00040047u,
+    0x00000048u,
+    0x00000022u,
+    0x00000005u,
+    0x00040047u,
+    0x00000048u,
+    0x00000021u,
+    0x00000000u,
+    0x00040047u,
+    0x00000096u,
+    0x0000001eu,
+    0x00000007u,
+    0x00040048u,
+    0x00000104u,
+    0x00000000u,
+    0x00000005u,
+    0x00050048u,
+    0x00000104u,
+    0x00000000u,
+    0x00000023u,
+    0x00000000u,
+    0x00050048u,
+    0x00000104u,
+    0x00000000u,
+    0x00000007u,
+    0x00000010u,
+    0x00050048u,
+    0x00000104u,
+    0x00000001u,
+    0x00000023u,
+    0x00000040u,
+    0x00050048u,
+    0x00000104u,
+    0x00000002u,
+    0x00000023u,
+    0x00000050u,
+    0x00050048u,
+    0x00000104u,
+    0x00000003u,
+    0x00000023u,
+    0x00000060u,
+    0x00050048u,
+    0x00000104u,
+    0x00000004u,
+    0x00000023u,
+    0x00000070u,
+    0x00030047u,
+    0x00000104u,
+    0x00000002u,
+    0x00040047u,
+    0x0000011bu,
+    0x0000001eu,
+    0x00000000u,
+    0x00040047u,
+    0x00000130u,
+    0x0000001eu,
+    0x00000003u,
+    0x00040047u,
+    0x00000145u,
+    0x0000001eu,
+    0x00000004u,
+    0x00040047u,
+    0x00000164u,
+    0x0000001eu,
+    0x00000008u,
+    0x00040047u,
+    0x00000166u,
+    0x0000001eu,
+    0x00000009u,
+    0x00040047u,
+    0x00000168u,
+    0x0000001eu,
+    0x0000000au,
+    0x00040047u,
+    0x0000016au,
+    0x0000001eu,
+    0x0000000bu,
+    0x00040047u,
+    0x0000018au,
+    0x0000001eu,
+    0x00000004u,
+    0x00040047u,
+    0x0000019cu,
+    0x0000001eu,
+    0x0000000cu,
+    0x00040047u,
+    0x0000019eu,
+    0x0000001eu,
+    0x0000000du,
+    0x00040047u,
+    0x000001a0u,
+    0x0000001eu,
+    0x0000000eu,
+    0x00040047u,
+    0x000001a2u,
+    0x0000001eu,
+    0x0000000fu,
+    0x00040047u,
+    0x000001bau,
+    0x0000001eu,
+    0x00000005u,
+    0x00050048u,
+    0x000001c4u,
+    0x00000000u,
+    0x0000000bu,
+    0x00000000u,
+    0x00050048u,
+    0x000001c4u,
+    0x00000001u,
+    0x0000000bu,
+    0x00000001u,
+    0x00050048u,
+    0x000001c4u,
+    0x00000002u,
+    0x0000000bu,
+    0x00000003u,
+    0x00050048u,
+    0x000001c4u,
+    0x00000003u,
+    0x0000000bu,
+    0x00000004u,
+    0x00030047u,
+    0x000001c4u,
+    0x00000002u,
+    0x00040047u,
+    0x000001d6u,
+    0x0000001eu,
+    0x00000000u,
+    0x00040047u,
+    0x000001d7u,
+    0x0000001eu,
+    0x00000001u,
+    0x00040047u,
+    0x000001dcu,
+    0x0000001eu,
+    0x00000001u,
+    0x00040047u,
+    0x000001deu,
+    0x0000001eu,
+    0x00000002u,
+    0x00040047u,
+    0x000001fbu,
+    0x0000001eu,
+    0x00000002u,
+    0x00040047u,
+    0x00000200u,
+    0x0000001eu,
+    0x00000003u,
+    0x00020013u,
+    0x00000002u,
+    0x00030021u,
+    0x00000003u,
+    0x00000002u,
+    0x00030016u,
+    0x00000006u,
+    0x00000020u,
+    0x00040017u,
+    0x00000007u,
+    0x00000006u,
+    0x00000003u,
+    0x00040020u,
+    0x00000008u,
+    0x00000007u,
+    0x00000007u,
+    0x00040021u,
+    0x00000009u,
+    0x00000007u,
+    0x00000008u,
+    0x00040017u,
+    0x00000010u,
+    0x00000006u,
+    0x00000004u,
+    0x00040018u,
+    0x00000011u,
+    0x00000010u,
+    0x00000003u,
+    0x00030021u,
+    0x00000012u,
+    0x00000011u,
+    0x00040020u,
+    0x0000001bu,
+    0x00000007u,
+    0x00000010u,
+    0x0004002bu,
+    0x00000006u,
+    0x0000001eu,
+    0x3f800000u,
+    0x0004002bu,
+    0x00000006u,
+    0x00000024u,
+    0x00000000u,
+    0x0006002cu,
+    0x00000007u,
+    0x00000025u,
+    0x00000024u,
+    0x00000024u,
+    0x00000024u,
+    0x00040015u,
+    0x00000026u,
+    0x00000020u,
+    0x00000001u,
+    0x00040020u,
+    0x00000027u,
+    0x00000007u,
+    0x00000026u,
+    0x0004002bu,
+    0x00000026u,
+    0x00000029u,
+    0x00000000u,
+    0x0004002bu,
+    0x00000026u,
+    0x00000030u,
+    0x00000004u,
+    0x00020014u,
+    0x00000031u,
+    0x00040015u,
+    0x00000033u,
+    0x00000020u,
+    0x00000000u,
+    0x00040020u,
+    0x00000034u,
+    0x00000007u,
+    0x00000033u,
+    0x00040017u,
+    0x00000036u,
+    0x00000033u,
+    0x00000004u,
+    0x00040020u,
+    0x00000037u,
+    0x00000001u,
+    0x00000036u,
+    0x0004003bu,
+    0x00000037u,
+    0x00000038u,
+    0x00000001u,
+    0x00040020u,
+    0x0000003au,
+    0x00000001u,
+    0x00000033u,
+    0x0004002bu,
+    0x00000033u,
+    0x0000003du,
+    0x00000003u,
+    0x00040020u,
+    0x0000003fu,
+    0x00000001u,
+    0x00000010u,
+    0x0004003bu,
+    0x0000003fu,
+    0x00000040u,
+    0x00000001u,
+    0x00040020u,
+    0x00000042u,
+    0x00000001u,
+    0x00000006u,
+    0x0003001du,
+    0x00000045u,
+    0x00000010u,
+    0x0003001eu,
+    0x00000046u,
+    0x00000045u,
+    0x00040020u,
+    0x00000047u,
+    0x00000002u,
+    0x00000046u,
+    0x0004003bu,
+    0x00000047u,
+    0x00000048u,
+    0x00000002u,
+    0x00040020u,
+    0x0000004au,
+    0x00000002u,
+    0x00000010u,
+    0x0004002bu,
+    0x00000033u,
+    0x00000050u,
+    0x00000001u,
+    0x0004002bu,
+    0x00000033u,
+    0x00000057u,
+    0x00000002u,
+    0x0004002bu,
+    0x00000026u,
+    0x00000062u,
+    0x00000001u,
+    0x0004003bu,
+    0x00000042u,
+    0x00000096u,
+    0x00000001u,
+    0x00040020u,
+    0x0000009au,
+    0x00000007u,
+    0x00000006u,
+    0x0004002bu,
+    0x00000026u,
+    0x000000a9u,
+    0x0000000fu,
+    0x0004002bu,
+    0x00000026u,
+    0x000000aeu,
+    0x00000010u,
+    0x0004002bu,
+    0x00000026u,
+    0x000000b6u,
+    0x0000001eu,
+    0x0004002bu,
+    0x00000026u,
+    0x000000bbu,
+    0x0000001fu,
+    0x00040020u,
+    0x000000d4u,
+    0x00000007u,
+    0x00000011u,
+    0x0004002bu,
+    0x00000026u,
+    0x000000e5u,
+    0x00000002u,
+    0x00040020u,
+    0x00000101u,
+    0x00000007u,
+    0x00000031u,
+    0x00040018u,
+    0x00000103u,
+    0x00000010u,
+    0x00000004u,
+    0x0007001eu,
+    0x00000104u,
+    0x00000103u,
+    0x00000010u,
+    0x00000010u,
+    0x00000010u,
+    0x00000010u,
+    0x00040020u,
+    0x00000105u,
+    0x00000009u,
+    0x00000104u,
+    0x0004003bu,
+    0x00000105u,
+    0x00000106u,
+    0x00000009u,
+    0x00040020u,
+    0x00000107u,
+    0x00000009u,
+    0x00000006u,
+    0x0004002bu,
+    0x00000026u,
+    0x0000010bu,
+    0x00000020u,
+    0x0004002bu,
+    0x00000026u,
+    0x00000112u,
+    0x00000040u,
+    0x00040020u,
+    0x0000011au,
+    0x00000001u,
+    0x00000007u,
+    0x0004003bu,
+    0x0000011au,
+    0x0000011bu,
+    0x00000001u,
+    0x0004003bu,
+    0x0000011au,
+    0x00000130u,
+    0x00000001u,
+    0x0004003bu,
+    0x0000003fu,
+    0x00000145u,
+    0x00000001u,
+    0x00040020u,
+    0x00000159u,
+    0x00000009u,
+    0x00000103u,
+    0x00040020u,
+    0x00000162u,
+    0x00000007u,
+    0x00000103u,
+    0x0004003bu,
+    0x0000003fu,
+    0x00000164u,
+    0x00000001u,
+    0x0004003bu,
+    0x0000003fu,
+    0x00000166u,
+    0x00000001u,
+    0x0004003bu,
+    0x0000003fu,
+    0x00000168u,
+    0x00000001u,
+    0x0004003bu,
+    0x0000003fu,
+    0x0000016au,
+    0x00000001u,
+    0x00040020u,
+    0x00000189u,
+    0x00000003u,
+    0x00000010u,
+    0x0004003bu,
+    0x00000189u,
+    0x0000018au,
+    0x00000003u,
+    0x0004002bu,
+    0x00000033u,
+    0x0000018bu,
+    0x00000000u,
+    0x0004002bu,
+    0x00000006u,
+    0x00000196u,
+    0x3f000000u,
+    0x0004003bu,
+    0x0000003fu,
+    0x0000019cu,
+    0x00000001u,
+    0x0004003bu,
+    0x0000003fu,
+    0x0000019eu,
+    0x00000001u,
+    0x0004003bu,
+    0x0000003fu,
+    0x000001a0u,
+    0x00000001u,
+    0x0004003bu,
+    0x0000003fu,
+    0x000001a2u,
+    0x00000001u,
+    0x00040020u,
+    0x000001b9u,
+    0x00000003u,
+    0x00000007u,
+    0x0004003bu,
+    0x000001b9u,
+    0x000001bau,
+    0x00000003u,
+    0x0004001cu,
+    0x000001c3u,
+    0x00000006u,
+    0x00000050u,
+    0x0006001eu,
+    0x000001c4u,
+    0x00000010u,
+    0x00000006u,
+    0x000001c3u,
+    0x000001c3u,
+    0x00040020u,
+    0x000001c5u,
+    0x00000003u,
+    0x000001c4u,
+    0x0004003bu,
+    0x000001c5u,
+    0x000001c6u,
+    0x00000003u,
+    0x0004003bu,
+    0x000001b9u,
+    0x000001d6u,
+    0x00000003u,
+    0x0004003bu,
+    0x0000003fu,
+    0x000001d7u,
+    0x00000001u,
+    0x00040017u,
+    0x000001dau,
+    0x00000006u,
+    0x00000002u,
+    0x00040020u,
+    0x000001dbu,
+    0x00000003u,
+    0x000001dau,
+    0x0004003bu,
+    0x000001dbu,
+    0x000001dcu,
+    0x00000003u,
+    0x00040020u,
+    0x000001ddu,
+    0x00000001u,
+    0x000001dau,
+    0x0004003bu,
+    0x000001ddu,
+    0x000001deu,
+    0x00000001u,
+    0x00040018u,
+    0x000001e0u,
+    0x00000007u,
+    0x00000003u,
+    0x00040020u,
+    0x000001e1u,
+    0x00000007u,
+    0x000001e0u,
+    0x00040020u,
+    0x000001e3u,
+    0x00000009u,
+    0x00000010u,
+    0x0004002bu,
+    0x00000026u,
+    0x000001eau,
+    0x00000003u,
+    0x0004003bu,
+    0x000001b9u,
+    0x000001fbu,
+    0x00000003u,
+    0x0004003bu,
+    0x00000189u,
+    0x00000200u,
+    0x00000003u,
+    0x00050036u,
+    0x00000002u,
+    0x00000004u,
+    0x00000000u,
+    0x00000003u,
+    0x000200f8u,
+    0x00000005u,
+    0x0004003bu,
+    0x00000101u,
+    0x00000102u,
+    0x00000007u,
+    0x0004003bu,
+    0x00000101u,
+    0x0000010eu,
+    0x00000007u,
+    0x0004003bu,
+    0x00000008u,
+    0x00000115u,
+    0x00000007u,
+    0x0004003bu,
+    0x00000008u,
+    0x00000117u,
+    0x00000007u,
+    0x0004003bu,
+    0x00000008u,
+    0x0000011cu,
+    0x00000007u,
+    0x0004003bu,
+    0x00000008u,
+    0x00000121u,
+    0x00000007u,
+    0x0004003bu,
+    0x00000008u,
+    0x00000124u,
+    0x00000007u,
+    0x0004003bu,
+    0x00000008u,
+    0x0000012bu,
+    0x00000007u,
+    0x0004003bu,
+    0x00000008u,
+    0x0000012du,
+    0x00000007u,
+    0x0004003bu,
+    0x00000008u,
+    0x00000131u,
+    0x00000007u,
+    0x0004003bu,
+    0x00000008u,
+    0x00000136u,
+    0x00000007u,
+    0x0004003bu,
+    0x00000008u,
+    0x00000139u,
+    0x00000007u,
+    0x0004003bu,
+    0x00000008u,
+    0x00000140u,
+    0x00000007u,
+    0x0004003bu,
+    0x00000008u,
+    0x00000142u,
+    0x00000007u,
+    0x0004003bu,
+    0x00000008u,
+    0x00000146u,
+    0x00000007u,
+    0x0004003bu,
+    0x00000008u,
+    0x0000014cu,
+    0x00000007u,
+    0x0004003bu,
+    0x00000008u,
+    0x0000014fu,
+    0x00000007u,
+    0x0004003bu,
+    0x0000001bu,
+    0x00000158u,
+    0x00000007u,
+    0x0004003bu,
+    0x00000162u,
+    0x00000163u,
+    0x00000007u,
+    0x0004003bu,
+    0x0000001bu,
+    0x00000181u,
+    0x00000007u,
+    0x0004003bu,
+    0x00000162u,
+    0x0000019bu,
+    0x00000007u,
+    0x0004003bu,
+    0x000001e1u,
+    0x000001e2u,
+    0x00000007u,
+    0x00060041u,
+    0x00000107u,
+    0x00000108u,
+    0x00000106u,
+    0x00000030u,
+    0x0000003du,
+    0x0004003du,
+    0x00000006u,
+    0x00000109u,
+    0x00000108u,
+    0x0004006eu,
+    0x00000026u,
+    0x0000010au,
+    0x00000109u,
+    0x000500c7u,
+    0x00000026u,
+    0x0000010cu,
+    0x0000010au,
+    0x0000010bu,
+    0x000500abu,
+    0x00000031u,
+    0x0000010du,
+    0x0000010cu,
+    0x00000029u,
+    0x0003003eu,
+    0x00000102u,
+    0x0000010du,
+    0x00060041u,
+    0x00000107u,
+    0x0000010fu,
+    0x00000106u,
+    0x00000030u,
+    0x0000003du,
+    0x0004003du,
+    0x00000006u,
+    0x00000110u,
+    0x0000010fu,
+    0x0004006eu,
+    0x00000026u,
+    0x00000111u,
+    0x00000110u,
+    0x000500c7u,
+    0x00000026u,
+    0x00000113u,
+    0x00000111u,
+    0x00000112u,
+    0x000500abu,
+    0x00000031u,
+    0x00000114u,
+    0x00000113u,
+    0x00000029u,
+    0x0003003eu,
+    0x0000010eu,
+    0x00000114u,
+    0x0004003du,
+    0x00000031u,
+    0x00000116u,
+    0x0000010eu,
+    0x000300f7u,
+    0x00000119u,
+    0x00000000u,
+    0x000400fau,
+    0x00000116u,
+    0x00000118u,
+    0x0000011fu,
+    0x000200f8u,
+    0x00000118u,
+    0x0004003du,
+    0x00000007u,
+    0x0000011du,
+    0x0000011bu,
+    0x0003003eu,
+    0x0000011cu,
+    0x0000011du,
+    0x00050039u,
+    0x00000007u,
+    0x0000011eu,
+    0x00000016u,
+    0x0000011cu,
+    0x0003003eu,
+    0x00000117u,
+    0x0000011eu,
+    0x000200f9u,
+    0x00000119u,
+    0x000200f8u,
+    0x0000011fu,
+    0x0004003du,
+    0x00000031u,
+    0x00000120u,
+    0x00000102u,
+    0x000300f7u,
+    0x00000123u,
+    0x00000000u,
+    0x000400fau,
+    0x00000120u,
+    0x00000122u,
+    0x00000127u,
+    0x000200f8u,
+    0x00000122u,
+    0x0004003du,
+    0x00000007u,
+    0x00000125u,
+    0x0000011bu,
+    0x0003003eu,
+    0x00000124u,
+    0x00000125u,
+    0x00050039u,
+    0x00000007u,
+    0x00000126u,
+    0x0000000bu,
+    0x00000124u,
+    0x0003003eu,
+    0x00000121u,
+    0x00000126u,
+    0x000200f9u,
+    0x00000123u,
+    0x000200f8u,
+    0x00000127u,
+    0x0004003du,
+    0x00000007u,
+    0x00000128u,
+    0x0000011bu,
+    0x0003003eu,
+    0x00000121u,
+    0x00000128u,
+    0x000200f9u,
+    0x00000123u,
+    0x000200f8u,
+    0x00000123u,
+    0x0004003du,
+    0x00000007u,
+    0x00000129u,
+    0x00000121u,
+    0x0003003eu,
+    0x00000117u,
+    0x00000129u,
+    0x000200f9u,
+    0x00000119u,
+    0x000200f8u,
+    0x00000119u,
+    0x0004003du,
+    0x00000007u,
+    0x0000012au,
+    0x00000117u,
+    0x0003003eu,
+    0x00000115u,
+    0x0000012au,
+    0x0004003du,
+    0x00000031u,
+    0x0000012cu,
+    0x0000010eu,
+    0x000300f7u,
+    0x0000012fu,
+    0x00000000u,
+    0x000400fau,
+    0x0000012cu,
+    0x0000012eu,
+    0x00000134u,
+    0x000200f8u,
+    0x0000012eu,
+    0x0004003du,
+    0x00000007u,
+    0x00000132u,
+    0x00000130u,
+    0x0003003eu,
+    0x00000131u,
+    0x00000132u,
+    0x00050039u,
+    0x00000007u,
+    0x00000133u,
+    0x00000019u,
+    0x00000131u,
+    0x0003003eu,
+    0x0000012du,
+    0x00000133u,
+    0x000200f9u,
+    0x0000012fu,
+    0x000200f8u,
+    0x00000134u,
+    0x0004003du,
+    0x00000031u,
+    0x00000135u,
+    0x00000102u,
+    0x000300f7u,
+    0x00000138u,
+    0x00000000u,
+    0x000400fau,
+    0x00000135u,
+    0x00000137u,
+    0x0000013cu,
+    0x000200f8u,
+    0x00000137u,
+    0x0004003du,
+    0x00000007u,
+    0x0000013au,
+    0x00000130u,
+    0x0003003eu,
+    0x00000139u,
+    0x0000013au,
+    0x00050039u,
+    0x00000007u,
+    0x0000013bu,
+    0x0000000eu,
+    0x00000139u,
+    0x0003003eu,
+    0x00000136u,
+    0x0000013bu,
+    0x000200f9u,
+    0x00000138u,
+    0x000200f8u,
+    0x0000013cu,
+    0x0004003du,
+    0x00000007u,
+    0x0000013du,
+    0x00000130u,
+    0x0003003eu,
+    0x00000136u,
+    0x0000013du,
+    0x000200f9u,
+    0x00000138u,
+    0x000200f8u,
+    0x00000138u,
+    0x0004003du,
+    0x00000007u,
+    0x0000013eu,
+    0x00000136u,
+    0x0003003eu,
+    0x0000012du,
+    0x0000013eu,
+    0x000200f9u,
+    0x0000012fu,
+    0x000200f8u,
+    0x0000012fu,
+    0x0004003du,
+    0x00000007u,
+    0x0000013fu,
+    0x0000012du,
+    0x0003003eu,
+    0x0000012bu,
+    0x0000013fu,
+    0x0004003du,
+    0x00000031u,
+    0x00000141u,
+    0x0000010eu,
+    0x000300f7u,
+    0x00000144u,
+    0x00000000u,
+    0x000400fau,
+    0x00000141u,
+    0x00000143u,
+    0x0000014au,
+    0x000200f8u,
+    0x00000143u,
+    0x0004003du,
+    0x00000010u,
+    0x00000147u,
+    0x00000145u,
+    0x0008004fu,
+    0x00000007u,
+    0x00000148u,
+    0x00000147u,
+    0x00000147u,
+    0x00000000u,
+    0x00000001u,
+    0x00000002u,
+    0x0003003eu,
+    0x00000146u,
+    0x00000148u,
+    0x00050039u,
+    0x00000007u,
+    0x00000149u,
+    0x00000019u,
+    0x00000146u,
+    0x0003003eu,
+    0x00000142u,
+    0x00000149u,
+    0x000200f9u,
+    0x00000144u,
+    0x000200f8u,
+    0x0000014au,
+    0x0004003du,
+    0x00000031u,
+    0x0000014bu,
+    0x00000102u,
+    0x000300f7u,
+    0x0000014eu,
+    0x00000000u,
+    0x000400fau,
+    0x0000014bu,
+    0x0000014du,
+    0x00000153u,
+    0x000200f8u,
+    0x0000014du,
+    0x0004003du,
+    0x00000010u,
+    0x00000150u,
+    0x00000145u,
+    0x0008004fu,
+    0x00000007u,
+    0x00000151u,
+    0x00000150u,
+    0x00000150u,
+    0x00000000u,
+    0x00000001u,
+    0x00000002u,
+    0x0003003eu,
+    0x0000014fu,
+    0x00000151u,
+    0x00050039u,
+    0x00000007u,
+    0x00000152u,
+    0x0000000eu,
+    0x0000014fu,
+    0x0003003eu,
+    0x0000014cu,
+    0x00000152u,
+    0x000200f9u,
+    0x0000014eu,
+    0x000200f8u,
+    0x00000153u,
+    0x0004003du,
+    0x00000010u,
+    0x00000154u,
+    0x00000145u,
+    0x0008004fu,
+    0x00000007u,
+    0x00000155u,
+    0x00000154u,
+    0x00000154u,
+    0x00000000u,
+    0x00000001u,
+    0x00000002u,
+    0x0003003eu,
+    0x0000014cu,
+    0x00000155u,
+    0x000200f9u,
+    0x0000014eu,
+    0x000200f8u,
+    0x0000014eu,
+    0x0004003du,
+    0x00000007u,
+    0x00000156u,
+    0x0000014cu,
+    0x0003003eu,
+    0x00000142u,
+    0x00000156u,
+    0x000200f9u,
+    0x00000144u,
+    0x000200f8u,
+    0x00000144u,
+    0x0004003du,
+    0x00000007u,
+    0x00000157u,
+    0x00000142u,
+    0x0003003eu,
+    0x00000140u,
+    0x00000157u,
+    0x00050041u,
+    0x00000159u,
+    0x0000015au,
+    0x00000106u,
+    0x00000029u,
+    0x0004003du,
+    0x00000103u,
+    0x0000015bu,
+    0x0000015au,
+    0x0004003du,
+    0x00000007u,
+    0x0000015cu,
+    0x00000115u,
+    0x00050051u,
+    0x00000006u,
+    0x0000015du,
+    0x0000015cu,
+    0x00000000u,
+    0x00050051u,
+    0x00000006u,
+    0x0000015eu,
+    0x0000015cu,
+    0x00000001u,
+    0x00050051u,
+    0x00000006u,
+    0x0000015fu,
+    0x0000015cu,
+    0x00000002u,
+    0x00070050u,
+    0x00000010u,
+    0x00000160u,
+    0x0000015du,
+    0x0000015eu,
+    0x0000015fu,
+    0x0000001eu,
+    0x00050091u,
+    0x00000010u,
+    0x00000161u,
+    0x0000015bu,
+    0x00000160u,
+    0x0003003eu,
+    0x00000158u,
+    0x00000161u,
+    0x0004003du,
+    0x00000010u,
+    0x00000165u,
+    0x00000164u,
+    0x0004003du,
+    0x00000010u,
+    0x00000167u,
+    0x00000166u,
+    0x0004003du,
+    0x00000010u,
+    0x00000169u,
+    0x00000168u,
+    0x0004003du,
+    0x00000010u,
+    0x0000016bu,
+    0x0000016au,
+    0x00050051u,
+    0x00000006u,
+    0x0000016cu,
+    0x00000165u,
+    0x00000000u,
+    0x00050051u,
+    0x00000006u,
+    0x0000016du,
+    0x00000165u,
+    0x00000001u,
+    0x00050051u,
+    0x00000006u,
+    0x0000016eu,
+    0x00000165u,
+    0x00000002u,
+    0x00050051u,
+    0x00000006u,
+    0x0000016fu,
+    0x00000165u,
+    0x00000003u,
+    0x00050051u,
+    0x00000006u,
+    0x00000170u,
+    0x00000167u,
+    0x00000000u,
+    0x00050051u,
+    0x00000006u,
+    0x00000171u,
+    0x00000167u,
+    0x00000001u,
+    0x00050051u,
+    0x00000006u,
+    0x00000172u,
+    0x00000167u,
+    0x00000002u,
+    0x00050051u,
+    0x00000006u,
+    0x00000173u,
+    0x00000167u,
+    0x00000003u,
+    0x00050051u,
+    0x00000006u,
+    0x00000174u,
+    0x00000169u,
+    0x00000000u,
+    0x00050051u,
+    0x00000006u,
+    0x00000175u,
+    0x00000169u,
+    0x00000001u,
+    0x00050051u,
+    0x00000006u,
+    0x00000176u,
+    0x00000169u,
+    0x00000002u,
+    0x00050051u,
+    0x00000006u,
+    0x00000177u,
+    0x00000169u,
+    0x00000003u,
+    0x00050051u,
+    0x00000006u,
+    0x00000178u,
+    0x0000016bu,
+    0x00000000u,
+    0x00050051u,
+    0x00000006u,
+    0x00000179u,
+    0x0000016bu,
+    0x00000001u,
+    0x00050051u,
+    0x00000006u,
+    0x0000017au,
+    0x0000016bu,
+    0x00000002u,
+    0x00050051u,
+    0x00000006u,
+    0x0000017bu,
+    0x0000016bu,
+    0x00000003u,
+    0x00070050u,
+    0x00000010u,
+    0x0000017cu,
+    0x0000016cu,
+    0x0000016du,
+    0x0000016eu,
+    0x0000016fu,
+    0x00070050u,
+    0x00000010u,
+    0x0000017du,
+    0x00000170u,
+    0x00000171u,
+    0x00000172u,
+    0x00000173u,
+    0x00070050u,
+    0x00000010u,
+    0x0000017eu,
+    0x00000174u,
+    0x00000175u,
+    0x00000176u,
+    0x00000177u,
+    0x00070050u,
+    0x00000010u,
+    0x0000017fu,
+    0x00000178u,
+    0x00000179u,
+    0x0000017au,
+    0x0000017bu,
+    0x00070050u,
+    0x00000103u,
+    0x00000180u,
+    0x0000017cu,
+    0x0000017du,
+    0x0000017eu,
+    0x0000017fu,
+    0x0003003eu,
+    0x00000163u,
+    0x00000180u,
+    0x0004003du,
+    0x00000103u,
+    0x00000182u,
+    0x00000163u,
+    0x0004003du,
+    0x00000007u,
+    0x00000183u,
+    0x00000115u,
+    0x00050051u,
+    0x00000006u,
+    0x00000184u,
+    0x00000183u,
+    0x00000000u,
+    0x00050051u,
+    0x00000006u,
+    0x00000185u,
+    0x00000183u,
+    0x00000001u,
+    0x00050051u,
+    0x00000006u,
+    0x00000186u,
+    0x00000183u,
+    0x00000002u,
+    0x00070050u,
+    0x00000010u,
+    0x00000187u,
+    0x00000184u,
+    0x00000185u,
+    0x00000186u,
+    0x0000001eu,
+    0x00050091u,
+    0x00000010u,
+    0x00000188u,
+    0x00000182u,
+    0x00000187u,
+    0x0003003eu,
+    0x00000181u,
+    0x00000188u,
+    0x00050041u,
+    0x0000009au,
+    0x0000018cu,
+    0x00000181u,
+    0x0000018bu,
+    0x0004003du,
+    0x00000006u,
+    0x0000018du,
+    0x0000018cu,
+    0x00050041u,
+    0x0000009au,
+    0x0000018eu,
+    0x00000181u,
+    0x00000050u,
+    0x0004003du,
+    0x00000006u,
+    0x0000018fu,
+    0x0000018eu,
+    0x0004007fu,
+    0x00000006u,
+    0x00000190u,
+    0x0000018fu,
+    0x00050041u,
+    0x0000009au,
+    0x00000191u,
+    0x00000181u,
+    0x00000057u,
+    0x0004003du,
+    0x00000006u,
+    0x00000192u,
+    0x00000191u,
+    0x00050041u,
+    0x0000009au,
+    0x00000193u,
+    0x00000181u,
+    0x0000003du,
+    0x0004003du,
+    0x00000006u,
+    0x00000194u,
+    0x00000193u,
+    0x00050081u,
+    0x00000006u,
+    0x00000195u,
+    0x00000192u,
+    0x00000194u,
+    0x00050085u,
+    0x00000006u,
+    0x00000197u,
+    0x00000195u,
+    0x00000196u,
+    0x00050041u,
+    0x0000009au,
+    0x00000198u,
+    0x00000181u,
+    0x0000003du,
+    0x0004003du,
+    0x00000006u,
+    0x00000199u,
+    0x00000198u,
+    0x00070050u,
+    0x00000010u,
+    0x0000019au,
+    0x0000018du,
+    0x00000190u,
+    0x00000197u,
+    0x00000199u,
+    0x0003003eu,
+    0x0000018au,
+    0x0000019au,
+    0x0004003du,
+    0x00000010u,
+    0x0000019du,
+    0x0000019cu,
+    0x0004003du,
+    0x00000010u,
+    0x0000019fu,
+    0x0000019eu,
+    0x0004003du,
+    0x00000010u,
+    0x000001a1u,
+    0x000001a0u,
+    0x0004003du,
+    0x00000010u,
+    0x000001a3u,
+    0x000001a2u,
+    0x00050051u,
+    0x00000006u,
+    0x000001a4u,
+    0x0000019du,
+    0x00000000u,
+    0x00050051u,
+    0x00000006u,
+    0x000001a5u,
+    0x0000019du,
+    0x00000001u,
+    0x00050051u,
+    0x00000006u,
+    0x000001a6u,
+    0x0000019du,
+    0x00000002u,
+    0x00050051u,
+    0x00000006u,
+    0x000001a7u,
+    0x0000019du,
+    0x00000003u,
+    0x00050051u,
+    0x00000006u,
+    0x000001a8u,
+    0x0000019fu,
+    0x00000000u,
+    0x00050051u,
+    0x00000006u,
+    0x000001a9u,
+    0x0000019fu,
+    0x00000001u,
+    0x00050051u,
+    0x00000006u,
+    0x000001aau,
+    0x0000019fu,
+    0x00000002u,
+    0x00050051u,
+    0x00000006u,
+    0x000001abu,
+    0x0000019fu,
+    0x00000003u,
+    0x00050051u,
+    0x00000006u,
+    0x000001acu,
+    0x000001a1u,
+    0x00000000u,
+    0x00050051u,
+    0x00000006u,
+    0x000001adu,
+    0x000001a1u,
+    0x00000001u,
+    0x00050051u,
+    0x00000006u,
+    0x000001aeu,
+    0x000001a1u,
+    0x00000002u,
+    0x00050051u,
+    0x00000006u,
+    0x000001afu,
+    0x000001a1u,
+    0x00000003u,
+    0x00050051u,
+    0x00000006u,
+    0x000001b0u,
+    0x000001a3u,
+    0x00000000u,
+    0x00050051u,
+    0x00000006u,
+    0x000001b1u,
+    0x000001a3u,
+    0x00000001u,
+    0x00050051u,
+    0x00000006u,
+    0x000001b2u,
+    0x000001a3u,
+    0x00000002u,
+    0x00050051u,
+    0x00000006u,
+    0x000001b3u,
+    0x000001a3u,
+    0x00000003u,
+    0x00070050u,
+    0x00000010u,
+    0x000001b4u,
+    0x000001a4u,
+    0x000001a5u,
+    0x000001a6u,
+    0x000001a7u,
+    0x00070050u,
+    0x00000010u,
+    0x000001b5u,
+    0x000001a8u,
+    0x000001a9u,
+    0x000001aau,
+    0x000001abu,
+    0x00070050u,
+    0x00000010u,
+    0x000001b6u,
+    0x000001acu,
+    0x000001adu,
+    0x000001aeu,
+    0x000001afu,
+    0x00070050u,
+    0x00000010u,
+    0x000001b7u,
+    0x000001b0u,
+    0x000001b1u,
+    0x000001b2u,
+    0x000001b3u,
+    0x00070050u,
+    0x00000103u,
+    0x000001b8u,
+    0x000001b4u,
+    0x000001b5u,
+    0x000001b6u,
+    0x000001b7u,
+    0x0003003eu,
+    0x0000019bu,
+    0x000001b8u,
+    0x0004003du,
+    0x00000103u,
+    0x000001bbu,
+    0x0000019bu,
+    0x0004003du,
+    0x00000007u,
+    0x000001bcu,
+    0x00000115u,
+    0x00050051u,
+    0x00000006u,
+    0x000001bdu,
+    0x000001bcu,
+    0x00000000u,
+    0x00050051u,
+    0x00000006u,
+    0x000001beu,
+    0x000001bcu,
+    0x00000001u,
+    0x00050051u,
+    0x00000006u,
+    0x000001bfu,
+    0x000001bcu,
+    0x00000002u,
+    0x00070050u,
+    0x00000010u,
+    0x000001c0u,
+    0x000001bdu,
+    0x000001beu,
+    0x000001bfu,
+    0x0000001eu,
+    0x00050091u,
+    0x00000010u,
+    0x000001c1u,
+    0x000001bbu,
+    0x000001c0u,
+    0x0008004fu,
+    0x00000007u,
+    0x000001c2u,
+    0x000001c1u,
+    0x000001c1u,
+    0x00000000u,
+    0x00000001u,
+    0x00000002u,
+    0x0003003eu,
+    0x000001bau,
+    0x000001c2u,
+    0x00050041u,
+    0x0000009au,
+    0x000001c7u,
+    0x00000158u,
+    0x0000018bu,
+    0x0004003du,
+    0x00000006u,
+    0x000001c8u,
+    0x000001c7u,
+    0x00050041u,
+    0x0000009au,
+    0x000001c9u,
+    0x00000158u,
+    0x00000050u,
+    0x0004003du,
+    0x00000006u,
+    0x000001cau,
+    0x000001c9u,
+    0x0004007fu,
+    0x00000006u,
+    0x000001cbu,
+    0x000001cau,
+    0x00050041u,
+    0x0000009au,
+    0x000001ccu,
+    0x00000158u,
+    0x00000057u,
+    0x0004003du,
+    0x00000006u,
+    0x000001cdu,
+    0x000001ccu,
+    0x00050041u,
+    0x0000009au,
+    0x000001ceu,
+    0x00000158u,
+    0x0000003du,
+    0x0004003du,
+    0x00000006u,
+    0x000001cfu,
+    0x000001ceu,
+    0x00050081u,
+    0x00000006u,
+    0x000001d0u,
+    0x000001cdu,
+    0x000001cfu,
+    0x00050085u,
+    0x00000006u,
+    0x000001d1u,
+    0x000001d0u,
+    0x00000196u,
+    0x00050041u,
+    0x0000009au,
+    0x000001d2u,
+    0x00000158u,
+    0x0000003du,
+    0x0004003du,
+    0x00000006u,
+    0x000001d3u,
+    0x000001d2u,
+    0x00070050u,
+    0x00000010u,
+    0x000001d4u,
+    0x000001c8u,
+    0x000001cbu,
+    0x000001d1u,
+    0x000001d3u,
+    0x00050041u,
+    0x00000189u,
+    0x000001d5u,
+    0x000001c6u,
+    0x00000029u,
+    0x0003003eu,
+    0x000001d5u,
+    0x000001d4u,
+    0x0004003du,
+    0x00000010u,
+    0x000001d8u,
+    0x000001d7u,
+    0x0008004fu,
+    0x00000007u,
+    0x000001d9u,
+    0x000001d8u,
+    0x000001d8u,
+    0x00000000u,
+    0x00000001u,
+    0x00000002u,
+    0x0003003eu,
+    0x000001d6u,
+    0x000001d9u,
+    0x0004003du,
+    0x000001dau,
+    0x000001dfu,
+    0x000001deu,
+    0x0003003eu,
+    0x000001dcu,
+    0x000001dfu,
+    0x00050041u,
+    0x000001e3u,
+    0x000001e4u,
+    0x00000106u,
+    0x00000062u,
+    0x0004003du,
+    0x00000010u,
+    0x000001e5u,
+    0x000001e4u,
+    0x0008004fu,
+    0x00000007u,
+    0x000001e6u,
+    0x000001e5u,
+    0x000001e5u,
+    0x00000000u,
+    0x00000001u,
+    0x00000002u,
+    0x00050041u,
+    0x000001e3u,
+    0x000001e7u,
+    0x00000106u,
+    0x000000e5u,
+    0x0004003du,
+    0x00000010u,
+    0x000001e8u,
+    0x000001e7u,
+    0x0008004fu,
+    0x00000007u,
+    0x000001e9u,
+    0x000001e8u,
+    0x000001e8u,
+    0x00000000u,
+    0x00000001u,
+    0x00000002u,
+    0x00050041u,
+    0x000001e3u,
+    0x000001ebu,
+    0x00000106u,
+    0x000001eau,
+    0x0004003du,
+    0x00000010u,
+    0x000001ecu,
+    0x000001ebu,
+    0x0008004fu,
+    0x00000007u,
+    0x000001edu,
+    0x000001ecu,
+    0x000001ecu,
+    0x00000000u,
+    0x00000001u,
+    0x00000002u,
+    0x00050051u,
+    0x00000006u,
+    0x000001eeu,
+    0x000001e6u,
+    0x00000000u,
+    0x00050051u,
+    0x00000006u,
+    0x000001efu,
+    0x000001e6u,
+    0x00000001u,
+    0x00050051u,
+    0x00000006u,
+    0x000001f0u,
+    0x000001e6u,
+    0x00000002u,
+    0x00050051u,
+    0x00000006u,
+    0x000001f1u,
+    0x000001e9u,
+    0x00000000u,
+    0x00050051u,
+    0x00000006u,
+    0x000001f2u,
+    0x000001e9u,
+    0x00000001u,
+    0x00050051u,
+    0x00000006u,
+    0x000001f3u,
+    0x000001e9u,
+    0x00000002u,
+    0x00050051u,
+    0x00000006u,
+    0x000001f4u,
+    0x000001edu,
+    0x00000000u,
+    0x00050051u,
+    0x00000006u,
+    0x000001f5u,
+    0x000001edu,
+    0x00000001u,
+    0x00050051u,
+    0x00000006u,
+    0x000001f6u,
+    0x000001edu,
+    0x00000002u,
+    0x00060050u,
+    0x00000007u,
+    0x000001f7u,
+    0x000001eeu,
+    0x000001efu,
+    0x000001f0u,
+    0x00060050u,
+    0x00000007u,
+    0x000001f8u,
+    0x000001f1u,
+    0x000001f2u,
+    0x000001f3u,
+    0x00060050u,
+    0x00000007u,
+    0x000001f9u,
+    0x000001f4u,
+    0x000001f5u,
+    0x000001f6u,
+    0x00060050u,
+    0x000001e0u,
+    0x000001fau,
+    0x000001f7u,
+    0x000001f8u,
+    0x000001f9u,
+    0x0003003eu,
+    0x000001e2u,
+    0x000001fau,
+    0x0004003du,
+    0x000001e0u,
+    0x000001fcu,
+    0x000001e2u,
+    0x0004003du,
+    0x00000007u,
+    0x000001fdu,
+    0x0000012bu,
+    0x00050091u,
+    0x00000007u,
+    0x000001feu,
+    0x000001fcu,
+    0x000001fdu,
+    0x0006000cu,
+    0x00000007u,
+    0x000001ffu,
+    0x00000001u,
+    0x00000045u,
+    0x000001feu,
+    0x0003003eu,
+    0x000001fbu,
+    0x000001ffu,
+    0x0004003du,
+    0x000001e0u,
+    0x00000201u,
+    0x000001e2u,
+    0x0004003du,
+    0x00000007u,
+    0x00000202u,
+    0x00000140u,
+    0x00050091u,
+    0x00000007u,
+    0x00000203u,
+    0x00000201u,
+    0x00000202u,
+    0x0006000cu,
+    0x00000007u,
+    0x00000204u,
+    0x00000001u,
+    0x00000045u,
+    0x00000203u,
+    0x00050041u,
+    0x00000042u,
+    0x00000205u,
+    0x00000145u,
+    0x0000003du,
+    0x0004003du,
+    0x00000006u,
+    0x00000206u,
+    0x00000205u,
+    0x00050051u,
+    0x00000006u,
+    0x00000207u,
+    0x00000204u,
+    0x00000000u,
+    0x00050051u,
+    0x00000006u,
+    0x00000208u,
+    0x00000204u,
+    0x00000001u,
+    0x00050051u,
+    0x00000006u,
+    0x00000209u,
+    0x00000204u,
+    0x00000002u,
+    0x00070050u,
+    0x00000010u,
+    0x0000020au,
+    0x00000207u,
+    0x00000208u,
+    0x00000209u,
+    0x00000206u,
+    0x0003003eu,
+    0x00000200u,
+    0x0000020au,
+    0x000100fdu,
+    0x00010038u,
+    0x00050036u,
+    0x00000007u,
+    0x0000000bu,
+    0x00000000u,
+    0x00000009u,
+    0x00030037u,
+    0x00000008u,
+    0x0000000au,
+    0x000200f8u,
+    0x0000000cu,
+    0x0004003bu,
+    0x0000001bu,
+    0x0000001cu,
+    0x00000007u,
+    0x0004003bu,
+    0x00000008u,
+    0x00000023u,
+    0x00000007u,
+    0x0004003bu,
+    0x00000027u,
+    0x00000028u,
+    0x00000007u,
+    0x0004003bu,
+    0x00000034u,
+    0x00000035u,
+    0x00000007u,
+    0x0004003du,
+    0x00000007u,
+    0x0000001du,
+    0x0000000au,
+    0x00050051u,
+    0x00000006u,
+    0x0000001fu,
+    0x0000001du,
+    0x00000000u,
+    0x00050051u,
+    0x00000006u,
+    0x00000020u,
+    0x0000001du,
+    0x00000001u,
+    0x00050051u,
+    0x00000006u,
+    0x00000021u,
+    0x0000001du,
+    0x00000002u,
+    0x00070050u,
+    0x00000010u,
+    0x00000022u,
+    0x0000001fu,
+    0x00000020u,
+    0x00000021u,
+    0x0000001eu,
+    0x0003003eu,
+    0x0000001cu,
+    0x00000022u,
+    0x0003003eu,
+    0x00000023u,
+    0x00000025u,
+    0x0003003eu,
+    0x00000028u,
+    0x00000029u,
+    0x000200f9u,
+    0x0000002au,
+    0x000200f8u,
+    0x0000002au,
+    0x000400f6u,
+    0x0000002cu,
+    0x0000002du,
+    0x00000000u,
+    0x000200f9u,
+    0x0000002eu,
+    0x000200f8u,
+    0x0000002eu,
+    0x0004003du,
+    0x00000026u,
+    0x0000002fu,
+    0x00000028u,
+    0x000500b1u,
+    0x00000031u,
+    0x00000032u,
+    0x0000002fu,
+    0x00000030u,
+    0x000400fau,
+    0x00000032u,
+    0x0000002bu,
+    0x0000002cu,
+    0x000200f8u,
+    0x0000002bu,
+    0x0004003du,
+    0x00000026u,
+    0x00000039u,
+    0x00000028u,
+    0x00050041u,
+    0x0000003au,
+    0x0000003bu,
+    0x00000038u,
+    0x00000039u,
+    0x0004003du,
+    0x00000033u,
+    0x0000003cu,
+    0x0000003bu,
+    0x00050084u,
+    0x00000033u,
+    0x0000003eu,
+    0x0000003cu,
+    0x0000003du,
+    0x0003003eu,
+    0x00000035u,
+    0x0000003eu,
+    0x0004003du,
+    0x00000026u,
+    0x00000041u,
+    0x00000028u,
+    0x00050041u,
+    0x00000042u,
+    0x00000043u,
+    0x00000040u,
+    0x00000041u,
+    0x0004003du,
+    0x00000006u,
+    0x00000044u,
+    0x00000043u,
+    0x0004003du,
+    0x00000033u,
+    0x00000049u,
+    0x00000035u,
+    0x00060041u,
+    0x0000004au,
+    0x0000004bu,
+    0x00000048u,
+    0x00000029u,
+    0x00000049u,
+    0x0004003du,
+    0x00000010u,
+    0x0000004cu,
+    0x0000004bu,
+    0x0004003du,
+    0x00000010u,
+    0x0000004du,
+    0x0000001cu,
+    0x00050094u,
+    0x00000006u,
+    0x0000004eu,
+    0x0000004cu,
+    0x0000004du,
+    0x0004003du,
+    0x00000033u,
+    0x0000004fu,
+    0x00000035u,
+    0x00050080u,
+    0x00000033u,
+    0x00000051u,
+    0x0000004fu,
+    0x00000050u,
+    0x00060041u,
+    0x0000004au,
+    0x00000052u,
+    0x00000048u,
+    0x00000029u,
+    0x00000051u,
+    0x0004003du,
+    0x00000010u,
+    0x00000053u,
+    0x00000052u,
+    0x0004003du,
+    0x00000010u,
+    0x00000054u,
+    0x0000001cu,
+    0x00050094u,
+    0x00000006u,
+    0x00000055u,
+    0x00000053u,
+    0x00000054u,
+    0x0004003du,
+    0x00000033u,
+    0x00000056u,
+    0x00000035u,
+    0x00050080u,
+    0x00000033u,
+    0x00000058u,
+    0x00000056u,
+    0x00000057u,
+    0x00060041u,
+    0x0000004au,
+    0x00000059u,
+    0x00000048u,
+    0x00000029u,
+    0x00000058u,
+    0x0004003du,
+    0x00000010u,
+    0x0000005au,
+    0x00000059u,
+    0x0004003du,
+    0x00000010u,
+    0x0000005bu,
+    0x0000001cu,
+    0x00050094u,
+    0x00000006u,
+    0x0000005cu,
+    0x0000005au,
+    0x0000005bu,
+    0x00060050u,
+    0x00000007u,
+    0x0000005du,
+    0x0000004eu,
+    0x00000055u,
+    0x0000005cu,
+    0x0005008eu,
+    0x00000007u,
+    0x0000005eu,
+    0x0000005du,
+    0x00000044u,
+    0x0004003du,
+    0x00000007u,
+    0x0000005fu,
+    0x00000023u,
+    0x00050081u,
+    0x00000007u,
+    0x00000060u,
+    0x0000005fu,
+    0x0000005eu,
+    0x0003003eu,
+    0x00000023u,
+    0x00000060u,
+    0x000200f9u,
+    0x0000002du,
+    0x000200f8u,
+    0x0000002du,
+    0x0004003du,
+    0x00000026u,
+    0x00000061u,
+    0x00000028u,
+    0x00050080u,
+    0x00000026u,
+    0x00000063u,
+    0x00000061u,
+    0x00000062u,
+    0x0003003eu,
+    0x00000028u,
+    0x00000063u,
+    0x000200f9u,
+    0x0000002au,
+    0x000200f8u,
+    0x0000002cu,
+    0x0004003du,
+    0x00000007u,
+    0x00000064u,
+    0x00000023u,
+    0x000200feu,
+    0x00000064u,
+    0x00010038u,
+    0x00050036u,
+    0x00000007u,
+    0x0000000eu,
+    0x00000000u,
+    0x00000009u,
+    0x00030037u,
+    0x00000008u,
+    0x0000000du,
+    0x000200f8u,
+    0x0000000fu,
+    0x0004003bu,
+    0x00000008u,
+    0x00000067u,
+    0x00000007u,
+    0x0004003bu,
+    0x00000027u,
+    0x00000068u,
+    0x00000007u,
+    0x0004003bu,
+    0x00000034u,
+    0x00000070u,
+    0x00000007u,
+    0x0003003eu,
+    0x00000067u,
+    0x00000025u,
+    0x0003003eu,
+    0x00000068u,
+    0x00000029u,
+    0x000200f9u,
+    0x00000069u,
+    0x000200f8u,
+    0x00000069u,
+    0x000400f6u,
+    0x0000006bu,
+    0x0000006cu,
+    0x00000000u,
+    0x000200f9u,
+    0x0000006du,
+    0x000200f8u,
+    0x0000006du,
+    0x0004003du,
+    0x00000026u,
+    0x0000006eu,
+    0x00000068u,
+    0x000500b1u,
+    0x00000031u,
+    0x0000006fu,
+    0x0000006eu,
+    0x00000030u,
+    0x000400fau,
+    0x0000006fu,
+    0x0000006au,
+    0x0000006bu,
+    0x000200f8u,
+    0x0000006au,
+    0x0004003du,
+    0x00000026u,
+    0x00000071u,
+    0x00000068u,
+    0x00050041u,
+    0x0000003au,
+    0x00000072u,
+    0x00000038u,
+    0x00000071u,
+    0x0004003du,
+    0x00000033u,
+    0x00000073u,
+    0x00000072u,
+    0x00050084u,
+    0x00000033u,
+    0x00000074u,
+    0x00000073u,
+    0x0000003du,
+    0x0003003eu,
+    0x00000070u,
+    0x00000074u,
+    0x0004003du,
+    0x00000026u,
+    0x00000075u,
+    0x00000068u,
+    0x00050041u,
+    0x00000042u,
+    0x00000076u,
+    0x00000040u,
+    0x00000075u,
+    0x0004003du,
+    0x00000006u,
+    0x00000077u,
+    0x00000076u,
+    0x0004003du,
+    0x00000033u,
+    0x00000078u,
+    0x00000070u,
+    0x00060041u,
+    0x0000004au,
+    0x00000079u,
+    0x00000048u,
+    0x00000029u,
+    0x00000078u,
+    0x0004003du,
+    0x00000010u,
+    0x0000007au,
+    0x00000079u,
+    0x0008004fu,
+    0x00000007u,
+    0x0000007bu,
+    0x0000007au,
+    0x0000007au,
+    0x00000000u,
+    0x00000001u,
+    0x00000002u,
+    0x0004003du,
+    0x00000007u,
+    0x0000007cu,
+    0x0000000du,
+    0x00050094u,
+    0x00000006u,
+    0x0000007du,
+    0x0000007bu,
+    0x0000007cu,
+    0x0004003du,
+    0x00000033u,
+    0x0000007eu,
+    0x00000070u,
+    0x00050080u,
+    0x00000033u,
+    0x0000007fu,
+    0x0000007eu,
+    0x00000050u,
+    0x00060041u,
+    0x0000004au,
+    0x00000080u,
+    0x00000048u,
+    0x00000029u,
+    0x0000007fu,
+    0x0004003du,
+    0x00000010u,
+    0x00000081u,
+    0x00000080u,
+    0x0008004fu,
+    0x00000007u,
+    0x00000082u,
+    0x00000081u,
+    0x00000081u,
+    0x00000000u,
+    0x00000001u,
+    0x00000002u,
+    0x0004003du,
+    0x00000007u,
+    0x00000083u,
+    0x0000000du,
+    0x00050094u,
+    0x00000006u,
+    0x00000084u,
+    0x00000082u,
+    0x00000083u,
+    0x0004003du,
+    0x00000033u,
+    0x00000085u,
+    0x00000070u,
+    0x00050080u,
+    0x00000033u,
+    0x00000086u,
+    0x00000085u,
+    0x00000057u,
+    0x00060041u,
+    0x0000004au,
+    0x00000087u,
+    0x00000048u,
+    0x00000029u,
+    0x00000086u,
+    0x0004003du,
+    0x00000010u,
+    0x00000088u,
+    0x00000087u,
+    0x0008004fu,
+    0x00000007u,
+    0x00000089u,
+    0x00000088u,
+    0x00000088u,
+    0x00000000u,
+    0x00000001u,
+    0x00000002u,
+    0x0004003du,
+    0x00000007u,
+    0x0000008au,
+    0x0000000du,
+    0x00050094u,
+    0x00000006u,
+    0x0000008bu,
+    0x00000089u,
+    0x0000008au,
+    0x00060050u,
+    0x00000007u,
+    0x0000008cu,
+    0x0000007du,
+    0x00000084u,
+    0x0000008bu,
+    0x0005008eu,
+    0x00000007u,
+    0x0000008du,
+    0x0000008cu,
+    0x00000077u,
+    0x0004003du,
+    0x00000007u,
+    0x0000008eu,
+    0x00000067u,
+    0x00050081u,
+    0x00000007u,
+    0x0000008fu,
+    0x0000008eu,
+    0x0000008du,
+    0x0003003eu,
+    0x00000067u,
+    0x0000008fu,
+    0x000200f9u,
+    0x0000006cu,
+    0x000200f8u,
+    0x0000006cu,
+    0x0004003du,
+    0x00000026u,
+    0x00000090u,
+    0x00000068u,
+    0x00050080u,
+    0x00000026u,
+    0x00000091u,
+    0x00000090u,
+    0x00000062u,
+    0x0003003eu,
+    0x00000068u,
+    0x00000091u,
+    0x000200f9u,
+    0x00000069u,
+    0x000200f8u,
+    0x0000006bu,
+    0x0004003du,
+    0x00000007u,
+    0x00000092u,
+    0x00000067u,
+    0x000200feu,
+    0x00000092u,
+    0x00010038u,
+    0x00050036u,
+    0x00000011u,
+    0x00000013u,
+    0x00000000u,
+    0x00000012u,
+    0x000200f8u,
+    0x00000014u,
+    0x0004003bu,
+    0x00000027u,
+    0x00000095u,
+    0x00000007u,
+    0x0004003bu,
+    0x0000009au,
+    0x0000009bu,
+    0x00000007u,
+    0x0004003du,
+    0x00000006u,
+    0x00000097u,
+    0x00000096u,
+    0x0006000cu,
+    0x00000006u,
+    0x00000098u,
+    0x00000001u,
+    0x00000008u,
+    0x00000097u,
+    0x0004006eu,
+    0x00000026u,
+    0x00000099u,
+    0x00000098u,
+    0x0003003eu,
+    0x00000095u,
+    0x00000099u,
+    0x0004003du,
+    0x00000006u,
+    0x0000009cu,
+    0x00000096u,
+    0x0006000cu,
+    0x00000006u,
+    0x0000009du,
+    0x00000001u,
+    0x0000000au,
+    0x0000009cu,
+    0x0003003eu,
+    0x0000009bu,
+    0x0000009du,
+    0x0004003du,
+    0x00000026u,
+    0x0000009eu,
+    0x00000095u,
+    0x00060041u,
+    0x0000004au,
+    0x0000009fu,
+    0x00000048u,
+    0x00000029u,
+    0x0000009eu,
+    0x0004003du,
+    0x00000010u,
+    0x000000a0u,
+    0x0000009fu,
+    0x0004003du,
+    0x00000026u,
+    0x000000a1u,
+    0x00000095u,
+    0x00050080u,
+    0x00000026u,
+    0x000000a2u,
+    0x000000a1u,
+    0x00000062u,
+    0x00060041u,
+    0x0000004au,
+    0x000000a3u,
+    0x00000048u,
+    0x00000029u,
+    0x000000a2u,
+    0x0004003du,
+    0x00000010u,
+    0x000000a4u,
+    0x000000a3u,
+    0x0004003du,
+    0x00000006u,
+    0x000000a5u,
+    0x0000009bu,
+    0x00070050u,
+    0x00000010u,
+    0x000000a6u,
+    0x000000a5u,
+    0x000000a5u,
+    0x000000a5u,
+    0x000000a5u,
+    0x0008000cu,
+    0x00000010u,
+    0x000000a7u,
+    0x00000001u,
+    0x0000002eu,
+    0x000000a0u,
+    0x000000a4u,
+    0x000000a6u,
+    0x0004003du,
+    0x00000026u,
+    0x000000a8u,
+    0x00000095u,
+    0x00050080u,
+    0x00000026u,
+    0x000000aau,
+    0x000000a8u,
+    0x000000a9u,
+    0x00060041u,
+    0x0000004au,
+    0x000000abu,
+    0x00000048u,
+    0x00000029u,
+    0x000000aau,
+    0x0004003du,
+    0x00000010u,
+    0x000000acu,
+    0x000000abu,
+    0x0004003du,
+    0x00000026u,
+    0x000000adu,
+    0x00000095u,
+    0x00050080u,
+    0x00000026u,
+    0x000000afu,
+    0x000000adu,
+    0x000000aeu,
+    0x00060041u,
+    0x0000004au,
+    0x000000b0u,
+    0x00000048u,
+    0x00000029u,
+    0x000000afu,
+    0x0004003du,
+    0x00000010u,
+    0x000000b1u,
+    0x000000b0u,
+    0x0004003du,
+    0x00000006u,
+    0x000000b2u,
+    0x0000009bu,
+    0x00070050u,
+    0x00000010u,
+    0x000000b3u,
+    0x000000b2u,
+    0x000000b2u,
+    0x000000b2u,
+    0x000000b2u,
+    0x0008000cu,
+    0x00000010u,
+    0x000000b4u,
+    0x00000001u,
+    0x0000002eu,
+    0x000000acu,
+    0x000000b1u,
+    0x000000b3u,
+    0x0004003du,
+    0x00000026u,
+    0x000000b5u,
+    0x00000095u,
+    0x00050080u,
+    0x00000026u,
+    0x000000b7u,
+    0x000000b5u,
+    0x000000b6u,
+    0x00060041u,
+    0x0000004au,
+    0x000000b8u,
+    0x00000048u,
+    0x00000029u,
+    0x000000b7u,
+    0x0004003du,
+    0x00000010u,
+    0x000000b9u,
+    0x000000b8u,
+    0x0004003du,
+    0x00000026u,
+    0x000000bau,
+    0x00000095u,
+    0x00050080u,
+    0x00000026u,
+    0x000000bcu,
+    0x000000bau,
+    0x000000bbu,
+    0x00060041u,
+    0x0000004au,
+    0x000000bdu,
+    0x00000048u,
+    0x00000029u,
+    0x000000bcu,
+    0x0004003du,
+    0x00000010u,
+    0x000000beu,
+    0x000000bdu,
+    0x0004003du,
+    0x00000006u,
+    0x000000bfu,
+    0x0000009bu,
+    0x00070050u,
+    0x00000010u,
+    0x000000c0u,
+    0x000000bfu,
+    0x000000bfu,
+    0x000000bfu,
+    0x000000bfu,
+    0x0008000cu,
+    0x00000010u,
+    0x000000c1u,
+    0x00000001u,
+    0x0000002eu,
+    0x000000b9u,
+    0x000000beu,
+    0x000000c0u,
+    0x00050051u,
+    0x00000006u,
+    0x000000c2u,
+    0x000000a7u,
+    0x00000000u,
+    0x00050051u,
+    0x00000006u,
+    0x000000c3u,
+    0x000000a7u,
+    0x00000001u,
+    0x00050051u,
+    0x00000006u,
+    0x000000c4u,
+    0x000000a7u,
+    0x00000002u,
+    0x00050051u,
+    0x00000006u,
+    0x000000c5u,
+    0x000000a7u,
+    0x00000003u,
+    0x00050051u,
+    0x00000006u,
+    0x000000c6u,
+    0x000000b4u,
+    0x00000000u,
+    0x00050051u,
+    0x00000006u,
+    0x000000c7u,
+    0x000000b4u,
+    0x00000001u,
+    0x00050051u,
+    0x00000006u,
+    0x000000c8u,
+    0x000000b4u,
+    0x00000002u,
+    0x00050051u,
+    0x00000006u,
+    0x000000c9u,
+    0x000000b4u,
+    0x00000003u,
+    0x00050051u,
+    0x00000006u,
+    0x000000cau,
+    0x000000c1u,
+    0x00000000u,
+    0x00050051u,
+    0x00000006u,
+    0x000000cbu,
+    0x000000c1u,
+    0x00000001u,
+    0x00050051u,
+    0x00000006u,
+    0x000000ccu,
+    0x000000c1u,
+    0x00000002u,
+    0x00050051u,
+    0x00000006u,
+    0x000000cdu,
+    0x000000c1u,
+    0x00000003u,
+    0x00070050u,
+    0x00000010u,
+    0x000000ceu,
+    0x000000c2u,
+    0x000000c3u,
+    0x000000c4u,
+    0x000000c5u,
+    0x00070050u,
+    0x00000010u,
+    0x000000cfu,
+    0x000000c6u,
+    0x000000c7u,
+    0x000000c8u,
+    0x000000c9u,
+    0x00070050u,
+    0x00000010u,
+    0x000000d0u,
+    0x000000cau,
+    0x000000cbu,
+    0x000000ccu,
+    0x000000cdu,
+    0x00060050u,
+    0x00000011u,
+    0x000000d1u,
+    0x000000ceu,
+    0x000000cfu,
+    0x000000d0u,
+    0x000200feu,
+    0x000000d1u,
+    0x00010038u,
+    0x00050036u,
+    0x00000007u,
+    0x00000016u,
+    0x00000000u,
+    0x00000009u,
+    0x00030037u,
+    0x00000008u,
+    0x00000015u,
+    0x000200f8u,
+    0x00000017u,
+    0x0004003bu,
+    0x000000d4u,
+    0x000000d5u,
+    0x00000007u,
+    0x0004003bu,
+    0x0000001bu,
+    0x000000d7u,
+    0x00000007u,
+    0x00040039u,
+    0x00000011u,
+    0x000000d6u,
+    0x00000013u,
+    0x0003003eu,
+    0x000000d5u,
+    0x000000d6u,
+    0x0004003du,
+    0x00000007u,
+    0x000000d8u,
+    0x00000015u,
+    0x00050051u,
+    0x00000006u,
+    0x000000d9u,
+    0x000000d8u,
+    0x00000000u,
+    0x00050051u,
+    0x00000006u,
+    0x000000dau,
+    0x000000d8u,
+    0x00000001u,
+    0x00050051u,
+    0x00000006u,
+    0x000000dbu,
+    0x000000d8u,
+    0x00000002u,
+    0x00070050u,
+    0x00000010u,
+    0x000000dcu,
+    0x000000d9u,
+    0x000000dau,
+    0x000000dbu,
+    0x0000001eu,
+    0x0003003eu,
+    0x000000d7u,
+    0x000000dcu,
+    0x00050041u,
+    0x0000001bu,
+    0x000000ddu,
+    0x000000d5u,
+    0x00000029u,
+    0x0004003du,
+    0x00000010u,
+    0x000000deu,
+    0x000000ddu,
+    0x0004003du,
+    0x00000010u,
+    0x000000dfu,
+    0x000000d7u,
+    0x00050094u,
+    0x00000006u,
+    0x000000e0u,
+    0x000000deu,
+    0x000000dfu,
+    0x00050041u,
+    0x0000001bu,
+    0x000000e1u,
+    0x000000d5u,
+    0x00000062u,
+    0x0004003du,
+    0x00000010u,
+    0x000000e2u,
+    0x000000e1u,
+    0x0004003du,
+    0x00000010u,
+    0x000000e3u,
+    0x000000d7u,
+    0x00050094u,
+    0x00000006u,
+    0x000000e4u,
+    0x000000e2u,
+    0x000000e3u,
+    0x00050041u,
+    0x0000001bu,
+    0x000000e6u,
+    0x000000d5u,
+    0x000000e5u,
+    0x0004003du,
+    0x00000010u,
+    0x000000e7u,
+    0x000000e6u,
+    0x0004003du,
+    0x00000010u,
+    0x000000e8u,
+    0x000000d7u,
+    0x00050094u,
+    0x00000006u,
+    0x000000e9u,
+    0x000000e7u,
+    0x000000e8u,
+    0x00060050u,
+    0x00000007u,
+    0x000000eau,
+    0x000000e0u,
+    0x000000e4u,
+    0x000000e9u,
+    0x000200feu,
+    0x000000eau,
+    0x00010038u,
+    0x00050036u,
+    0x00000007u,
+    0x00000019u,
+    0x00000000u,
+    0x00000009u,
+    0x00030037u,
+    0x00000008u,
+    0x00000018u,
+    0x000200f8u,
+    0x0000001au,
+    0x0004003bu,
+    0x000000d4u,
+    0x000000edu,
+    0x00000007u,
+    0x00040039u,
+    0x00000011u,
+    0x000000eeu,
+    0x00000013u,
+    0x0003003eu,
+    0x000000edu,
+    0x000000eeu,
+    0x00050041u,
+    0x0000001bu,
+    0x000000efu,
+    0x000000edu,
+    0x00000029u,
+    0x0004003du,
+    0x00000010u,
+    0x000000f0u,
+    0x000000efu,
+    0x0008004fu,
+    0x00000007u,
+    0x000000f1u,
+    0x000000f0u,
+    0x000000f0u,
+    0x00000000u,
+    0x00000001u,
+    0x00000002u,
+    0x0004003du,
+    0x00000007u,
+    0x000000f2u,
+    0x00000018u,
+    0x00050094u,
+    0x00000006u,
+    0x000000f3u,
+    0x000000f1u,
+    0x000000f2u,
+    0x00050041u,
+    0x0000001bu,
+    0x000000f4u,
+    0x000000edu,
+    0x00000062u,
+    0x0004003du,
+    0x00000010u,
+    0x000000f5u,
+    0x000000f4u,
+    0x0008004fu,
+    0x00000007u,
+    0x000000f6u,
+    0x000000f5u,
+    0x000000f5u,
+    0x00000000u,
+    0x00000001u,
+    0x00000002u,
+    0x0004003du,
+    0x00000007u,
+    0x000000f7u,
+    0x00000018u,
+    0x00050094u,
+    0x00000006u,
+    0x000000f8u,
+    0x000000f6u,
+    0x000000f7u,
+    0x00050041u,
+    0x0000001bu,
+    0x000000f9u,
+    0x000000edu,
+    0x000000e5u,
+    0x0004003du,
+    0x00000010u,
+    0x000000fau,
+    0x000000f9u,
+    0x0008004fu,
+    0x00000007u,
+    0x000000fbu,
+    0x000000fau,
+    0x000000fau,
+    0x00000000u,
+    0x00000001u,
+    0x00000002u,
+    0x0004003du,
+    0x00000007u,
+    0x000000fcu,
+    0x00000018u,
+    0x00050094u,
+    0x00000006u,
+    0x000000fdu,
+    0x000000fbu,
+    0x000000fcu,
+    0x00060050u,
+    0x00000007u,
+    0x000000feu,
+    0x000000f3u,
+    0x000000f8u,
+    0x000000fdu,
+    0x000200feu,
+    0x000000feu,
+    0x00010038u
+};
+
+inline constexpr std::uint32_t validation_fragment_spirv[] = {
+    0x07230203u,
+    0x00010000u,
+    0x00080008u,
+    0x0000038fu,
+    0x00000000u,
+    0x00020011u,
+    0x00000001u,
+    0x00020011u,
+    0x0000002du,
+    0x00020011u,
+    0x00000032u,
+    0x0006000bu,
+    0x00000001u,
+    0x4c534c47u,
+    0x6474732eu,
+    0x3035342eu,
+    0x00000000u,
+    0x0003000eu,
+    0x00000000u,
+    0x00000001u,
+    0x000c000fu,
+    0x00000004u,
+    0x00000004u,
+    0x6e69616du,
+    0x00000000u,
+    0x00000011u,
+    0x00000083u,
+    0x000000eeu,
+    0x00000139u,
+    0x00000158u,
+    0x0000019du,
+    0x000002bdu,
+    0x00030010u,
+    0x00000004u,
+    0x00000007u,
+    0x00030003u,
+    0x00000002u,
+    0x000001c2u,
+    0x00040005u,
+    0x00000004u,
+    0x6e69616du,
+    0x00000000u,
+    0x00040005u,
+    0x00000009u,
+    0x706d6173u,
+    0x0064656cu,
+    0x00060005u,
+    0x0000000du,
+    0x66666964u,
+    0x5f657375u,
+    0x74786574u,
+    0x00657275u,
+    0x00060005u,
+    0x00000011u,
+    0x74726576u,
+    0x745f7865u,
+    0x6f637865u,
+    0x0064726fu,
+    0x00050005u,
+    0x00000015u,
+    0x6574614du,
+    0x6c616972u,
+    0x00000000u,
+    0x00080006u,
+    0x00000015u,
+    0x00000000u,
+    0x73756e75u,
+    0x745f6465u,
+    0x736e6172u,
+    0x6d726f66u,
+    0x00000000u,
+    0x00070006u,
+    0x00000015u,
+    0x00000001u,
+    0x6d726f6eu,
+    0x635f6c61u,
+    0x6d756c6fu,
+    0x0000306eu,
+    0x00070006u,
+    0x00000015u,
+    0x00000002u,
+    0x6d726f6eu,
+    0x635f6c61u,
+    0x6d756c6fu,
+    0x0000316eu,
+    0x00070006u,
+    0x00000015u,
+    0x00000003u,
+    0x6d726f6eu,
+    0x635f6c61u,
+    0x6d756c6fu,
+    0x0000326eu,
+    0x00070006u,
+    0x00000015u,
+    0x00000004u,
+    0x6574616du,
+    0x6c616972u,
+    0x616c665fu,
+    0x00007367u,
+    0x00050005u,
+    0x00000017u,
+    0x6574616du,
+    0x6c616972u,
+    0x00000000u,
+    0x00040005u,
+    0x00000030u,
+    0x67616c66u,
+    0x00000073u,
+    0x00040005u,
+    0x00000036u,
+    0x65746177u,
+    0x00000072u,
+    0x00050005u,
+    0x0000003cu,
+    0x65646e75u,
+    0x74617772u,
+    0x00007265u,
+    0x00030005u,
+    0x00000041u,
+    0x00796b73u,
+    0x00040005u,
+    0x00000046u,
+    0x756f6c63u,
+    0x00000064u,
+    0x00030005u,
+    0x0000004bu,
+    0x006e7573u,
+    0x00040005u,
+    0x00000050u,
+    0x6e6f6f6du,
+    0x00000000u,
+    0x00040005u,
+    0x00000055u,
+    0x64616873u,
+    0x0000776fu,
+    0x00060005u,
+    0x0000005au,
+    0x65636572u,
+    0x73657669u,
+    0x6168735fu,
+    0x00776f64u,
+    0x00070005u,
+    0x0000005fu,
+    0x6c666572u,
+    0x69746365u,
+    0x705f6e6fu,
+    0x65626f72u,
+    0x00000073u,
+    0x00050005u,
+    0x00000064u,
+    0x69766e65u,
+    0x6d6e6f72u,
+    0x00746e65u,
+    0x00050005u,
+    0x00000070u,
+    0x69746c61u,
+    0x65647574u,
+    0x00000000u,
+    0x00040005u,
+    0x00000078u,
+    0x69726f68u,
+    0x006e6f7au,
+    0x00040005u,
+    0x0000007du,
+    0x696e657au,
+    0x00006874u,
+    0x00060005u,
+    0x00000083u,
+    0x67617266u,
+    0x746e656du,
+    0x6c6f635fu,
+    0x0000726fu,
+    0x00040005u,
+    0x00000091u,
+    0x736e6564u,
+    0x00797469u,
+    0x00050005u,
+    0x00000097u,
+    0x756f6c63u,
+    0x6f635f64u,
+    0x00726f6cu,
+    0x00040005u,
+    0x000000b0u,
+    0x63736964u,
+    0x00000000u,
+    0x00040005u,
+    0x000000e0u,
+    0x65766177u,
+    0x00000073u,
+    0x00060005u,
+    0x000000e1u,
+    0x6d726f6eu,
+    0x745f6c61u,
+    0x75747865u,
+    0x00006572u,
+    0x00060005u,
+    0x000000ecu,
+    0x66727573u,
+    0x5f656361u,
+    0x6d726f6eu,
+    0x00006c61u,
+    0x00060005u,
+    0x000000eeu,
+    0x74726576u,
+    0x6e5f7865u,
+    0x616d726fu,
+    0x0000006cu,
+    0x00040005u,
+    0x000000fdu,
+    0x69636166u,
+    0x0000676eu,
+    0x00040005u,
+    0x00000102u,
+    0x73657266u,
+    0x006c656eu,
+    0x00050005u,
+    0x00000107u,
+    0x70656564u,
+    0x6c6f635fu,
+    0x0000726fu,
+    0x00070005u,
+    0x00000113u,
+    0x6c666572u,
+    0x69746365u,
+    0x635f6e6fu,
+    0x726f6c6fu,
+    0x00000000u,
+    0x00050005u,
+    0x0000011fu,
+    0x65746177u,
+    0x6f635f72u,
+    0x00726f6cu,
+    0x00060005u,
+    0x00000139u,
+    0x74726576u,
+    0x635f7865u,
+    0x726f6c6fu,
+    0x00000000u,
+    0x00060005u,
+    0x0000014du,
+    0x66727573u,
+    0x5f656361u,
+    0x6d726f6eu,
+    0x00006c61u,
+    0x00040005u,
+    0x00000156u,
+    0x676e6174u,
+    0x00746e65u,
+    0x00060005u,
+    0x00000158u,
+    0x74726576u,
+    0x745f7865u,
+    0x65676e61u,
+    0x0000746eu,
+    0x00050005u,
+    0x00000163u,
+    0x61746962u,
+    0x6e65676eu,
+    0x00000074u,
+    0x00060005u,
+    0x0000016au,
+    0x7070616du,
+    0x6e5f6465u,
+    0x616d726fu,
+    0x0000006cu,
+    0x00060005u,
+    0x00000186u,
+    0x6867696cu,
+    0x69645f74u,
+    0x74636572u,
+    0x006e6f69u,
+    0x00040005u,
+    0x0000018bu,
+    0x66666964u,
+    0x00657375u,
+    0x00070005u,
+    0x0000019du,
+    0x74726576u,
+    0x735f7865u,
+    0x6f646168u,
+    0x6f635f77u,
+    0x0064726fu,
+    0x00050005u,
+    0x000001a4u,
+    0x6a6f7270u,
+    0x65746365u,
+    0x00000064u,
+    0x00050005u,
+    0x000001acu,
+    0x64616873u,
+    0x755f776fu,
+    0x00000076u,
+    0x00050005u,
+    0x000001b2u,
+    0x69736976u,
+    0x696c6962u,
+    0x00007974u,
+    0x00040005u,
+    0x000001cdu,
+    0x65786574u,
+    0x0000006cu,
+    0x00060005u,
+    0x000001d1u,
+    0x64616873u,
+    0x745f776fu,
+    0x75747865u,
+    0x00006572u,
+    0x00030005u,
+    0x000001d9u,
+    0x00000079u,
+    0x00030005u,
+    0x000001e3u,
+    0x00000078u,
+    0x00050005u,
+    0x00000216u,
+    0x666c6168u,
+    0x6365765fu,
+    0x00726f74u,
+    0x00060005u,
+    0x0000021bu,
+    0x6574616du,
+    0x6c616972u,
+    0x6d61735fu,
+    0x00656c70u,
+    0x00070005u,
+    0x0000021cu,
+    0x63657073u,
+    0x72616c75u,
+    0x7865745fu,
+    0x65727574u,
+    0x00000000u,
+    0x00030005u,
+    0x00000221u,
+    0x00726270u,
+    0x00060005u,
+    0x00000225u,
+    0x6174656du,
+    0x63696c6cu,
+    0x6361665fu,
+    0x00726f74u,
+    0x00070005u,
+    0x00000228u,
+    0x67756f72u,
+    0x73656e68u,
+    0x61665f73u,
+    0x726f7463u,
+    0x00000000u,
+    0x00050005u,
+    0x0000022cu,
+    0x67756f72u,
+    0x73656e68u,
+    0x00000073u,
+    0x00050005u,
+    0x00000239u,
+    0x6f707865u,
+    0x746e656eu,
+    0x00000000u,
+    0x00050005u,
+    0x00000245u,
+    0x68676968u,
+    0x6867696cu,
+    0x00000074u,
+    0x00050005u,
+    0x0000024cu,
+    0x6174656du,
+    0x63696c6cu,
+    0x00000000u,
+    0x00050005u,
+    0x00000257u,
+    0x65727473u,
+    0x6874676eu,
+    0x00000000u,
+    0x00040005u,
+    0x00000270u,
+    0x746f646eu,
+    0x00000076u,
+    0x00040005u,
+    0x00000274u,
+    0x73657266u,
+    0x006c656eu,
+    0x00030005u,
+    0x00000279u,
+    0x00003066u,
+    0x00060005u,
+    0x00000280u,
+    0x73657266u,
+    0x5f6c656eu,
+    0x6f6c6f63u,
+    0x00000072u,
+    0x00050005u,
+    0x0000028bu,
+    0x6c666572u,
+    0x65746365u,
+    0x00000064u,
+    0x00050005u,
+    0x00000290u,
+    0x61727269u,
+    0x6e616964u,
+    0x00006563u,
+    0x00050005u,
+    0x00000291u,
+    0x69646172u,
+    0x65636e61u,
+    0x00000000u,
+    0x00060005u,
+    0x00000292u,
+    0x626f7270u,
+    0x65775f65u,
+    0x74686769u,
+    0x00000000u,
+    0x00050005u,
+    0x00000293u,
+    0x626f7270u,
+    0x6e695f65u,
+    0x00786564u,
+    0x00050005u,
+    0x0000029bu,
+    0x626f7250u,
+    0x63655265u,
+    0x0064726fu,
+    0x00040006u,
+    0x0000029bu,
+    0x00000000u,
+    0x00786f62u,
+    0x00050006u,
+    0x0000029bu,
+    0x00000001u,
+    0x65687073u,
+    0x00006572u,
+    0x00060006u,
+    0x0000029bu,
+    0x00000002u,
+    0x61726170u,
+    0x6574656du,
+    0x00007372u,
+    0x00050006u,
+    0x0000029bu,
+    0x00000003u,
+    0x69646e69u,
+    0x00736563u,
+    0x00060005u,
+    0x0000029du,
+    0x626f7250u,
+    0x74654d65u,
+    0x74616461u,
+    0x00000061u,
+    0x00050006u,
+    0x0000029du,
+    0x00000000u,
+    0x6f636572u,
+    0x00736472u,
+    0x00040005u,
+    0x0000029fu,
+    0x626f7270u,
+    0x00007365u,
+    0x00050005u,
+    0x000002a3u,
+    0x626f7250u,
+    0x63655265u,
+    0x0064726fu,
+    0x00040006u,
+    0x000002a3u,
+    0x00000000u,
+    0x00786f62u,
+    0x00050006u,
+    0x000002a3u,
+    0x00000001u,
+    0x65687073u,
+    0x00006572u,
+    0x00060006u,
+    0x000002a3u,
+    0x00000002u,
+    0x61726170u,
+    0x6574656du,
+    0x00007372u,
+    0x00050006u,
+    0x000002a3u,
+    0x00000003u,
+    0x69646e69u,
+    0x00736563u,
+    0x00040005u,
+    0x000002a5u,
+    0x626f7270u,
+    0x00000065u,
+    0x00040005u,
+    0x000002bau,
+    0x61636f6cu,
+    0x0000006cu,
+    0x00080005u,
+    0x000002bdu,
+    0x74726576u,
+    0x765f7865u,
+    0x5f776569u,
+    0x69736f70u,
+    0x6e6f6974u,
+    0x00000000u,
+    0x00040005u,
+    0x000002c5u,
+    0x67696577u,
+    0x00007468u,
+    0x00040005u,
+    0x000002d4u,
+    0x69646172u,
+    0x00007375u,
+    0x00040005u,
+    0x000002edu,
+    0x6579616cu,
+    0x00000072u,
+    0x00070005u,
+    0x000002f5u,
+    0x61727269u,
+    0x6e616964u,
+    0x705f6563u,
+    0x65626f72u,
+    0x00000073u,
+    0x00060005u,
+    0x00000307u,
+    0x69646172u,
+    0x65636e61u,
+    0x6f72705fu,
+    0x00736562u,
+    0x00050005u,
+    0x0000032du,
+    0x66666964u,
+    0x5f657375u,
+    0x006c6269u,
+    0x00060005u,
+    0x00000335u,
+    0x63657073u,
+    0x72616c75u,
+    0x6c62695fu,
+    0x00000000u,
+    0x00050005u,
+    0x00000360u,
+    0x6c63636fu,
+    0x6f697375u,
+    0x0000006eu,
+    0x00070005u,
+    0x00000361u,
+    0x6c63636fu,
+    0x6f697375u,
+    0x65745f6eu,
+    0x72757478u,
+    0x00000065u,
+    0x00050005u,
+    0x00000377u,
+    0x73696d65u,
+    0x65766973u,
+    0x00000000u,
+    0x00070005u,
+    0x00000378u,
+    0x73696d65u,
+    0x65766973u,
+    0x7865745fu,
+    0x65727574u,
+    0x00000000u,
+    0x00040047u,
+    0x0000000du,
+    0x00000022u,
+    0x00000000u,
+    0x00040047u,
+    0x0000000du,
+    0x00000021u,
+    0x00000000u,
+    0x00040047u,
+    0x00000011u,
+    0x0000001eu,
+    0x00000001u,
+    0x00040048u,
+    0x00000015u,
+    0x00000000u,
+    0x00000005u,
+    0x00050048u,
+    0x00000015u,
+    0x00000000u,
+    0x00000023u,
+    0x00000000u,
+    0x00050048u,
+    0x00000015u,
+    0x00000000u,
+    0x00000007u,
+    0x00000010u,
+    0x00050048u,
+    0x00000015u,
+    0x00000001u,
+    0x00000023u,
+    0x00000040u,
+    0x00050048u,
+    0x00000015u,
+    0x00000002u,
+    0x00000023u,
+    0x00000050u,
+    0x00050048u,
+    0x00000015u,
+    0x00000003u,
+    0x00000023u,
+    0x00000060u,
+    0x00050048u,
+    0x00000015u,
+    0x00000004u,
+    0x00000023u,
+    0x00000070u,
+    0x00030047u,
+    0x00000015u,
+    0x00000002u,
+    0x00040047u,
+    0x00000083u,
+    0x0000001eu,
+    0x00000000u,
+    0x00040047u,
+    0x000000e1u,
+    0x00000022u,
+    0x00000001u,
+    0x00040047u,
+    0x000000e1u,
+    0x00000021u,
+    0x00000000u,
+    0x00040047u,
+    0x000000eeu,
+    0x0000001eu,
+    0x00000002u,
+    0x00040047u,
+    0x00000139u,
+    0x0000001eu,
+    0x00000000u,
+    0x00040047u,
+    0x00000158u,
+    0x0000001eu,
+    0x00000003u,
+    0x00040047u,
+    0x0000019du,
+    0x0000001eu,
+    0x00000004u,
+    0x00040047u,
+    0x000001d1u,
+    0x00000022u,
+    0x00000006u,
+    0x00040047u,
+    0x000001d1u,
+    0x00000021u,
+    0x00000000u,
+    0x00040047u,
+    0x0000021cu,
+    0x00000022u,
+    0x00000002u,
+    0x00040047u,
+    0x0000021cu,
+    0x00000021u,
+    0x00000000u,
+    0x00040048u,
+    0x0000029bu,
+    0x00000000u,
+    0x00000005u,
+    0x00050048u,
+    0x0000029bu,
+    0x00000000u,
+    0x00000023u,
+    0x00000000u,
+    0x00050048u,
+    0x0000029bu,
+    0x00000000u,
+    0x00000007u,
+    0x00000010u,
+    0x00050048u,
+    0x0000029bu,
+    0x00000001u,
+    0x00000023u,
+    0x00000040u,
+    0x00050048u,
+    0x0000029bu,
+    0x00000002u,
+    0x00000023u,
+    0x00000050u,
+    0x00050048u,
+    0x0000029bu,
+    0x00000003u,
+    0x00000023u,
+    0x00000060u,
+    0x00040047u,
+    0x0000029cu,
+    0x00000006u,
+    0x00000070u,
+    0x00040048u,
+    0x0000029du,
+    0x00000000u,
+    0x00000018u,
+    0x00050048u,
+    0x0000029du,
+    0x00000000u,
+    0x00000023u,
+    0x00000000u,
+    0x00030047u,
+    0x0000029du,
+    0x00000003u,
+    0x00040047u,
+    0x0000029fu,
+    0x00000022u,
+    0x00000009u,
+    0x00040047u,
+    0x0000029fu,
+    0x00000021u,
+    0x00000000u,
+    0x00040047u,
+    0x000002bdu,
+    0x0000001eu,
+    0x00000005u,
+    0x00040047u,
+    0x000002f5u,
+    0x00000022u,
+    0x00000008u,
+    0x00040047u,
+    0x000002f5u,
+    0x00000021u,
+    0x00000000u,
+    0x00040047u,
+    0x00000307u,
+    0x00000022u,
+    0x00000007u,
+    0x00040047u,
+    0x00000307u,
+    0x00000021u,
+    0x00000000u,
+    0x00040047u,
+    0x00000361u,
+    0x00000022u,
+    0x00000003u,
+    0x00040047u,
+    0x00000361u,
+    0x00000021u,
+    0x00000000u,
+    0x00040047u,
+    0x00000378u,
+    0x00000022u,
+    0x00000004u,
+    0x00040047u,
+    0x00000378u,
+    0x00000021u,
+    0x00000000u,
+    0x00020013u,
+    0x00000002u,
+    0x00030021u,
+    0x00000003u,
+    0x00000002u,
+    0x00030016u,
+    0x00000006u,
+    0x00000020u,
+    0x00040017u,
+    0x00000007u,
+    0x00000006u,
+    0x00000004u,
+    0x00040020u,
+    0x00000008u,
+    0x00000007u,
+    0x00000007u,
+    0x00090019u,
+    0x0000000au,
+    0x00000006u,
+    0x00000001u,
+    0x00000000u,
+    0x00000000u,
+    0x00000000u,
+    0x00000001u,
+    0x00000000u,
+    0x0003001bu,
+    0x0000000bu,
+    0x0000000au,
+    0x00040020u,
+    0x0000000cu,
+    0x00000000u,
+    0x0000000bu,
+    0x0004003bu,
+    0x0000000cu,
+    0x0000000du,
+    0x00000000u,
+    0x00040017u,
+    0x0000000fu,
+    0x00000006u,
+    0x00000002u,
+    0x00040020u,
+    0x00000010u,
+    0x00000001u,
+    0x0000000fu,
+    0x0004003bu,
+    0x00000010u,
+    0x00000011u,
+    0x00000001u,
+    0x00040018u,
+    0x00000014u,
+    0x00000007u,
+    0x00000004u,
+    0x0007001eu,
+    0x00000015u,
+    0x00000014u,
+    0x00000007u,
+    0x00000007u,
+    0x00000007u,
+    0x00000007u,
+    0x00040020u,
+    0x00000016u,
+    0x00000009u,
+    0x00000015u,
+    0x0004003bu,
+    0x00000016u,
+    0x00000017u,
+    0x00000009u,
+    0x00040015u,
+    0x00000018u,
+    0x00000020u,
+    0x00000001u,
+    0x0004002bu,
+    0x00000018u,
+    0x00000019u,
+    0x00000004u,
+    0x00040015u,
+    0x0000001au,
+    0x00000020u,
+    0x00000000u,
+    0x0004002bu,
+    0x0000001au,
+    0x0000001bu,
+    0x00000002u,
+    0x00040020u,
+    0x0000001cu,
+    0x00000009u,
+    0x00000006u,
+    0x0004002bu,
+    0x00000006u,
+    0x0000001fu,
+    0x3f000000u,
+    0x00020014u,
+    0x00000020u,
+    0x00040017u,
+    0x00000024u,
+    0x00000006u,
+    0x00000003u,
+    0x0004002bu,
+    0x00000006u,
+    0x00000027u,
+    0x00000000u,
+    0x0006002cu,
+    0x00000024u,
+    0x00000028u,
+    0x00000027u,
+    0x00000027u,
+    0x00000027u,
+    0x0004002bu,
+    0x00000006u,
+    0x0000002au,
+    0x400ccccdu,
+    0x0006002cu,
+    0x00000024u,
+    0x0000002bu,
+    0x0000002au,
+    0x0000002au,
+    0x0000002au,
+    0x00040020u,
+    0x0000002fu,
+    0x00000007u,
+    0x00000018u,
+    0x0004002bu,
+    0x0000001au,
+    0x00000031u,
+    0x00000003u,
+    0x00040020u,
+    0x00000035u,
+    0x00000007u,
+    0x00000020u,
+    0x0004002bu,
+    0x00000018u,
+    0x00000038u,
+    0x00000080u,
+    0x0004002bu,
+    0x00000018u,
+    0x0000003au,
+    0x00000000u,
+    0x0004002bu,
+    0x00000018u,
+    0x0000003eu,
+    0x00000100u,
+    0x0004002bu,
+    0x00000018u,
+    0x00000043u,
+    0x00000200u,
+    0x0004002bu,
+    0x00000018u,
+    0x00000048u,
+    0x00000400u,
+    0x0004002bu,
+    0x00000018u,
+    0x0000004du,
+    0x00000800u,
+    0x0004002bu,
+    0x00000018u,
+    0x00000052u,
+    0x00001000u,
+    0x0004002bu,
+    0x00000018u,
+    0x00000057u,
+    0x00002000u,
+    0x0004002bu,
+    0x00000018u,
+    0x0000005cu,
+    0x00004000u,
+    0x0004002bu,
+    0x00000018u,
+    0x00000061u,
+    0x00008000u,
+    0x00040020u,
+    0x0000006fu,
+    0x00000007u,
+    0x00000006u,
+    0x0004002bu,
+    0x0000001au,
+    0x00000071u,
+    0x00000001u,
+    0x00040020u,
+    0x00000072u,
+    0x00000001u,
+    0x00000006u,
+    0x0004002bu,
+    0x00000006u,
+    0x00000075u,
+    0x3f800000u,
+    0x00040020u,
+    0x00000077u,
+    0x00000007u,
+    0x00000024u,
+    0x0004002bu,
+    0x00000006u,
+    0x00000079u,
+    0x3ed70a3du,
+    0x0004002bu,
+    0x00000006u,
+    0x0000007au,
+    0x3f147ae1u,
+    0x0004002bu,
+    0x00000006u,
+    0x0000007bu,
+    0x3f428f5cu,
+    0x0006002cu,
+    0x00000024u,
+    0x0000007cu,
+    0x00000079u,
+    0x0000007au,
+    0x0000007bu,
+    0x0004002bu,
+    0x00000006u,
+    0x0000007eu,
+    0x3d6147aeu,
+    0x0004002bu,
+    0x00000006u,
+    0x0000007fu,
+    0x3e23d70au,
+    0x0004002bu,
+    0x00000006u,
+    0x00000080u,
+    0x3ec28f5cu,
+    0x0006002cu,
+    0x00000024u,
+    0x00000081u,
+    0x0000007eu,
+    0x0000007fu,
+    0x00000080u,
+    0x00040020u,
+    0x00000082u,
+    0x00000003u,
+    0x00000007u,
+    0x0004003bu,
+    0x00000082u,
+    0x00000083u,
+    0x00000003u,
+    0x0004002bu,
+    0x00000006u,
+    0x00000092u,
+    0x3ea3d70au,
+    0x0004002bu,
+    0x0000001au,
+    0x00000093u,
+    0x00000000u,
+    0x0004002bu,
+    0x00000006u,
+    0x00000098u,
+    0x3ef5c28fu,
+    0x0004002bu,
+    0x00000006u,
+    0x00000099u,
+    0x3f07ae14u,
+    0x0004002bu,
+    0x00000006u,
+    0x0000009au,
+    0x3f19999au,
+    0x0006002cu,
+    0x00000024u,
+    0x0000009bu,
+    0x00000098u,
+    0x00000099u,
+    0x0000009au,
+    0x0004002bu,
+    0x00000006u,
+    0x0000009cu,
+    0x3f75c28fu,
+    0x0004002bu,
+    0x00000006u,
+    0x0000009du,
+    0x3f7851ecu,
+    0x0006002cu,
+    0x00000024u,
+    0x0000009eu,
+    0x0000009cu,
+    0x0000009du,
+    0x00000075u,
+    0x0004002bu,
+    0x00000006u,
+    0x000000beu,
+    0x3faccccdu,
+    0x0004002bu,
+    0x00000006u,
+    0x000000bfu,
+    0x3f8f5c29u,
+    0x0004002bu,
+    0x00000006u,
+    0x000000c0u,
+    0x3f51eb85u,
+    0x0006002cu,
+    0x00000024u,
+    0x000000c1u,
+    0x000000beu,
+    0x000000bfu,
+    0x000000c0u,
+    0x0004002bu,
+    0x00000006u,
+    0x000000ceu,
+    0x3f3851ecu,
+    0x0006002cu,
+    0x00000024u,
+    0x000000cfu,
+    0x000000ceu,
+    0x000000c0u,
+    0x00000075u,
+    0x0004003bu,
+    0x0000000cu,
+    0x000000e1u,
+    0x00000000u,
+    0x0004002bu,
+    0x00000006u,
+    0x000000e4u,
+    0x40400000u,
+    0x0004002bu,
+    0x00000006u,
+    0x000000e8u,
+    0x40000000u,
+    0x00040020u,
+    0x000000edu,
+    0x00000001u,
+    0x00000024u,
+    0x0004003bu,
+    0x000000edu,
+    0x000000eeu,
+    0x00000001u,
+    0x0004002bu,
+    0x00000006u,
+    0x000000f6u,
+    0x3e3851ecu,
+    0x0004002bu,
+    0x00000006u,
+    0x00000105u,
+    0x40800000u,
+    0x0004002bu,
+    0x00000006u,
+    0x00000109u,
+    0x3c75c28fu,
+    0x0004002bu,
+    0x00000006u,
+    0x0000010au,
+    0x3df5c28fu,
+    0x0006002cu,
+    0x00000024u,
+    0x0000010bu,
+    0x00000109u,
+    0x0000010au,
+    0x0000007fu,
+    0x0004002bu,
+    0x00000006u,
+    0x0000010cu,
+    0x3ccccccdu,
+    0x0004002bu,
+    0x00000006u,
+    0x0000010du,
+    0x3e4ccccdu,
+    0x0004002bu,
+    0x00000006u,
+    0x0000010eu,
+    0x3e8a3d71u,
+    0x0006002cu,
+    0x00000024u,
+    0x0000010fu,
+    0x0000010cu,
+    0x0000010du,
+    0x0000010eu,
+    0x00040017u,
+    0x00000110u,
+    0x00000020u,
+    0x00000003u,
+    0x0004002bu,
+    0x00000006u,
+    0x00000115u,
+    0x3da3d70au,
+    0x0004002bu,
+    0x00000006u,
+    0x00000116u,
+    0x3e8f5c29u,
+    0x0004002bu,
+    0x00000006u,
+    0x00000117u,
+    0x3e9eb852u,
+    0x0006002cu,
+    0x00000024u,
+    0x00000118u,
+    0x00000115u,
+    0x00000116u,
+    0x00000117u,
+    0x0004002bu,
+    0x00000006u,
+    0x00000119u,
+    0x3eb851ecu,
+    0x0004002bu,
+    0x00000006u,
+    0x0000011au,
+    0x3f0ccccdu,
+    0x0004002bu,
+    0x00000006u,
+    0x0000011bu,
+    0x3f2e147bu,
+    0x0006002cu,
+    0x00000024u,
+    0x0000011cu,
+    0x00000119u,
+    0x0000011au,
+    0x0000011bu,
+    0x0004002bu,
+    0x00000006u,
+    0x0000012au,
+    0x3d75c28fu,
+    0x0004002bu,
+    0x00000006u,
+    0x00000131u,
+    0x3f666666u,
+    0x0004003bu,
+    0x000000edu,
+    0x00000139u,
+    0x00000001u,
+    0x00040020u,
+    0x00000157u,
+    0x00000001u,
+    0x00000007u,
+    0x0004003bu,
+    0x00000157u,
+    0x00000158u,
+    0x00000001u,
+    0x00040018u,
+    0x00000175u,
+    0x00000024u,
+    0x00000003u,
+    0x0004002bu,
+    0x00000006u,
+    0x00000187u,
+    0x3eb36ef4u,
+    0x0004002bu,
+    0x00000006u,
+    0x00000188u,
+    0x3ee6b33au,
+    0x0004002bu,
+    0x00000006u,
+    0x00000189u,
+    0x3f523185u,
+    0x0006002cu,
+    0x00000024u,
+    0x0000018au,
+    0x00000187u,
+    0x00000188u,
+    0x00000189u,
+    0x0004002bu,
+    0x00000006u,
+    0x00000190u,
+    0x3e800000u,
+    0x0004002bu,
+    0x00000006u,
+    0x00000191u,
+    0x3f400000u,
+    0x0004003bu,
+    0x00000157u,
+    0x0000019du,
+    0x00000001u,
+    0x00040020u,
+    0x000001abu,
+    0x00000007u,
+    0x0000000fu,
+    0x0005002cu,
+    0x0000000fu,
+    0x000001b4u,
+    0x00000027u,
+    0x00000027u,
+    0x00040017u,
+    0x000001b5u,
+    0x00000020u,
+    0x00000002u,
+    0x0005002cu,
+    0x0000000fu,
+    0x000001bbu,
+    0x00000075u,
+    0x00000075u,
+    0x00090019u,
+    0x000001ceu,
+    0x00000006u,
+    0x00000001u,
+    0x00000001u,
+    0x00000000u,
+    0x00000000u,
+    0x00000001u,
+    0x00000000u,
+    0x0003001bu,
+    0x000001cfu,
+    0x000001ceu,
+    0x00040020u,
+    0x000001d0u,
+    0x00000000u,
+    0x000001cfu,
+    0x0004003bu,
+    0x000001d0u,
+    0x000001d1u,
+    0x00000000u,
+    0x00040017u,
+    0x000001d4u,
+    0x00000018u,
+    0x00000002u,
+    0x0004002bu,
+    0x00000018u,
+    0x000001dau,
+    0xffffffffu,
+    0x0004002bu,
+    0x00000018u,
+    0x000001e1u,
+    0x00000001u,
+    0x0004002bu,
+    0x00000006u,
+    0x000001f7u,
+    0x3ac49ba6u,
+    0x0004002bu,
+    0x00000006u,
+    0x00000204u,
+    0x41100000u,
+    0x0006002cu,
+    0x00000024u,
+    0x00000218u,
+    0x00000027u,
+    0x00000027u,
+    0x00000075u,
+    0x0004003bu,
+    0x0000000cu,
+    0x0000021cu,
+    0x00000000u,
+    0x0004002bu,
+    0x00000018u,
+    0x00000229u,
+    0x00000002u,
+    0x0004002bu,
+    0x00000006u,
+    0x00000235u,
+    0x3d23d70au,
+    0x0004002bu,
+    0x00000006u,
+    0x0000023eu,
+    0x42c00000u,
+    0x0004002bu,
+    0x00000006u,
+    0x0000023fu,
+    0x41000000u,
+    0x0004002bu,
+    0x00000006u,
+    0x00000243u,
+    0x42000000u,
+    0x0004002bu,
+    0x00000006u,
+    0x00000265u,
+    0x3eb33333u,
+    0x0004002bu,
+    0x00000006u,
+    0x00000277u,
+    0x40a00000u,
+    0x0006002cu,
+    0x00000024u,
+    0x0000027au,
+    0x00000235u,
+    0x00000235u,
+    0x00000235u,
+    0x0004002bu,
+    0x00000006u,
+    0x0000028cu,
+    0xbf800000u,
+    0x0006002cu,
+    0x00000024u,
+    0x0000028du,
+    0x00000027u,
+    0x00000027u,
+    0x0000028cu,
+    0x00040017u,
+    0x0000029au,
+    0x00000018u,
+    0x00000004u,
+    0x0006001eu,
+    0x0000029bu,
+    0x00000014u,
+    0x00000007u,
+    0x00000007u,
+    0x0000029au,
+    0x0003001du,
+    0x0000029cu,
+    0x0000029bu,
+    0x0003001eu,
+    0x0000029du,
+    0x0000029cu,
+    0x00040020u,
+    0x0000029eu,
+    0x00000002u,
+    0x0000029du,
+    0x0004003bu,
+    0x0000029eu,
+    0x0000029fu,
+    0x00000002u,
+    0x0006001eu,
+    0x000002a3u,
+    0x00000014u,
+    0x00000007u,
+    0x00000007u,
+    0x0000029au,
+    0x00040020u,
+    0x000002a4u,
+    0x00000007u,
+    0x000002a3u,
+    0x00040020u,
+    0x000002a7u,
+    0x00000002u,
+    0x0000029bu,
+    0x00040020u,
+    0x000002abu,
+    0x00000007u,
+    0x00000014u,
+    0x0004002bu,
+    0x00000018u,
+    0x000002b2u,
+    0x00000003u,
+    0x00040020u,
+    0x000002b3u,
+    0x00000007u,
+    0x0000029au,
+    0x0004003bu,
+    0x000000edu,
+    0x000002bdu,
+    0x00000001u,
+    0x0004002bu,
+    0x00000006u,
+    0x000002d7u,
+    0x3a83126fu,
+    0x0004002bu,
+    0x00000006u,
+    0x000002e8u,
+    0x38d1b717u,
+    0x00090019u,
+    0x000002f2u,
+    0x00000006u,
+    0x00000003u,
+    0x00000000u,
+    0x00000001u,
+    0x00000000u,
+    0x00000001u,
+    0x00000000u,
+    0x0003001bu,
+    0x000002f3u,
+    0x000002f2u,
+    0x00040020u,
+    0x000002f4u,
+    0x00000000u,
+    0x000002f3u,
+    0x0004003bu,
+    0x000002f4u,
+    0x000002f5u,
+    0x00000000u,
+    0x0004003bu,
+    0x000002f4u,
+    0x00000307u,
+    0x00000000u,
+    0x0004002bu,
+    0x00000006u,
+    0x00000311u,
+    0x40e00000u,
+    0x0004002bu,
+    0x00000006u,
+    0x0000033du,
+    0x3e6147aeu,
+    0x0004002bu,
+    0x00000006u,
+    0x00000348u,
+    0x3cf5c28fu,
+    0x0006002cu,
+    0x00000024u,
+    0x00000349u,
+    0x00000348u,
+    0x00000348u,
+    0x00000348u,
+    0x0004002bu,
+    0x00000006u,
+    0x00000351u,
+    0x3e19999au,
+    0x0004003bu,
+    0x0000000cu,
+    0x00000361u,
+    0x00000000u,
+    0x0004002bu,
+    0x00000006u,
+    0x00000367u,
+    0x3f266666u,
+    0x0004002bu,
+    0x00000006u,
+    0x00000370u,
+    0x41800000u,
+    0x0004003bu,
+    0x0000000cu,
+    0x00000378u,
+    0x00000000u,
+    0x00040020u,
+    0x00000386u,
+    0x00000003u,
+    0x00000006u,
+    0x00050036u,
+    0x00000002u,
+    0x00000004u,
+    0x00000000u,
+    0x00000003u,
+    0x000200f8u,
+    0x00000005u,
+    0x0004003bu,
+    0x00000008u,
+    0x00000009u,
+    0x00000007u,
+    0x0004003bu,
+    0x0000002fu,
+    0x00000030u,
+    0x00000007u,
+    0x0004003bu,
+    0x00000035u,
+    0x00000036u,
+    0x00000007u,
+    0x0004003bu,
+    0x00000035u,
+    0x0000003cu,
+    0x00000007u,
+    0x0004003bu,
+    0x00000035u,
+    0x00000041u,
+    0x00000007u,
+    0x0004003bu,
+    0x00000035u,
+    0x00000046u,
+    0x00000007u,
+    0x0004003bu,
+    0x00000035u,
+    0x0000004bu,
+    0x00000007u,
+    0x0004003bu,
+    0x00000035u,
+    0x00000050u,
+    0x00000007u,
+    0x0004003bu,
+    0x00000035u,
+    0x00000055u,
+    0x00000007u,
+    0x0004003bu,
+    0x00000035u,
+    0x0000005au,
+    0x00000007u,
+    0x0004003bu,
+    0x00000035u,
+    0x0000005fu,
+    0x00000007u,
+    0x0004003bu,
+    0x00000035u,
+    0x00000064u,
+    0x00000007u,
+    0x0004003bu,
+    0x0000006fu,
+    0x00000070u,
+    0x00000007u,
+    0x0004003bu,
+    0x00000077u,
+    0x00000078u,
+    0x00000007u,
+    0x0004003bu,
+    0x00000077u,
+    0x0000007du,
+    0x00000007u,
+    0x0004003bu,
+    0x0000006fu,
+    0x00000091u,
+    0x00000007u,
+    0x0004003bu,
+    0x00000077u,
+    0x00000097u,
+    0x00000007u,
+    0x0004003bu,
+    0x0000006fu,
+    0x000000b0u,
+    0x00000007u,
+    0x0004003bu,
+    0x00000077u,
+    0x000000e0u,
+    0x00000007u,
+    0x0004003bu,
+    0x00000077u,
+    0x000000ecu,
+    0x00000007u,
+    0x0004003bu,
+    0x0000006fu,
+    0x000000fdu,
+    0x00000007u,
+    0x0004003bu,
+    0x0000006fu,
+    0x00000102u,
+    0x00000007u,
+    0x0004003bu,
+    0x00000077u,
+    0x00000107u,
+    0x00000007u,
+    0x0004003bu,
+    0x00000077u,
+    0x00000113u,
+    0x00000007u,
+    0x0004003bu,
+    0x00000077u,
+    0x0000011fu,
+    0x00000007u,
+    0x0004003bu,
+    0x00000077u,
+    0x0000014du,
+    0x00000007u,
+    0x0004003bu,
+    0x00000077u,
+    0x00000156u,
+    0x00000007u,
+    0x0004003bu,
+    0x00000077u,
+    0x00000163u,
+    0x00000007u,
+    0x0004003bu,
+    0x00000077u,
+    0x0000016au,
+    0x00000007u,
+    0x0004003bu,
+    0x00000077u,
+    0x00000186u,
+    0x00000007u,
+    0x0004003bu,
+    0x0000006fu,
+    0x0000018bu,
+    0x00000007u,
+    0x0004003bu,
+    0x00000077u,
+    0x000001a4u,
+    0x00000007u,
+    0x0004003bu,
+    0x000001abu,
+    0x000001acu,
+    0x00000007u,
+    0x0004003bu,
+    0x0000006fu,
+    0x000001b2u,
+    0x00000007u,
+    0x0004003bu,
+    0x000001abu,
+    0x000001cdu,
+    0x00000007u,
+    0x0004003bu,
+    0x0000002fu,
+    0x000001d9u,
+    0x00000007u,
+    0x0004003bu,
+    0x0000002fu,
+    0x000001e3u,
+    0x00000007u,
+    0x0004003bu,
+    0x00000077u,
+    0x00000216u,
+    0x00000007u,
+    0x0004003bu,
+    0x00000077u,
+    0x0000021bu,
+    0x00000007u,
+    0x0004003bu,
+    0x00000035u,
+    0x00000221u,
+    0x00000007u,
+    0x0004003bu,
+    0x0000006fu,
+    0x00000225u,
+    0x00000007u,
+    0x0004003bu,
+    0x0000006fu,
+    0x00000228u,
+    0x00000007u,
+    0x0004003bu,
+    0x0000006fu,
+    0x0000022cu,
+    0x00000007u,
+    0x0004003bu,
+    0x0000006fu,
+    0x0000022eu,
+    0x00000007u,
+    0x0004003bu,
+    0x0000006fu,
+    0x00000239u,
+    0x00000007u,
+    0x0004003bu,
+    0x0000006fu,
+    0x0000023bu,
+    0x00000007u,
+    0x0004003bu,
+    0x0000006fu,
+    0x00000245u,
+    0x00000007u,
+    0x0004003bu,
+    0x0000006fu,
+    0x0000024cu,
+    0x00000007u,
+    0x0004003bu,
+    0x0000006fu,
+    0x0000024eu,
+    0x00000007u,
+    0x0004003bu,
+    0x0000006fu,
+    0x00000257u,
+    0x00000007u,
+    0x0004003bu,
+    0x0000006fu,
+    0x00000259u,
+    0x00000007u,
+    0x0004003bu,
+    0x0000006fu,
+    0x00000270u,
+    0x00000007u,
+    0x0004003bu,
+    0x0000006fu,
+    0x00000274u,
+    0x00000007u,
+    0x0004003bu,
+    0x00000077u,
+    0x00000279u,
+    0x00000007u,
+    0x0004003bu,
+    0x00000077u,
+    0x00000280u,
+    0x00000007u,
+    0x0004003bu,
+    0x00000077u,
+    0x0000028bu,
+    0x00000007u,
+    0x0004003bu,
+    0x00000077u,
+    0x00000290u,
+    0x00000007u,
+    0x0004003bu,
+    0x00000077u,
+    0x00000291u,
+    0x00000007u,
+    0x0004003bu,
+    0x0000006fu,
+    0x00000292u,
+    0x00000007u,
+    0x0004003bu,
+    0x0000002fu,
+    0x00000293u,
+    0x00000007u,
+    0x0004003bu,
+    0x000002a4u,
+    0x000002a5u,
+    0x00000007u,
+    0x0004003bu,
+    0x00000077u,
+    0x000002bau,
+    0x00000007u,
+    0x0004003bu,
+    0x0000006fu,
+    0x000002c5u,
+    0x00000007u,
+    0x0004003bu,
+    0x0000006fu,
+    0x000002d4u,
+    0x00000007u,
+    0x0004003bu,
+    0x0000006fu,
+    0x000002edu,
+    0x00000007u,
+    0x0004003bu,
+    0x00000077u,
+    0x0000032du,
+    0x00000007u,
+    0x0004003bu,
+    0x00000077u,
+    0x00000335u,
+    0x00000007u,
+    0x0004003bu,
+    0x0000006fu,
+    0x00000360u,
+    0x00000007u,
+    0x0004003bu,
+    0x00000077u,
+    0x00000377u,
+    0x00000007u,
+    0x0004003du,
+    0x0000000bu,
+    0x0000000eu,
+    0x0000000du,
+    0x0004003du,
+    0x0000000fu,
+    0x00000012u,
+    0x00000011u,
+    0x00050057u,
+    0x00000007u,
+    0x00000013u,
+    0x0000000eu,
+    0x00000012u,
+    0x0003003eu,
+    0x00000009u,
+    0x00000013u,
+    0x00060041u,
+    0x0000001cu,
+    0x0000001du,
+    0x00000017u,
+    0x00000019u,
+    0x0000001bu,
+    0x0004003du,
+    0x00000006u,
+    0x0000001eu,
+    0x0000001du,
+    0x000500bau,
+    0x00000020u,
+    0x00000021u,
+    0x0000001eu,
+    0x0000001fu,
+    0x000300f7u,
+    0x00000023u,
+    0x00000000u,
+    0x000400fau,
+    0x00000021u,
+    0x00000022u,
+    0x00000023u,
+    0x000200f8u,
+    0x00000022u,
+    0x0004003du,
+    0x00000007u,
+    0x00000025u,
+    0x00000009u,
+    0x0008004fu,
+    0x00000024u,
+    0x00000026u,
+    0x00000025u,
+    0x00000025u,
+    0x00000000u,
+    0x00000001u,
+    0x00000002u,
+    0x0007000cu,
+    0x00000024u,
+    0x00000029u,
+    0x00000001u,
+    0x00000028u,
+    0x00000026u,
+    0x00000028u,
+    0x0007000cu,
+    0x00000024u,
+    0x0000002cu,
+    0x00000001u,
+    0x0000001au,
+    0x00000029u,
+    0x0000002bu,
+    0x0004003du,
+    0x00000007u,
+    0x0000002du,
+    0x00000009u,
+    0x0009004fu,
+    0x00000007u,
+    0x0000002eu,
+    0x0000002du,
+    0x0000002cu,
+    0x00000004u,
+    0x00000005u,
+    0x00000006u,
+    0x00000003u,
+    0x0003003eu,
+    0x00000009u,
+    0x0000002eu,
+    0x000200f9u,
+    0x00000023u,
+    0x000200f8u,
+    0x00000023u,
+    0x00060041u,
+    0x0000001cu,
+    0x00000032u,
+    0x00000017u,
+    0x00000019u,
+    0x00000031u,
+    0x0004003du,
+    0x00000006u,
+    0x00000033u,
+    0x00000032u,
+    0x0004006eu,
+    0x00000018u,
+    0x00000034u,
+    0x00000033u,
+    0x0003003eu,
+    0x00000030u,
+    0x00000034u,
+    0x0004003du,
+    0x00000018u,
+    0x00000037u,
+    0x00000030u,
+    0x000500c7u,
+    0x00000018u,
+    0x00000039u,
+    0x00000037u,
+    0x00000038u,
+    0x000500abu,
+    0x00000020u,
+    0x0000003bu,
+    0x00000039u,
+    0x0000003au,
+    0x0003003eu,
+    0x00000036u,
+    0x0000003bu,
+    0x0004003du,
+    0x00000018u,
+    0x0000003du,
+    0x00000030u,
+    0x000500c7u,
+    0x00000018u,
+    0x0000003fu,
+    0x0000003du,
+    0x0000003eu,
+    0x000500abu,
+    0x00000020u,
+    0x00000040u,
+    0x0000003fu,
+    0x0000003au,
+    0x0003003eu,
+    0x0000003cu,
+    0x00000040u,
+    0x0004003du,
+    0x00000018u,
+    0x00000042u,
+    0x00000030u,
+    0x000500c7u,
+    0x00000018u,
+    0x00000044u,
+    0x00000042u,
+    0x00000043u,
+    0x000500abu,
+    0x00000020u,
+    0x00000045u,
+    0x00000044u,
+    0x0000003au,
+    0x0003003eu,
+    0x00000041u,
+    0x00000045u,
+    0x0004003du,
+    0x00000018u,
+    0x00000047u,
+    0x00000030u,
+    0x000500c7u,
+    0x00000018u,
+    0x00000049u,
+    0x00000047u,
+    0x00000048u,
+    0x000500abu,
+    0x00000020u,
+    0x0000004au,
+    0x00000049u,
+    0x0000003au,
+    0x0003003eu,
+    0x00000046u,
+    0x0000004au,
+    0x0004003du,
+    0x00000018u,
+    0x0000004cu,
+    0x00000030u,
+    0x000500c7u,
+    0x00000018u,
+    0x0000004eu,
+    0x0000004cu,
+    0x0000004du,
+    0x000500abu,
+    0x00000020u,
+    0x0000004fu,
+    0x0000004eu,
+    0x0000003au,
+    0x0003003eu,
+    0x0000004bu,
+    0x0000004fu,
+    0x0004003du,
+    0x00000018u,
+    0x00000051u,
+    0x00000030u,
+    0x000500c7u,
+    0x00000018u,
+    0x00000053u,
+    0x00000051u,
+    0x00000052u,
+    0x000500abu,
+    0x00000020u,
+    0x00000054u,
+    0x00000053u,
+    0x0000003au,
+    0x0003003eu,
+    0x00000050u,
+    0x00000054u,
+    0x0004003du,
+    0x00000018u,
+    0x00000056u,
+    0x00000030u,
+    0x000500c7u,
+    0x00000018u,
+    0x00000058u,
+    0x00000056u,
+    0x00000057u,
+    0x000500abu,
+    0x00000020u,
+    0x00000059u,
+    0x00000058u,
+    0x0000003au,
+    0x0003003eu,
+    0x00000055u,
+    0x00000059u,
+    0x0004003du,
+    0x00000018u,
+    0x0000005bu,
+    0x00000030u,
+    0x000500c7u,
+    0x00000018u,
+    0x0000005du,
+    0x0000005bu,
+    0x0000005cu,
+    0x000500abu,
+    0x00000020u,
+    0x0000005eu,
+    0x0000005du,
+    0x0000003au,
+    0x0003003eu,
+    0x0000005au,
+    0x0000005eu,
+    0x0004003du,
+    0x00000018u,
+    0x00000060u,
+    0x00000030u,
+    0x000500c7u,
+    0x00000018u,
+    0x00000062u,
+    0x00000060u,
+    0x00000061u,
+    0x000500abu,
+    0x00000020u,
+    0x00000063u,
+    0x00000062u,
+    0x0000003au,
+    0x0003003eu,
+    0x0000005fu,
+    0x00000063u,
+    0x0004003du,
+    0x00000020u,
+    0x00000065u,
+    0x00000041u,
+    0x0004003du,
+    0x00000020u,
+    0x00000066u,
+    0x00000046u,
+    0x000500a6u,
+    0x00000020u,
+    0x00000067u,
+    0x00000065u,
+    0x00000066u,
+    0x0004003du,
+    0x00000020u,
+    0x00000068u,
+    0x0000004bu,
+    0x000500a6u,
+    0x00000020u,
+    0x00000069u,
+    0x00000067u,
+    0x00000068u,
+    0x0004003du,
+    0x00000020u,
+    0x0000006au,
+    0x00000050u,
+    0x000500a6u,
+    0x00000020u,
+    0x0000006bu,
+    0x00000069u,
+    0x0000006au,
+    0x0003003eu,
+    0x00000064u,
+    0x0000006bu,
+    0x0004003du,
+    0x00000020u,
+    0x0000006cu,
+    0x00000041u,
+    0x000300f7u,
+    0x0000006eu,
+    0x00000000u,
+    0x000400fau,
+    0x0000006cu,
+    0x0000006du,
+    0x0000008du,
+    0x000200f8u,
+    0x0000006du,
+    0x00050041u,
+    0x00000072u,
+    0x00000073u,
+    0x00000011u,
+    0x00000071u,
+    0x0004003du,
+    0x00000006u,
+    0x00000074u,
+    0x00000073u,
+    0x0008000cu,
+    0x00000006u,
+    0x00000076u,
+    0x00000001u,
+    0x0000002bu,
+    0x00000074u,
+    0x00000027u,
+    0x00000075u,
+    0x0003003eu,
+    0x00000070u,
+    0x00000076u,
+    0x0003003eu,
+    0x00000078u,
+    0x0000007cu,
+    0x0003003eu,
+    0x0000007du,
+    0x00000081u,
+    0x0004003du,
+    0x00000024u,
+    0x00000084u,
+    0x00000078u,
+    0x0004003du,
+    0x00000024u,
+    0x00000085u,
+    0x0000007du,
+    0x0004003du,
+    0x00000006u,
+    0x00000086u,
+    0x00000070u,
+    0x00060050u,
+    0x00000024u,
+    0x00000087u,
+    0x00000086u,
+    0x00000086u,
+    0x00000086u,
+    0x0008000cu,
+    0x00000024u,
+    0x00000088u,
+    0x00000001u,
+    0x0000002eu,
+    0x00000084u,
+    0x00000085u,
+    0x00000087u,
+    0x00050051u,
+    0x00000006u,
+    0x00000089u,
+    0x00000088u,
+    0x00000000u,
+    0x00050051u,
+    0x00000006u,
+    0x0000008au,
+    0x00000088u,
+    0x00000001u,
+    0x00050051u,
+    0x00000006u,
+    0x0000008bu,
+    0x00000088u,
+    0x00000002u,
+    0x00070050u,
+    0x00000007u,
+    0x0000008cu,
+    0x00000089u,
+    0x0000008au,
+    0x0000008bu,
+    0x00000075u,
+    0x0003003eu,
+    0x00000083u,
+    0x0000008cu,
+    0x000200f9u,
+    0x0000006eu,
+    0x000200f8u,
+    0x0000008du,
+    0x0004003du,
+    0x00000020u,
+    0x0000008eu,
+    0x00000046u,
+    0x000300f7u,
+    0x00000090u,
+    0x00000000u,
+    0x000400fau,
+    0x0000008eu,
+    0x0000008fu,
+    0x000000acu,
+    0x000200f8u,
+    0x0000008fu,
+    0x00050041u,
+    0x0000006fu,
+    0x00000094u,
+    0x00000009u,
+    0x00000093u,
+    0x0004003du,
+    0x00000006u,
+    0x00000095u,
+    0x00000094u,
+    0x0008000cu,
+    0x00000006u,
+    0x00000096u,
+    0x00000001u,
+    0x00000031u,
+    0x00000092u,
+    0x0000007bu,
+    0x00000095u,
+    0x0003003eu,
+    0x00000091u,
+    0x00000096u,
+    0x00050041u,
+    0x0000006fu,
+    0x0000009fu,
+    0x00000009u,
+    0x00000071u,
+    0x0004003du,
+    0x00000006u,
+    0x000000a0u,
+    0x0000009fu,
+    0x00060050u,
+    0x00000024u,
+    0x000000a1u,
+    0x000000a0u,
+    0x000000a0u,
+    0x000000a0u,
+    0x0008000cu,
+    0x00000024u,
+    0x000000a2u,
+    0x00000001u,
+    0x0000002eu,
+    0x0000009bu,
+    0x0000009eu,
+    0x000000a1u,
+    0x0003003eu,
+    0x00000097u,
+    0x000000a2u,
+    0x0004003du,
+    0x00000024u,
+    0x000000a3u,
+    0x00000097u,
+    0x0004003du,
+    0x00000006u,
+    0x000000a4u,
+    0x00000091u,
+    0x00050041u,
+    0x0000006fu,
+    0x000000a5u,
+    0x00000009u,
+    0x00000031u,
+    0x0004003du,
+    0x00000006u,
+    0x000000a6u,
+    0x000000a5u,
+    0x00050085u,
+    0x00000006u,
+    0x000000a7u,
+    0x000000a4u,
+    0x000000a6u,
+    0x00050051u,
+    0x00000006u,
+    0x000000a8u,
+    0x000000a3u,
+    0x00000000u,
+    0x00050051u,
+    0x00000006u,
+    0x000000a9u,
+    0x000000a3u,
+    0x00000001u,
+    0x00050051u,
+    0x00000006u,
+    0x000000aau,
+    0x000000a3u,
+    0x00000002u,
+    0x00070050u,
+    0x00000007u,
+    0x000000abu,
+    0x000000a8u,
+    0x000000a9u,
+    0x000000aau,
+    0x000000a7u,
+    0x0003003eu,
+    0x00000083u,
+    0x000000abu,
+    0x000200f9u,
+    0x00000090u,
+    0x000200f8u,
+    0x000000acu,
+    0x0004003du,
+    0x00000020u,
+    0x000000adu,
+    0x0000004bu,
+    0x000300f7u,
+    0x000000afu,
+    0x00000000u,
+    0x000400fau,
+    0x000000adu,
+    0x000000aeu,
+    0x000000c8u,
+    0x000200f8u,
+    0x000000aeu,
+    0x00050041u,
+    0x0000006fu,
+    0x000000b1u,
+    0x00000009u,
+    0x00000031u,
+    0x0004003du,
+    0x00000006u,
+    0x000000b2u,
+    0x000000b1u,
+    0x00050041u,
+    0x0000006fu,
+    0x000000b3u,
+    0x00000009u,
+    0x00000093u,
+    0x0004003du,
+    0x00000006u,
+    0x000000b4u,
+    0x000000b3u,
+    0x00050041u,
+    0x0000006fu,
+    0x000000b5u,
+    0x00000009u,
+    0x00000071u,
+    0x0004003du,
+    0x00000006u,
+    0x000000b6u,
+    0x000000b5u,
+    0x0007000cu,
+    0x00000006u,
+    0x000000b7u,
+    0x00000001u,
+    0x00000028u,
+    0x000000b4u,
+    0x000000b6u,
+    0x00050041u,
+    0x0000006fu,
+    0x000000b8u,
+    0x00000009u,
+    0x0000001bu,
+    0x0004003du,
+    0x00000006u,
+    0x000000b9u,
+    0x000000b8u,
+    0x0007000cu,
+    0x00000006u,
+    0x000000bau,
+    0x00000001u,
+    0x00000028u,
+    0x000000b7u,
+    0x000000b9u,
+    0x00050085u,
+    0x00000006u,
+    0x000000bbu,
+    0x000000b2u,
+    0x000000bau,
+    0x0003003eu,
+    0x000000b0u,
+    0x000000bbu,
+    0x0004003du,
+    0x00000007u,
+    0x000000bcu,
+    0x00000009u,
+    0x0008004fu,
+    0x00000024u,
+    0x000000bdu,
+    0x000000bcu,
+    0x000000bcu,
+    0x00000000u,
+    0x00000001u,
+    0x00000002u,
+    0x00050085u,
+    0x00000024u,
+    0x000000c2u,
+    0x000000bdu,
+    0x000000c1u,
+    0x0004003du,
+    0x00000006u,
+    0x000000c3u,
+    0x000000b0u,
+    0x00050051u,
+    0x00000006u,
+    0x000000c4u,
+    0x000000c2u,
+    0x00000000u,
+    0x00050051u,
+    0x00000006u,
+    0x000000c5u,
+    0x000000c2u,
+    0x00000001u,
+    0x00050051u,
+    0x00000006u,
+    0x000000c6u,
+    0x000000c2u,
+    0x00000002u,
+    0x00070050u,
+    0x00000007u,
+    0x000000c7u,
+    0x000000c4u,
+    0x000000c5u,
+    0x000000c6u,
+    0x000000c3u,
+    0x0003003eu,
+    0x00000083u,
+    0x000000c7u,
+    0x000200f9u,
+    0x000000afu,
+    0x000200f8u,
+    0x000000c8u,
+    0x0004003du,
+    0x00000020u,
+    0x000000c9u,
+    0x00000050u,
+    0x000300f7u,
+    0x000000cbu,
+    0x00000000u,
+    0x000400fau,
+    0x000000c9u,
+    0x000000cau,
+    0x000000d7u,
+    0x000200f8u,
+    0x000000cau,
+    0x0004003du,
+    0x00000007u,
+    0x000000ccu,
+    0x00000009u,
+    0x0008004fu,
+    0x00000024u,
+    0x000000cdu,
+    0x000000ccu,
+    0x000000ccu,
+    0x00000000u,
+    0x00000001u,
+    0x00000002u,
+    0x00050085u,
+    0x00000024u,
+    0x000000d0u,
+    0x000000cdu,
+    0x000000cfu,
+    0x00050041u,
+    0x0000006fu,
+    0x000000d1u,
+    0x00000009u,
+    0x00000031u,
+    0x0004003du,
+    0x00000006u,
+    0x000000d2u,
+    0x000000d1u,
+    0x00050051u,
+    0x00000006u,
+    0x000000d3u,
+    0x000000d0u,
+    0x00000000u,
+    0x00050051u,
+    0x00000006u,
+    0x000000d4u,
+    0x000000d0u,
+    0x00000001u,
+    0x00050051u,
+    0x00000006u,
+    0x000000d5u,
+    0x000000d0u,
+    0x00000002u,
+    0x00070050u,
+    0x00000007u,
+    0x000000d6u,
+    0x000000d3u,
+    0x000000d4u,
+    0x000000d5u,
+    0x000000d2u,
+    0x0003003eu,
+    0x00000083u,
+    0x000000d6u,
+    0x000200f9u,
+    0x000000cbu,
+    0x000200f8u,
+    0x000000d7u,
+    0x0004003du,
+    0x00000020u,
+    0x000000d8u,
+    0x00000055u,
+    0x000300f7u,
+    0x000000dau,
+    0x00000000u,
+    0x000400fau,
+    0x000000d8u,
+    0x000000d9u,
+    0x000000dcu,
+    0x000200f8u,
+    0x000000d9u,
+    0x0004003du,
+    0x00000007u,
+    0x000000dbu,
+    0x00000009u,
+    0x0003003eu,
+    0x00000083u,
+    0x000000dbu,
+    0x000200f9u,
+    0x000000dau,
+    0x000200f8u,
+    0x000000dcu,
+    0x0004003du,
+    0x00000020u,
+    0x000000ddu,
+    0x00000036u,
+    0x000300f7u,
+    0x000000dfu,
+    0x00000000u,
+    0x000400fau,
+    0x000000ddu,
+    0x000000deu,
+    0x00000137u,
+    0x000200f8u,
+    0x000000deu,
+    0x0004003du,
+    0x0000000bu,
+    0x000000e2u,
+    0x000000e1u,
+    0x0004003du,
+    0x0000000fu,
+    0x000000e3u,
+    0x00000011u,
+    0x0005008eu,
+    0x0000000fu,
+    0x000000e5u,
+    0x000000e3u,
+    0x000000e4u,
+    0x00050057u,
+    0x00000007u,
+    0x000000e6u,
+    0x000000e2u,
+    0x000000e5u,
+    0x0008004fu,
+    0x00000024u,
+    0x000000e7u,
+    0x000000e6u,
+    0x000000e6u,
+    0x00000000u,
+    0x00000001u,
+    0x00000002u,
+    0x0005008eu,
+    0x00000024u,
+    0x000000e9u,
+    0x000000e7u,
+    0x000000e8u,
+    0x00060050u,
+    0x00000024u,
+    0x000000eau,
+    0x00000075u,
+    0x00000075u,
+    0x00000075u,
+    0x00050083u,
+    0x00000024u,
+    0x000000ebu,
+    0x000000e9u,
+    0x000000eau,
+    0x0003003eu,
+    0x000000e0u,
+    0x000000ebu,
+    0x0004003du,
+    0x00000024u,
+    0x000000efu,
+    0x000000eeu,
+    0x0004003du,
+    0x00000024u,
+    0x000000f0u,
+    0x000000e0u,
+    0x0007004fu,
+    0x0000000fu,
+    0x000000f1u,
+    0x000000f0u,
+    0x000000f0u,
+    0x00000000u,
+    0x00000001u,
+    0x0005008eu,
+    0x0000000fu,
+    0x000000f2u,
+    0x000000f1u,
+    0x00000080u,
+    0x00050041u,
+    0x0000006fu,
+    0x000000f3u,
+    0x000000e0u,
+    0x0000001bu,
+    0x0004003du,
+    0x00000006u,
+    0x000000f4u,
+    0x000000f3u,
+    0x0006000cu,
+    0x00000006u,
+    0x000000f5u,
+    0x00000001u,
+    0x00000004u,
+    0x000000f4u,
+    0x00050085u,
+    0x00000006u,
+    0x000000f7u,
+    0x000000f5u,
+    0x000000f6u,
+    0x00050051u,
+    0x00000006u,
+    0x000000f8u,
+    0x000000f2u,
+    0x00000000u,
+    0x00050051u,
+    0x00000006u,
+    0x000000f9u,
+    0x000000f2u,
+    0x00000001u,
+    0x00060050u,
+    0x00000024u,
+    0x000000fau,
+    0x000000f8u,
+    0x000000f9u,
+    0x000000f7u,
+    0x00050081u,
+    0x00000024u,
+    0x000000fbu,
+    0x000000efu,
+    0x000000fau,
+    0x0006000cu,
+    0x00000024u,
+    0x000000fcu,
+    0x00000001u,
+    0x00000045u,
+    0x000000fbu,
+    0x0003003eu,
+    0x000000ecu,
+    0x000000fcu,
+    0x00050041u,
+    0x0000006fu,
+    0x000000feu,
+    0x000000ecu,
+    0x0000001bu,
+    0x0004003du,
+    0x00000006u,
+    0x000000ffu,
+    0x000000feu,
+    0x0006000cu,
+    0x00000006u,
+    0x00000100u,
+    0x00000001u,
+    0x00000004u,
+    0x000000ffu,
+    0x0008000cu,
+    0x00000006u,
+    0x00000101u,
+    0x00000001u,
+    0x0000002bu,
+    0x00000100u,
+    0x00000027u,
+    0x00000075u,
+    0x0003003eu,
+    0x000000fdu,
+    0x00000101u,
+    0x0004003du,
+    0x00000006u,
+    0x00000103u,
+    0x000000fdu,
+    0x00050083u,
+    0x00000006u,
+    0x00000104u,
+    0x00000075u,
+    0x00000103u,
+    0x0007000cu,
+    0x00000006u,
+    0x00000106u,
+    0x00000001u,
+    0x0000001au,
+    0x00000104u,
+    0x00000105u,
+    0x0003003eu,
+    0x00000102u,
+    0x00000106u,
+    0x0004003du,
+    0x00000020u,
+    0x00000108u,
+    0x0000003cu,
+    0x00060050u,
+    0x00000110u,
+    0x00000111u,
+    0x00000108u,
+    0x00000108u,
+    0x00000108u,
+    0x000600a9u,
+    0x00000024u,
+    0x00000112u,
+    0x00000111u,
+    0x0000010bu,
+    0x0000010fu,
+    0x0003003eu,
+    0x00000107u,
+    0x00000112u,
+    0x0004003du,
+    0x00000020u,
+    0x00000114u,
+    0x0000003cu,
+    0x00060050u,
+    0x00000110u,
+    0x0000011du,
+    0x00000114u,
+    0x00000114u,
+    0x00000114u,
+    0x000600a9u,
+    0x00000024u,
+    0x0000011eu,
+    0x0000011du,
+    0x00000118u,
+    0x0000011cu,
+    0x0003003eu,
+    0x00000113u,
+    0x0000011eu,
+    0x0004003du,
+    0x00000024u,
+    0x00000120u,
+    0x00000107u,
+    0x0004003du,
+    0x00000024u,
+    0x00000121u,
+    0x00000113u,
+    0x0004003du,
+    0x00000006u,
+    0x00000122u,
+    0x00000102u,
+    0x00050081u,
+    0x00000006u,
+    0x00000123u,
+    0x000000f6u,
+    0x00000122u,
+    0x0008000cu,
+    0x00000006u,
+    0x00000124u,
+    0x00000001u,
+    0x0000002bu,
+    0x00000123u,
+    0x00000027u,
+    0x00000075u,
+    0x00060050u,
+    0x00000024u,
+    0x00000125u,
+    0x00000124u,
+    0x00000124u,
+    0x00000124u,
+    0x0008000cu,
+    0x00000024u,
+    0x00000126u,
+    0x00000001u,
+    0x0000002eu,
+    0x00000120u,
+    0x00000121u,
+    0x00000125u,
+    0x0003003eu,
+    0x0000011fu,
+    0x00000126u,
+    0x0004003du,
+    0x00000007u,
+    0x00000127u,
+    0x00000009u,
+    0x0008004fu,
+    0x00000024u,
+    0x00000128u,
+    0x00000127u,
+    0x00000127u,
+    0x00000000u,
+    0x00000001u,
+    0x00000002u,
+    0x0004003du,
+    0x00000020u,
+    0x00000129u,
+    0x0000003cu,
+    0x000600a9u,
+    0x00000006u,
+    0x0000012bu,
+    0x00000129u,
+    0x0000012au,
+    0x0000010au,
+    0x0005008eu,
+    0x00000024u,
+    0x0000012cu,
+    0x00000128u,
+    0x0000012bu,
+    0x0004003du,
+    0x00000024u,
+    0x0000012du,
+    0x0000011fu,
+    0x00050081u,
+    0x00000024u,
+    0x0000012eu,
+    0x0000012du,
+    0x0000012cu,
+    0x0003003eu,
+    0x0000011fu,
+    0x0000012eu,
+    0x0004003du,
+    0x00000024u,
+    0x0000012fu,
+    0x0000011fu,
+    0x0004003du,
+    0x00000020u,
+    0x00000130u,
+    0x0000003cu,
+    0x000600a9u,
+    0x00000006u,
+    0x00000132u,
+    0x00000130u,
+    0x00000131u,
+    0x000000ceu,
+    0x00050051u,
+    0x00000006u,
+    0x00000133u,
+    0x0000012fu,
+    0x00000000u,
+    0x00050051u,
+    0x00000006u,
+    0x00000134u,
+    0x0000012fu,
+    0x00000001u,
+    0x00050051u,
+    0x00000006u,
+    0x00000135u,
+    0x0000012fu,
+    0x00000002u,
+    0x00070050u,
+    0x00000007u,
+    0x00000136u,
+    0x00000133u,
+    0x00000134u,
+    0x00000135u,
+    0x00000132u,
+    0x0003003eu,
+    0x00000083u,
+    0x00000136u,
+    0x000200f9u,
+    0x000000dfu,
+    0x000200f8u,
+    0x00000137u,
+    0x0004003du,
+    0x00000007u,
+    0x00000138u,
+    0x00000009u,
+    0x0004003du,
+    0x00000024u,
+    0x0000013au,
+    0x00000139u,
+    0x00050051u,
+    0x00000006u,
+    0x0000013bu,
+    0x0000013au,
+    0x00000000u,
+    0x00050051u,
+    0x00000006u,
+    0x0000013cu,
+    0x0000013au,
+    0x00000001u,
+    0x00050051u,
+    0x00000006u,
+    0x0000013du,
+    0x0000013au,
+    0x00000002u,
+    0x00070050u,
+    0x00000007u,
+    0x0000013eu,
+    0x0000013bu,
+    0x0000013cu,
+    0x0000013du,
+    0x00000075u,
+    0x00050085u,
+    0x00000007u,
+    0x0000013fu,
+    0x00000138u,
+    0x0000013eu,
+    0x0003003eu,
+    0x00000083u,
+    0x0000013fu,
+    0x000200f9u,
+    0x000000dfu,
+    0x000200f8u,
+    0x000000dfu,
+    0x000200f9u,
+    0x000000dau,
+    0x000200f8u,
+    0x000000dau,
+    0x000200f9u,
+    0x000000cbu,
+    0x000200f8u,
+    0x000000cbu,
+    0x000200f9u,
+    0x000000afu,
+    0x000200f8u,
+    0x000000afu,
+    0x000200f9u,
+    0x00000090u,
+    0x000200f8u,
+    0x00000090u,
+    0x000200f9u,
+    0x0000006eu,
+    0x000200f8u,
+    0x0000006eu,
+    0x0004003du,
+    0x00000020u,
+    0x00000140u,
+    0x00000036u,
+    0x000400a8u,
+    0x00000020u,
+    0x00000141u,
+    0x00000140u,
+    0x0004003du,
+    0x00000020u,
+    0x00000142u,
+    0x00000064u,
+    0x000400a8u,
+    0x00000020u,
+    0x00000143u,
+    0x00000142u,
+    0x000500a7u,
+    0x00000020u,
+    0x00000144u,
+    0x00000141u,
+    0x00000143u,
+    0x000300f7u,
+    0x00000146u,
+    0x00000000u,
+    0x000400fau,
+    0x00000144u,
+    0x00000145u,
+    0x00000146u,
+    0x000200f8u,
+    0x00000145u,
+    0x00060041u,
+    0x0000001cu,
+    0x00000147u,
+    0x00000017u,
+    0x00000019u,
+    0x00000071u,
+    0x0004003du,
+    0x00000006u,
+    0x00000148u,
+    0x00000147u,
+    0x000500bau,
+    0x00000020u,
+    0x00000149u,
+    0x00000148u,
+    0x0000001fu,
+    0x000200f9u,
+    0x00000146u,
+    0x000200f8u,
+    0x00000146u,
+    0x000700f5u,
+    0x00000020u,
+    0x0000014au,
+    0x00000144u,
+    0x0000006eu,
+    0x00000149u,
+    0x00000145u,
+    0x000300f7u,
+    0x0000014cu,
+    0x00000000u,
+    0x000400fau,
+    0x0000014au,
+    0x0000014bu,
+    0x0000014cu,
+    0x000200f8u,
+    0x0000014bu,
+    0x0004003du,
+    0x00000024u,
+    0x0000014eu,
+    0x000000eeu,
+    0x0006000cu,
+    0x00000024u,
+    0x0000014fu,
+    0x00000001u,
+    0x00000045u,
+    0x0000014eu,
+    0x0003003eu,
+    0x0000014du,
+    0x0000014fu,
+    0x00060041u,
+    0x0000001cu,
+    0x00000150u,
+    0x00000017u,
+    0x00000019u,
+    0x00000031u,
+    0x0004003du,
+    0x00000006u,
+    0x00000151u,
+    0x00000150u,
+    0x0005008du,
+    0x00000006u,
+    0x00000152u,
+    0x00000151u,
+    0x000000e8u,
+    0x000500bau,
+    0x00000020u,
+    0x00000153u,
+    0x00000152u,
+    0x0000001fu,
+    0x000300f7u,
+    0x00000155u,
+    0x00000000u,
+    0x000400fau,
+    0x00000153u,
+    0x00000154u,
+    0x00000155u,
+    0x000200f8u,
+    0x00000154u,
+    0x0004003du,
+    0x00000007u,
+    0x00000159u,
+    0x00000158u,
+    0x0008004fu,
+    0x00000024u,
+    0x0000015au,
+    0x00000159u,
+    0x00000159u,
+    0x00000000u,
+    0x00000001u,
+    0x00000002u,
+    0x0004003du,
+    0x00000024u,
+    0x0000015bu,
+    0x0000014du,
+    0x0004003du,
+    0x00000024u,
+    0x0000015cu,
+    0x0000014du,
+    0x0004003du,
+    0x00000007u,
+    0x0000015du,
+    0x00000158u,
+    0x0008004fu,
+    0x00000024u,
+    0x0000015eu,
+    0x0000015du,
+    0x0000015du,
+    0x00000000u,
+    0x00000001u,
+    0x00000002u,
+    0x00050094u,
+    0x00000006u,
+    0x0000015fu,
+    0x0000015cu,
+    0x0000015eu,
+    0x0005008eu,
+    0x00000024u,
+    0x00000160u,
+    0x0000015bu,
+    0x0000015fu,
+    0x00050083u,
+    0x00000024u,
+    0x00000161u,
+    0x0000015au,
+    0x00000160u,
+    0x0006000cu,
+    0x00000024u,
+    0x00000162u,
+    0x00000001u,
+    0x00000045u,
+    0x00000161u,
+    0x0003003eu,
+    0x00000156u,
+    0x00000162u,
+    0x0004003du,
+    0x00000024u,
+    0x00000164u,
+    0x0000014du,
+    0x0004003du,
+    0x00000024u,
+    0x00000165u,
+    0x00000156u,
+    0x0007000cu,
+    0x00000024u,
+    0x00000166u,
+    0x00000001u,
+    0x00000044u,
+    0x00000164u,
+    0x00000165u,
+    0x00050041u,
+    0x00000072u,
+    0x00000167u,
+    0x00000158u,
+    0x00000031u,
+    0x0004003du,
+    0x00000006u,
+    0x00000168u,
+    0x00000167u,
+    0x0005008eu,
+    0x00000024u,
+    0x00000169u,
+    0x00000166u,
+    0x00000168u,
+    0x0003003eu,
+    0x00000163u,
+    0x00000169u,
+    0x0004003du,
+    0x0000000bu,
+    0x0000016bu,
+    0x000000e1u,
+    0x0004003du,
+    0x0000000fu,
+    0x0000016cu,
+    0x00000011u,
+    0x00050057u,
+    0x00000007u,
+    0x0000016du,
+    0x0000016bu,
+    0x0000016cu,
+    0x0008004fu,
+    0x00000024u,
+    0x0000016eu,
+    0x0000016du,
+    0x0000016du,
+    0x00000000u,
+    0x00000001u,
+    0x00000002u,
+    0x0005008eu,
+    0x00000024u,
+    0x0000016fu,
+    0x0000016eu,
+    0x000000e8u,
+    0x00060050u,
+    0x00000024u,
+    0x00000170u,
+    0x00000075u,
+    0x00000075u,
+    0x00000075u,
+    0x00050083u,
+    0x00000024u,
+    0x00000171u,
+    0x0000016fu,
+    0x00000170u,
+    0x0003003eu,
+    0x0000016au,
+    0x00000171u,
+    0x0004003du,
+    0x00000024u,
+    0x00000172u,
+    0x00000156u,
+    0x0004003du,
+    0x00000024u,
+    0x00000173u,
+    0x00000163u,
+    0x0004003du,
+    0x00000024u,
+    0x00000174u,
+    0x0000014du,
+    0x00050051u,
+    0x00000006u,
+    0x00000176u,
+    0x00000172u,
+    0x00000000u,
+    0x00050051u,
+    0x00000006u,
+    0x00000177u,
+    0x00000172u,
+    0x00000001u,
+    0x00050051u,
+    0x00000006u,
+    0x00000178u,
+    0x00000172u,
+    0x00000002u,
+    0x00050051u,
+    0x00000006u,
+    0x00000179u,
+    0x00000173u,
+    0x00000000u,
+    0x00050051u,
+    0x00000006u,
+    0x0000017au,
+    0x00000173u,
+    0x00000001u,
+    0x00050051u,
+    0x00000006u,
+    0x0000017bu,
+    0x00000173u,
+    0x00000002u,
+    0x00050051u,
+    0x00000006u,
+    0x0000017cu,
+    0x00000174u,
+    0x00000000u,
+    0x00050051u,
+    0x00000006u,
+    0x0000017du,
+    0x00000174u,
+    0x00000001u,
+    0x00050051u,
+    0x00000006u,
+    0x0000017eu,
+    0x00000174u,
+    0x00000002u,
+    0x00060050u,
+    0x00000024u,
+    0x0000017fu,
+    0x00000176u,
+    0x00000177u,
+    0x00000178u,
+    0x00060050u,
+    0x00000024u,
+    0x00000180u,
+    0x00000179u,
+    0x0000017au,
+    0x0000017bu,
+    0x00060050u,
+    0x00000024u,
+    0x00000181u,
+    0x0000017cu,
+    0x0000017du,
+    0x0000017eu,
+    0x00060050u,
+    0x00000175u,
+    0x00000182u,
+    0x0000017fu,
+    0x00000180u,
+    0x00000181u,
+    0x0004003du,
+    0x00000024u,
+    0x00000183u,
+    0x0000016au,
+    0x00050091u,
+    0x00000024u,
+    0x00000184u,
+    0x00000182u,
+    0x00000183u,
+    0x0006000cu,
+    0x00000024u,
+    0x00000185u,
+    0x00000001u,
+    0x00000045u,
+    0x00000184u,
+    0x0003003eu,
+    0x0000014du,
+    0x00000185u,
+    0x000200f9u,
+    0x00000155u,
+    0x000200f8u,
+    0x00000155u,
+    0x0003003eu,
+    0x00000186u,
+    0x0000018au,
+    0x0004003du,
+    0x00000024u,
+    0x0000018cu,
+    0x0000014du,
+    0x0004003du,
+    0x00000024u,
+    0x0000018du,
+    0x00000186u,
+    0x00050094u,
+    0x00000006u,
+    0x0000018eu,
+    0x0000018cu,
+    0x0000018du,
+    0x0007000cu,
+    0x00000006u,
+    0x0000018fu,
+    0x00000001u,
+    0x00000028u,
+    0x0000018eu,
+    0x00000027u,
+    0x0003003eu,
+    0x0000018bu,
+    0x0000018fu,
+    0x0004003du,
+    0x00000006u,
+    0x00000192u,
+    0x0000018bu,
+    0x00050085u,
+    0x00000006u,
+    0x00000193u,
+    0x00000191u,
+    0x00000192u,
+    0x00050081u,
+    0x00000006u,
+    0x00000194u,
+    0x00000190u,
+    0x00000193u,
+    0x0004003du,
+    0x00000007u,
+    0x00000195u,
+    0x00000083u,
+    0x0008004fu,
+    0x00000024u,
+    0x00000196u,
+    0x00000195u,
+    0x00000195u,
+    0x00000000u,
+    0x00000001u,
+    0x00000002u,
+    0x0005008eu,
+    0x00000024u,
+    0x00000197u,
+    0x00000196u,
+    0x00000194u,
+    0x0004003du,
+    0x00000007u,
+    0x00000198u,
+    0x00000083u,
+    0x0009004fu,
+    0x00000007u,
+    0x00000199u,
+    0x00000198u,
+    0x00000197u,
+    0x00000004u,
+    0x00000005u,
+    0x00000006u,
+    0x00000003u,
+    0x0003003eu,
+    0x00000083u,
+    0x00000199u,
+    0x0004003du,
+    0x00000020u,
+    0x0000019au,
+    0x0000005au,
+    0x000300f7u,
+    0x0000019cu,
+    0x00000000u,
+    0x000400fau,
+    0x0000019au,
+    0x0000019bu,
+    0x0000019cu,
+    0x000200f8u,
+    0x0000019bu,
+    0x00050041u,
+    0x00000072u,
+    0x0000019eu,
+    0x0000019du,
+    0x00000031u,
+    0x0004003du,
+    0x00000006u,
+    0x0000019fu,
+    0x0000019eu,
+    0x000500bau,
+    0x00000020u,
+    0x000001a0u,
+    0x0000019fu,
+    0x00000027u,
+    0x000200f9u,
+    0x0000019cu,
+    0x000200f8u,
+    0x0000019cu,
+    0x000700f5u,
+    0x00000020u,
+    0x000001a1u,
+    0x0000019au,
+    0x00000155u,
+    0x000001a0u,
+    0x0000019bu,
+    0x000300f7u,
+    0x000001a3u,
+    0x00000000u,
+    0x000400fau,
+    0x000001a1u,
+    0x000001a2u,
+    0x000001a3u,
+    0x000200f8u,
+    0x000001a2u,
+    0x0004003du,
+    0x00000007u,
+    0x000001a5u,
+    0x0000019du,
+    0x0008004fu,
+    0x00000024u,
+    0x000001a6u,
+    0x000001a5u,
+    0x000001a5u,
+    0x00000000u,
+    0x00000001u,
+    0x00000002u,
+    0x00050041u,
+    0x00000072u,
+    0x000001a7u,
+    0x0000019du,
+    0x00000031u,
+    0x0004003du,
+    0x00000006u,
+    0x000001a8u,
+    0x000001a7u,
+    0x00060050u,
+    0x00000024u,
+    0x000001a9u,
+    0x000001a8u,
+    0x000001a8u,
+    0x000001a8u,
+    0x00050088u,
+    0x00000024u,
+    0x000001aau,
+    0x000001a6u,
+    0x000001a9u,
+    0x0003003eu,
+    0x000001a4u,
+    0x000001aau,
+    0x0004003du,
+    0x00000024u,
+    0x000001adu,
+    0x000001a4u,
+    0x0007004fu,
+    0x0000000fu,
+    0x000001aeu,
+    0x000001adu,
+    0x000001adu,
+    0x00000000u,
+    0x00000001u,
+    0x0005008eu,
+    0x0000000fu,
+    0x000001afu,
+    0x000001aeu,
+    0x0000001fu,
+    0x00050050u,
+    0x0000000fu,
+    0x000001b0u,
+    0x0000001fu,
+    0x0000001fu,
+    0x00050081u,
+    0x0000000fu,
+    0x000001b1u,
+    0x000001afu,
+    0x000001b0u,
+    0x0003003eu,
+    0x000001acu,
+    0x000001b1u,
+    0x0003003eu,
+    0x000001b2u,
+    0x00000075u,
+    0x0004003du,
+    0x0000000fu,
+    0x000001b3u,
+    0x000001acu,
+    0x000500beu,
+    0x000001b5u,
+    0x000001b6u,
+    0x000001b3u,
+    0x000001b4u,
+    0x0004009bu,
+    0x00000020u,
+    0x000001b7u,
+    0x000001b6u,
+    0x000300f7u,
+    0x000001b9u,
+    0x00000000u,
+    0x000400fau,
+    0x000001b7u,
+    0x000001b8u,
+    0x000001b9u,
+    0x000200f8u,
+    0x000001b8u,
+    0x0004003du,
+    0x0000000fu,
+    0x000001bau,
+    0x000001acu,
+    0x000500bcu,
+    0x000001b5u,
+    0x000001bcu,
+    0x000001bau,
+    0x000001bbu,
+    0x0004009bu,
+    0x00000020u,
+    0x000001bdu,
+    0x000001bcu,
+    0x000200f9u,
+    0x000001b9u,
+    0x000200f8u,
+    0x000001b9u,
+    0x000700f5u,
+    0x00000020u,
+    0x000001beu,
+    0x000001b7u,
+    0x000001a2u,
+    0x000001bdu,
+    0x000001b8u,
+    0x000300f7u,
+    0x000001c0u,
+    0x00000000u,
+    0x000400fau,
+    0x000001beu,
+    0x000001bfu,
+    0x000001c0u,
+    0x000200f8u,
+    0x000001bfu,
+    0x00050041u,
+    0x0000006fu,
+    0x000001c1u,
+    0x000001a4u,
+    0x0000001bu,
+    0x0004003du,
+    0x00000006u,
+    0x000001c2u,
+    0x000001c1u,
+    0x000500beu,
+    0x00000020u,
+    0x000001c3u,
+    0x000001c2u,
+    0x00000027u,
+    0x000200f9u,
+    0x000001c0u,
+    0x000200f8u,
+    0x000001c0u,
+    0x000700f5u,
+    0x00000020u,
+    0x000001c4u,
+    0x000001beu,
+    0x000001b9u,
+    0x000001c3u,
+    0x000001bfu,
+    0x000300f7u,
+    0x000001c6u,
+    0x00000000u,
+    0x000400fau,
+    0x000001c4u,
+    0x000001c5u,
+    0x000001c6u,
+    0x000200f8u,
+    0x000001c5u,
+    0x00050041u,
+    0x0000006fu,
+    0x000001c7u,
+    0x000001a4u,
+    0x0000001bu,
+    0x0004003du,
+    0x00000006u,
+    0x000001c8u,
+    0x000001c7u,
+    0x000500bcu,
+    0x00000020u,
+    0x000001c9u,
+    0x000001c8u,
+    0x00000075u,
+    0x000200f9u,
+    0x000001c6u,
+    0x000200f8u,
+    0x000001c6u,
+    0x000700f5u,
+    0x00000020u,
+    0x000001cau,
+    0x000001c4u,
+    0x000001c0u,
+    0x000001c9u,
+    0x000001c5u,
+    0x000300f7u,
+    0x000001ccu,
+    0x00000000u,
+    0x000400fau,
+    0x000001cau,
+    0x000001cbu,
+    0x000001ccu,
+    0x000200f8u,
+    0x000001cbu,
+    0x0004003du,
+    0x000001cfu,
+    0x000001d2u,
+    0x000001d1u,
+    0x00040064u,
+    0x000001ceu,
+    0x000001d3u,
+    0x000001d2u,
+    0x00050067u,
+    0x000001d4u,
+    0x000001d5u,
+    0x000001d3u,
+    0x0000003au,
+    0x0004006fu,
+    0x0000000fu,
+    0x000001d6u,
+    0x000001d5u,
+    0x00050050u,
+    0x0000000fu,
+    0x000001d7u,
+    0x00000075u,
+    0x00000075u,
+    0x00050088u,
+    0x0000000fu,
+    0x000001d8u,
+    0x000001d7u,
+    0x000001d6u,
+    0x0003003eu,
+    0x000001cdu,
+    0x000001d8u,
+    0x0003003eu,
+    0x000001b2u,
+    0x00000027u,
+    0x0003003eu,
+    0x000001d9u,
+    0x000001dau,
+    0x000200f9u,
+    0x000001dbu,
+    0x000200f8u,
+    0x000001dbu,
+    0x000400f6u,
+    0x000001ddu,
+    0x000001deu,
+    0x00000000u,
+    0x000200f9u,
+    0x000001dfu,
+    0x000200f8u,
+    0x000001dfu,
+    0x0004003du,
+    0x00000018u,
+    0x000001e0u,
+    0x000001d9u,
+    0x000500b3u,
+    0x00000020u,
+    0x000001e2u,
+    0x000001e0u,
+    0x000001e1u,
+    0x000400fau,
+    0x000001e2u,
+    0x000001dcu,
+    0x000001ddu,
+    0x000200f8u,
+    0x000001dcu,
+    0x0003003eu,
+    0x000001e3u,
+    0x000001dau,
+    0x000200f9u,
+    0x000001e4u,
+    0x000200f8u,
+    0x000001e4u,
+    0x000400f6u,
+    0x000001e6u,
+    0x000001e7u,
+    0x00000000u,
+    0x000200f9u,
+    0x000001e8u,
+    0x000200f8u,
+    0x000001e8u,
+    0x0004003du,
+    0x00000018u,
+    0x000001e9u,
+    0x000001e3u,
+    0x000500b3u,
+    0x00000020u,
+    0x000001eau,
+    0x000001e9u,
+    0x000001e1u,
+    0x000400fau,
+    0x000001eau,
+    0x000001e5u,
+    0x000001e6u,
+    0x000200f8u,
+    0x000001e5u,
+    0x0004003du,
+    0x000001cfu,
+    0x000001ebu,
+    0x000001d1u,
+    0x0004003du,
+    0x0000000fu,
+    0x000001ecu,
+    0x000001acu,
+    0x0004003du,
+    0x00000018u,
+    0x000001edu,
+    0x000001e3u,
+    0x0004006fu,
+    0x00000006u,
+    0x000001eeu,
+    0x000001edu,
+    0x0004003du,
+    0x00000018u,
+    0x000001efu,
+    0x000001d9u,
+    0x0004006fu,
+    0x00000006u,
+    0x000001f0u,
+    0x000001efu,
+    0x00050050u,
+    0x0000000fu,
+    0x000001f1u,
+    0x000001eeu,
+    0x000001f0u,
+    0x0004003du,
+    0x0000000fu,
+    0x000001f2u,
+    0x000001cdu,
+    0x00050085u,
+    0x0000000fu,
+    0x000001f3u,
+    0x000001f1u,
+    0x000001f2u,
+    0x00050081u,
+    0x0000000fu,
+    0x000001f4u,
+    0x000001ecu,
+    0x000001f3u,
+    0x00050041u,
+    0x0000006fu,
+    0x000001f5u,
+    0x000001a4u,
+    0x0000001bu,
+    0x0004003du,
+    0x00000006u,
+    0x000001f6u,
+    0x000001f5u,
+    0x00050083u,
+    0x00000006u,
+    0x000001f8u,
+    0x000001f6u,
+    0x000001f7u,
+    0x00050051u,
+    0x00000006u,
+    0x000001f9u,
+    0x000001f4u,
+    0x00000000u,
+    0x00050051u,
+    0x00000006u,
+    0x000001fau,
+    0x000001f4u,
+    0x00000001u,
+    0x00060050u,
+    0x00000024u,
+    0x000001fbu,
+    0x000001f9u,
+    0x000001fau,
+    0x000001f8u,
+    0x00050051u,
+    0x00000006u,
+    0x000001fcu,
+    0x000001fbu,
+    0x00000002u,
+    0x00060059u,
+    0x00000006u,
+    0x000001fdu,
+    0x000001ebu,
+    0x000001fbu,
+    0x000001fcu,
+    0x0004003du,
+    0x00000006u,
+    0x000001feu,
+    0x000001b2u,
+    0x00050081u,
+    0x00000006u,
+    0x000001ffu,
+    0x000001feu,
+    0x000001fdu,
+    0x0003003eu,
+    0x000001b2u,
+    0x000001ffu,
+    0x000200f9u,
+    0x000001e7u,
+    0x000200f8u,
+    0x000001e7u,
+    0x0004003du,
+    0x00000018u,
+    0x00000200u,
+    0x000001e3u,
+    0x00050080u,
+    0x00000018u,
+    0x00000201u,
+    0x00000200u,
+    0x000001e1u,
+    0x0003003eu,
+    0x000001e3u,
+    0x00000201u,
+    0x000200f9u,
+    0x000001e4u,
+    0x000200f8u,
+    0x000001e6u,
+    0x000200f9u,
+    0x000001deu,
+    0x000200f8u,
+    0x000001deu,
+    0x0004003du,
+    0x00000018u,
+    0x00000202u,
+    0x000001d9u,
+    0x00050080u,
+    0x00000018u,
+    0x00000203u,
+    0x00000202u,
+    0x000001e1u,
+    0x0003003eu,
+    0x000001d9u,
+    0x00000203u,
+    0x000200f9u,
+    0x000001dbu,
+    0x000200f8u,
+    0x000001ddu,
+    0x0004003du,
+    0x00000006u,
+    0x00000205u,
+    0x000001b2u,
+    0x00050088u,
+    0x00000006u,
+    0x00000206u,
+    0x00000205u,
+    0x00000204u,
+    0x0003003eu,
+    0x000001b2u,
+    0x00000206u,
+    0x000200f9u,
+    0x000001ccu,
+    0x000200f8u,
+    0x000001ccu,
+    0x0004003du,
+    0x00000006u,
+    0x00000207u,
+    0x000001b2u,
+    0x0008000cu,
+    0x00000006u,
+    0x00000208u,
+    0x00000001u,
+    0x0000002eu,
+    0x00000080u,
+    0x00000075u,
+    0x00000207u,
+    0x0004003du,
+    0x00000007u,
+    0x00000209u,
+    0x00000083u,
+    0x0008004fu,
+    0x00000024u,
+    0x0000020au,
+    0x00000209u,
+    0x00000209u,
+    0x00000000u,
+    0x00000001u,
+    0x00000002u,
+    0x0005008eu,
+    0x00000024u,
+    0x0000020bu,
+    0x0000020au,
+    0x00000208u,
+    0x0004003du,
+    0x00000007u,
+    0x0000020cu,
+    0x00000083u,
+    0x0009004fu,
+    0x00000007u,
+    0x0000020du,
+    0x0000020cu,
+    0x0000020bu,
+    0x00000004u,
+    0x00000005u,
+    0x00000006u,
+    0x00000003u,
+    0x0003003eu,
+    0x00000083u,
+    0x0000020du,
+    0x000200f9u,
+    0x000001a3u,
+    0x000200f8u,
+    0x000001a3u,
+    0x00060041u,
+    0x0000001cu,
+    0x0000020eu,
+    0x00000017u,
+    0x00000019u,
+    0x00000031u,
+    0x0004003du,
+    0x00000006u,
+    0x0000020fu,
+    0x0000020eu,
+    0x00050088u,
+    0x00000006u,
+    0x00000210u,
+    0x0000020fu,
+    0x000000e8u,
+    0x0006000cu,
+    0x00000006u,
+    0x00000211u,
+    0x00000001u,
+    0x00000008u,
+    0x00000210u,
+    0x0005008du,
+    0x00000006u,
+    0x00000212u,
+    0x00000211u,
+    0x000000e8u,
+    0x000500bau,
+    0x00000020u,
+    0x00000213u,
+    0x00000212u,
+    0x0000001fu,
+    0x000300f7u,
+    0x00000215u,
+    0x00000000u,
+    0x000400fau,
+    0x00000213u,
+    0x00000214u,
+    0x00000215u,
+    0x000200f8u,
+    0x00000214u,
+    0x0004003du,
+    0x00000024u,
+    0x00000217u,
+    0x00000186u,
+    0x00050081u,
+    0x00000024u,
+    0x00000219u,
+    0x00000217u,
+    0x00000218u,
+    0x0006000cu,
+    0x00000024u,
+    0x0000021au,
+    0x00000001u,
+    0x00000045u,
+    0x00000219u,
+    0x0003003eu,
+    0x00000216u,
+    0x0000021au,
+    0x0004003du,
+    0x0000000bu,
+    0x0000021du,
+    0x0000021cu,
+    0x0004003du,
+    0x0000000fu,
+    0x0000021eu,
+    0x00000011u,
+    0x00050057u,
+    0x00000007u,
+    0x0000021fu,
+    0x0000021du,
+    0x0000021eu,
+    0x0008004fu,
+    0x00000024u,
+    0x00000220u,
+    0x0000021fu,
+    0x0000021fu,
+    0x00000000u,
+    0x00000001u,
+    0x00000002u,
+    0x0003003eu,
+    0x0000021bu,
+    0x00000220u,
+    0x0004003du,
+    0x00000018u,
+    0x00000222u,
+    0x00000030u,
+    0x000500c7u,
+    0x00000018u,
+    0x00000223u,
+    0x00000222u,
+    0x00000019u,
+    0x000500abu,
+    0x00000020u,
+    0x00000224u,
+    0x00000223u,
+    0x0000003au,
+    0x0003003eu,
+    0x00000221u,
+    0x00000224u,
+    0x00060041u,
+    0x0000001cu,
+    0x00000226u,
+    0x00000017u,
+    0x000001e1u,
+    0x00000031u,
+    0x0004003du,
+    0x00000006u,
+    0x00000227u,
+    0x00000226u,
+    0x0003003eu,
+    0x00000225u,
+    0x00000227u,
+    0x00060041u,
+    0x0000001cu,
+    0x0000022au,
+    0x00000017u,
+    0x00000229u,
+    0x00000031u,
+    0x0004003du,
+    0x00000006u,
+    0x0000022bu,
+    0x0000022au,
+    0x0003003eu,
+    0x00000228u,
+    0x0000022bu,
+    0x0004003du,
+    0x00000020u,
+    0x0000022du,
+    0x00000221u,
+    0x000300f7u,
+    0x00000230u,
+    0x00000000u,
+    0x000400fau,
+    0x0000022du,
+    0x0000022fu,
+    0x00000237u,
+    0x000200f8u,
+    0x0000022fu,
+    0x00050041u,
+    0x0000006fu,
+    0x00000231u,
+    0x0000021bu,
+    0x00000071u,
+    0x0004003du,
+    0x00000006u,
+    0x00000232u,
+    0x00000231u,
+    0x0004003du,
+    0x00000006u,
+    0x00000233u,
+    0x00000228u,
+    0x00050085u,
+    0x00000006u,
+    0x00000234u,
+    0x00000232u,
+    0x00000233u,
+    0x0008000cu,
+    0x00000006u,
+    0x00000236u,
+    0x00000001u,
+    0x0000002bu,
+    0x00000234u,
+    0x00000235u,
+    0x00000075u,
+    0x0003003eu,
+    0x0000022eu,
+    0x00000236u,
+    0x000200f9u,
+    0x00000230u,
+    0x000200f8u,
+    0x00000237u,
+    0x0003003eu,
+    0x0000022eu,
+    0x0000001fu,
+    0x000200f9u,
+    0x00000230u,
+    0x000200f8u,
+    0x00000230u,
+    0x0004003du,
+    0x00000006u,
+    0x00000238u,
+    0x0000022eu,
+    0x0003003eu,
+    0x0000022cu,
+    0x00000238u,
+    0x0004003du,
+    0x00000020u,
+    0x0000023au,
+    0x00000221u,
+    0x000300f7u,
+    0x0000023du,
+    0x00000000u,
+    0x000400fau,
+    0x0000023au,
+    0x0000023cu,
+    0x00000242u,
+    0x000200f8u,
+    0x0000023cu,
+    0x0004003du,
+    0x00000006u,
+    0x00000240u,
+    0x0000022cu,
+    0x0008000cu,
+    0x00000006u,
+    0x00000241u,
+    0x00000001u,
+    0x0000002eu,
+    0x0000023eu,
+    0x0000023fu,
+    0x00000240u,
+    0x0003003eu,
+    0x0000023bu,
+    0x00000241u,
+    0x000200f9u,
+    0x0000023du,
+    0x000200f8u,
+    0x00000242u,
+    0x0003003eu,
+    0x0000023bu,
+    0x00000243u,
+    0x000200f9u,
+    0x0000023du,
+    0x000200f8u,
+    0x0000023du,
+    0x0004003du,
+    0x00000006u,
+    0x00000244u,
+    0x0000023bu,
+    0x0003003eu,
+    0x00000239u,
+    0x00000244u,
+    0x0004003du,
+    0x00000024u,
+    0x00000246u,
+    0x0000014du,
+    0x0004003du,
+    0x00000024u,
+    0x00000247u,
+    0x00000216u,
+    0x00050094u,
+    0x00000006u,
+    0x00000248u,
+    0x00000246u,
+    0x00000247u,
+    0x0007000cu,
+    0x00000006u,
+    0x00000249u,
+    0x00000001u,
+    0x00000028u,
+    0x00000248u,
+    0x00000027u,
+    0x0004003du,
+    0x00000006u,
+    0x0000024au,
+    0x00000239u,
+    0x0007000cu,
+    0x00000006u,
+    0x0000024bu,
+    0x00000001u,
+    0x0000001au,
+    0x00000249u,
+    0x0000024au,
+    0x0003003eu,
+    0x00000245u,
+    0x0000024bu,
+    0x0004003du,
+    0x00000020u,
+    0x0000024du,
+    0x00000221u,
+    0x000300f7u,
+    0x00000250u,
+    0x00000000u,
+    0x000400fau,
+    0x0000024du,
+    0x0000024fu,
+    0x00000255u,
+    0x000200f8u,
+    0x0000024fu,
+    0x00050041u,
+    0x0000006fu,
+    0x00000251u,
+    0x0000021bu,
+    0x0000001bu,
+    0x0004003du,
+    0x00000006u,
+    0x00000252u,
+    0x00000251u,
+    0x0004003du,
+    0x00000006u,
+    0x00000253u,
+    0x00000225u,
+    0x00050085u,
+    0x00000006u,
+    0x00000254u,
+    0x00000252u,
+    0x00000253u,
+    0x0003003eu,
+    0x0000024eu,
+    0x00000254u,
+    0x000200f9u,
+    0x00000250u,
+    0x000200f8u,
+    0x00000255u,
+    0x0003003eu,
+    0x0000024eu,
+    0x00000027u,
+    0x000200f9u,
+    0x00000250u,
+    0x000200f8u,
+    0x00000250u,
+    0x0004003du,
+    0x00000006u,
+    0x00000256u,
+    0x0000024eu,
+    0x0003003eu,
+    0x0000024cu,
+    0x00000256u,
+    0x0004003du,
+    0x00000020u,
+    0x00000258u,
+    0x00000221u,
+    0x000300f7u,
+    0x0000025bu,
+    0x00000000u,
+    0x000400fau,
+    0x00000258u,
+    0x0000025au,
+    0x0000025eu,
+    0x000200f8u,
+    0x0000025au,
+    0x0004003du,
+    0x00000006u,
+    0x0000025cu,
+    0x0000024cu,
+    0x0008000cu,
+    0x00000006u,
+    0x0000025du,
+    0x00000001u,
+    0x0000002eu,
+    0x00000235u,
+    0x00000075u,
+    0x0000025cu,
+    0x0003003eu,
+    0x00000259u,
+    0x0000025du,
+    0x000200f9u,
+    0x0000025bu,
+    0x000200f8u,
+    0x0000025eu,
+    0x00050041u,
+    0x0000006fu,
+    0x0000025fu,
+    0x0000021bu,
+    0x00000093u,
+    0x0004003du,
+    0x00000006u,
+    0x00000260u,
+    0x0000025fu,
+    0x0003003eu,
+    0x00000259u,
+    0x00000260u,
+    0x000200f9u,
+    0x0000025bu,
+    0x000200f8u,
+    0x0000025bu,
+    0x0004003du,
+    0x00000006u,
+    0x00000261u,
+    0x00000259u,
+    0x0003003eu,
+    0x00000257u,
+    0x00000261u,
+    0x0004003du,
+    0x00000006u,
+    0x00000262u,
+    0x00000245u,
+    0x0004003du,
+    0x00000006u,
+    0x00000263u,
+    0x00000257u,
+    0x00050085u,
+    0x00000006u,
+    0x00000264u,
+    0x00000262u,
+    0x00000263u,
+    0x00050085u,
+    0x00000006u,
+    0x00000266u,
+    0x00000264u,
+    0x00000265u,
+    0x00060050u,
+    0x00000024u,
+    0x00000267u,
+    0x00000266u,
+    0x00000266u,
+    0x00000266u,
+    0x0004003du,
+    0x00000007u,
+    0x00000268u,
+    0x00000083u,
+    0x0008004fu,
+    0x00000024u,
+    0x00000269u,
+    0x00000268u,
+    0x00000268u,
+    0x00000000u,
+    0x00000001u,
+    0x00000002u,
+    0x00050081u,
+    0x00000024u,
+    0x0000026au,
+    0x00000269u,
+    0x00000267u,
+    0x0004003du,
+    0x00000007u,
+    0x0000026bu,
+    0x00000083u,
+    0x0009004fu,
+    0x00000007u,
+    0x0000026cu,
+    0x0000026bu,
+    0x0000026au,
+    0x00000004u,
+    0x00000005u,
+    0x00000006u,
+    0x00000003u,
+    0x0003003eu,
+    0x00000083u,
+    0x0000026cu,
+    0x0004003du,
+    0x00000020u,
+    0x0000026du,
+    0x00000221u,
+    0x000300f7u,
+    0x0000026fu,
+    0x00000000u,
+    0x000400fau,
+    0x0000026du,
+    0x0000026eu,
+    0x0000026fu,
+    0x000200f8u,
+    0x0000026eu,
+    0x00050041u,
+    0x0000006fu,
+    0x00000271u,
+    0x0000014du,
+    0x0000001bu,
+    0x0004003du,
+    0x00000006u,
+    0x00000272u,
+    0x00000271u,
+    0x0007000cu,
+    0x00000006u,
+    0x00000273u,
+    0x00000001u,
+    0x00000028u,
+    0x00000272u,
+    0x00000027u,
+    0x0003003eu,
+    0x00000270u,
+    0x00000273u,
+    0x0004003du,
+    0x00000006u,
+    0x00000275u,
+    0x00000270u,
+    0x00050083u,
+    0x00000006u,
+    0x00000276u,
+    0x00000075u,
+    0x00000275u,
+    0x0007000cu,
+    0x00000006u,
+    0x00000278u,
+    0x00000001u,
+    0x0000001au,
+    0x00000276u,
+    0x00000277u,
+    0x0003003eu,
+    0x00000274u,
+    0x00000278u,
+    0x0004003du,
+    0x00000007u,
+    0x0000027bu,
+    0x00000009u,
+    0x0008004fu,
+    0x00000024u,
+    0x0000027cu,
+    0x0000027bu,
+    0x0000027bu,
+    0x00000000u,
+    0x00000001u,
+    0x00000002u,
+    0x0004003du,
+    0x00000006u,
+    0x0000027du,
+    0x0000024cu,
+    0x00060050u,
+    0x00000024u,
+    0x0000027eu,
+    0x0000027du,
+    0x0000027du,
+    0x0000027du,
+    0x0008000cu,
+    0x00000024u,
+    0x0000027fu,
+    0x00000001u,
+    0x0000002eu,
+    0x0000027au,
+    0x0000027cu,
+    0x0000027eu,
+    0x0003003eu,
+    0x00000279u,
+    0x0000027fu,
+    0x0004003du,
+    0x00000024u,
+    0x00000281u,
+    0x00000279u,
+    0x0004003du,
+    0x00000024u,
+    0x00000282u,
+    0x00000279u,
+    0x00060050u,
+    0x00000024u,
+    0x00000283u,
+    0x00000075u,
+    0x00000075u,
+    0x00000075u,
+    0x00050083u,
+    0x00000024u,
+    0x00000284u,
+    0x00000283u,
+    0x00000282u,
+    0x0004003du,
+    0x00000006u,
+    0x00000285u,
+    0x00000274u,
+    0x0005008eu,
+    0x00000024u,
+    0x00000286u,
+    0x00000284u,
+    0x00000285u,
+    0x00050081u,
+    0x00000024u,
+    0x00000287u,
+    0x00000281u,
+    0x00000286u,
+    0x0003003eu,
+    0x00000280u,
+    0x00000287u,
+    0x0004003du,
+    0x00000020u,
+    0x00000288u,
+    0x0000005fu,
+    0x000300f7u,
+    0x0000028au,
+    0x00000000u,
+    0x000400fau,
+    0x00000288u,
+    0x00000289u,
+    0x00000347u,
+    0x000200f8u,
+    0x00000289u,
+    0x0004003du,
+    0x00000024u,
+    0x0000028eu,
+    0x0000014du,
+    0x0007000cu,
+    0x00000024u,
+    0x0000028fu,
+    0x00000001u,
+    0x00000047u,
+    0x0000028du,
+    0x0000028eu,
+    0x0003003eu,
+    0x0000028bu,
+    0x0000028fu,
+    0x0003003eu,
+    0x00000290u,
+    0x00000028u,
+    0x0003003eu,
+    0x00000291u,
+    0x00000028u,
+    0x0003003eu,
+    0x00000292u,
+    0x00000027u,
+    0x0003003eu,
+    0x00000293u,
+    0x0000003au,
+    0x000200f9u,
+    0x00000294u,
+    0x000200f8u,
+    0x00000294u,
+    0x000400f6u,
+    0x00000296u,
+    0x00000297u,
+    0x00000000u,
+    0x000200f9u,
+    0x00000298u,
+    0x000200f8u,
+    0x00000298u,
+    0x0004003du,
+    0x00000018u,
+    0x00000299u,
+    0x00000293u,
+    0x00050044u,
+    0x0000001au,
+    0x000002a0u,
+    0x0000029fu,
+    0x00000000u,
+    0x0004007cu,
+    0x00000018u,
+    0x000002a1u,
+    0x000002a0u,
+    0x000500b1u,
+    0x00000020u,
+    0x000002a2u,
+    0x00000299u,
+    0x000002a1u,
+    0x000400fau,
+    0x000002a2u,
+    0x00000295u,
+    0x00000296u,
+    0x000200f8u,
+    0x00000295u,
+    0x0004003du,
+    0x00000018u,
+    0x000002a6u,
+    0x00000293u,
+    0x00060041u,
+    0x000002a7u,
+    0x000002a8u,
+    0x0000029fu,
+    0x0000003au,
+    0x000002a6u,
+    0x0004003du,
+    0x0000029bu,
+    0x000002a9u,
+    0x000002a8u,
+    0x00050051u,
+    0x00000014u,
+    0x000002aau,
+    0x000002a9u,
+    0x00000000u,
+    0x00050041u,
+    0x000002abu,
+    0x000002acu,
+    0x000002a5u,
+    0x0000003au,
+    0x0003003eu,
+    0x000002acu,
+    0x000002aau,
+    0x00050051u,
+    0x00000007u,
+    0x000002adu,
+    0x000002a9u,
+    0x00000001u,
+    0x00050041u,
+    0x00000008u,
+    0x000002aeu,
+    0x000002a5u,
+    0x000001e1u,
+    0x0003003eu,
+    0x000002aeu,
+    0x000002adu,
+    0x00050051u,
+    0x00000007u,
+    0x000002afu,
+    0x000002a9u,
+    0x00000002u,
+    0x00050041u,
+    0x00000008u,
+    0x000002b0u,
+    0x000002a5u,
+    0x00000229u,
+    0x0003003eu,
+    0x000002b0u,
+    0x000002afu,
+    0x00050051u,
+    0x0000029au,
+    0x000002b1u,
+    0x000002a9u,
+    0x00000003u,
+    0x00050041u,
+    0x000002b3u,
+    0x000002b4u,
+    0x000002a5u,
+    0x000002b2u,
+    0x0003003eu,
+    0x000002b4u,
+    0x000002b1u,
+    0x00060041u,
+    0x0000002fu,
+    0x000002b5u,
+    0x000002a5u,
+    0x000002b2u,
+    0x00000031u,
+    0x0004003du,
+    0x00000018u,
+    0x000002b6u,
+    0x000002b5u,
+    0x000500b1u,
+    0x00000020u,
+    0x000002b7u,
+    0x000002b6u,
+    0x0000003au,
+    0x000300f7u,
+    0x000002b9u,
+    0x00000000u,
+    0x000400fau,
+    0x000002b7u,
+    0x000002b8u,
+    0x000002d3u,
+    0x000200f8u,
+    0x000002b8u,
+    0x00050041u,
+    0x000002abu,
+    0x000002bbu,
+    0x000002a5u,
+    0x0000003au,
+    0x0004003du,
+    0x00000014u,
+    0x000002bcu,
+    0x000002bbu,
+    0x0004003du,
+    0x00000024u,
+    0x000002beu,
+    0x000002bdu,
+    0x00050051u,
+    0x00000006u,
+    0x000002bfu,
+    0x000002beu,
+    0x00000000u,
+    0x00050051u,
+    0x00000006u,
+    0x000002c0u,
+    0x000002beu,
+    0x00000001u,
+    0x00050051u,
+    0x00000006u,
+    0x000002c1u,
+    0x000002beu,
+    0x00000002u,
+    0x00070050u,
+    0x00000007u,
+    0x000002c2u,
+    0x000002bfu,
+    0x000002c0u,
+    0x000002c1u,
+    0x00000075u,
+    0x00050091u,
+    0x00000007u,
+    0x000002c3u,
+    0x000002bcu,
+    0x000002c2u,
+    0x0008004fu,
+    0x00000024u,
+    0x000002c4u,
+    0x000002c3u,
+    0x000002c3u,
+    0x00000000u,
+    0x00000001u,
+    0x00000002u,
+    0x0003003eu,
+    0x000002bau,
+    0x000002c4u,
+    0x00050041u,
+    0x0000006fu,
+    0x000002c6u,
+    0x000002bau,
+    0x00000093u,
+    0x0004003du,
+    0x00000006u,
+    0x000002c7u,
+    0x000002c6u,
+    0x0006000cu,
+    0x00000006u,
+    0x000002c8u,
+    0x00000001u,
+    0x00000004u,
+    0x000002c7u,
+    0x00050041u,
+    0x0000006fu,
+    0x000002c9u,
+    0x000002bau,
+    0x00000071u,
+    0x0004003du,
+    0x00000006u,
+    0x000002cau,
+    0x000002c9u,
+    0x0006000cu,
+    0x00000006u,
+    0x000002cbu,
+    0x00000001u,
+    0x00000004u,
+    0x000002cau,
+    0x0007000cu,
+    0x00000006u,
+    0x000002ccu,
+    0x00000001u,
+    0x00000028u,
+    0x000002c8u,
+    0x000002cbu,
+    0x00050041u,
+    0x0000006fu,
+    0x000002cdu,
+    0x000002bau,
+    0x0000001bu,
+    0x0004003du,
+    0x00000006u,
+    0x000002ceu,
+    0x000002cdu,
+    0x0006000cu,
+    0x00000006u,
+    0x000002cfu,
+    0x00000001u,
+    0x00000004u,
+    0x000002ceu,
+    0x0007000cu,
+    0x00000006u,
+    0x000002d0u,
+    0x00000001u,
+    0x00000028u,
+    0x000002ccu,
+    0x000002cfu,
+    0x00050083u,
+    0x00000006u,
+    0x000002d1u,
+    0x00000075u,
+    0x000002d0u,
+    0x0008000cu,
+    0x00000006u,
+    0x000002d2u,
+    0x00000001u,
+    0x0000002bu,
+    0x000002d1u,
+    0x00000027u,
+    0x00000075u,
+    0x0003003eu,
+    0x000002c5u,
+    0x000002d2u,
+    0x000200f9u,
+    0x000002b9u,
+    0x000200f8u,
+    0x000002d3u,
+    0x00060041u,
+    0x0000006fu,
+    0x000002d5u,
+    0x000002a5u,
+    0x000001e1u,
+    0x00000031u,
+    0x0004003du,
+    0x00000006u,
+    0x000002d6u,
+    0x000002d5u,
+    0x0007000cu,
+    0x00000006u,
+    0x000002d8u,
+    0x00000001u,
+    0x00000028u,
+    0x000002d6u,
+    0x000002d7u,
+    0x0003003eu,
+    0x000002d4u,
+    0x000002d8u,
+    0x0004003du,
+    0x00000024u,
+    0x000002d9u,
+    0x000002bdu,
+    0x00050041u,
+    0x00000008u,
+    0x000002dau,
+    0x000002a5u,
+    0x000001e1u,
+    0x0004003du,
+    0x00000007u,
+    0x000002dbu,
+    0x000002dau,
+    0x0008004fu,
+    0x00000024u,
+    0x000002dcu,
+    0x000002dbu,
+    0x000002dbu,
+    0x00000000u,
+    0x00000001u,
+    0x00000002u,
+    0x0007000cu,
+    0x00000006u,
+    0x000002ddu,
+    0x00000001u,
+    0x00000043u,
+    0x000002d9u,
+    0x000002dcu,
+    0x0004003du,
+    0x00000006u,
+    0x000002deu,
+    0x000002d4u,
+    0x00050088u,
+    0x00000006u,
+    0x000002dfu,
+    0x000002ddu,
+    0x000002deu,
+    0x00050083u,
+    0x00000006u,
+    0x000002e0u,
+    0x00000075u,
+    0x000002dfu,
+    0x0008000cu,
+    0x00000006u,
+    0x000002e1u,
+    0x00000001u,
+    0x0000002bu,
+    0x000002e0u,
+    0x00000027u,
+    0x00000075u,
+    0x0003003eu,
+    0x000002c5u,
+    0x000002e1u,
+    0x000200f9u,
+    0x000002b9u,
+    0x000200f8u,
+    0x000002b9u,
+    0x00060041u,
+    0x0000006fu,
+    0x000002e2u,
+    0x000002a5u,
+    0x00000229u,
+    0x0000001bu,
+    0x0004003du,
+    0x00000006u,
+    0x000002e3u,
+    0x000002e2u,
+    0x0008000cu,
+    0x00000006u,
+    0x000002e4u,
+    0x00000001u,
+    0x0000002bu,
+    0x000002e3u,
+    0x00000027u,
+    0x00000075u,
+    0x0004003du,
+    0x00000006u,
+    0x000002e5u,
+    0x000002c5u,
+    0x00050085u,
+    0x00000006u,
+    0x000002e6u,
+    0x000002e5u,
+    0x000002e4u,
+    0x0003003eu,
+    0x000002c5u,
+    0x000002e6u,
+    0x0004003du,
+    0x00000006u,
+    0x000002e7u,
+    0x000002c5u,
+    0x000500bcu,
+    0x00000020u,
+    0x000002e9u,
+    0x000002e7u,
+    0x000002e8u,
+    0x000300f7u,
+    0x000002ebu,
+    0x00000000u,
+    0x000400fau,
+    0x000002e9u,
+    0x000002eau,
+    0x000002ebu,
+    0x000200f8u,
+    0x000002eau,
+    0x000200f9u,
+    0x00000297u,
+    0x000200f8u,
+    0x000002ebu,
+    0x00060041u,
+    0x0000002fu,
+    0x000002eeu,
+    0x000002a5u,
+    0x000002b2u,
+    0x00000093u,
+    0x0004003du,
+    0x00000018u,
+    0x000002efu,
+    0x000002eeu,
+    0x0007000cu,
+    0x00000018u,
+    0x000002f0u,
+    0x00000001u,
+    0x0000002au,
+    0x000002efu,
+    0x0000003au,
+    0x0004006fu,
+    0x00000006u,
+    0x000002f1u,
+    0x000002f0u,
+    0x0003003eu,
+    0x000002edu,
+    0x000002f1u,
+    0x0004003du,
+    0x000002f3u,
+    0x000002f6u,
+    0x000002f5u,
+    0x0004003du,
+    0x00000024u,
+    0x000002f7u,
+    0x0000014du,
+    0x0006000cu,
+    0x00000024u,
+    0x000002f8u,
+    0x00000001u,
+    0x00000045u,
+    0x000002f7u,
+    0x0004003du,
+    0x00000006u,
+    0x000002f9u,
+    0x000002edu,
+    0x00050051u,
+    0x00000006u,
+    0x000002fau,
+    0x000002f8u,
+    0x00000000u,
+    0x00050051u,
+    0x00000006u,
+    0x000002fbu,
+    0x000002f8u,
+    0x00000001u,
+    0x00050051u,
+    0x00000006u,
+    0x000002fcu,
+    0x000002f8u,
+    0x00000002u,
+    0x00070050u,
+    0x00000007u,
+    0x000002fdu,
+    0x000002fau,
+    0x000002fbu,
+    0x000002fcu,
+    0x000002f9u,
+    0x00070058u,
+    0x00000007u,
+    0x000002feu,
+    0x000002f6u,
+    0x000002fdu,
+    0x00000002u,
+    0x00000027u,
+    0x0008004fu,
+    0x00000024u,
+    0x000002ffu,
+    0x000002feu,
+    0x000002feu,
+    0x00000000u,
+    0x00000001u,
+    0x00000002u,
+    0x00060041u,
+    0x0000006fu,
+    0x00000300u,
+    0x000002a5u,
+    0x00000229u,
+    0x00000093u,
+    0x0004003du,
+    0x00000006u,
+    0x00000301u,
+    0x00000300u,
+    0x0005008eu,
+    0x00000024u,
+    0x00000302u,
+    0x000002ffu,
+    0x00000301u,
+    0x0004003du,
+    0x00000006u,
+    0x00000303u,
+    0x000002c5u,
+    0x0005008eu,
+    0x00000024u,
+    0x00000304u,
+    0x00000302u,
+    0x00000303u,
+    0x0004003du,
+    0x00000024u,
+    0x00000305u,
+    0x00000290u,
+    0x00050081u,
+    0x00000024u,
+    0x00000306u,
+    0x00000305u,
+    0x00000304u,
+    0x0003003eu,
+    0x00000290u,
+    0x00000306u,
+    0x0004003du,
+    0x000002f3u,
+    0x00000308u,
+    0x00000307u,
+    0x0004003du,
+    0x00000024u,
+    0x00000309u,
+    0x0000028bu,
+    0x0006000cu,
+    0x00000024u,
+    0x0000030au,
+    0x00000001u,
+    0x00000045u,
+    0x00000309u,
+    0x0004003du,
+    0x00000006u,
+    0x0000030bu,
+    0x000002edu,
+    0x00050051u,
+    0x00000006u,
+    0x0000030cu,
+    0x0000030au,
+    0x00000000u,
+    0x00050051u,
+    0x00000006u,
+    0x0000030du,
+    0x0000030au,
+    0x00000001u,
+    0x00050051u,
+    0x00000006u,
+    0x0000030eu,
+    0x0000030au,
+    0x00000002u,
+    0x00070050u,
+    0x00000007u,
+    0x0000030fu,
+    0x0000030cu,
+    0x0000030du,
+    0x0000030eu,
+    0x0000030bu,
+    0x0004003du,
+    0x00000006u,
+    0x00000310u,
+    0x0000022cu,
+    0x00050085u,
+    0x00000006u,
+    0x00000312u,
+    0x00000310u,
+    0x00000311u,
+    0x00070058u,
+    0x00000007u,
+    0x00000313u,
+    0x00000308u,
+    0x0000030fu,
+    0x00000002u,
+    0x00000312u,
+    0x0008004fu,
+    0x00000024u,
+    0x00000314u,
+    0x00000313u,
+    0x00000313u,
+    0x00000000u,
+    0x00000001u,
+    0x00000002u,
+    0x00060041u,
+    0x0000006fu,
+    0x00000315u,
+    0x000002a5u,
+    0x00000229u,
+    0x00000071u,
+    0x0004003du,
+    0x00000006u,
+    0x00000316u,
+    0x00000315u,
+    0x0005008eu,
+    0x00000024u,
+    0x00000317u,
+    0x00000314u,
+    0x00000316u,
+    0x0004003du,
+    0x00000006u,
+    0x00000318u,
+    0x000002c5u,
+    0x0005008eu,
+    0x00000024u,
+    0x00000319u,
+    0x00000317u,
+    0x00000318u,
+    0x0004003du,
+    0x00000024u,
+    0x0000031au,
+    0x00000291u,
+    0x00050081u,
+    0x00000024u,
+    0x0000031bu,
+    0x0000031au,
+    0x00000319u,
+    0x0003003eu,
+    0x00000291u,
+    0x0000031bu,
+    0x0004003du,
+    0x00000006u,
+    0x0000031cu,
+    0x000002c5u,
+    0x0004003du,
+    0x00000006u,
+    0x0000031du,
+    0x00000292u,
+    0x00050081u,
+    0x00000006u,
+    0x0000031eu,
+    0x0000031du,
+    0x0000031cu,
+    0x0003003eu,
+    0x00000292u,
+    0x0000031eu,
+    0x000200f9u,
+    0x00000297u,
+    0x000200f8u,
+    0x00000297u,
+    0x0004003du,
+    0x00000018u,
+    0x0000031fu,
+    0x00000293u,
+    0x00050080u,
+    0x00000018u,
+    0x00000320u,
+    0x0000031fu,
+    0x000001e1u,
+    0x0003003eu,
+    0x00000293u,
+    0x00000320u,
+    0x000200f9u,
+    0x00000294u,
+    0x000200f8u,
+    0x00000296u,
+    0x0004003du,
+    0x00000006u,
+    0x00000321u,
+    0x00000292u,
+    0x000500bau,
+    0x00000020u,
+    0x00000322u,
+    0x00000321u,
+    0x000002e8u,
+    0x000300f7u,
+    0x00000324u,
+    0x00000000u,
+    0x000400fau,
+    0x00000322u,
+    0x00000323u,
+    0x00000324u,
+    0x000200f8u,
+    0x00000323u,
+    0x0004003du,
+    0x00000006u,
+    0x00000325u,
+    0x00000292u,
+    0x0004003du,
+    0x00000024u,
+    0x00000326u,
+    0x00000290u,
+    0x00060050u,
+    0x00000024u,
+    0x00000327u,
+    0x00000325u,
+    0x00000325u,
+    0x00000325u,
+    0x00050088u,
+    0x00000024u,
+    0x00000328u,
+    0x00000326u,
+    0x00000327u,
+    0x0003003eu,
+    0x00000290u,
+    0x00000328u,
+    0x0004003du,
+    0x00000006u,
+    0x00000329u,
+    0x00000292u,
+    0x0004003du,
+    0x00000024u,
+    0x0000032au,
+    0x00000291u,
+    0x00060050u,
+    0x00000024u,
+    0x0000032bu,
+    0x00000329u,
+    0x00000329u,
+    0x00000329u,
+    0x00050088u,
+    0x00000024u,
+    0x0000032cu,
+    0x0000032au,
+    0x0000032bu,
+    0x0003003eu,
+    0x00000291u,
+    0x0000032cu,
+    0x000200f9u,
+    0x00000324u,
+    0x000200f8u,
+    0x00000324u,
+    0x0004003du,
+    0x00000024u,
+    0x0000032eu,
+    0x00000290u,
+    0x0004003du,
+    0x00000007u,
+    0x0000032fu,
+    0x00000009u,
+    0x0008004fu,
+    0x00000024u,
+    0x00000330u,
+    0x0000032fu,
+    0x0000032fu,
+    0x00000000u,
+    0x00000001u,
+    0x00000002u,
+    0x00050085u,
+    0x00000024u,
+    0x00000331u,
+    0x0000032eu,
+    0x00000330u,
+    0x0004003du,
+    0x00000006u,
+    0x00000332u,
+    0x0000024cu,
+    0x00050083u,
+    0x00000006u,
+    0x00000333u,
+    0x00000075u,
+    0x00000332u,
+    0x0005008eu,
+    0x00000024u,
+    0x00000334u,
+    0x00000331u,
+    0x00000333u,
+    0x0003003eu,
+    0x0000032du,
+    0x00000334u,
+    0x0004003du,
+    0x00000024u,
+    0x00000336u,
+    0x00000291u,
+    0x0004003du,
+    0x00000024u,
+    0x00000337u,
+    0x00000280u,
+    0x00050085u,
+    0x00000024u,
+    0x00000338u,
+    0x00000336u,
+    0x00000337u,
+    0x0004003du,
+    0x00000006u,
+    0x00000339u,
+    0x0000022cu,
+    0x0008000cu,
+    0x00000006u,
+    0x0000033au,
+    0x00000001u,
+    0x0000002eu,
+    0x00000075u,
+    0x00000265u,
+    0x00000339u,
+    0x0005008eu,
+    0x00000024u,
+    0x0000033bu,
+    0x00000338u,
+    0x0000033au,
+    0x0003003eu,
+    0x00000335u,
+    0x0000033bu,
+    0x0004003du,
+    0x00000024u,
+    0x0000033cu,
+    0x0000032du,
+    0x0005008eu,
+    0x00000024u,
+    0x0000033eu,
+    0x0000033cu,
+    0x0000033du,
+    0x0004003du,
+    0x00000024u,
+    0x0000033fu,
+    0x00000335u,
+    0x0005008eu,
+    0x00000024u,
+    0x00000340u,
+    0x0000033fu,
+    0x00000092u,
+    0x00050081u,
+    0x00000024u,
+    0x00000341u,
+    0x0000033eu,
+    0x00000340u,
+    0x0004003du,
+    0x00000007u,
+    0x00000342u,
+    0x00000083u,
+    0x0008004fu,
+    0x00000024u,
+    0x00000343u,
+    0x00000342u,
+    0x00000342u,
+    0x00000000u,
+    0x00000001u,
+    0x00000002u,
+    0x00050081u,
+    0x00000024u,
+    0x00000344u,
+    0x00000343u,
+    0x00000341u,
+    0x0004003du,
+    0x00000007u,
+    0x00000345u,
+    0x00000083u,
+    0x0009004fu,
+    0x00000007u,
+    0x00000346u,
+    0x00000345u,
+    0x00000344u,
+    0x00000004u,
+    0x00000005u,
+    0x00000006u,
+    0x00000003u,
+    0x0003003eu,
+    0x00000083u,
+    0x00000346u,
+    0x000200f9u,
+    0x0000028au,
+    0x000200f8u,
+    0x00000347u,
+    0x0004003du,
+    0x00000007u,
+    0x0000034au,
+    0x00000083u,
+    0x0008004fu,
+    0x00000024u,
+    0x0000034bu,
+    0x0000034au,
+    0x0000034au,
+    0x00000000u,
+    0x00000001u,
+    0x00000002u,
+    0x0004003du,
+    0x00000006u,
+    0x0000034cu,
+    0x0000024cu,
+    0x00060050u,
+    0x00000024u,
+    0x0000034du,
+    0x0000034cu,
+    0x0000034cu,
+    0x0000034cu,
+    0x0008000cu,
+    0x00000024u,
+    0x0000034eu,
+    0x00000001u,
+    0x0000002eu,
+    0x00000349u,
+    0x0000034bu,
+    0x0000034du,
+    0x0004003du,
+    0x00000006u,
+    0x0000034fu,
+    0x00000274u,
+    0x0005008eu,
+    0x00000024u,
+    0x00000350u,
+    0x0000034eu,
+    0x0000034fu,
+    0x0005008eu,
+    0x00000024u,
+    0x00000352u,
+    0x00000350u,
+    0x00000351u,
+    0x0004003du,
+    0x00000007u,
+    0x00000353u,
+    0x00000083u,
+    0x0008004fu,
+    0x00000024u,
+    0x00000354u,
+    0x00000353u,
+    0x00000353u,
+    0x00000000u,
+    0x00000001u,
+    0x00000002u,
+    0x00050081u,
+    0x00000024u,
+    0x00000355u,
+    0x00000354u,
+    0x00000352u,
+    0x0004003du,
+    0x00000007u,
+    0x00000356u,
+    0x00000083u,
+    0x0009004fu,
+    0x00000007u,
+    0x00000357u,
+    0x00000356u,
+    0x00000355u,
+    0x00000004u,
+    0x00000005u,
+    0x00000006u,
+    0x00000003u,
+    0x0003003eu,
+    0x00000083u,
+    0x00000357u,
+    0x000200f9u,
+    0x0000028au,
+    0x000200f8u,
+    0x0000028au,
+    0x000200f9u,
+    0x0000026fu,
+    0x000200f8u,
+    0x0000026fu,
+    0x000200f9u,
+    0x00000215u,
+    0x000200f8u,
+    0x00000215u,
+    0x000200f9u,
+    0x0000014cu,
+    0x000200f8u,
+    0x0000014cu,
+    0x00060041u,
+    0x0000001cu,
+    0x00000358u,
+    0x00000017u,
+    0x00000019u,
+    0x00000031u,
+    0x0004003du,
+    0x00000006u,
+    0x00000359u,
+    0x00000358u,
+    0x00050088u,
+    0x00000006u,
+    0x0000035au,
+    0x00000359u,
+    0x0000023fu,
+    0x0006000cu,
+    0x00000006u,
+    0x0000035bu,
+    0x00000001u,
+    0x00000008u,
+    0x0000035au,
+    0x0005008du,
+    0x00000006u,
+    0x0000035cu,
+    0x0000035bu,
+    0x000000e8u,
+    0x000500bau,
+    0x00000020u,
+    0x0000035du,
+    0x0000035cu,
+    0x0000001fu,
+    0x000300f7u,
+    0x0000035fu,
+    0x00000000u,
+    0x000400fau,
+    0x0000035du,
+    0x0000035eu,
+    0x0000035fu,
+    0x000200f8u,
+    0x0000035eu,
+    0x0004003du,
+    0x0000000bu,
+    0x00000362u,
+    0x00000361u,
+    0x0004003du,
+    0x0000000fu,
+    0x00000363u,
+    0x00000011u,
+    0x00050057u,
+    0x00000007u,
+    0x00000364u,
+    0x00000362u,
+    0x00000363u,
+    0x00050051u,
+    0x00000006u,
+    0x00000365u,
+    0x00000364u,
+    0x00000000u,
+    0x0003003eu,
+    0x00000360u,
+    0x00000365u,
+    0x0004003du,
+    0x00000006u,
+    0x00000366u,
+    0x00000360u,
+    0x0008000cu,
+    0x00000006u,
+    0x00000368u,
+    0x00000001u,
+    0x0000002eu,
+    0x00000075u,
+    0x00000366u,
+    0x00000367u,
+    0x0004003du,
+    0x00000007u,
+    0x00000369u,
+    0x00000083u,
+    0x0008004fu,
+    0x00000024u,
+    0x0000036au,
+    0x00000369u,
+    0x00000369u,
+    0x00000000u,
+    0x00000001u,
+    0x00000002u,
+    0x0005008eu,
+    0x00000024u,
+    0x0000036bu,
+    0x0000036au,
+    0x00000368u,
+    0x0004003du,
+    0x00000007u,
+    0x0000036cu,
+    0x00000083u,
+    0x0009004fu,
+    0x00000007u,
+    0x0000036du,
+    0x0000036cu,
+    0x0000036bu,
+    0x00000004u,
+    0x00000005u,
+    0x00000006u,
+    0x00000003u,
+    0x0003003eu,
+    0x00000083u,
+    0x0000036du,
+    0x000200f9u,
+    0x0000035fu,
+    0x000200f8u,
+    0x0000035fu,
+    0x00060041u,
+    0x0000001cu,
+    0x0000036eu,
+    0x00000017u,
+    0x00000019u,
+    0x00000031u,
+    0x0004003du,
+    0x00000006u,
+    0x0000036fu,
+    0x0000036eu,
+    0x00050088u,
+    0x00000006u,
+    0x00000371u,
+    0x0000036fu,
+    0x00000370u,
+    0x0006000cu,
+    0x00000006u,
+    0x00000372u,
+    0x00000001u,
+    0x00000008u,
+    0x00000371u,
+    0x0005008du,
+    0x00000006u,
+    0x00000373u,
+    0x00000372u,
+    0x000000e8u,
+    0x000500bau,
+    0x00000020u,
+    0x00000374u,
+    0x00000373u,
+    0x0000001fu,
+    0x000300f7u,
+    0x00000376u,
+    0x00000000u,
+    0x000400fau,
+    0x00000374u,
+    0x00000375u,
+    0x00000376u,
+    0x000200f8u,
+    0x00000375u,
+    0x0004003du,
+    0x0000000bu,
+    0x00000379u,
+    0x00000378u,
+    0x0004003du,
+    0x0000000fu,
+    0x0000037au,
+    0x00000011u,
+    0x00050057u,
+    0x00000007u,
+    0x0000037bu,
+    0x00000379u,
+    0x0000037au,
+    0x0008004fu,
+    0x00000024u,
+    0x0000037cu,
+    0x0000037bu,
+    0x0000037bu,
+    0x00000000u,
+    0x00000001u,
+    0x00000002u,
+    0x0003003eu,
+    0x00000377u,
+    0x0000037cu,
+    0x0004003du,
+    0x00000024u,
+    0x0000037du,
+    0x00000377u,
+    0x00060041u,
+    0x0000001cu,
+    0x0000037eu,
+    0x00000017u,
+    0x000002b2u,
+    0x00000031u,
+    0x0004003du,
+    0x00000006u,
+    0x0000037fu,
+    0x0000037eu,
+    0x0005008eu,
+    0x00000024u,
+    0x00000380u,
+    0x0000037du,
+    0x0000037fu,
+    0x0004003du,
+    0x00000007u,
+    0x00000381u,
+    0x00000083u,
+    0x0008004fu,
+    0x00000024u,
+    0x00000382u,
+    0x00000381u,
+    0x00000381u,
+    0x00000000u,
+    0x00000001u,
+    0x00000002u,
+    0x00050081u,
+    0x00000024u,
+    0x00000383u,
+    0x00000382u,
+    0x00000380u,
+    0x0004003du,
+    0x00000007u,
+    0x00000384u,
+    0x00000083u,
+    0x0009004fu,
+    0x00000007u,
+    0x00000385u,
+    0x00000384u,
+    0x00000383u,
+    0x00000004u,
+    0x00000005u,
+    0x00000006u,
+    0x00000003u,
+    0x0003003eu,
+    0x00000083u,
+    0x00000385u,
+    0x000200f9u,
+    0x00000376u,
+    0x000200f8u,
+    0x00000376u,
+    0x00050041u,
+    0x00000386u,
+    0x00000387u,
+    0x00000083u,
+    0x00000031u,
+    0x0004003du,
+    0x00000006u,
+    0x00000388u,
+    0x00000387u,
+    0x00060041u,
+    0x0000001cu,
+    0x00000389u,
+    0x00000017u,
+    0x00000019u,
+    0x00000093u,
+    0x0004003du,
+    0x00000006u,
+    0x0000038au,
+    0x00000389u,
+    0x000500b8u,
+    0x00000020u,
+    0x0000038bu,
+    0x00000388u,
+    0x0000038au,
+    0x000300f7u,
+    0x0000038du,
+    0x00000000u,
+    0x000400fau,
+    0x0000038bu,
+    0x0000038cu,
+    0x0000038du,
+    0x000200f8u,
+    0x0000038cu,
+    0x000100fcu,
+    0x000200f8u,
+    0x0000038du,
+    0x000100fdu,
+    0x00010038u
+};
+
+} // namespace LLVulkanValidationShaders
+
+#endif
diff --git a/indra/llrender/shaders/vulkan/validation.frag b/indra/llrender/shaders/vulkan/validation.frag
new file mode 100644
index 0000000000..d28390d9e0
--- /dev/null
+++ b/indra/llrender/shaders/vulkan/validation.frag
@@ -0,0 +1,230 @@
+#version 450
+
+layout(location = 0) in vec3 vertex_color;
+layout(location = 1) in vec2 vertex_texcoord;
+layout(location = 2) in vec3 vertex_normal;
+layout(location = 3) in vec4 vertex_tangent;
+layout(location = 4) in vec4 vertex_shadow_coord;
+layout(location = 5) in vec3 vertex_view_position;
+layout(location = 0) out vec4 fragment_color;
+layout(set = 0, binding = 0) uniform sampler2D diffuse_texture;
+layout(set = 1, binding = 0) uniform sampler2D normal_texture;
+layout(set = 2, binding = 0) uniform sampler2D specular_texture;
+layout(set = 3, binding = 0) uniform sampler2D occlusion_texture;
+layout(set = 4, binding = 0) uniform sampler2D emissive_texture;
+layout(set = 6, binding = 0) uniform sampler2DShadow shadow_texture;
+layout(set = 7, binding = 0) uniform samplerCubeArray radiance_probes;
+layout(set = 8, binding = 0) uniform samplerCubeArray irradiance_probes;
+struct ProbeRecord
+{
+    mat4 box;
+    vec4 sphere;
+    vec4 parameters;
+    ivec4 indices;
+};
+layout(set = 9, binding = 0, std430) readonly buffer ProbeMetadata
+{
+    ProbeRecord records[];
+} probes;
+layout(push_constant) uniform Material
+{
+    mat4 unused_transform;
+    vec4 normal_column0;
+    vec4 normal_column1;
+    vec4 normal_column2;
+    vec4 material_flags;
+} material;
+
+void main()
+{
+    vec4 sampled = texture(diffuse_texture, vertex_texcoord);
+    if (material.material_flags.z > 0.5)
+        sampled.rgb = pow(max(sampled.rgb, vec3(0.0)), vec3(2.2));
+    int flags = int(material.material_flags.w);
+    bool water = (flags & 128) != 0;
+    bool underwater = (flags & 256) != 0;
+    bool sky = (flags & 512) != 0;
+    bool cloud = (flags & 1024) != 0;
+    bool sun = (flags & 2048) != 0;
+    bool moon = (flags & 4096) != 0;
+    bool shadow = (flags & 8192) != 0;
+    bool receives_shadow = (flags & 16384) != 0;
+    bool reflection_probes = (flags & 32768) != 0;
+    bool environment = sky || cloud || sun || moon;
+    if (sky)
+    {
+        float altitude = clamp(vertex_texcoord.y, 0.0, 1.0);
+        vec3 horizon = vec3(0.42, 0.58, 0.76);
+        vec3 zenith = vec3(0.055, 0.16, 0.38);
+        fragment_color = vec4(mix(horizon, zenith, altitude), 1.0);
+    }
+    else if (cloud)
+    {
+        float density = smoothstep(0.32, 0.76, sampled.r);
+        vec3 cloud_color = mix(vec3(0.48, 0.53, 0.60),
+                               vec3(0.96, 0.97, 1.0), sampled.g);
+        fragment_color = vec4(cloud_color, density * sampled.a);
+    }
+    else if (sun)
+    {
+        float disc = sampled.a * max(max(sampled.r, sampled.g), sampled.b);
+        fragment_color = vec4(sampled.rgb * vec3(1.35, 1.12, 0.82), disc);
+    }
+    else if (moon)
+    {
+        fragment_color = vec4(sampled.rgb * vec3(0.72, 0.82, 1.0),
+                              sampled.a);
+    }
+    else if (shadow)
+    {
+        fragment_color = sampled;
+    }
+    else if (water)
+    {
+        vec3 waves = texture(normal_texture, vertex_texcoord * 3.0).xyz *
+                     2.0 - 1.0;
+        vec3 surface_normal = normalize(vertex_normal +
+            vec3(waves.xy * 0.38, abs(waves.z) * 0.18));
+        float facing = clamp(abs(surface_normal.z), 0.0, 1.0);
+        float fresnel = pow(1.0 - facing, 4.0);
+        vec3 deep_color = underwater ? vec3(0.015, 0.12, 0.16) :
+                                      vec3(0.025, 0.20, 0.27);
+        vec3 reflection_color = underwater ? vec3(0.08, 0.28, 0.31) :
+                                            vec3(0.36, 0.55, 0.68);
+        vec3 water_color = mix(deep_color, reflection_color,
+                               clamp(0.18 + fresnel, 0.0, 1.0));
+        water_color += sampled.rgb * (underwater ? 0.06 : 0.12);
+        fragment_color = vec4(water_color, underwater ? 0.9 : 0.72);
+    }
+    else
+    {
+        fragment_color = sampled * vec4(vertex_color, 1.0);
+    }
+    if (!water && !environment && material.material_flags.y > 0.5)
+    {
+        vec3 surface_normal = normalize(vertex_normal);
+        if (mod(material.material_flags.w, 2.0) > 0.5)
+        {
+            vec3 tangent = normalize(vertex_tangent.xyz -
+                surface_normal * dot(surface_normal, vertex_tangent.xyz));
+            vec3 bitangent = cross(surface_normal, tangent) * vertex_tangent.w;
+            vec3 mapped_normal = texture(normal_texture, vertex_texcoord).xyz *
+                                 2.0 - 1.0;
+            surface_normal = normalize(mat3(tangent, bitangent,
+                                             surface_normal) * mapped_normal);
+        }
+        vec3 light_direction = normalize(vec3(0.35, 0.45, 0.82));
+        float diffuse = max(dot(surface_normal, light_direction), 0.0);
+        fragment_color.rgb *= 0.25 + 0.75 * diffuse;
+        if (receives_shadow && vertex_shadow_coord.w > 0.0)
+        {
+            vec3 projected = vertex_shadow_coord.xyz / vertex_shadow_coord.w;
+            vec2 shadow_uv = projected.xy * 0.5 + 0.5;
+            float visibility = 1.0;
+            if (all(greaterThanEqual(shadow_uv, vec2(0.0))) &&
+                all(lessThanEqual(shadow_uv, vec2(1.0))) &&
+                projected.z >= 0.0 && projected.z <= 1.0)
+            {
+                vec2 texel = 1.0 / vec2(textureSize(shadow_texture, 0));
+                visibility = 0.0;
+                for (int y = -1; y <= 1; ++y)
+                    for (int x = -1; x <= 1; ++x)
+                        visibility += texture(shadow_texture,
+                            vec3(shadow_uv + vec2(x, y) * texel,
+                                 projected.z - 0.0015));
+                visibility /= 9.0;
+            }
+            fragment_color.rgb *= mix(0.38, 1.0, visibility);
+        }
+        if (mod(floor(material.material_flags.w / 2.0), 2.0) > 0.5)
+        {
+            vec3 half_vector = normalize(light_direction + vec3(0.0, 0.0, 1.0));
+            vec3 material_sample = texture(specular_texture, vertex_texcoord).rgb;
+            bool pbr = (flags & 4) != 0;
+            float metallic_factor = material.normal_column0.w;
+            float roughness_factor = material.normal_column1.w;
+            float roughness = pbr ?
+                clamp(material_sample.g * roughness_factor, 0.04, 1.0) : 0.5;
+            float exponent = pbr ? mix(96.0, 8.0, roughness) : 32.0;
+            float highlight = pow(max(dot(surface_normal, half_vector), 0.0),
+                                  exponent);
+            float metallic = pbr ? material_sample.b * metallic_factor : 0.0;
+            float strength = pbr ? mix(0.04, 1.0, metallic) :
+                                   material_sample.r;
+            fragment_color.rgb += vec3(highlight * strength * 0.35);
+            if (pbr)
+            {
+                float ndotv = max(surface_normal.z, 0.0);
+                float fresnel = pow(1.0 - ndotv, 5.0);
+                vec3 f0 = mix(vec3(0.04), sampled.rgb, metallic);
+                vec3 fresnel_color = f0 + (1.0 - f0) * fresnel;
+                if (reflection_probes)
+                {
+                    vec3 reflected = reflect(vec3(0.0, 0.0, -1.0),
+                                             surface_normal);
+                    vec3 irradiance = vec3(0.0);
+                    vec3 radiance = vec3(0.0);
+                    float probe_weight = 0.0;
+                    for (int probe_index = 0;
+                         probe_index < probes.records.length(); ++probe_index)
+                    {
+                        ProbeRecord probe = probes.records[probe_index];
+                        float weight;
+                        if (probe.indices.w < 0)
+                        {
+                            vec3 local = (probe.box *
+                                vec4(vertex_view_position, 1.0)).xyz;
+                            weight = clamp(1.0 - max(max(abs(local.x),
+                                abs(local.y)), abs(local.z)), 0.0, 1.0);
+                        }
+                        else
+                        {
+                            float radius = max(probe.sphere.w, 0.001);
+                            weight = clamp(1.0 - distance(vertex_view_position,
+                                probe.sphere.xyz) / radius, 0.0, 1.0);
+                        }
+                        weight *= clamp(probe.parameters.z, 0.0, 1.0);
+                        if (weight <= 0.0001)
+                            continue;
+                        float layer = float(max(probe.indices.x, 0));
+                        irradiance += textureLod(irradiance_probes,
+                            vec4(normalize(surface_normal), layer), 0.0).rgb *
+                            probe.parameters.x * weight;
+                        radiance += textureLod(radiance_probes,
+                            vec4(normalize(reflected), layer),
+                            roughness * 7.0).rgb * probe.parameters.y * weight;
+                        probe_weight += weight;
+                    }
+                    if (probe_weight > 0.0001)
+                    {
+                        irradiance /= probe_weight;
+                        radiance /= probe_weight;
+                    }
+                    vec3 diffuse_ibl = irradiance * sampled.rgb *
+                                       (1.0 - metallic);
+                    vec3 specular_ibl = radiance * fresnel_color *
+                                        mix(1.0, 0.35, roughness);
+                    fragment_color.rgb += diffuse_ibl * 0.22 +
+                                          specular_ibl * 0.32;
+                }
+                else
+                {
+                    fragment_color.rgb += mix(vec3(0.03), fragment_color.rgb,
+                                              metallic) * fresnel * 0.15;
+                }
+            }
+        }
+    }
+    if (mod(floor(material.material_flags.w / 8.0), 2.0) > 0.5)
+    {
+        float occlusion = texture(occlusion_texture, vertex_texcoord).r;
+        fragment_color.rgb *= mix(1.0, occlusion, 0.65);
+    }
+    if (mod(floor(material.material_flags.w / 16.0), 2.0) > 0.5)
+    {
+        vec3 emissive = texture(emissive_texture, vertex_texcoord).rgb;
+        fragment_color.rgb += emissive * material.normal_column2.w;
+    }
+    if (fragment_color.a < material.material_flags.x)
+        discard;
+}
diff --git a/indra/llrender/shaders/vulkan/validation.vert b/indra/llrender/shaders/vulkan/validation.vert
new file mode 100644
index 0000000000..b1535fcd71
--- /dev/null
+++ b/indra/llrender/shaders/vulkan/validation.vert
@@ -0,0 +1,122 @@
+#version 450
+
+layout(location = 0) in vec3 position;
+layout(location = 1) in vec4 color;
+layout(location = 2) in vec2 texcoord;
+layout(location = 3) in vec3 normal;
+layout(location = 4) in vec4 tangent;
+layout(location = 5) in vec4 weights;
+layout(location = 6) in uvec4 joints;
+layout(location = 7) in float legacy_weight;
+layout(location = 8) in vec4 shadow_column0;
+layout(location = 9) in vec4 shadow_column1;
+layout(location = 10) in vec4 shadow_column2;
+layout(location = 11) in vec4 shadow_column3;
+layout(location = 12) in vec4 model_view_column0;
+layout(location = 13) in vec4 model_view_column1;
+layout(location = 14) in vec4 model_view_column2;
+layout(location = 15) in vec4 model_view_column3;
+layout(location = 0) out vec3 vertex_color;
+layout(location = 1) out vec2 vertex_texcoord;
+layout(location = 2) out vec3 vertex_normal;
+layout(location = 3) out vec4 vertex_tangent;
+layout(location = 4) out vec4 vertex_shadow_coord;
+layout(location = 5) out vec3 vertex_view_position;
+
+layout(push_constant) uniform Transform
+{
+    mat4 model_view_projection;
+    vec4 normal_column0;
+    vec4 normal_column1;
+    vec4 normal_column2;
+    vec4 material;
+} transform;
+
+layout(set = 5, binding = 0, std430) readonly buffer SkinPalette
+{
+    vec4 rows[];
+} skin;
+
+vec3 skin_position(vec3 value)
+{
+    vec4 source = vec4(value, 1.0);
+    vec3 result = vec3(0.0);
+    for (int influence = 0; influence < 4; ++influence)
+    {
+        uint row = joints[influence] * 3u;
+        result += weights[influence] * vec3(dot(skin.rows[row], source),
+                                            dot(skin.rows[row + 1u], source),
+                                            dot(skin.rows[row + 2u], source));
+    }
+    return result;
+}
+
+vec3 skin_direction(vec3 value)
+{
+    vec3 result = vec3(0.0);
+    for (int influence = 0; influence < 4; ++influence)
+    {
+        uint row = joints[influence] * 3u;
+        result += weights[influence] * vec3(dot(skin.rows[row].xyz, value),
+                                            dot(skin.rows[row + 1u].xyz, value),
+                                            dot(skin.rows[row + 2u].xyz, value));
+    }
+    return result;
+}
+
+mat3x4 legacy_skin_transform()
+{
+    int joint = int(floor(legacy_weight));
+    float blend = fract(legacy_weight);
+    return mat3x4(mix(skin.rows[joint], skin.rows[joint + 1], blend),
+                  mix(skin.rows[joint + 15], skin.rows[joint + 16], blend),
+                  mix(skin.rows[joint + 30], skin.rows[joint + 31], blend));
+}
+
+vec3 legacy_skin_position(vec3 value)
+{
+    mat3x4 transform = legacy_skin_transform();
+    vec4 source = vec4(value, 1.0);
+    return vec3(dot(transform[0], source), dot(transform[1], source),
+                dot(transform[2], source));
+}
+
+vec3 legacy_skin_direction(vec3 value)
+{
+    mat3x4 transform = legacy_skin_transform();
+    return vec3(dot(transform[0].xyz, value),
+                dot(transform[1].xyz, value),
+                dot(transform[2].xyz, value));
+}
+
+void main()
+{
+    bool skinned = (int(transform.material.w) & 32) != 0;
+    bool legacy_avatar = (int(transform.material.w) & 64) != 0;
+    vec3 draw_position = legacy_avatar ? legacy_skin_position(position) :
+                         (skinned ? skin_position(position) : position);
+    vec3 draw_normal = legacy_avatar ? legacy_skin_direction(normal) :
+                       (skinned ? skin_direction(normal) : normal);
+    vec3 draw_tangent = legacy_avatar ? legacy_skin_direction(tangent.xyz) :
+                        (skinned ? skin_direction(tangent.xyz) : tangent.xyz);
+    vec4 clip = transform.model_view_projection * vec4(draw_position, 1.0);
+    mat4 shadow_matrix = mat4(shadow_column0, shadow_column1,
+                              shadow_column2, shadow_column3);
+    vec4 shadow_clip = shadow_matrix * vec4(draw_position, 1.0);
+    vertex_shadow_coord = vec4(shadow_clip.x, -shadow_clip.y,
+                               (shadow_clip.z + shadow_clip.w) * 0.5,
+                               shadow_clip.w);
+    mat4 model_view_matrix = mat4(model_view_column0, model_view_column1,
+                                  model_view_column2, model_view_column3);
+    vertex_view_position = (model_view_matrix *
+                            vec4(draw_position, 1.0)).xyz;
+    gl_Position = vec4(clip.x, -clip.y, (clip.z + clip.w) * 0.5, clip.w);
+    vertex_color = color.rgb;
+    vertex_texcoord = texcoord;
+    mat3 normal_matrix = mat3(transform.normal_column0.xyz,
+                              transform.normal_column1.xyz,
+                              transform.normal_column2.xyz);
+    vertex_normal = normalize(normal_matrix * draw_normal);
+    vertex_tangent = vec4(normalize(normal_matrix * draw_tangent),
+                          tangent.w);
+}
diff --git a/indra/newview/llappviewer.cpp b/indra/newview/llappviewer.cpp
index 7d129dc7cf..b25c6c1b8e 100644
--- a/indra/newview/llappviewer.cpp
+++ b/indra/newview/llappviewer.cpp
@@ -71,6 +71,9 @@
 #include "lltrace.h"
 #include "lltracethreadrecorder.h"
 #include "llviewerwindow.h"
+#include "llrenderbackend.h"
+#include "llvulkanprobe.h"
+#include "llvulkancontext.h"
 #include "llviewerdisplay.h"
 #include "llviewermedia.h"
 #include "llviewerparcelaskplay.h"
@@ -1195,9 +1198,97 @@ bool LLAppViewer::init()
     //
     // Initialize the window
     //
+    const LLRenderBackend requested_backend =
+        LLRenderBackendSelection::requestedFromEnvironment();
+    const LLVulkanProbeResult vulkan_probe = LLVulkanProbe::run();
+    bool vulkan_context_ready = false;
+#if FIRESTORM_VULKAN_EXPERIMENTAL
+    if (requested_backend == LLRenderBackend::VULKAN)
+    {
+        vulkan_context_ready = LLVulkanContext::instance().initialize();
+        if (!vulkan_context_ready)
+        {
+            LL_WARNS("RenderInit")
+                << "Vulkan context initialization failed: "
+                << LLVulkanContext::instance().error() << LL_ENDL;
+        }
+        else
+        {
+            LL_INFOS("RenderInit")
+                << "Vulkan context initialized on device: "
+                << LLVulkanContext::instance().deviceName() << LL_ENDL;
+        }
+    }
+#endif
+    // Vulkan can now validate a Win32 surface and swapchain, but it does not
+    // submit Firestorm draw commands yet. Keep the active renderer on OpenGL.
+    constexpr bool vulkan_backend_compiled = false;
+    const LLRenderBackend active_backend = LLRenderBackendSelection::resolve(
+        requested_backend, vulkan_backend_compiled,
+        vulkan_probe.loader_available && vulkan_probe.api_version != 0);
+
+    LL_INFOS("RenderInit")
+        << "Requested renderer: " << LLRenderBackendSelection::name(requested_backend)
+        << "; Vulkan loader: " << vulkan_probe.loader_name
+        << "; Vulkan API: " << vulkan_probe.versionString()
+        << "; Vulkan context: " << (vulkan_context_ready ? "ready" : "inactive")
+        << "; active renderer: " << LLRenderBackendSelection::name(active_backend)
+        << LL_ENDL;
+    if (!vulkan_probe.error.empty())
+    {
+        LL_WARNS("RenderInit") << "Vulkan probe: " << vulkan_probe.error << LL_ENDL;
+    }
+    if (requested_backend == LLRenderBackend::VULKAN &&
+        active_backend != LLRenderBackend::VULKAN)
+    {
+        LL_WARNS("RenderInit")
+            << "Vulkan was requested but the presentation backend is not yet "
+               "available; falling back to OpenGL."
+            << LL_ENDL;
+    }
+
     gGLActive = true;
     initWindow();
     LL_INFOS("InitInfo") << "Window is initialized." << LL_ENDL ;
+#if FIRESTORM_VULKAN_EXPERIMENTAL && LL_WINDOWS
+    if (vulkan_context_ready && requested_backend == LLRenderBackend::VULKAN &&
+        gViewerWindow && gViewerWindow->getWindow())
+    {
+        LLCoordWindow window_size;
+        gViewerWindow->getWindow()->getSize(&window_size);
+        LLVulkanContext& context = LLVulkanContext::instance();
+        if (context.attachWindow(gViewerWindow->getPlatformWindow(),
+                                 static_cast<unsigned>(window_size.mX),
+                                 static_cast<unsigned>(window_size.mY)))
+        {
+            LL_INFOS("RenderInit")
+                << "Vulkan Win32 swapchain ready: "
+                << context.swapchainWidth() << 'x' << context.swapchainHeight()
+                << ", " << context.swapchainImageCount() << " images"
+                << LL_ENDL;
+            if (LLRenderBackendSelection::validationPresentRequestedFromEnvironment())
+            {
+                const LLVulkanFrameResult frame_result =
+                    context.drawValidationFrame(0.035f, 0.055f, 0.085f);
+                LL_INFOS("RenderInit")
+                    << "Vulkan validation present result: "
+                    << context.frameResultName(frame_result) << LL_ENDL;
+                if (frame_result == LLVulkanFrameResult::ERROR)
+                {
+                    LL_WARNS("RenderInit")
+                        << "Vulkan validation frame failed: "
+                        << context.error() << LL_ENDL;
+                }
+            }
+        }
+        else
+        {
+            LL_WARNS("RenderInit")
+                << "Vulkan Win32 presentation initialization failed: "
+                << context.error() << LL_ENDL;
+        }
+    }
+#endif
     // <FS:Beq> allow detected hardware to be overridden.
     gGLManager.mVRAMDetected = gGLManager.mVRAM;
     LL_INFOS("AppInit") << "VRAM detected: " << gGLManager.mVRAMDetected << LL_ENDL;
diff --git a/indra/newview/llreflectionmapmanager.cpp b/indra/newview/llreflectionmapmanager.cpp
index efde612f76..c6eebb3486 100644
--- a/indra/newview/llreflectionmapmanager.cpp
+++ b/indra/newview/llreflectionmapmanager.cpp
@@ -40,6 +40,7 @@
 #include "llstartup.h"
 #include "llviewermenufile.h"
 #include "llnotificationsutil.h"
+#include "llvulkancontext.h"
 
 #if LL_WINDOWS
 #pragma warning (push)
@@ -1013,6 +1014,49 @@ void LLReflectionMapManager::updateProbeFace(LLReflectionMap* probe, U32 face)
             gIrradianceGenProgram.unbind();
         }
 
+#if FIRESTORM_VULKAN_EXPERIMENTAL && LL_WINDOWS
+        // OpenGL owns Firestorm's probe generator today. Mirror completed cubes
+        // into the Vulkan backend while preserving the existing cube index.
+        // This correctness bridge can later be replaced by external-memory
+        // interop without changing the renderer-facing queue contract.
+        if (glGetTextureSubImage != nullptr && probe->mCubeIndex >= 0)
+        {
+            LLCubeMapArray* source = isRadiancePass() ?
+                mTexture.get() : mIrradianceMaps.get();
+            const U32 mip_count = isRadiancePass() ?
+                static_cast<U32>(mMipChain.size()) : 1U;
+            U32 width = isRadiancePass() ? mProbeResolution :
+                static_cast<U32>(LL_IRRADIANCE_MAP_RESOLUTION);
+            std::vector<LLVulkanProbeFace> mirrored_faces;
+            mirrored_faces.reserve(mip_count * 6);
+            for (U32 mip = 0; mip < mip_count; ++mip)
+            {
+                for (U32 cube_face = 0; cube_face < 6; ++cube_face)
+                {
+                    LLVulkanProbeFace mirrored;
+                    mirrored.cube_index = static_cast<U32>(probe->mCubeIndex);
+                    mirrored.face = cube_face;
+                    mirrored.mip_level = mip;
+                    mirrored.width = width;
+                    mirrored.irradiance = !isRadiancePass();
+                    mirrored.rgba16f.resize(
+                        static_cast<std::size_t>(width) * width * 4);
+                    glGetTextureSubImage(
+                        source->getGLName(), mip, 0, 0,
+                        probe->mCubeIndex * 6 + cube_face,
+                        width, width, 1, GL_RGBA, GL_HALF_FLOAT,
+                        static_cast<GLsizei>(mirrored.rgba16f.size() *
+                                             sizeof(std::uint16_t)),
+                        mirrored.rgba16f.data());
+                    mirrored_faces.push_back(std::move(mirrored));
+                }
+                width = llmax(1U, width / 2);
+            }
+            LLVulkanContext::instance().queueProbeFaces(
+                std::move(mirrored_faces));
+        }
+#endif
+
         mMipChain[0].flush();
     }
 }
@@ -1280,6 +1324,27 @@ void LLReflectionMapManager::updateUniforms()
 
     mProbeData.refmapCount = count;
 
+    std::vector<LLVulkanProbeRecord> vulkan_probes;
+    vulkan_probes.reserve(count);
+    for (S32 probe_index = 0; probe_index < count; ++probe_index)
+    {
+        LLVulkanProbeRecord record;
+        const F32* box = &mProbeData.refBox[probe_index].mMatrix[0][0];
+        std::copy(box, box + record.box.size(), record.box.begin());
+        std::copy(mProbeData.refSphere[probe_index].mV,
+                  mProbeData.refSphere[probe_index].mV + record.sphere.size(),
+                  record.sphere.begin());
+        std::copy(mProbeData.refParams[probe_index].mV,
+                  mProbeData.refParams[probe_index].mV +
+                      record.parameters.size(),
+                  record.parameters.begin());
+        std::copy(mProbeData.refIndex[probe_index],
+                  mProbeData.refIndex[probe_index] + record.indices.size(),
+                  record.indices.begin());
+        vulkan_probes.push_back(record);
+    }
+    LLVulkanContext::instance().queueProbeMetadata(vulkan_probes);
+
     gPipeline.mHeroProbeManager.updateUniforms();
 
     // Get the hero data.
diff --git a/indra/newview/llviewershadermgr.cpp b/indra/newview/llviewershadermgr.cpp
index eaf195cf9e..16199f05c8 100644
--- a/indra/newview/llviewershadermgr.cpp
+++ b/indra/newview/llviewershadermgr.cpp
@@ -945,6 +945,7 @@ bool LLViewerShaderMgr::loadShadersWater()
         gWaterProgram.mFeatures.hasReflectionProbes = true;
         gWaterProgram.mFeatures.hasTonemap = true;
         gWaterProgram.mFeatures.hasShadows = use_sun_shadow;
+        gWaterProgram.mFeatures.isWater = true;
         gWaterProgram.mShaderFiles.clear();
         gWaterProgram.mShaderFiles.push_back(make_pair("environment/waterV.glsl", GL_VERTEX_SHADER));
         gWaterProgram.mShaderFiles.push_back(make_pair("environment/waterF.glsl", GL_FRAGMENT_SHADER));
@@ -971,6 +972,8 @@ bool LLViewerShaderMgr::loadShadersWater()
         gUnderWaterProgram.mName = "Underwater Shader";
         gUnderWaterProgram.mFeatures.calculatesAtmospherics = true;
         gUnderWaterProgram.mFeatures.hasAtmospherics = true;
+        gUnderWaterProgram.mFeatures.isWater = true;
+        gUnderWaterProgram.mFeatures.isUnderwater = true;
         gUnderWaterProgram.mShaderFiles.clear();
         gUnderWaterProgram.mShaderFiles.push_back(make_pair("environment/waterV.glsl", GL_VERTEX_SHADER));
         gUnderWaterProgram.mShaderFiles.push_back(make_pair("environment/underWaterF.glsl", GL_FRAGMENT_SHADER));
@@ -1594,6 +1597,7 @@ bool LLViewerShaderMgr::loadShadersDeferred()
     if (success)
     {
         gDeferredTreeShadowProgram.mName = "Deferred Tree Shadow Shader";
+        gDeferredTreeShadowProgram.mFeatures.isShadow = true;
         gDeferredTreeShadowProgram.mShaderFiles.clear();
         gDeferredTreeShadowProgram.mShaderFiles.push_back(make_pair("deferred/treeShadowV.glsl", GL_VERTEX_SHADER));
         gDeferredTreeShadowProgram.mShaderFiles.push_back(make_pair("deferred/treeShadowF.glsl", GL_FRAGMENT_SHADER));
@@ -1606,6 +1610,7 @@ bool LLViewerShaderMgr::loadShadersDeferred()
     if (success)
     {
         gDeferredSkinnedTreeShadowProgram.mName = "Deferred Skinned Tree Shadow Shader";
+        gDeferredSkinnedTreeShadowProgram.mFeatures.isShadow = true;
         gDeferredSkinnedTreeShadowProgram.mShaderFiles.clear();
         gDeferredSkinnedTreeShadowProgram.mFeatures.hasObjectSkinning = true;
         gDeferredSkinnedTreeShadowProgram.mShaderFiles.push_back(make_pair("deferred/treeShadowSkinnedV.glsl", GL_VERTEX_SHADER));
@@ -2216,6 +2221,7 @@ bool LLViewerShaderMgr::loadShadersDeferred()
     if (success)
     {
         gDeferredShadowProgram.mName = "Deferred Shadow Shader";
+        gDeferredShadowProgram.mFeatures.isShadow = true;
         gDeferredShadowProgram.mShaderFiles.clear();
         gDeferredShadowProgram.mShaderFiles.push_back(make_pair("deferred/shadowV.glsl", GL_VERTEX_SHADER));
         gDeferredShadowProgram.mShaderFiles.push_back(make_pair("deferred/shadowF.glsl", GL_FRAGMENT_SHADER));
@@ -2228,6 +2234,7 @@ bool LLViewerShaderMgr::loadShadersDeferred()
     if (success)
     {
         gDeferredSkinnedShadowProgram.mName = "Deferred Skinned Shadow Shader";
+        gDeferredSkinnedShadowProgram.mFeatures.isShadow = true;
         gDeferredSkinnedShadowProgram.mFeatures.isDeferred = true;
         gDeferredSkinnedShadowProgram.mFeatures.hasShadows = true;
         gDeferredSkinnedShadowProgram.mFeatures.hasObjectSkinning = true;
@@ -2246,6 +2253,7 @@ bool LLViewerShaderMgr::loadShadersDeferred()
     if (success)
     {
         gDeferredShadowCubeProgram.mName = "Deferred Shadow Cube Shader";
+        gDeferredShadowCubeProgram.mFeatures.isShadow = true;
         gDeferredShadowCubeProgram.mFeatures.isDeferred = true;
         gDeferredShadowCubeProgram.mFeatures.hasShadows = true;
         gDeferredShadowCubeProgram.mShaderFiles.clear();
@@ -2260,6 +2268,7 @@ bool LLViewerShaderMgr::loadShadersDeferred()
     if (success)
     {
         gDeferredShadowFullbrightAlphaMaskProgram.mName = "Deferred Shadow Fullbright Alpha Mask Shader";
+        gDeferredShadowFullbrightAlphaMaskProgram.mFeatures.isShadow = true;
         gDeferredShadowFullbrightAlphaMaskProgram.mFeatures.mIndexedTextureChannels = LLGLSLShader::sIndexedTextureChannels;
 
         gDeferredShadowFullbrightAlphaMaskProgram.mShaderFiles.clear();
@@ -2281,6 +2290,7 @@ bool LLViewerShaderMgr::loadShadersDeferred()
     if (success)
     {
         gDeferredShadowAlphaMaskProgram.mName = "Deferred Shadow Alpha Mask Shader";
+        gDeferredShadowAlphaMaskProgram.mFeatures.isShadow = true;
         gDeferredShadowAlphaMaskProgram.mFeatures.mIndexedTextureChannels = LLGLSLShader::sIndexedTextureChannels;
 
         gDeferredShadowAlphaMaskProgram.mShaderFiles.clear();
@@ -2296,6 +2306,7 @@ bool LLViewerShaderMgr::loadShadersDeferred()
     if (success)
     {
         gDeferredShadowGLTFAlphaMaskProgram.mName = "Deferred GLTF Shadow Alpha Mask Shader";
+        gDeferredShadowGLTFAlphaMaskProgram.mFeatures.isShadow = true;
         gDeferredShadowGLTFAlphaMaskProgram.mShaderFiles.clear();
         gDeferredShadowGLTFAlphaMaskProgram.mShaderFiles.push_back(make_pair("deferred/pbrShadowAlphaMaskV.glsl", GL_VERTEX_SHADER));
         gDeferredShadowGLTFAlphaMaskProgram.mShaderFiles.push_back(make_pair("deferred/pbrShadowAlphaMaskF.glsl", GL_FRAGMENT_SHADER));
@@ -2312,6 +2323,7 @@ bool LLViewerShaderMgr::loadShadersDeferred()
     if (success)
     {
         gDeferredShadowGLTFAlphaBlendProgram.mName = "Deferred GLTF Shadow Alpha Blend Shader";
+        gDeferredShadowGLTFAlphaBlendProgram.mFeatures.isShadow = true;
         gDeferredShadowGLTFAlphaBlendProgram.mShaderFiles.clear();
         gDeferredShadowGLTFAlphaBlendProgram.mShaderFiles.push_back(make_pair("deferred/pbrShadowAlphaMaskV.glsl", GL_VERTEX_SHADER));
         gDeferredShadowGLTFAlphaBlendProgram.mShaderFiles.push_back(make_pair("deferred/pbrShadowAlphaBlendF.glsl", GL_FRAGMENT_SHADER));
@@ -2328,6 +2340,7 @@ bool LLViewerShaderMgr::loadShadersDeferred()
     if (success)
     {
         gDeferredAvatarShadowProgram.mName = "Deferred Avatar Shadow Shader";
+        gDeferredAvatarShadowProgram.mFeatures.isShadow = true;
         gDeferredAvatarShadowProgram.mFeatures.hasSkinning = true;
 
         gDeferredAvatarShadowProgram.mShaderFiles.clear();
@@ -2341,6 +2354,7 @@ bool LLViewerShaderMgr::loadShadersDeferred()
     if (success)
     {
         gDeferredAvatarAlphaShadowProgram.mName = "Deferred Avatar Alpha Shadow Shader";
+        gDeferredAvatarAlphaShadowProgram.mFeatures.isShadow = true;
         gDeferredAvatarAlphaShadowProgram.mFeatures.hasSkinning = true;
         gDeferredAvatarAlphaShadowProgram.mShaderFiles.clear();
         gDeferredAvatarAlphaShadowProgram.mShaderFiles.push_back(make_pair("deferred/avatarAlphaShadowV.glsl", GL_VERTEX_SHADER));
@@ -2352,6 +2366,7 @@ bool LLViewerShaderMgr::loadShadersDeferred()
     if (success)
     {
         gDeferredAvatarAlphaMaskShadowProgram.mName = "Deferred Avatar Alpha Mask Shadow Shader";
+        gDeferredAvatarAlphaMaskShadowProgram.mFeatures.isShadow = true;
         gDeferredAvatarAlphaMaskShadowProgram.mFeatures.hasSkinning  = true;
         gDeferredAvatarAlphaMaskShadowProgram.mShaderFiles.clear();
         gDeferredAvatarAlphaMaskShadowProgram.mShaderFiles.push_back(make_pair("deferred/avatarAlphaShadowV.glsl", GL_VERTEX_SHADER));
@@ -2902,6 +2917,7 @@ bool LLViewerShaderMgr::loadShadersDeferred()
         gDeferredWLSkyProgram.mFeatures.hasAtmospherics = true;
         gDeferredWLSkyProgram.mFeatures.hasGamma = true;
         gDeferredWLSkyProgram.mFeatures.hasSrgb = true;
+        gDeferredWLSkyProgram.mFeatures.isSky = true;
 
         gDeferredWLSkyProgram.mShaderFiles.push_back(make_pair("deferred/skyV.glsl", GL_VERTEX_SHADER));
         gDeferredWLSkyProgram.mShaderFiles.push_back(make_pair("deferred/skyF.glsl", GL_FRAGMENT_SHADER));
@@ -2922,6 +2938,7 @@ bool LLViewerShaderMgr::loadShadersDeferred()
         gDeferredWLCloudProgram.mFeatures.hasAtmospherics = true;
         gDeferredWLCloudProgram.mFeatures.hasGamma = true;
         gDeferredWLCloudProgram.mFeatures.hasSrgb = true;
+        gDeferredWLCloudProgram.mFeatures.isCloud = true;
 
         gDeferredWLCloudProgram.mShaderFiles.push_back(make_pair("deferred/cloudsV.glsl", GL_VERTEX_SHADER));
         gDeferredWLCloudProgram.mShaderFiles.push_back(make_pair("deferred/cloudsF.glsl", GL_FRAGMENT_SHADER));
@@ -2943,6 +2960,7 @@ bool LLViewerShaderMgr::loadShadersDeferred()
         gDeferredWLSunProgram.mFeatures.hasGamma = true;
         gDeferredWLSunProgram.mFeatures.hasAtmospherics = true;
         gDeferredWLSunProgram.mFeatures.hasSrgb = true;
+        gDeferredWLSunProgram.mFeatures.isSun = true;
         gDeferredWLSunProgram.mShaderFiles.clear();
         gDeferredWLSunProgram.mShaderFiles.push_back(make_pair("deferred/sunDiscV.glsl", GL_VERTEX_SHADER));
         gDeferredWLSunProgram.mShaderFiles.push_back(make_pair("deferred/sunDiscF.glsl", GL_FRAGMENT_SHADER));
@@ -2963,6 +2981,7 @@ bool LLViewerShaderMgr::loadShadersDeferred()
         gDeferredWLMoonProgram.mFeatures.hasGamma = true;
         gDeferredWLMoonProgram.mFeatures.hasAtmospherics = true;
         gDeferredWLMoonProgram.mFeatures.hasSrgb = true;
+        gDeferredWLMoonProgram.mFeatures.isMoon = true;
 
         gDeferredWLMoonProgram.mShaderFiles.clear();
         gDeferredWLMoonProgram.mShaderFiles.push_back(make_pair("deferred/moonV.glsl", GL_VERTEX_SHADER));
@@ -3625,4 +3644,3 @@ LLViewerShaderMgr::shader_iter LLViewerShaderMgr::endShaders() const
 {
     return mShaderList.end();
 }
-
diff --git a/indra/newview/llviewerwindow.cpp b/indra/newview/llviewerwindow.cpp
index 56a80754f0..c2a7d8a144 100644
--- a/indra/newview/llviewerwindow.cpp
+++ b/indra/newview/llviewerwindow.cpp
@@ -58,6 +58,7 @@
 #include "llxmltree.h"
 #include "llslurl.h"
 #include "llrender.h"
+#include "llvulkancontext.h"
 
 #include "stringize.h"
 
@@ -1621,6 +1622,20 @@ void LLViewerWindow::handleResize(LLWindow *window,  S32 width,  S32 height)
 {
     reshape(width, height);
     mResDirty = true;
+#if FIRESTORM_VULKAN_EXPERIMENTAL && LL_WINDOWS
+    LLVulkanContext& context = LLVulkanContext::instance();
+    if (context.presentationReady() && width > 0 && height > 0)
+    {
+        if (!context.attachWindow(window->getPlatformWindow(),
+                                  static_cast<unsigned>(width),
+                                  static_cast<unsigned>(height)))
+        {
+            LL_WARNS("RenderInit")
+                << "Vulkan swapchain recreation failed after resize: "
+                << context.error() << LL_ENDL;
+        }
+    }
+#endif
 }
 
 // The top-level window has gained focus (e.g. via ALT-TAB)
