#ifndef TEXTUREMANAGER_H
#define TEXTUREMANAGER_H

#include <SDL2/SDL.h>
#include <unordered_map>
#include <string>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include "stb_image.h"
#include "AssetData.h"

class TextureManager {
public:
    static TextureManager& getInstance() {
        static TextureManager instance;
        return instance;
    }

    bool init(SDL_Renderer* renderer) {
        m_renderer = renderer;
        bool allSuccess = true;

        for (int i = 0; i < TOTAL_EMBEDDED_ASSETS; ++i) {
            const auto& asset = ALL_EMBEDDED_ASSETS[i];
            int width = 0, height = 0, channels = 0;
            
            // Load PNG bytes from memory using stb_image
            unsigned char* pixels = stbi_load_from_memory(asset.data, asset.dataSize, &width, &height, &channels, 4);
            if (!pixels) {
                std::cerr << "Failed to decode PNG asset: " << asset.name << std::endl;
                allSuccess = false;
                continue;
            }

            // Create SDL Texture
            SDL_Texture* texture = SDL_CreateTexture(
                renderer,
                SDL_PIXELFORMAT_RGBA32,
                SDL_TEXTUREACCESS_STATIC,
                width,
                height
            );

            if (!texture) {
                std::cerr << "Failed to create SDL_Texture for: " << asset.name << " (" << SDL_GetError() << ")" << std::endl;
                stbi_image_free(pixels);
                allSuccess = false;
                continue;
            }

            SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
            SDL_UpdateTexture(texture, nullptr, pixels, width * 4);
            stbi_image_free(pixels);

            m_textures[asset.name] = texture;
            m_sizes[asset.name] = { width, height };
        }

        std::cout << "TextureManager initialized " << m_textures.size() << " textures successfully!" << std::endl;
        return allSuccess;
    }

    void cleanup() {
        for (auto& pair : m_textures) {
            if (pair.second) {
                SDL_DestroyTexture(pair.second);
            }
        }
        m_textures.clear();
        m_sizes.clear();
    }

    SDL_Texture* getTexture(const std::string& name) {
        auto it = m_textures.find(name);
        return (it != m_textures.end()) ? it->second : nullptr;
    }

    void draw(const std::string& name, float x, float y, float w, float h, float angleRad = 0.0f, uint8_t alpha = 255) {
        SDL_Texture* tex = getTexture(name);
        if (!tex || !m_renderer) return;

        SDL_Rect dstRect = { (int)(x - w * 0.5f), (int)(y - h * 0.5f), (int)w, (int)h };
        SDL_SetTextureAlphaMod(tex, alpha);

        double angleDeg = angleRad * (180.0 / 3.14159265358979323846);
        SDL_Point center = { (int)(w * 0.5f), (int)(h * 0.5f) };

        SDL_RenderCopyEx(m_renderer, tex, nullptr, &dstRect, angleDeg, &center, SDL_FLIP_NONE);
    }

    void drawTopLeft(const std::string& name, float x, float y, float w, float h, uint8_t alpha = 255) {
        SDL_Texture* tex = getTexture(name);
        if (!tex || !m_renderer) return;

        SDL_Rect dstRect = { (int)x, (int)y, (int)w, (int)h };
        SDL_SetTextureAlphaMod(tex, alpha);
        SDL_RenderCopy(m_renderer, tex, nullptr, &dstRect);
    }

private:
    TextureManager() : m_renderer(nullptr) {}
    ~TextureManager() { cleanup(); }

    SDL_Renderer* m_renderer;
    std::unordered_map<std::string, SDL_Texture*> m_textures;
    std::unordered_map<std::string, std::pair<int, int>> m_sizes;
};

#endif // TEXTUREMANAGER_H
