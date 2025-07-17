# vkstart

Vulkan, C++ style, with RAII,
following [this vulkan tutorial](https://docs.vulkan.org/tutorial/latest/00_Introduction.html).

You'll need the [Vulkan SDK](https://vulkan.lunarg.com/sdk/home) installed,
or you [build it yourself](https://github.com/dirkz/vulkan-sdk-builder).

Other dependencies are "vendored in", like GLM. In this case, that means
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