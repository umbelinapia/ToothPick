import cv2
import numpy as np
import math

img = cv2.imread("image2.jpeg")
if img is None:
    raise RuntimeError("Could not read image")

h, w = img.shape[:2]

# Crop away border
margin_x = 80
margin_y = 120
roi = img[margin_y:h-margin_y, margin_x:w-margin_x]

# Grayscale
gray = cv2.cvtColor(roi, cv2.COLOR_BGR2GRAY)

# Threshold bright object
threshold_value = 180
_, mask = cv2.threshold(gray, threshold_value, 255, cv2.THRESH_BINARY)

# Morphology cleanup
kernel = np.ones((5, 5), np.uint8)
mask = cv2.morphologyEx(mask, cv2.MORPH_OPEN, kernel)
mask = cv2.morphologyEx(mask, cv2.MORPH_CLOSE, kernel)

# Find white pixels
ys, xs = np.where(mask > 0)

if len(xs) < 50:
    print("No cylinder detected")
else:
    cx = float(np.mean(xs))
    cy = float(np.mean(ys))
    radius = math.sqrt(len(xs) / math.pi)

    full_x = cx + margin_x
    full_y = cy + margin_y

    print(f"Detected centroid: ({full_x:.2f}, {full_y:.2f})")
    print(f"Estimated radius: {radius:.2f} px")

    # Draw on processed grayscale image
    processed_vis = cv2.cvtColor(gray, cv2.COLOR_GRAY2BGR)
    cv2.line(processed_vis, (int(cx)-20, int(cy)), (int(cx)+20, int(cy)), (0, 0, 255), 2)
    cv2.line(processed_vis, (int(cx), int(cy)-20), (int(cx), int(cy)+20), (0, 0, 255), 2)
    cv2.circle(processed_vis, (int(cx), int(cy)), int(radius), (0, 255, 0), 2)

    # Draw on original full image
    full_vis = img.copy()
    cv2.line(full_vis, (int(full_x)-25, int(full_y)), (int(full_x)+25, int(full_y)), (0, 255, 0), 3)
    cv2.line(full_vis, (int(full_x), int(full_y)-25), (int(full_x), int(full_y)+25), (0, 255, 0), 3)
    cv2.circle(full_vis, (int(full_x), int(full_y)), int(radius), (255, 255, 0), 3)

    # cv2.imshow("Gray", gray)
    cv2.imshow("Mask", mask)
    # cv2.imshow("Processed with cross", processed_vis)
    # cv2.imshow("Full image with centroid", full_vis)
    cv2.waitKey(0)
    cv2.destroyAllWindows()