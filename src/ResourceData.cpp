#include "ResourceData.hpp"
#include "PerlinNoise/PerlinNoise.hpp"

Image LoadImageFromPerlinNoise(uint32_t seed, int w, int h, Vector2 uv, int octaves) {
	const siv::PerlinNoise perlin{ seed };
    Image image = GenImageColor(w, h, BLACK);
	for (int x = 0; x < w; x++) {
		for (int y = 0; y < h; y++) {
			const double noise = perlin.octave2D_01((x * uv.x), (y * uv.y), octaves);
            unsigned char intensity = (unsigned char)(noise * 255.0);
            Color color = { intensity, intensity, intensity, 255 };
            ImageDrawPixel(&image, x, y, color);
		}
	}
    return image;
}

// ORM packing: R = AO, G = Roughness, B = Metallic.
Image LoadImageORM(const char *aoPath, const char *roughnessPath, const char *metallicPath) {
    Image ao = LoadImage(aoPath);
    Image roughness = LoadImage(roughnessPath);
    Image metallic = LoadImage(metallicPath);

    if (ao.data == NULL || roughness.data == NULL || metallic.data == NULL) {
        TraceLog(LOG_ERROR, "Failed to load one or more ORM source images");
        UnloadImage(ao); UnloadImage(roughness); UnloadImage(metallic);
        return (Image){0};
    }
    if (ao.width != roughness.width || ao.width != metallic.width ||
        ao.height != roughness.height || ao.height != metallic.height) {
        TraceLog(LOG_ERROR, "ORM source images must have identical dimensions");
        UnloadImage(ao); UnloadImage(roughness); UnloadImage(metallic);
        return (Image){0};
    }
    int w = ao.width;
    int h = ao.height;

    ImageFormat(&ao, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    ImageFormat(&roughness, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    ImageFormat(&metallic, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);

    Color *aoPixels = LoadImageColors(ao);
    Color *roughPixels = LoadImageColors(roughness);
    Color *metalPixels = LoadImageColors(metallic);

    Image ormImage = GenImageColor(w, h, BLACK);
    ImageFormat(&ormImage, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);

	for (int x = 0; x < w; x++) {
		for (int y = 0; y < h; y++) {
            int i = x*w+y;
            Color clr = {
                aoPixels[i].r,
                roughPixels[i].r,
                metalPixels[i].r,
                255
            };
            ImageDrawPixel(&ormImage, x, y, clr);
        }
    }

    UnloadImageColors(aoPixels);
    UnloadImageColors(roughPixels);
    UnloadImageColors(metalPixels);

    UnloadImage(ao);
    UnloadImage(roughness);
    UnloadImage(metallic);
    UnloadImage(ormImage);

    return ormImage;
}