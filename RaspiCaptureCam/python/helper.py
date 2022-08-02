import cv2
import os
from gpiozero import MotionSensor

pir = MotionSensor(27)
vid = cv2.VideoCapture(0)

while True:
    pir.wait_for_motion()
    print("Motion detected!")
    cmd = 'raspistill -e png -o ~/Pictures/new_image.png -w 1280 -h 720 -vf -t 2000'
    os.system(cmd)
    pir.wait_for_no_motion()