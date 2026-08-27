import fs from 'fs';
import path from 'path';
import { fileURLToPath } from 'url';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

function cleanHtmlFile(filePath) {
  if (!fs.existsSync(filePath)) return;
  let html = fs.readFileSync(filePath, 'utf8');

  // Fix DOCTYPE
  html = html.replace(/<!doctypehtml>/gi, '<!DOCTYPE html>\n');
  if (!html.startsWith('<!DOCTYPE html>')) {
    html = '<!DOCTYPE html>\n' + html.replace(/^<!DOCTYPE[^>]*>/i, '');
  }

  // Fix missing whitespace between HTML attributes inside tags (parse5 requirement)
  // E.g., class="softkey left"id="lsk-btn" -> class="softkey left" id="lsk-btn"
  // E.g., content="width=..."name="viewport" -> content="width=..." name="viewport"
  html = html.replace(/<[^>]+>/g, (tag) => {
    return tag.replace(/(["'])([a-zA-Z0-9-]+)=/g, '$1 $2=');
  });

  fs.writeFileSync(filePath, html);
  console.log('Successfully formatted HTML attributes in:', filePath);
}

cleanHtmlFile(path.join(__dirname, '../dist/index.html'));
cleanHtmlFile(path.join(__dirname, '../index.html'));
