from PIL import Image

# Input file
input_file = "Default_metalRoughness.jpg"

# Load image
img = Image.open(input_file).convert("RGB")
r, g, b = img.split()  # r=Red, g=Green, b=Blue

# Metallic image (from Blue channel -> Red channel)
metal_img = Image.merge("RGB", (b, b, b))  # optional: just keep R, but some viewers prefer RGB
metal_img.save("Default_metal.jpg")

# Roughness image (from Green channel -> Red channel)
roughness_img = Image.merge("RGB", (g, g, g))
roughness_img.save("Default_roughness.jpg")

print("Done! Created Default_metal.jpg and Default_roughness.jpg")
