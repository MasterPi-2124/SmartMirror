import os
import cv2
import numpy as np
from tensorflow.keras.applications import MobileNetV2
from tensorflow.keras import Model
from tensorflow.keras.layers import *
from tensorflow.keras.losses import CategoricalCrossentropy
from tensorflow.keras.preprocessing.image import load_img
from tensorflow.keras.models import load_model
import tensorflow as tf
import seaborn as sn
import pandas as pd
import random
import time
from datetime import datetime


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
img_path = '{}/res/result.png'.format(working_directory)
model_link = '{}/result.h5'.format(working_directory)
model = build_model()
model.load_weights(model_link)

def predict():
    img_size = (224, 224)
    classes = ['1','L','nogesture','paper','rock','scissor','u']
    #model = load_model(model_link)
    # for i, w in enumerate(model.weights):
      # print(i, w.name)
    # model.save_weights('my_weights.h5')
    img =  cv2.imread(img_path)
    img = cv2.resize(img, img_size)
    img = img/255.0
    img = img.reshape(1,224,224,3)
    
    #print(img.shape)
    y = model.predict(img)
    print(classes[np.argmax(y[0])])
    return

while True:
    predict()
    time.sleep(5)
