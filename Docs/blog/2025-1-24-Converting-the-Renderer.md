---
slug: converting-the-renderer
title: Converting the Renderer & Creating the Project System
authors: [elias]
tags: [renderer, backends, opengl]
---

Yesterday, i finally finished converting the renderer to something i can use across multiple apis *(e.g. vulkan, dx11, opengl)*. And started to work on the early Project System
<!-- truncate -->


## Renderer
I started by creating a `PipelineState` class that contains data like Cull Mode, Depth Function, Vertex Array, Buffers, Textures, etc. and then created an OpenGL specific implementation. The First Class that i changed was Geometry Renderer, since it didnt use any other system (like the Fullscreen Renderer). That was the first test to see if the Pipeline State worked.

After a few bugs, it "seemed" to work (with a few issues). Then came the Problems, i needed to convert every system at once. I did that by keeping a lot of legacy system (*Bind()* and *Unbind()* Methods, etc.) while converting stuff like the PBR Renderer and the Post Effects. 

While working, i stumbled upon some other stuff that needed fixing like the Scene Renderer that used the same buffer to write and read (which i didnt even know was bad). Anyways, after the refactor there were still a few issues like binding textures to the same unit as cubemaps.

## Project
The Next Thing i wanted to do, was create a Project System (for now a simple one, like in [Hazel](https://github.com/TheCherno/Haze)), because i didnt want to forever use the Workspace to modify and create projects and / or copy the binaries everywhere.

The First Thing i did, was copy all of the Assets to `Example/assets` and create a Project File in `Example`. Afterwards i implemented `Project` and `ProjectSerializer`. Then i needed to implement the custom Asset Path in `SceneSerializer` and the Editor.

That nearly worked, but it still needed fixing. I implemented Internal Paths *(e.g. `models/foliage/tree.fbx`)* to correctly save paths in Scenes and Materials. After that it was implemented to the point it is [right now](https://github.com/TerraCraftere3/TitanEngine/commit/09967acd5d4df4dd9ace77263e1c17b58743ffc3)

## TL;DR
I was able to fully refactor the renderer to be cross platform expandable and created a project system