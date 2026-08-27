import { Resvg } from '@resvg/resvg-js';
import fs from 'fs';
import path from 'path';

const ASSETS_DIR = path.resolve('assets/textures');
const SVG_DIR = path.resolve('assets/svgs');
const HEADER_PATH = path.resolve('cpp/include/AssetData.h');

if (!fs.existsSync(ASSETS_DIR)) fs.mkdirSync(ASSETS_DIR, { recursive: true });
if (!fs.existsSync(SVG_DIR)) fs.mkdirSync(SVG_DIR, { recursive: true });

// Dictionary of all high-detail vector SVGs for the Catapult Game
const SVG_DEFS = {
  // --- PROJECTILES / AMMO ---
  projectile_red: {
    w: 64, h: 64,
    svg: `
    <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 64 64" width="64" height="64">
      <defs>
        <radialGradient id="redGrad" cx="35%" cy="30%" r="65%">
          <stop offset="0%" stop-color="#FF5252"/>
          <stop offset="60%" stop-color="#E53935"/>
          <stop offset="100%" stop-color="#B71C1C"/>
        </radialGradient>
        <radialGradient id="bellyGrad" cx="50%" cy="40%" r="60%">
          <stop offset="0%" stop-color="#FFF8E1"/>
          <stop offset="100%" stop-color="#FFE082"/>
        </radialGradient>
        <linearGradient id="beakGrad" x1="0%" y1="0%" x2="100%" y2="100%">
          <stop offset="0%" stop-color="#FFD54F"/>
          <stop offset="100%" stop-color="#FF8F00"/>
        </linearGradient>
        <filter id="dropShadow" x="-20%" y="-20%" width="140%" height="140%">
          <feDropShadow dx="0" dy="2" stdDeviation="1.5" flood-color="#000000" flood-opacity="0.4"/>
        </filter>
      </defs>
      <!-- Tail Feathers -->
      <path d="M 6 36 L 0 32 L 6 28 L 1 24 L 8 22 L 14 30 Z" fill="#212121"/>
      <!-- Main Body -->
      <circle cx="34" cy="34" r="26" fill="url(#redGrad)" stroke="#880E4F" stroke-width="2.5" filter="url(#dropShadow)"/>
      <!-- Light Belly -->
      <path d="M 20 44 C 20 54, 48 54, 48 44 C 48 38, 20 38, 20 44 Z" fill="url(#bellyGrad)"/>
      <!-- Eyes & Brows -->
      <!-- Left Eye -->
      <circle cx="34" cy="28" r="7" fill="#FFFFFF" stroke="#37474F" stroke-width="1.5"/>
      <circle cx="36" cy="28" r="3" fill="#212121"/>
      <circle cx="38" cy="26" r="1" fill="#FFFFFF"/>
      <!-- Right Eye -->
      <circle cx="48" cy="28" r="7" fill="#FFFFFF" stroke="#37474F" stroke-width="1.5"/>
      <circle cx="50" cy="28" r="3" fill="#212121"/>
      <circle cx="52" cy="26" r="1" fill="#FFFFFF"/>
      <!-- Angry Eyebrows -->
      <polygon points="26,20 41,27 41,23 26,17" fill="#212121"/>
      <polygon points="41,27 57,20 57,17 41,23" fill="#212121"/>
      <!-- Beak -->
      <polygon points="38,32 58,35 38,42 41,35" fill="url(#beakGrad)" stroke="#E65100" stroke-width="1.5"/>
      <!-- Top Crest Feathers -->
      <path d="M 28 9 C 24 0, 32 -2, 34 8 Z" fill="#D32F2F" stroke="#880E4F" stroke-width="1.5"/>
      <path d="M 33 9 C 32 1, 39 0, 38 8 Z" fill="#E53935" stroke="#880E4F" stroke-width="1.5"/>
    </svg>`
  },

  projectile_bomb: {
    w: 64, h: 64,
    svg: `
    <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 64 64" width="64" height="64">
      <defs>
        <radialGradient id="bombGrad" cx="35%" cy="30%" r="65%">
          <stop offset="0%" stop-color="#424242"/>
          <stop offset="50%" stop-color="#212121"/>
          <stop offset="100%" stop-color="#000000"/>
        </radialGradient>
        <radialGradient id="sparkGrad" cx="50%" cy="50%" r="50%">
          <stop offset="0%" stop-color="#FFF9C4"/>
          <stop offset="40%" stop-color="#FFB300"/>
          <stop offset="100%" stop-color="#FF3D00"/>
        </radialGradient>
      </defs>
      <!-- Fuse -->
      <path d="M 32 16 Q 38 6, 48 8" fill="none" stroke="#8D6E63" stroke-width="3.5" stroke-linecap="round"/>
      <!-- Glowing Fuse Spark -->
      <circle cx="48" cy="8" r="6" fill="url(#sparkGrad)"/>
      <line x1="48" y1="2" x2="48" y2="14" stroke="#FFF" stroke-width="1.5"/>
      <line x1="42" y1="8" x2="54" y2="8" stroke="#FFF" stroke-width="1.5"/>
      <!-- Cap -->
      <rect x="27" y="14" width="10" height="6" rx="2" fill="#757575" stroke="#212121" stroke-width="1"/>
      <!-- Bomb Body -->
      <circle cx="32" cy="36" r="24" fill="url(#bombGrad)" stroke="#111" stroke-width="2.5"/>
      <!-- Forehead Fuse Feather (Yellow Tip) -->
      <path d="M 32 12 Q 28 4, 32 2 Q 36 4, 32 12 Z" fill="#FFEB3B"/>
      <!-- Eyes & Brow -->
      <circle cx="28" cy="33" r="5.5" fill="#FFFFFF"/>
      <circle cx="30" cy="33" r="2.5" fill="#212121"/>
      <circle cx="42" cy="33" r="5.5" fill="#FFFFFF"/>
      <circle cx="40" cy="33" r="2.5" fill="#212121"/>
      <!-- Big Red Angered Eyebrows -->
      <polygon points="20,24 35,30 35,27 20,22" fill="#E53935"/>
      <polygon points="35,30 50,24 50,22 35,27" fill="#E53935"/>
      <!-- Beak -->
      <polygon points="31,37 41,37 36,46" fill="#FFA000" stroke="#E65100" stroke-width="1"/>
      <!-- Chest Spot -->
      <circle cx="32" cy="49" r="6" fill="#37474F"/>
    </svg>`
  },

  projectile_split: {
    w: 64, h: 64,
    svg: `
    <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 64 64" width="64" height="64">
      <defs>
        <radialGradient id="splitGrad" cx="35%" cy="30%" r="65%">
          <stop offset="0%" stop-color="#4FC3F7"/>
          <stop offset="60%" stop-color="#0288D1"/>
          <stop offset="100%" stop-color="#01579B"/>
        </radialGradient>
      </defs>
      <!-- Tail -->
      <path d="M 8 36 L 2 32 L 8 28 Z" fill="#01579B"/>
      <!-- Body -->
      <circle cx="34" cy="34" r="22" fill="url(#splitGrad)" stroke="#01579B" stroke-width="2"/>
      <!-- Energetic Eyes -->
      <circle cx="34" cy="29" r="6.5" fill="#FFFFFF"/>
      <circle cx="36" cy="29" r="3" fill="#D81B60"/>
      <circle cx="37" cy="28" r="1" fill="#FFFFFF"/>
      <circle cx="46" cy="29" r="6.5" fill="#FFFFFF"/>
      <circle cx="48" cy="29" r="3" fill="#D81B60"/>
      <circle cx="49" cy="28" r="1" fill="#FFFFFF"/>
      <!-- Beak -->
      <polygon points="37,35 52,38 37,43" fill="#FFCA28" stroke="#F57C00" stroke-width="1.2"/>
      <!-- Cheek Flush -->
      <ellipse cx="26" cy="38" rx="3.5" ry="2" fill="#E91E63" opacity="0.6"/>
      <ellipse cx="53" cy="38" rx="3.5" ry="2" fill="#E91E63" opacity="0.6"/>
      <!-- Head Tufts -->
      <path d="M 30 12 Q 33 4, 37 11 Z" fill="#0288D1"/>
    </svg>`
  },

  projectile_drill: {
    w: 64, h: 64,
    svg: `
    <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 64 64" width="64" height="64">
      <defs>
        <radialGradient id="metalSphere" cx="35%" cy="30%" r="65%">
          <stop offset="0%" stop-color="#B0BEC5"/>
          <stop offset="50%" stop-color="#78909C"/>
          <stop offset="100%" stop-color="#37474F"/>
        </radialGradient>
        <linearGradient id="drillMetal" x1="0%" y1="0%" x2="100%" y2="0%">
          <stop offset="0%" stop-color="#ECEFF1"/>
          <stop offset="50%" stop-color="#B0BEC5"/>
          <stop offset="100%" stop-color="#546E7A"/>
        </linearGradient>
      </defs>
      <!-- Heavy Iron Sphere -->
      <circle cx="28" cy="32" r="22" fill="url(#metalSphere)" stroke="#263238" stroke-width="2.5"/>
      <!-- Iron Rivets -->
      <circle cx="16" cy="22" r="2" fill="#CFD8DC" stroke="#263238" stroke-width="0.8"/>
      <circle cx="14" cy="34" r="2" fill="#CFD8DC" stroke="#263238" stroke-width="0.8"/>
      <circle cx="18" cy="44" r="2" fill="#CFD8DC" stroke="#263238" stroke-width="0.8"/>
      <!-- Glowing Cyber Visor Eyes -->
      <path d="M 22 25 L 36 28 L 34 33 L 22 30 Z" fill="#00E676" stroke="#1B5E20" stroke-width="1"/>
      <line x1="24" y1="27" x2="34" y2="29" stroke="#E8F5E9" stroke-width="1.5"/>
      <!-- Front Steel Drill Cone -->
      <polygon points="34,22 62,32 34,42" fill="url(#drillMetal)" stroke="#263238" stroke-width="2"/>
      <!-- Drill Grooves -->
      <path d="M 40 25 Q 46 32, 40 39" fill="none" stroke="#37474F" stroke-width="2"/>
      <path d="M 48 27 Q 53 32, 48 37" fill="none" stroke="#37474F" stroke-width="2"/>
      <path d="M 55 30 Q 58 32, 55 34" fill="none" stroke="#37474F" stroke-width="1.5"/>
    </svg>`
  },

  projectile_rubber: {
    w: 64, h: 64,
    svg: `
    <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 64 64" width="64" height="64">
      <defs>
        <radialGradient id="rubberGrad" cx="35%" cy="30%" r="65%">
          <stop offset="0%" stop-color="#FF8A80"/>
          <stop offset="60%" stop-color="#FF5252"/>
          <stop offset="100%" stop-color="#D50000"/>
        </radialGradient>
      </defs>
      <!-- Spring Coil Behind -->
      <path d="M 12 32 C 6 24, 6 40, 0 32" fill="none" stroke="#FF5252" stroke-width="3" stroke-linecap="round"/>
      <!-- Sphere -->
      <circle cx="34" cy="32" r="22" fill="url(#rubberGrad)" stroke="#B71C1C" stroke-width="2.5"/>
      <!-- Glossy Specular Highlight -->
      <ellipse cx="26" cy="22" rx="7" ry="4" transform="rotate(-30 26 22)" fill="#FFFFFF" opacity="0.6"/>
      <!-- Crazy Bouncing Eyes -->
      <circle cx="34" cy="28" r="6" fill="#FFF"/>
      <circle cx="35" cy="28" r="3" fill="#212121"/>
      <circle cx="47" cy="28" r="6" fill="#FFF"/>
      <circle cx="46" cy="28" r="3" fill="#212121"/>
      <!-- Happy Wide Mouth -->
      <path d="M 32 38 Q 42 46, 50 38" fill="none" stroke="#880E4F" stroke-width="2.5" stroke-linecap="round"/>
    </svg>`
  },

  // --- ENEMIES (PIGS / GOBLINS) ---
  enemy_grunt: {
    w: 64, h: 64,
    svg: `
    <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 64 64" width="64" height="64">
      <defs>
        <radialGradient id="pigGreen" cx="35%" cy="30%" r="65%">
          <stop offset="0%" stop-color="#AEEA00"/>
          <stop offset="60%" stop-color="#7CB342"/>
          <stop offset="100%" stop-color="#33691E"/>
        </radialGradient>
        <radialGradient id="snoutGrad" cx="40%" cy="35%" r="60%">
          <stop offset="0%" stop-color="#C5E1A5"/>
          <stop offset="100%" stop-color="#8BC34A"/>
        </radialGradient>
      </defs>
      <!-- Pig Ears -->
      <ellipse cx="14" cy="20" rx="6" ry="9" transform="rotate(-25 14 20)" fill="#7CB342" stroke="#33691E" stroke-width="1.8"/>
      <ellipse cx="14" cy="20" rx="3" ry="5" transform="rotate(-25 14 20)" fill="#558B2F"/>
      <ellipse cx="50" cy="20" rx="6" ry="9" transform="rotate(25 50 20)" fill="#7CB342" stroke="#33691E" stroke-width="1.8"/>
      <ellipse cx="50" cy="20" rx="3" ry="5" transform="rotate(25 50 20)" fill="#558B2F"/>
      <!-- Body -->
      <circle cx="32" cy="35" r="23" fill="url(#pigGreen)" stroke="#2E7D32" stroke-width="2.5"/>
      <!-- Big Googly Eyes -->
      <circle cx="23" cy="28" r="6.5" fill="#FFFFFF" stroke="#33691E" stroke-width="1.2"/>
      <circle cx="23" cy="28" r="2.8" fill="#212121"/>
      <circle cx="24" cy="27" r="1" fill="#FFFFFF"/>
      <circle cx="41" cy="28" r="6.5" fill="#FFFFFF" stroke="#33691E" stroke-width="1.2"/>
      <circle cx="41" cy="28" r="2.8" fill="#212121"/>
      <circle cx="42" cy="27" r="1" fill="#FFFFFF"/>
      <!-- Snout -->
      <ellipse cx="32" cy="38" rx="10" ry="7" fill="url(#snoutGrad)" stroke="#33691E" stroke-width="1.8"/>
      <ellipse cx="28" cy="38" rx="2.5" ry="3.5" fill="#33691E"/>
      <ellipse cx="36" cy="38" rx="2.5" ry="3.5" fill="#33691E"/>
      <!-- Smirk Mouth -->
      <path d="M 28 47 Q 32 50, 36 47" fill="none" stroke="#2E7D32" stroke-width="2" stroke-linecap="round"/>
    </svg>`
  },

  enemy_helmet: {
    w: 64, h: 64,
    svg: `
    <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 64 64" width="64" height="64">
      <defs>
        <radialGradient id="pigGreen2" cx="35%" cy="30%" r="65%">
          <stop offset="0%" stop-color="#AEEA00"/>
          <stop offset="60%" stop-color="#7CB342"/>
          <stop offset="100%" stop-color="#33691E"/>
        </radialGradient>
        <linearGradient id="helmGrad" x1="0%" y1="0%" x2="0%" y2="100%">
          <stop offset="0%" stop-color="#CFD8DC"/>
          <stop offset="40%" stop-color="#78909C"/>
          <stop offset="100%" stop-color="#37474F"/>
        </linearGradient>
      </defs>
      <!-- Base Body -->
      <circle cx="32" cy="36" r="22" fill="url(#pigGreen2)" stroke="#2E7D32" stroke-width="2"/>
      <!-- Knight Helmet Dome -->
      <path d="M 12 30 C 12 12, 52 12, 52 30 L 54 32 L 10 32 Z" fill="url(#helmGrad)" stroke="#263238" stroke-width="2"/>
      <!-- Helmet Crest & Visor Rim -->
      <rect x="8" y="29" width="48" height="5" rx="2" fill="#90A4AE" stroke="#263238" stroke-width="1.5"/>
      <circle cx="32" cy="11" r="4" fill="#B0BEC5" stroke="#263238" stroke-width="1"/>
      <path d="M 32 7 Q 38 1, 46 4" fill="none" stroke="#D32F2F" stroke-width="3" stroke-linecap="round"/>
      <!-- Eyes peering under helm -->
      <circle cx="23" cy="38" r="5" fill="#FFF"/>
      <circle cx="23" cy="38" r="2.2" fill="#212121"/>
      <circle cx="41" cy="38" r="5" fill="#FFF"/>
      <circle cx="41" cy="38" r="2.2" fill="#212121"/>
      <!-- Snout -->
      <ellipse cx="32" cy="46" rx="8.5" ry="6" fill="#8BC34A" stroke="#33691E" stroke-width="1.5"/>
      <circle cx="29" cy="46" r="2" fill="#33691E"/>
      <circle cx="35" cy="46" r="2" fill="#33691E"/>
    </svg>`
  },

  enemy_king: {
    w: 64, h: 64,
    svg: `
    <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 64 64" width="64" height="64">
      <defs>
        <radialGradient id="kingPig" cx="35%" cy="30%" r="65%">
          <stop offset="0%" stop-color="#C6FF00"/>
          <stop offset="60%" stop-color="#84CC16"/>
          <stop offset="100%" stop-color="#3F6212"/>
        </radialGradient>
        <linearGradient id="goldCrown" x1="0%" y1="0%" x2="100%" y2="100%">
          <stop offset="0%" stop-color="#FDE047"/>
          <stop offset="50%" stop-color="#EAB308"/>
          <stop offset="100%" stop-color="#A16207"/>
        </linearGradient>
      </defs>
      <!-- Ears -->
      <ellipse cx="12" cy="24" rx="5" ry="8" transform="rotate(-25 12 24)" fill="#84CC16" stroke="#3F6212" stroke-width="1.5"/>
      <ellipse cx="52" cy="24" rx="5" ry="8" transform="rotate(25 52 24)" fill="#84CC16" stroke="#3F6212" stroke-width="1.5"/>
      <!-- Body -->
      <circle cx="32" cy="38" r="22" fill="url(#kingPig)" stroke="#3F6212" stroke-width="2.5"/>
      <!-- Golden Crown -->
      <polygon points="16,24 20,10 27,18 32,7 37,18 44,10 48,24" fill="url(#goldCrown)" stroke="#713F12" stroke-width="1.5"/>
      <!-- Crown Jewels -->
      <circle cx="32" cy="11" r="2" fill="#EF4444"/>
      <circle cx="20" cy="13" r="1.5" fill="#3B82F6"/>
      <circle cx="44" cy="13" r="1.5" fill="#3B82F6"/>
      <rect x="16" y="22" width="32" height="4" fill="#CA8A04" stroke="#713F12" stroke-width="1"/>
      <!-- Royal Mustache -->
      <path d="M 22 43 C 26 40, 32 44, 32 44 C 32 44, 38 40, 42 43 C 40 47, 34 46, 32 45 C 30 46, 24 47, 22 43 Z" fill="#65A30D" stroke="#365314" stroke-width="1"/>
      <!-- Royal Eyes -->
      <circle cx="23" cy="32" r="5" fill="#FFF"/>
      <circle cx="24" cy="32" r="2.2" fill="#18181B"/>
      <circle cx="41" cy="32" r="5" fill="#FFF"/>
      <circle cx="40" cy="32" r="2.2" fill="#18181B"/>
      <!-- Royal Snout -->
      <ellipse cx="32" cy="39" rx="8" ry="5.5" fill="#A3E635" stroke="#3F6212" stroke-width="1.5"/>
      <circle cx="29" cy="39" r="1.8" fill="#365314"/>
      <circle cx="35" cy="39" r="1.8" fill="#365314"/>
    </svg>`
  },

  // --- CATAPULT & SLINGSHOT ---
  catapult_fork_front: {
    w: 80, h: 120,
    svg: `
    <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 80 120" width="80" height="120">
      <defs>
        <linearGradient id="woodFork" x1="0%" y1="0%" x2="100%" y2="0%">
          <stop offset="0%" stop-color="#8D6E63"/>
          <stop offset="30%" stop-color="#6D4C41"/>
          <stop offset="70%" stop-color="#5D4037"/>
          <stop offset="100%" stop-color="#3E2723"/>
        </linearGradient>
      </defs>
      <!-- Base Main Post -->
      <path d="M 32 120 L 32 55 L 14 18 C 10 10, 22 4, 28 12 L 40 38 L 52 12 C 58 4, 70 10, 66 18 L 48 55 L 48 120 Z" 
            fill="url(#woodFork)" stroke="#271406" stroke-width="3"/>
      <!-- Iron Bands & Bolts -->
      <rect x="28" y="70" width="24" height="8" rx="2" fill="#455A64" stroke="#263238" stroke-width="1.5"/>
      <circle cx="32" cy="74" r="1.5" fill="#B0BEC5"/>
      <circle cx="48" cy="74" r="1.5" fill="#B0BEC5"/>
      <rect x="28" y="100" width="24" height="8" rx="2" fill="#455A64" stroke="#263238" stroke-width="1.5"/>
      <!-- Left Hook -->
      <circle cx="20" cy="14" r="6" fill="#37474F" stroke="#212121" stroke-width="1.5"/>
      <circle cx="20" cy="14" r="3" fill="#B0BEC5"/>
      <!-- Right Hook -->
      <circle cx="60" cy="14" r="6" fill="#37474F" stroke="#212121" stroke-width="1.5"/>
      <circle cx="60" cy="14" r="3" fill="#B0BEC5"/>
      <!-- Wood Grain Highlights -->
      <path d="M 36 60 L 36 115" stroke="#A1887F" stroke-width="1.2" opacity="0.6"/>
      <path d="M 44 60 L 44 115" stroke="#4E342E" stroke-width="1.5"/>
    </svg>`
  },

  slingshot_pouch: {
    w: 36, h: 36,
    svg: `
    <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 36 36" width="36" height="36">
      <defs>
        <radialGradient id="leatherGrad" cx="30%" cy="30%" r="70%">
          <stop offset="0%" stop-color="#8D6E63"/>
          <stop offset="60%" stop-color="#5D4037"/>
          <stop offset="100%" stop-color="#3E2723"/>
        </radialGradient>
      </defs>
      <!-- Leather Pouch Pad -->
      <rect x="4" y="6" width="28" height="24" rx="6" fill="url(#leatherGrad)" stroke="#1A0C00" stroke-width="2"/>
      <!-- Stitches -->
      <line x1="8" y1="9" x2="28" y2="9" stroke="#D7CCC8" stroke-width="1" stroke-dasharray="2,2"/>
      <line x1="8" y1="27" x2="28" y2="27" stroke="#D7CCC8" stroke-width="1" stroke-dasharray="2,2"/>
      <!-- Rivet Rings -->
      <circle cx="8" cy="18" r="3.5" fill="#455A64" stroke="#263238" stroke-width="1"/>
      <circle cx="8" cy="18" r="1.5" fill="#000"/>
      <circle cx="28" cy="18" r="3.5" fill="#455A64" stroke="#263238" stroke-width="1"/>
      <circle cx="28" cy="18" r="1.5" fill="#000"/>
    </svg>`
  },

  // --- BUILDING BLOCKS & OBSTACLES ---
  wood_block: {
    w: 48, h: 48,
    svg: `
    <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 48 48" width="48" height="48">
      <defs>
        <linearGradient id="woodSquare" x1="0%" y1="0%" x2="100%" y2="100%">
          <stop offset="0%" stop-color="#A1887F"/>
          <stop offset="50%" stop-color="#795548"/>
          <stop offset="100%" stop-color="#4E342E"/>
        </linearGradient>
      </defs>
      <rect x="2" y="2" width="44" height="44" rx="4" fill="url(#woodSquare)" stroke="#3E2723" stroke-width="2.5"/>
      <!-- Wood Bevel Inner Frame -->
      <rect x="6" y="6" width="36" height="36" rx="2" fill="none" stroke="#BCAAA4" stroke-width="1.2" opacity="0.7"/>
      <!-- Wood Grain Details -->
      <path d="M 6 18 Q 24 16, 42 18" fill="none" stroke="#3E2723" stroke-width="1.5" opacity="0.7"/>
      <path d="M 6 30 Q 24 33, 42 30" fill="none" stroke="#3E2723" stroke-width="1.5" opacity="0.7"/>
      <!-- Knothole -->
      <ellipse cx="28" cy="24" rx="4" ry="2.5" fill="#3E2723" opacity="0.6"/>
      <circle cx="28" cy="24" r="1.2" fill="#5D4037"/>
      <!-- Corner Nails -->
      <circle cx="6" cy="6" r="1.5" fill="#37474F"/>
      <circle cx="42" cy="6" r="1.5" fill="#37474F"/>
      <circle cx="6" cy="42" r="1.5" fill="#37474F"/>
      <circle cx="42" cy="42" r="1.5" fill="#37474F"/>
    </svg>`
  },

  wood_beam_h: {
    w: 120, h: 28,
    svg: `
    <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 120 28" width="120" height="28">
      <defs>
        <linearGradient id="woodBeam" x1="0%" y1="0%" x2="0%" y2="100%">
          <stop offset="0%" stop-color="#BCAAA4"/>
          <stop offset="30%" stop-color="#8D6E63"/>
          <stop offset="70%" stop-color="#6D4C41"/>
          <stop offset="100%" stop-color="#4E342E"/>
        </linearGradient>
      </defs>
      <rect x="2" y="2" width="116" height="24" rx="3" fill="url(#woodBeam)" stroke="#3E2723" stroke-width="2"/>
      <line x1="4" y1="10" x2="116" y2="10" stroke="#3E2723" stroke-width="1.2" opacity="0.6"/>
      <line x1="4" y1="18" x2="116" y2="18" stroke="#3E2723" stroke-width="1.2" opacity="0.6"/>
      <line x1="4" y1="4" x2="116" y2="4" stroke="#D7CCC8" stroke-width="1" opacity="0.8"/>
      <!-- Nails -->
      <circle cx="8" cy="14" r="1.8" fill="#37474F"/>
      <circle cx="112" cy="14" r="1.8" fill="#37474F"/>
    </svg>`
  },

  wood_beam_v: {
    w: 28, h: 120,
    svg: `
    <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 28 120" width="28" height="120">
      <defs>
        <linearGradient id="woodBeamV" x1="0%" y1="0%" x2="100%" y2="0%">
          <stop offset="0%" stop-color="#BCAAA4"/>
          <stop offset="30%" stop-color="#8D6E63"/>
          <stop offset="70%" stop-color="#6D4C41"/>
          <stop offset="100%" stop-color="#4E342E"/>
        </linearGradient>
      </defs>
      <rect x="2" y="2" width="24" height="116" rx="3" fill="url(#woodBeamV)" stroke="#3E2723" stroke-width="2"/>
      <line x1="10" y1="4" x2="10" y2="116" stroke="#3E2723" stroke-width="1.2" opacity="0.6"/>
      <line x1="18" y1="4" x2="18" y2="116" stroke="#3E2723" stroke-width="1.2" opacity="0.6"/>
      <circle cx="14" cy="8" r="1.8" fill="#37474F"/>
      <circle cx="14" cy="112" r="1.8" fill="#37474F"/>
    </svg>`
  },

  stone_block: {
    w: 48, h: 48,
    svg: `
    <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 48 48" width="48" height="48">
      <defs>
        <linearGradient id="stoneGrad" x1="0%" y1="0%" x2="100%" y2="100%">
          <stop offset="0%" stop-color="#B0BEC5"/>
          <stop offset="50%" stop-color="#78909C"/>
          <stop offset="100%" stop-color="#37474F"/>
        </linearGradient>
      </defs>
      <rect x="2" y="2" width="44" height="44" rx="4" fill="url(#stoneGrad)" stroke="#263238" stroke-width="2.5"/>
      <!-- Stone Cracks & Texture -->
      <path d="M 8 10 L 18 16 L 22 14 L 30 22" fill="none" stroke="#263238" stroke-width="1.5"/>
      <path d="M 28 32 L 36 38 L 42 34" fill="none" stroke="#263238" stroke-width="1.5"/>
      <!-- Chiseled Edge Highlights -->
      <polyline points="4,44 4,4 44,4" fill="none" stroke="#ECEFF1" stroke-width="1.5" opacity="0.7"/>
      <polyline points="44,4 44,44 4,44" fill="none" stroke="#263238" stroke-width="2"/>
    </svg>`
  },

  stone_beam_h: {
    w: 120, h: 28,
    svg: `
    <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 120 28" width="120" height="28">
      <defs>
        <linearGradient id="stoneH" x1="0%" y1="0%" x2="0%" y2="100%">
          <stop offset="0%" stop-color="#CFD8DC"/>
          <stop offset="40%" stop-color="#90A4AE"/>
          <stop offset="100%" stop-color="#455A64"/>
        </linearGradient>
      </defs>
      <rect x="2" y="2" width="116" height="24" rx="3" fill="url(#stoneH)" stroke="#263238" stroke-width="2"/>
      <polyline points="4,24 4,4 116,4" fill="none" stroke="#ECEFF1" stroke-width="1.5" opacity="0.8"/>
      <!-- Masonry Section Lines -->
      <line x1="40" y1="4" x2="40" y2="24" stroke="#263238" stroke-width="1.5"/>
      <line x1="80" y1="4" x2="80" y2="24" stroke="#263238" stroke-width="1.5"/>
      <path d="M 48 10 L 58 14 L 64 12" fill="none" stroke="#37474F" stroke-width="1.2"/>
    </svg>`
  },

  stone_beam_v: {
    w: 28, h: 120,
    svg: `
    <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 28 120" width="28" height="120">
      <defs>
        <linearGradient id="stoneV" x1="0%" y1="0%" x2="100%" y2="0%">
          <stop offset="0%" stop-color="#CFD8DC"/>
          <stop offset="40%" stop-color="#90A4AE"/>
          <stop offset="100%" stop-color="#455A64"/>
        </linearGradient>
      </defs>
      <rect x="2" y="2" width="24" height="116" rx="3" fill="url(#stoneV)" stroke="#263238" stroke-width="2"/>
      <line x1="4" y1="40" x2="24" y2="40" stroke="#263238" stroke-width="1.5"/>
      <line x1="4" y1="80" x2="24" y2="80" stroke="#263238" stroke-width="1.5"/>
    </svg>`
  },

  glass_block: {
    w: 48, h: 48,
    svg: `
    <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 48 48" width="48" height="48">
      <defs>
        <linearGradient id="glassGrad" x1="0%" y1="0%" x2="100%" y2="100%">
          <stop offset="0%" stop-color="#E0F7FA" stop-opacity="0.9"/>
          <stop offset="50%" stop-color="#80DEEA" stop-opacity="0.75"/>
          <stop offset="100%" stop-color="#00ACC1" stop-opacity="0.85"/>
        </linearGradient>
      </defs>
      <rect x="2" y="2" width="44" height="44" rx="4" fill="url(#glassGrad)" stroke="#00838F" stroke-width="2"/>
      <!-- Glass Refraction Diagonals -->
      <polygon points="6,6 18,6 6,18" fill="#FFFFFF" opacity="0.6"/>
      <polygon points="42,42 30,42 42,30" fill="#FFFFFF" opacity="0.4"/>
      <line x1="12" y1="40" x2="36" y2="8" stroke="#FFFFFF" stroke-width="2" opacity="0.7"/>
      <line x1="18" y1="42" x2="42" y2="12" stroke="#E0F7FA" stroke-width="1" opacity="0.5"/>
    </svg>`
  },

  glass_beam_v: {
    w: 24, h: 120,
    svg: `
    <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 120" width="24" height="120">
      <defs>
        <linearGradient id="glassV" x1="0%" y1="0%" x2="100%" y2="0%">
          <stop offset="0%" stop-color="#E0F7FA" stop-opacity="0.9"/>
          <stop offset="50%" stop-color="#80DEEA" stop-opacity="0.7"/>
          <stop offset="100%" stop-color="#0097A7" stop-opacity="0.85"/>
        </linearGradient>
      </defs>
      <rect x="2" y="2" width="20" height="116" rx="3" fill="url(#glassV)" stroke="#00838F" stroke-width="2"/>
      <line x1="6" y1="10" x2="18" y2="110" stroke="#FFFFFF" stroke-width="1.8" opacity="0.7"/>
    </svg>`
  },

  metal_girder: {
    w: 120, h: 32,
    svg: `
    <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 120 32" width="120" height="32">
      <defs>
        <linearGradient id="girderGrad" x1="0%" y1="0%" x2="0%" y2="100%">
          <stop offset="0%" stop-color="#90A4AE"/>
          <stop offset="40%" stop-color="#546E7A"/>
          <stop offset="100%" stop-color="#263238"/>
        </linearGradient>
      </defs>
      <!-- Girder Body -->
      <rect x="2" y="2" width="116" height="28" rx="3" fill="url(#girderGrad)" stroke="#1A237E" stroke-width="2"/>
      <!-- I-Beam Flanges -->
      <rect x="2" y="2" width="116" height="6" fill="#78909C" stroke="#263238" stroke-width="1"/>
      <rect x="2" y="24" width="116" height="6" fill="#37474F" stroke="#263238" stroke-width="1"/>
      <!-- Triangular Truss Openings -->
      <polygon points="16,10 28,10 22,22" fill="#1E293B" stroke="#37474F" stroke-width="1"/>
      <polygon points="40,22 52,22 46,10" fill="#1E293B" stroke="#37474F" stroke-width="1"/>
      <polygon points="64,10 76,10 70,22" fill="#1E293B" stroke="#37474F" stroke-width="1"/>
      <polygon points="88,22 100,22 94,10" fill="#1E293B" stroke="#37474F" stroke-width="1"/>
      <!-- Heavy Hex Bolts -->
      <circle cx="8" cy="5" r="1.8" fill="#ECEFF1"/>
      <circle cx="112" cy="5" r="1.8" fill="#ECEFF1"/>
      <circle cx="8" cy="27" r="1.8" fill="#ECEFF1"/>
      <circle cx="112" cy="27" r="1.8" fill="#ECEFF1"/>
    </svg>`
  },

  tnt_crate: {
    w: 52, h: 52,
    svg: `
    <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 52 52" width="52" height="52">
      <defs>
        <radialGradient id="tntBg" cx="30%" cy="30%" r="70%">
          <stop offset="0%" stop-color="#EF5350"/>
          <stop offset="60%" stop-color="#C62828"/>
          <stop offset="100%" stop-color="#7F0000"/>
        </radialGradient>
      </defs>
      <!-- Crate Outline -->
      <rect x="2" y="2" width="48" height="48" rx="4" fill="url(#tntBg)" stroke="#3E2723" stroke-width="2.5"/>
      <!-- Diagonal Yellow/Black Hazard Stripes on Border -->
      <path d="M 4 4 L 48 48 M 4 48 L 48 4" stroke="#FFE082" stroke-width="3" opacity="0.6"/>
      <!-- Wood Frame Edge -->
      <rect x="6" y="6" width="40" height="40" fill="none" stroke="#4E342E" stroke-width="3"/>
      <!-- TNT Danger Banner Center -->
      <rect x="8" y="16" width="36" height="20" rx="3" fill="#212121" stroke="#FFD54F" stroke-width="1.5"/>
      <!-- Bold TNT Text -->
      <text x="26" y="31" font-family="Arial, Helvetica, sans-serif" font-weight="900" font-size="14" fill="#FFEB3B" text-anchor="middle" letter-spacing="1">TNT</text>
      <!-- Glowing Fuse at Top -->
      <circle cx="26" cy="4" r="3" fill="#FF5722"/>
      <circle cx="26" cy="4" r="1.5" fill="#FFEB3B"/>
    </svg>`
  },

  // --- BACKGROUND & SCENERY ---
  background_sky: {
    w: 1200, h: 700,
    svg: `
    <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1200 700" width="1200" height="700">
      <defs>
        <linearGradient id="skyGrad" x1="0%" y1="0%" x2="0%" y2="100%">
          <stop offset="0%" stop-color="#0284C7"/>
          <stop offset="35%" stop-color="#38BDF8"/>
          <stop offset="65%" stop-color="#BAE6FD"/>
          <stop offset="90%" stop-color="#FEF08A"/>
          <stop offset="100%" stop-color="#FED7AA"/>
        </linearGradient>
        <radialGradient id="sunGlow" cx="50%" cy="50%" r="50%">
          <stop offset="0%" stop-color="#FFFBEB" stop-opacity="1"/>
          <stop offset="40%" stop-color="#FDE047" stop-opacity="0.8"/>
          <stop offset="100%" stop-color="#F59E0B" stop-opacity="0"/>
        </radialGradient>
        <linearGradient id="mtnFar" x1="0%" y1="0%" x2="0%" y2="100%">
          <stop offset="0%" stop-color="#93C5FD"/>
          <stop offset="100%" stop-color="#60A5FA"/>
        </linearGradient>
        <linearGradient id="mtnMid" x1="0%" y1="0%" x2="0%" y2="100%">
          <stop offset="0%" stop-color="#475569"/>
          <stop offset="100%" stop-color="#334155"/>
        </linearGradient>
        <linearGradient id="hillBack" x1="0%" y1="0%" x2="0%" y2="100%">
          <stop offset="0%" stop-color="#65A30D"/>
          <stop offset="100%" stop-color="#4D7C0F"/>
        </linearGradient>
        <linearGradient id="hillFront" x1="0%" y1="0%" x2="0%" y2="100%">
          <stop offset="0%" stop-color="#84CC16"/>
          <stop offset="100%" stop-color="#3F6212"/>
        </linearGradient>
      </defs>
      <!-- Sky -->
      <rect width="1200" height="700" fill="url(#skyGrad)"/>
      <!-- Sun -->
      <circle cx="950" cy="180" r="140" fill="url(#sunGlow)"/>
      <circle cx="950" cy="180" r="45" fill="#FFFBEB"/>

      <!-- Fluffy Stylized Clouds -->
      <!-- Cloud 1 -->
      <path d="M 120 180 Q 140 140, 180 150 Q 220 120, 260 150 Q 300 140, 320 180 Q 340 210, 300 220 L 140 220 Q 100 210, 120 180 Z" fill="#FFFFFF" opacity="0.85"/>
      <!-- Cloud 2 -->
      <path d="M 520 120 Q 545 90, 580 100 Q 615 75, 650 100 Q 685 90, 700 120 Q 720 145, 680 155 L 540 155 Q 500 145, 520 120 Z" fill="#FFFFFF" opacity="0.75"/>

      <!-- Distant Mountain Range -->
      <polygon points="-50,600 180,320 340,480 520,290 750,520 920,310 1150,550 1250,600 1250,700 -50,700" fill="url(#mtnFar)" opacity="0.6"/>
      <!-- Mid Mountain Range -->
      <polygon points="-50,600 120,380 280,510 460,360 680,540 850,390 1060,560 1250,620 1250,700 -50,700" fill="url(#mtnMid)" opacity="0.7"/>

      <!-- Rolling Green Hills Layer 1 -->
      <path d="M -50 560 Q 200 480, 480 540 Q 780 600, 1050 510 Q 1180 480, 1250 520 L 1250 700 L -50 700 Z" fill="url(#hillBack)"/>
      <!-- Rolling Green Hills Layer 2 (Foreground) -->
      <path d="M -50 620 Q 250 560, 550 610 Q 880 660, 1250 580 L 1250 700 L -50 700 Z" fill="url(#hillFront)"/>
    </svg>`
  },

  ground_plate: {
    w: 1200, h: 100,
    svg: `
    <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1200 100" width="1200" height="100">
      <defs>
        <linearGradient id="groundSoil" x1="0%" y1="0%" x2="0%" y2="100%">
          <stop offset="0%" stop-color="#84CC16"/>
          <stop offset="12%" stop-color="#4D7C0F"/>
          <stop offset="25%" stop-color="#78350F"/>
          <stop offset="100%" stop-color="#451A03"/>
        </linearGradient>
      </defs>
      <rect x="0" y="0" width="1200" height="100" fill="url(#groundSoil)"/>
      <!-- Grass Tufts Fringe on Top -->
      <path d="M 0 0 Q 30 8, 60 0 Q 90 8, 120 0 Q 150 8, 180 0 Q 210 8, 240 0 Q 270 8, 300 0 Q 330 8, 360 0 Q 390 8, 420 0 Q 450 8, 480 0 Q 510 8, 540 0 Q 570 8, 600 0 Q 630 8, 660 0 Q 690 8, 720 0 Q 750 8, 780 0 Q 810 8, 840 0 Q 870 8, 900 0 Q 930 8, 960 0 Q 990 8, 1020 0 Q 1050 8, 1080 0 Q 1110 8, 1140 0 Q 1170 8, 1200 0 L 1200 10 L 0 10 Z" fill="#A3E635"/>
      <!-- Soil Stratum Pebbles -->
      <circle cx="150" cy="45" r="6" fill="#573a25"/>
      <circle cx="180" cy="55" r="4" fill="#362215"/>
      <circle cx="450" cy="50" r="7" fill="#573a25"/>
      <circle cx="750" cy="40" r="5" fill="#362215"/>
      <circle cx="1020" cy="60" r="6" fill="#573a25"/>
    </svg>`
  },

  // --- VFX & PARTICLES ---
  vfx_explosion: {
    w: 96, h: 96,
    svg: `
    <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 96 96" width="96" height="96">
      <defs>
        <radialGradient id="blastGrad" cx="50%" cy="50%" r="50%">
          <stop offset="0%" stop-color="#FFFFFF"/>
          <stop offset="25%" stop-color="#FFEB3B"/>
          <stop offset="60%" stop-color="#FF5722"/>
          <stop offset="85%" stop-color="#D50000"/>
          <stop offset="100%" stop-color="#7F0000" stop-opacity="0"/>
        </radialGradient>
      </defs>
      <!-- Outer Fire Spikes -->
      <polygon points="48,0 58,28 88,12 74,40 96,48 74,56 88,84 58,68 48,96 38,68 8,84 22,56 0,48 22,40 8,12 38,28" fill="url(#blastGrad)"/>
      <!-- Center Bright Core -->
      <circle cx="48" cy="48" r="20" fill="#FFFFFF"/>
      <circle cx="48" cy="48" r="14" fill="#FFF59D"/>
    </svg>`
  },

  vfx_smoke: {
    w: 64, h: 64,
    svg: `
    <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 64 64" width="64" height="64">
      <defs>
        <radialGradient id="smokeGrad" cx="40%" cy="35%" r="60%">
          <stop offset="0%" stop-color="#ECEFF1" stop-opacity="0.9"/>
          <stop offset="60%" stop-color="#90A4AE" stop-opacity="0.6"/>
          <stop offset="100%" stop-color="#455A64" stop-opacity="0"/>
        </radialGradient>
      </defs>
      <circle cx="32" cy="32" r="28" fill="url(#smokeGrad)"/>
      <circle cx="22" cy="26" r="14" fill="#CFD8DC" opacity="0.6"/>
      <circle cx="40" cy="24" r="12" fill="#ECEFF1" opacity="0.7"/>
      <circle cx="36" cy="38" r="16" fill="#B0BEC5" opacity="0.5"/>
    </svg>`
  },

  vfx_star: {
    w: 48, h: 48,
    svg: `
    <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 48 48" width="48" height="48">
      <defs>
        <linearGradient id="starGold" x1="0%" y1="0%" x2="100%" y2="100%">
          <stop offset="0%" stop-color="#FEF08A"/>
          <stop offset="40%" stop-color="#FACC15"/>
          <stop offset="100%" stop-color="#CA8A04"/>
        </linearGradient>
      </defs>
      <polygon points="24,2 30,17 46,17 33,27 38,43 24,33 10,43 15,27 2,17 18,17" 
               fill="url(#starGold)" stroke="#854D0E" stroke-width="2"/>
      <polygon points="24,6 28,18 40,18 30,26 34,38 24,30 14,38 18,26 8,18 20,18" 
               fill="#FEF9C3" opacity="0.5"/>
    </svg>`
  }
};

async function buildAll() {
  console.log('Rendering high-detail SVGs to PNG textures...');
  const assetMetadata = {};

  for (const [key, item] of Object.entries(SVG_DEFS)) {
    const svgPath = path.join(SVG_DIR, `${key}.svg`);
    const pngPath = path.join(ASSETS_DIR, `${key}.png`);

    fs.writeFileSync(svgPath, item.svg.trim());

    const resvg = new Resvg(item.svg, {
      fitTo: { mode: 'width', value: item.w }
    });
    const pngData = resvg.render();
    const pngBuffer = pngData.asPng();
    fs.writeFileSync(pngPath, pngBuffer);

    assetMetadata[key] = {
      width: pngData.width,
      height: pngData.height,
      size: pngBuffer.length,
      pngBuffer: Array.from(pngBuffer)
    };
    console.log(`  ✓ Rendered ${key}.png (${pngData.width}x${pngData.height}, ${pngBuffer.length} bytes)`);
  }

  // Generate C++ AssetData.h header embedding all textures as raw const bytes for zero-latency Wasm/Native loading
  let cppHeader = `// Auto-generated by generate_svg_assets.js
#ifndef ASSETDATA_H
#define ASSETDATA_H

#include <vector>
#include <string>
#include <unordered_map>
#include <cstdint>

struct EmbeddedAsset {
    const char* name;
    int width;
    int height;
    int dataSize;
    const uint8_t* data;
};

`;

  for (const [key, meta] of Object.entries(assetMetadata)) {
    cppHeader += `static const uint8_t ASSET_${key.toUpperCase()}_PNG[] = {\n    `;
    const bytes = meta.pngBuffer;
    const hexBytes = bytes.map(b => `0x${b.toString(16).padStart(2, '0')}`);
    for (let i = 0; i < hexBytes.length; i++) {
      cppHeader += hexBytes[i];
      if (i < hexBytes.length - 1) {
        cppHeader += ', ';
        if ((i + 1) % 16 === 0) cppHeader += '\n    ';
      }
    }
    cppHeader += `\n};\n\n`;
  }

  cppHeader += `static const EmbeddedAsset ALL_EMBEDDED_ASSETS[] = {\n`;
  for (const [key, meta] of Object.entries(assetMetadata)) {
    cppHeader += `    { "${key}", ${meta.width}, ${meta.height}, (int)sizeof(ASSET_${key.toUpperCase()}_PNG), ASSET_${key.toUpperCase()}_PNG },\n`;
  }
  cppHeader += `};\n\n`;
  cppHeader += `static const int TOTAL_EMBEDDED_ASSETS = sizeof(ALL_EMBEDDED_ASSETS) / sizeof(EmbeddedAsset);\n\n`;
  cppHeader += `#endif // ASSETDATA_H\n`;

  fs.writeFileSync(HEADER_PATH, cppHeader);
  console.log(`\n🎉 All SVGs converted to PNG and embedded in ${HEADER_PATH}`);
}

buildAll().catch(err => {
  console.error(err);
  process.exit(1);
});
