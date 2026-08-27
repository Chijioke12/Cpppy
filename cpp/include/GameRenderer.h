#ifndef GAMERENDERER_H
#define GAMERENDERER_H

#include <SDL2/SDL.h>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include "Vector2.h"
#include "RigidBody.h"
#include "Constraint.h"
#include "ParticleSystem.h"
#include "PhysicsWorld.h"

class GameRenderer {
public:
    // Simple procedural 5x7 bitmap font data for standard ASCII (32-126)
    static void drawChar(SDL_Renderer* renderer, char c, int x, int y, int scale, SDL_Color color, bool withShadow = true) {
        if (c < 32 || c > 126) c = '?';
        const uint8_t* rows = FONT_DATA[c - 32];

        // Draw solid dark shadow / outline first for maximum legibility on any background
        if (withShadow) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            for (int r = 0; r < 7; ++r) {
                uint8_t rowBits = rows[r];
                for (int col = 0; col < 5; ++col) {
                    if (rowBits & (1 << (4 - col))) {
                        // 1px expanded dark border around every pixel block
                        SDL_Rect shadowRect = { x + col * scale - 1, y + r * scale - 1, scale + 2, scale + 2 };
                        SDL_RenderFillRect(renderer, &shadowRect);
                    }
                }
            }
        }

        // Draw foreground colored text
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        for (int r = 0; r < 7; ++r) {
            uint8_t rowBits = rows[r];
            for (int col = 0; col < 5; ++col) {
                if (rowBits & (1 << (4 - col))) {
                    SDL_Rect pixelRect = { x + col * scale, y + r * scale, scale, scale };
                    SDL_RenderFillRect(renderer, &pixelRect);
                }
            }
        }
    }

    static void drawText(SDL_Renderer* renderer, const std::string& text, int x, int y, int scale, SDL_Color color, bool withShadow = true) {
        int curX = x;
        for (char c : text) {
            drawChar(renderer, c, curX, y, scale, color, withShadow);
            curX += (5 * scale + std::max(2, scale));
        }
    }

    static void drawFilledCircle(SDL_Renderer* renderer, int cx, int cy, int radius, SDL_Color color) {
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        for (int w = 0; w <= radius * 2; ++w) {
            for (int h = 0; h <= radius * 2; ++h) {
                int dx = radius - w;
                int dy = radius - h;
                if ((dx * dx + dy * dy) <= (radius * radius)) {
                    SDL_RenderDrawPoint(renderer, cx + dx, cy + dy);
                }
            }
        }
    }

    static void drawCircleOutline(SDL_Renderer* renderer, int cx, int cy, int radius, SDL_Color color) {
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        int x = radius - 1;
        int y = 0;
        int dx = 1;
        int dy = 1;
        int err = dx - (radius << 1);

        while (x >= y) {
            SDL_RenderDrawPoint(renderer, cx + x, cy + y);
            SDL_RenderDrawPoint(renderer, cx + y, cy + x);
            SDL_RenderDrawPoint(renderer, cx - y, cy + x);
            SDL_RenderDrawPoint(renderer, cx - x, cy + y);
            SDL_RenderDrawPoint(renderer, cx - x, cy - y);
            SDL_RenderDrawPoint(renderer, cx - y, cy - x);
            SDL_RenderDrawPoint(renderer, cx + y, cy - x);
            SDL_RenderDrawPoint(renderer, cx + x, cy - y);

            if (err <= 0) {
                y++;
                err += dy;
                dy += 2;
            }
            if (err > 0) {
                x--;
                dx += 2;
                err += dx - (radius << 1);
            }
        }
    }

    static void drawPolygon(SDL_Renderer* renderer, const std::vector<Vector2>& verts, SDL_Color fillColor, SDL_Color outlineColor) {
        if (verts.size() < 3) return;

        // Scanline polygon fill
        float minY = verts[0].y, maxY = verts[0].y;
        for (const auto& v : verts) {
            minY = std::min(minY, v.y);
            maxY = std::max(maxY, v.y);
        }

        SDL_SetRenderDrawColor(renderer, fillColor.r, fillColor.g, fillColor.b, fillColor.a);
        for (int y = (int)minY; y <= (int)maxY; ++y) {
            std::vector<int> nodeX;
            size_t j = verts.size() - 1;
            for (size_t i = 0; i < verts.size(); ++i) {
                if ((verts[i].y < (float)y && verts[j].y >= (float)y) || (verts[j].y < (float)y && verts[i].y >= (float)y)) {
                    int x = (int)(verts[i].x + ((float)y - verts[i].y) / (verts[j].y - verts[i].y) * (verts[j].x - verts[i].x));
                    nodeX.push_back(x);
                }
                j = i;
            }
            std::sort(nodeX.begin(), nodeX.end());
            for (size_t k = 0; k + 1 < nodeX.size(); k += 2) {
                SDL_RenderDrawLine(renderer, nodeX[k], y, nodeX[k + 1], y);
            }
        }

        // Outline
        SDL_SetRenderDrawColor(renderer, outlineColor.r, outlineColor.g, outlineColor.b, outlineColor.a);
        for (size_t i = 0; i < verts.size(); ++i) {
            size_t next = (i + 1) % verts.size();
            SDL_RenderDrawLine(renderer, (int)verts[i].x, (int)verts[i].y, (int)verts[next].x, (int)verts[next].y);
        }
    }

    static SDL_Color hexToSDL(uint32_t hex) {
        SDL_Color c;
        c.r = (hex >> 16) & 0xFF;
        c.g = (hex >> 8) & 0xFF;
        c.b = hex & 0xFF;
        c.a = 255;
        return c;
    }
};

#endif // GAMERENDERER_H
