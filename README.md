# Vulkan, C++ style

You'll need the [Vulkan SDK](https://vulkan.lunarg.com/sdk/home), or you
[build it yourself](https://github.com/dirkz/vulkan-sdk-builder).

You also need SDL3.

Other dependencies are "vendored in", like GLM and GLfW. In this case, that means
they are integrated as git submodules, with their SSH URLs.

## How to check out (with all submodules)

Either from the start:

```
git clone --recurse-submodules <repo_url>
```

Or after a normal clone:

```
git submodule update --init --recursive
```