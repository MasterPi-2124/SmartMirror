import os
import cv2
import numpy as np
from tensorflow.keras.applications import MobileNetV2
from tensorflow.keras import Model
from tensorflow.keras.layers import *
from tensorflow.keras.losses import CategoricalCrossentropy
from tensorflow.keras.preprocessing.image import load_img
from tensorflow.keras.models import load_model
from time import sleep
from time import perf_counter
from gpiozero import MotionSensor


def build_model():
    img_size = (224, 224)
    base_model = MobileNetV2(weights=None, include_top=False, input_shape=img_size+(3,), alpha=0.75)
    x = base_model.get_layer('block_12_project').output
    x = GlobalAveragePooling2D()(x)
    x = BatchNormalization()(x)
    x = Dropout(0.3)(x)
    output = Dense(7, activation='softmax', kernel_initializer='he_normal')(x)
    model = Model(inputs=base_model.input, outputs=output)
    return model
    #model.summary()

working_directory = os.getcwd()
print(working_directory)
img_path = '{}/savedImage.png'.format(working_directory)
model_link = '{}/result.h5'.format(working_directory)
model = build_model()
model.load_weights(model_link)

def predict():
    img_size = (224, 224)
    classes = ['1','L','nogesture','paper','rock','scissor','u']
    img =  cv2.imread(img_path)
    img = cv2.resize(img, img_size)
    img = img/255.0
    img = img.reshape(1,224,224,3)
    
    #print(img.shape)
    y = model.predict(img)
    print(classes[np.argmax(y[0])])
    return

if __name__ == "__main__":
    pir = MotionSensor(27)
    print("File started!!!")
    while True:
        pir.wait_for_motion()
        t1_start = perf_counter()
        vid = cv2.VideoCapture(0)
        print("Motion detected! Sleep 3s before captured")
        sleep(3)
        ret, frame = vid.read()
        cv2.imwrite('savedImage.png', frame)
        vid.release()
        print("image saved at savedImage.png. Processing...")
        predict()
        t1_stop = perf_counter()
        print("Processed done. takes ", t1_stop - t1_start)
        pir.wait_for_no_motion()
        print("not detected motion.")
