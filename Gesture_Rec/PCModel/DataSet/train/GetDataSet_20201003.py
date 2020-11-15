import numpy as np
import cv2

if __name__ == "__main__":
    str_type = 'left_3'
    raw_data = np.genfromtxt("" + 'amg' + str_type + '.txt', delimiter=",", dtype=int)
    formed_data = raw_data.reshape((-1, 8, 8))
    print(formed_data.shape[0])
    for i in range(formed_data.shape[0]):
        cv2.imwrite(str_type + str(i) + ".jpg", formed_data[i])
