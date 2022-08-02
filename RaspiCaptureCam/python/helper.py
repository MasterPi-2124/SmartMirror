
# import the opencv library
import cv2

from gpiozero import MotionSensor

# pir pin = 17
pir = MotionSensor(27)
# define a video capture object
vid = cv2.VideoCapture(0)

while True:
	pir.wait_for_motion()
	print("Motion detected!")
	ret, frame = vid.read()
    cv2.imshow('frame', frame)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break
    pir.wait_for_no_motion()
  
# After the loop release the cap object
vid.release()
# Destroy all the windows
cv2.destroyAllWindows()
