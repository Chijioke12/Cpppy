import fs from 'fs';
import path from 'path';
import { fileURLToPath } from 'url';
import JSZip from 'jszip';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

async function createOmniSDPackage() {
  const distDir = path.join(__dirname, '../dist');
  const publicDir = path.join(__dirname, '../public');

  if (!fs.existsSync(distDir)) {
    console.error('dist directory does not exist!');
    process.exit(1);
  }

  console.log('=== Building OmniSD Sideloading Package ===');

  // 1. Create application.zip containing dist content
  const appZip = new JSZip();

  // Read manifest.webapp to get app id / version
  let appId = 'physics-demolition';
  let manifestPath = path.join(distDir, 'manifest.webapp');
  if (!fs.existsSync(manifestPath)) {
    manifestPath = path.join(__dirname, '../public/manifest.webapp');
  }

  if (fs.existsSync(manifestPath)) {
    try {
      const manifestObj = JSON.parse(fs.readFileSync(manifestPath, 'utf8'));
      if (manifestObj.name) {
        appId = manifestObj.name.toLowerCase().replace(/[^a-z0-9]+/g, '-').replace(/(^-|-$)/g, '');
      }
    } catch (e) {
      console.warn('Could not parse manifest.webapp:', e);
    }
  }

  const distFiles = fs.readdirSync(distDir);
  for (const file of distFiles) {
    const filePath = path.join(distDir, file);
    const stat = fs.statSync(filePath);
    if (stat.isFile()) {
      // Don't include existing zip archives inside application.zip
      if (file.endsWith('.zip')) continue;
      const content = fs.readFileSync(filePath);
      appZip.file(file, content);
    }
  }

  // Ensure manifest.webapp is present inside application.zip
  if (fs.existsSync(manifestPath) && !distFiles.includes('manifest.webapp')) {
    const manifestContent = fs.readFileSync(manifestPath);
    appZip.file('manifest.webapp', manifestContent);
  }

  const appZipBuffer = await appZip.generateAsync({
    type: 'nodebuffer',
    compression: 'DEFLATE',
    compressionOptions: { level: 9 }
  });

  // Save standalone application.zip
  const appZipPath = path.join(distDir, 'application.zip');
  fs.writeFileSync(appZipPath, appZipBuffer);
  if (fs.existsSync(publicDir)) {
    fs.writeFileSync(path.join(publicDir, 'application.zip'), appZipBuffer);
  }
  console.log('Created application.zip (size:', (appZipBuffer.length / 1024 / 1024).toFixed(2), 'MB)');

  // 2. Create Outer OmniSD Package Zip containing application.zip, update.webapp, metadata.json
  const omniZip = new JSZip();

  // Add application.zip
  omniZip.file('application.zip', appZipBuffer);

  // Add empty update.webapp file
  omniZip.file('update.webapp', '');

  // Add metadata.json file in exact OmniSD format
  const metadataContent = JSON.stringify({
    version: 1,
    manifestURL: `app://${appId}/manifest.webapp`
  }, null, 2);

  omniZip.file('metadata.json', metadataContent);

  const omniZipBuffer = await omniZip.generateAsync({
    type: 'nodebuffer',
    compression: 'STORE' // Outer zip holds pre-compressed application.zip
  });

  const omniZipPath = path.join(distDir, 'omnisd_package.zip');
  fs.writeFileSync(omniZipPath, omniZipBuffer);
  if (fs.existsSync(publicDir)) {
    fs.writeFileSync(path.join(publicDir, 'omnisd_package.zip'), omniZipBuffer);
  }

  console.log('Created OmniSD package: omnisd_package.zip (size:', (omniZipBuffer.length / 1024 / 1024).toFixed(2), 'MB)');
  console.log('Manifest URL configured as:', `app://${appId}/manifest.webapp`);
  console.log('=== OmniSD Sideloading Packaging Complete! ===');
}

createOmniSDPackage().catch((err) => {
  console.error('Error packaging OmniSD:', err);
  process.exit(1);
});
