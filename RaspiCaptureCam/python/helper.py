from gpiozero import MotionSensor
import cv2
from pip import main

def capture():
    vid = cv2.VideoCapture(0)
    while True:
        ret, frame = vid.read()
        break

    vid.release()

if __name__ == "__main__":

    pir = MotionSensor(4)


    while True:
        pir.wait_for_active()
        print("detect")
        capture()
        pir.wait_for_inactive()
