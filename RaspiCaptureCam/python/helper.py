import cv2
from gpiozero import MotionSensor

pir = MotionSensor(27)
vid = cv2.VideoCapture(0)

while True:
    pir.wait_for_motion()
    print("Motion detected!")

    while True:
        ret, frame = vid.read()
        cv2.imshow('frame', frame)
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break
    pir.wait_for_no_motion()

vid.release()
cv2.destroyAllWindows()