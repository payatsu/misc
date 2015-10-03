#!/usr/bin/python3

from PIL import Image

height = 1080
max_grad = 255
img = Image.new('RGB', (max_grad+1, height))
[img.paste((i, 0, 0), (i, 0, max_grad+1, height)) for i in range(max_grad+1)]
img.show()
