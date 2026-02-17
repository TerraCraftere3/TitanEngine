typealias ITexture2D<
    T : ITexelElement = int,
    let format : int = 0
> = _Texture<T, __Shape2D, 0, 0, 0, 0, 0, 1, format>;

typealias ISampler2D<
    T : ITexelElement = int,
    let sampleCount : int = 0,
    let format : int = 0
> = _Texture<T, __Shape2D, 0, 0, sampleCount, 0, 0, 1, format>;

typealias UTexture2D<
    T : ITexelElement = uint,
    let format : int = 0
> = _Texture<T, __Shape2D, 0, 0, 0, 0, 0, 1, format>;

typealias USampler2D<
    T : ITexelElement = uint,
    let sampleCount : int = 0,
    let format : int = 0
> = _Texture<T, __Shape2D, 0, 0, sampleCount, 0, 0, 1, format>;

typealias FTexture2D<
    T : ITexelElement = float,
    let format : int = 0
> = _Texture<T, __Shape2D, 0, 0, 0, 0, 0, 1, format>;

typealias FSampler2D<
    T : ITexelElement = float,
    let sampleCount : int = 0,
    let format : int = 0
> = _Texture<T, __Shape2D, 0, 0, sampleCount, 0, 0, 1, format>;


Sampler2D GetBindlessTexture(uint2 handle);
