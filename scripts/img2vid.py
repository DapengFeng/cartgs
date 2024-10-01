import cv2
import os
import argparse

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-i", type=str, help="Dir of images", required=True)
    parser.add_argument("-o", type=str, help="Path to video", required=True)

    args = parser.parse_args()

    files = os.listdir(args.i)
    files = sorted(files)

    img0 = cv2.imread(os.path.join(args.i, files[0]))
    h, w = img0.shape[0], img0.shape[1]
    fourcc = cv2.VideoWriter_fourcc(*"h264")

    video_writer = cv2.VideoWriter(args.o, fourcc, 30, (w, h))
    for i in range(len(files)):
        try:
            img = cv2.imread(os.path.join(args.i, "{}.jpg".format(i)))
            video_writer.write(img)
        except Exception:
            continue

    video_writer.release()
