# Firestorm Vulkan experimental builder

This repository builds the experimental Firestorm Vulkan bootstrap from the
official `Firestorm_Release_7.2.4.80712` source tag plus the patch stored in
`patches/firestorm-vulkan-0.31.patch`.

## Build an installer

1. Open the **Actions** tab.
2. Select **Build experimental Windows installer**.
3. Choose **Run workflow**.
4. When the run completes, download the `firestorm-vulkan-windows` artifact.

The installer is unsigned and Windows SmartScreen may warn before running it.
Install it separately from an official Firestorm installation. This remains a
development bootstrap: OpenGL is the safe fallback and the Vulkan renderer is
not yet feature-complete.

