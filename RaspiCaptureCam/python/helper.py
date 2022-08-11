from time import sleep
import cv2
import os
from time import perf_counter
from gpiozero import MotionSensor

if __name__ == "__main__":
    pir = MotionSensor(27)
    vid = cv2.VideoCapture(0)
    print("File started!!!")
    while True:
        t1_start = perf_counter()
        pir.wait_for_motion()
        print("Motion detected! Sleep 3s before captured")
        sleep(3)
        ret, frame = vid.read()
        cv2.imwrite('savedImage.png', frame)
        print("image saved at savedImage.png. Processing...")
        predict()
        t1_stop = perf_counter()
        print("Processed done. takes ", t1_stop - t1_start)
        pir.wait_for_no_motion()
        print("not detected motion.")

    vid.release()