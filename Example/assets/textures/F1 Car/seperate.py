import os
from PIL import Image

# Process every metallicRoughness image in the directory
for file in os.listdir("."):
    if not file.lower().endswith("metallicroughness.png"):
        continue

    print("Processing:", file)

    # Load image
    img = Image.open(file).convert("RGB")
    r, g, b = img.split()

    # Build output names
    base = file.replace("_metallicRoughness.png", "")
    metal_out = f"{base}_metallic.png"
    rough_out = f"{base}_roughness.png"

    # Metallic = Blue channel
    metal_img = Image.merge("RGB", (b, b, b))
    metal_img.save(metal_out)

    # Roughness = Green channel
    rough_img = Image.merge("RGB", (g, g, g))
    rough_img.save(rough_out)

    print(" → Saved:", metal_out)
    print(" → Saved:", rough_out)

print("Done!")
