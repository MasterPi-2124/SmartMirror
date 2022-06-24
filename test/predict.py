from time import perf_counter
import time
from tensorflow.keras.models import Sequential, Model 
from tensorflow.keras.layers import *
import os
import cv2
from tensorflow.python.keras import regularizers 
import numpy as np
from tensorflow.keras.callbacks import ModelCheckpoint, LearningRateScheduler, EarlyStopping, ReduceLROnPlateau
from sklearn.metrics import classification_report
import matplotlib.pyplot as plt
from tensorflow.keras.optimizers import Adam
from tensorflow.keras.applications import MobileNetV2
from tensorflow.keras import Model
import random
import keras
import re
from tensorflow.keras.models import load_model
import imutils
import tensorflow as tf
from datetime import datetime



data_dir = "datn_data/"
img_size = (224, 224)
seq_len = 16
classes = ["baby_shark", "bravo", "scissor", "stop", "typing", "no_gesture"]
class DepthwiseConv3D(keras.layers.Layer):
    def __init__(self, my_kernel_size, my_padding='VALID', my_strides=1, **kwargs):
        super(DepthwiseConv3D, self).__init__(**kwargs)
        self.my_kernel_size = my_kernel_size
        self.my_padding = my_padding #need upper case
        self.my_strides = my_strides

    def build(self, input_shape):
        bs, sl, h, w, f = input_shape
        self.w = self.add_weight(
            shape=(self.my_kernel_size, self.my_kernel_size, self.my_kernel_size, 1, f),
            initializer="random_normal",
            trainable=True,
            name='Variable0'
        )
        self.b = self.add_weight(
            shape=(f,), initializer="random_normal", trainable=True, name='Variable1'
        )

    def call(self, inputs):
        return tf.concat([(tf.nn.convolution(inputs[:,:,:,:,i,tf.newaxis], self.w[:,:,:,:,tf.newaxis,i], strides=self.my_strides, padding=self.my_padding) + self.b[i]) 
                            for i in range(inputs.shape[-1])], axis=-1)

    def get_config(self):
        config = super(DepthwiseConv3D, self).get_config()
        config.update({
            "my_kernel_size": self.my_kernel_size,
            "my_padding": self.my_padding,
            "my_strides": self.my_strides
        })
        return config
def downsize_block(x, filter1, filter2):
    x = TimeDistributed(Conv2D(filter1, 1, kernel_initializer='he_normal'))(x)
    x = BatchNormalization()(x)
    x = ReLU()(x)
    x = ZeroPadding3D(padding=(1, 1, 1))(x)
    x = DepthwiseConv3D(3, my_strides=2)(x)
    x = BatchNormalization()(x)
    x = ReLU()(x)
    x = TimeDistributed(Conv2D(filter2, 1, kernel_initializer='he_normal'))(x)
    x = BatchNormalization()(x)
    return x

def residual_block(x, filter1, filter2):
    m = TimeDistributed(Conv2D(filter1, 1, kernel_initializer='he_normal'))(x)
    m = BatchNormalization()(m)
    m = ReLU()(m)
    m = DepthwiseConv3D(3, my_padding="SAME")(m)
    m = BatchNormalization()(m)
    m = ReLU()(m)
    m = TimeDistributed(Conv2D(filter2, 1, kernel_initializer='he_normal'))(m)
    m = BatchNormalization()(m)
    return Add()([x, m])

def normal_block(x, filter1, filter2):
    x = TimeDistributed(Conv2D(filter1, 1, kernel_initializer='he_normal'))(x)
    x = BatchNormalization()(x)
    x = ReLU()(x)
    x = DepthwiseConv3D(3, my_padding="SAME")(x)
    x = BatchNormalization()(x)
    x = ReLU()(x)
    x = TimeDistributed(Conv2D(filter2, 1, kernel_initializer='he_normal'))(x)
    x = BatchNormalization()(x)
    return x
def build_model():
    input = Input(shape=(16,) + (224, 224) + (3,))
    x = ZeroPadding3D(padding=(1, 1, 1))(input)
    x = Conv3D(32, 3, strides=2, kernel_initializer='he_normal')(x)
    x = BatchNormalization()(x)
    x = ReLU()(x)
    x = DepthwiseConv3D(3, my_padding="SAME")(x)
    x = BatchNormalization()(x)
    x = ReLU()(x)
    x = TimeDistributed(Conv2D(16, 1, kernel_initializer='he_normal'))(x)
    x = BatchNormalization()(x)
    
    x = downsize_block(x, 64, 24)
    #x = Dropout(0.2)(x)
    
    x = residual_block(x, 96, 24)
    x = downsize_block(x, 96, 32)
    #x = Dropout(0.2)(x)
    
    x = residual_block(x, 128, 32)
    x = downsize_block(x, 128, 64)
    #x = Dropout(0.2)(x)
    
    x = residual_block(x, 256, 64)
    x = residual_block(x, 256, 64)
    x = normal_block(x, 256, 96)
    
    x = TimeDistributed(Conv2D(256, 1, kernel_initializer='he_normal'))(x)
    x = BatchNormalization()(x)
    x = ReLU()(x)
    x = GlobalAveragePooling3D()(x)
    x = Dropout(0.5)(x)
    
    output = Dense(6, activation = "softmax", kernel_initializer='he_normal')(x)
    model = Model(inputs=input, outputs=output)
    model.summary()
    return model
def atoi(text):
    return int(text) if text.isdigit() else text


def natural_keys(text):
    '''
    alist.sort(key=natural_keys) sorts in human order
    '''
    return [atoi(c) for c in re.split(r'(\d+)', text)]

def frame_extraction(video_path):
    frames_list = []
    img_list = os.listdir(video_path)
    img_list.sort(key=natural_keys)
    #index = [1,3,5,7,9,11,13,15,17,19,21,23,25,27,28,29]
    for i in img_list:
        #if j in index:
            image = cv2.imread(os.path.join(video_path, i))
            image = cv2.resize(image, img_size)
            frames_list.append(image)
    frames_list = np.asarray(frames_list)
    return frames_list
output_folder = r"D:/Hoctap/20211/datn/code_datn/res"
vid_link = r"D:/Hoctap/20211/datn/code_datn/res"


model = build_model()
def predict(model_link=None, output_folder=output_folder):
    classes = ["baby_shark", "bravo", "scissor", "stop", "typing", "no_gesture"]
    
    # model.compile(loss='categorical_crossentropy', optimizer=Adam(0.0001), metrics=["accuracy"])
    #model.load_weights('model_test.h5')
    # model = load_model(model_link)
    # for i, w in enumerate(model.weights):
      # print(i, w.name)
    # model.save_weights('my_weights.h5')
    img = frame_extraction(output_folder)
    img = (img / 127.5) - 1
    img = np.array([img])
    print(img.shape)
    y = model.predict(img)
    print(y.shape)
    print(classes[np.argmax(y[0])])
    return

while True:
    t1_start = perf_counter()
    predict(output_folder=vid_link)
    t1_stop = perf_counter()
    print("Elapsed time during the whole program in seconds:",t1_stop-t1_start)
    time.sleep(5)