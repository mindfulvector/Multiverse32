#!/usr/bin/env python3
"""
Script to create a Multiverse32 app icon
"""

from PIL import Image, ImageDraw, ImageFont
import os

def create_multiverse_icon():
    # Create a 1024x1024 image for the icon
    size = 1024
    img = Image.new('RGBA', (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    
    # Create a cosmic/multiverse themed icon
    # Background gradient (dark space)
    for y in range(size):
        alpha = int(255 * (1 - y / size * 0.3))
        color = (10, 5, 30, alpha)
        draw.line([(0, y), (size, y)], fill=color)
    
    # Draw multiple overlapping circles representing different universes
    center = size // 2
    
    # Large central universe (blue-purple)
    draw.ellipse([center-200, center-200, center+200, center+200], 
                fill=(100, 50, 200, 180), outline=(150, 100, 255, 255), width=8)
    
    # Smaller overlapping universes
    universes = [
        (center-120, center-120, 80, (200, 100, 50, 160)),  # Orange
        (center+60, center-80, 70, (50, 200, 100, 160)),    # Green
        (center-80, center+80, 60, (200, 50, 150, 160)),    # Pink
        (center+100, center+100, 50, (100, 200, 200, 160)), # Cyan
    ]
    
    for x, y, radius, color in universes:
        draw.ellipse([x-radius, y-radius, x+radius, y+radius], 
                    fill=color, outline=(255, 255, 255, 200), width=4)
    
    # Add some stars
    import random
    random.seed(42)  # For consistent results
    for _ in range(50):
        x = random.randint(0, size)
        y = random.randint(0, size)
        star_size = random.randint(2, 6)
        brightness = random.randint(150, 255)
        draw.ellipse([x-star_size//2, y-star_size//2, x+star_size//2, y+star_size//2], 
                    fill=(brightness, brightness, brightness, 200))
    
    # Add "32" text in the center
    try:
        # Try to use a system font
        font_size = 120
        font = ImageFont.truetype("/System/Library/Fonts/Helvetica.ttc", font_size)
    except:
        # Fallback to default font
        font = ImageFont.load_default()
    
    text = "32"
    # Get text bounding box
    bbox = draw.textbbox((0, 0), text, font=font)
    text_width = bbox[2] - bbox[0]
    text_height = bbox[3] - bbox[1]
    
    # Draw text with outline
    text_x = center - text_width // 2
    text_y = center - text_height // 2
    
    # Text outline
    for dx in [-2, -1, 0, 1, 2]:
        for dy in [-2, -1, 0, 1, 2]:
            if dx != 0 or dy != 0:
                draw.text((text_x + dx, text_y + dy), text, font=font, fill=(0, 0, 0, 255))
    
    # Main text
    draw.text((text_x, text_y), text, font=font, fill=(255, 255, 255, 255))
    
    return img

def create_icns_file():
    """Create an .icns file for macOS"""
    # Create the base icon
    base_icon = create_multiverse_icon()
    
    # Create different sizes needed for .icns
    sizes = [16, 32, 64, 128, 256, 512, 1024]
    
    # Create Resources directory
    os.makedirs("Resources", exist_ok=True)
    
    # Save individual PNG files for iconutil
    iconset_dir = "Resources/Multiverse32.iconset"
    os.makedirs(iconset_dir, exist_ok=True)
    
    for size in sizes:
        # Regular resolution
        resized = base_icon.resize((size, size), Image.Resampling.LANCZOS)
        resized.save(f"{iconset_dir}/icon_{size}x{size}.png")
        
        # High resolution (@2x) for larger sizes
        if size >= 16 and size <= 512:
            high_res = base_icon.resize((size * 2, size * 2), Image.Resampling.LANCZOS)
            high_res.save(f"{iconset_dir}/icon_{size}x{size}@2x.png")
    
    print(f"Created iconset at {iconset_dir}")
    print("To create .icns file, run: iconutil -c icns Resources/Multiverse32.iconset")
    
    # Also save a large PNG for reference
    base_icon.save("Resources/Multiverse32_1024.png")
    print("Saved reference PNG at Resources/Multiverse32_1024.png")

if __name__ == "__main__":
    try:
        create_icns_file()
        print("Icon creation completed successfully!")
    except ImportError:
        print("PIL (Pillow) is required to create the icon.")
        print("Install it with: pip install Pillow")
    except Exception as e:
        print(f"Error creating icon: {e}")
