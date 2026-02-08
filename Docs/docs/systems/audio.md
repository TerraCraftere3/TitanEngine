---
sidebar_position: 8
title: Audio
---

# Audio Playing and Processing System
## Overview
The current Implementation of the Audio System uses [OpenAL](https://github.com/kcat/openal-soft) to play sounds. It already has implementation for spatial audio, which is important.
For audio loading it uses [libsndfile](https://github.com/libsndfile/libsndfile), so for now it only supports the following formats:
- RIFF WAVE (.wav, .wave)
- AIFF / AIFC (.aiff, .aif)
- Au (.au, .snd)
- VOC (.voc)

## Concepts & Usage
### Creating Listener & Sources
```cpp
#include <Titan/Scene/Entity.h>

Entity listener = scene->CreateEntity("Listener");
listener.AddComponent<AudioListenerComponent>();

Entity source = scene->CreateEntity("Source");
auto sourceComponent = source.AddComponent<AudioSourceComponent>();
sourceComponent.Sound = Assets::Load<AudioSource>("sounds/example_sound.wav");
```
This will also manage the spatial audio when using the `Scene::OnUpdateRuntime` method.

### Modifying the Source
```cpp
Entity source = scene->CreateEntity("Source");
auto sourceComponent = source.AddComponent<AudioSourceComponent>();
sourceComponent.Sound = Assets::Load<AudioSource>("sounds/example_sound.wav");
sourceComponent.Volume = 1.0f; // AL_GAIN
sourceComponent.Pitch = 1.0f; // AL_PITCH
sourceComponent.Looping = false; // AL_LOOPING
```
All of the properties can be directly mapped to their OpenAL equivilant