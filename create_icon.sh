#!/bin/bash

# Create Multiverse32 app icon using macOS built-in tools

# Create Resources directory
mkdir -p Resources

# Create a simple icon using macOS built-in tools
# We'll create an SVG first, then convert it

cat > Resources/icon_temp.svg << 'EOF'
<?xml version="1.0" encoding="UTF-8"?>
<svg width="1024" height="1024" viewBox="0 0 1024 1024" xmlns="http://www.w3.org/2000/svg">
  <defs>
    <radialGradient id="spaceGradient" cx="50%" cy="50%" r="50%">
      <stop offset="0%" style="stop-color:#1a0a3a;stop-opacity:1" />
      <stop offset="100%" style="stop-color:#0a051e;stop-opacity:1" />
    </radialGradient>
    <radialGradient id="universeGradient1" cx="50%" cy="50%" r="50%">
      <stop offset="0%" style="stop-color:#6432c8;stop-opacity:0.8" />
      <stop offset="100%" style="stop-color:#9664ff;stop-opacity:0.3" />
    </radialGradient>
    <radialGradient id="universeGradient2" cx="50%" cy="50%" r="50%">
      <stop offset="0%" style="stop-color:#c86432;stop-opacity:0.7" />
      <stop offset="100%" style="stop-color:#ff9664;stop-opacity:0.3" />
    </radialGradient>
    <radialGradient id="universeGradient3" cx="50%" cy="50%" r="50%">
      <stop offset="0%" style="stop-color:#32c864;stop-opacity:0.7" />
      <stop offset="100%" style="stop-color:#64ff96;stop-opacity:0.3" />
    </radialGradient>
    <radialGradient id="universeGradient4" cx="50%" cy="50%" r="50%">
      <stop offset="0%" style="stop-color:#c83296;stop-opacity:0.7" />
      <stop offset="100%" style="stop-color:#ff64c8;stop-opacity:0.3" />
    </radialGradient>
  </defs>
  
  <!-- Space background -->
  <rect width="1024" height="1024" fill="url(#spaceGradient)"/>
  
  <!-- Stars -->
  <circle cx="100" cy="150" r="3" fill="white" opacity="0.8"/>
  <circle cx="200" cy="100" r="2" fill="white" opacity="0.9"/>
  <circle cx="300" cy="200" r="4" fill="white" opacity="0.7"/>
  <circle cx="150" cy="300" r="2" fill="white" opacity="0.8"/>
  <circle cx="400" cy="120" r="3" fill="white" opacity="0.9"/>
  <circle cx="500" cy="180" r="2" fill="white" opacity="0.7"/>
  <circle cx="600" cy="80" r="3" fill="white" opacity="0.8"/>
  <circle cx="700" cy="250" r="2" fill="white" opacity="0.9"/>
  <circle cx="800" cy="150" r="4" fill="white" opacity="0.7"/>
  <circle cx="900" cy="200" r="3" fill="white" opacity="0.8"/>
  <circle cx="120" cy="400" r="2" fill="white" opacity="0.9"/>
  <circle cx="250" cy="450" r="3" fill="white" opacity="0.7"/>
  <circle cx="380" cy="380" r="2" fill="white" opacity="0.8"/>
  <circle cx="520" cy="420" r="4" fill="white" opacity="0.9"/>
  <circle cx="650" cy="350" r="2" fill="white" opacity="0.7"/>
  <circle cx="780" cy="400" r="3" fill="white" opacity="0.8"/>
  <circle cx="850" cy="480" r="2" fill="white" opacity="0.9"/>
  <circle cx="180" cy="600" r="3" fill="white" opacity="0.7"/>
  <circle cx="320" cy="650" r="2" fill="white" opacity="0.8"/>
  <circle cx="480" cy="580" r="4" fill="white" opacity="0.9"/>
  <circle cx="620" cy="620" r="2" fill="white" opacity="0.7"/>
  <circle cx="750" cy="680" r="3" fill="white" opacity="0.8"/>
  <circle cx="880" cy="600" r="2" fill="white" opacity="0.9"/>
  <circle cx="150" cy="800" r="3" fill="white" opacity="0.7"/>
  <circle cx="300" cy="850" r="2" fill="white" opacity="0.8"/>
  <circle cx="450" cy="780" r="4" fill="white" opacity="0.9"/>
  <circle cx="600" cy="820" r="2" fill="white" opacity="0.7"/>
  <circle cx="750" cy="880" r="3" fill="white" opacity="0.8"/>
  <circle cx="900" cy="800" r="2" fill="white" opacity="0.9"/>
  
  <!-- Central large universe -->
  <circle cx="512" cy="512" r="200" fill="url(#universeGradient1)" stroke="#9664ff" stroke-width="8"/>
  
  <!-- Overlapping smaller universes -->
  <circle cx="392" cy="392" r="80" fill="url(#universeGradient2)" stroke="#ff9664" stroke-width="4"/>
  <circle cx="572" cy="432" r="70" fill="url(#universeGradient3)" stroke="#64ff96" stroke-width="4"/>
  <circle cx="432" cy="592" r="60" fill="url(#universeGradient4)" stroke="#ff64c8" stroke-width="4"/>
  <circle cx="612" cy="612" r="50" fill="url(#universeGradient2)" stroke="#64c8ff" stroke-width="4"/>
  
  <!-- "32" text -->
  <text x="512" y="580" font-family="Helvetica, Arial, sans-serif" font-size="120" font-weight="bold" 
        text-anchor="middle" fill="white" stroke="black" stroke-width="3">32</text>
</svg>
EOF

echo "Created SVG icon template"

# Convert SVG to PNG using built-in tools
# First try with qlmanage (Quick Look)
if command -v qlmanage &> /dev/null; then
    echo "Converting SVG to PNG using qlmanage..."
    qlmanage -t -s 1024 -o Resources/ Resources/icon_temp.svg
    if [ -f "Resources/icon_temp.svg.png" ]; then
        mv "Resources/icon_temp.svg.png" "Resources/Multiverse32_1024.png"
        echo "Successfully created PNG icon"
    fi
fi

# If qlmanage didn't work, try with rsvg-convert (if available via Homebrew)
if [ ! -f "Resources/Multiverse32_1024.png" ] && command -v rsvg-convert &> /dev/null; then
    echo "Converting SVG to PNG using rsvg-convert..."
    rsvg-convert -w 1024 -h 1024 Resources/icon_temp.svg -o Resources/Multiverse32_1024.png
fi

# If we have the PNG, create the iconset
if [ -f "Resources/Multiverse32_1024.png" ]; then
    echo "Creating iconset..."
    mkdir -p Resources/Multiverse32.iconset
    
    # Create different sizes
    sizes=(16 32 64 128 256 512 1024)
    for size in "${sizes[@]}"; do
        sips -z $size $size Resources/Multiverse32_1024.png --out Resources/Multiverse32.iconset/icon_${size}x${size}.png
        
        # Create @2x versions for smaller sizes
        if [ $size -le 512 ]; then
            double_size=$((size * 2))
            sips -z $double_size $double_size Resources/Multiverse32_1024.png --out Resources/Multiverse32.iconset/icon_${size}x${size}@2x.png
        fi
    done
    
    echo "Created iconset with all required sizes"
    
    # Create .icns file
    iconutil -c icns Resources/Multiverse32.iconset
    echo "Created Multiverse32.icns"
    
    # Move the .icns file to Resources
    if [ -f "Multiverse32.icns" ]; then
        mv Multiverse32.icns Resources/
        echo "Moved .icns file to Resources/"
    fi
else
    echo "Warning: Could not create PNG from SVG. You may need to install rsvg-convert:"
    echo "brew install librsvg"
    echo ""
    echo "For now, creating a simple text-based icon..."
    
    # Create a simple fallback using ImageMagick if available, or just use the SVG
    if command -v convert &> /dev/null; then
        convert -size 1024x1024 xc:'#1a0a3a' \
                -fill '#6432c8' -draw 'circle 512,512 312,512' \
                -fill white -pointsize 120 -gravity center -annotate +0+0 '32' \
                Resources/Multiverse32_1024.png
        echo "Created fallback PNG icon using ImageMagick"
    fi
fi

# Clean up temporary files
rm -f Resources/icon_temp.svg

echo "Icon creation completed!"
echo "Files created:"
ls -la Resources/
