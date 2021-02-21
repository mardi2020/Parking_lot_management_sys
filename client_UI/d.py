from __future__ import division

from models import *
from utils.utils import *
from utils.datasets import *
from utils.augmentations import *
from utils.transforms import *

import os
import sys
import time
import datetime
import argparse

from PIL import Image

import torch
import torchvision.transforms as transforms
from torch.utils.data import DataLoader
from torchvision import datasets
from torch.autograd import Variable

from matplotlib.ticker import NullLocator

import cv2
import numpy as np


# 모델 불러오기 (Darknet)
model = Darknet("config/yolov3-custom.cfg", img_size=416) # 번호판 탐지
model2 = Darknet("config/yolov3-custom2.cfg", img_size=416) # 숫자 탐지

# 모델 파라미터 불러오기
model.load_state_dict(torch.load("checkpoints_plate/yolov3_ckpt_70.pth"))
model2.load_state_dict(torch.load("checkpoints_number/yolov3_ckpt_99.pth"))
model.eval()
model2.eval()

# 이미지 리드
image1 = cv2.imread("data/custom/images/192.168.0.101_20190720-11374138_Motion.jpg")
image2 = cv2.imread("data/custom/images/192.168.0.101_20190720-11452888_Motion.jpg")
image3 = cv2.imread("data/custom/images/192.168.0.101_20190725-07062822_Motion.jpg")
image4 = cv2.imread("data/custom/images/192.168.0.101_20190814-07514322_Motion.jpg")


# model : 번호판 탐지 모델
# model2 : 번호판에있는 숫자 탐지 모델
# imagedata : 인풋 이미지 (cv2 imread로 읽어야함)
def generate_plate_num(model, model2, imagedata):
    classes = ['0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'ga',
           'na', 'da', 'ra', 'ma', 'ba', 'sa', 'a', 'ja', 'geo',
           'neo', 'deo', 'reo', 'meo', 'beo', 'seo', 'eo', 'jeo',
           'go', 'no', 'do', 'ro', 'mo', 'bo', 'so', 'o', 'jo',
           'gu', 'nu', 'du', 'ru', 'mu', 'bu', 'su', 'u', 'ju',
           'ha', 'heo', 'ho']

    result = []
    Tensor = torch.FloatTensor
    
    img = np.array(Image.fromarray(imagedata).convert('RGB'), dtype=np.uint8)
    boxes = np.zeros((1,5))
    trans = transforms.Compose([DEFAULT_TRANSFORMS, Resize(416)])

    img, _ = trans((img,boxes))
    img = img.unsqueeze(0)
    input_imgs = Variable(img.type(Tensor))
    with torch.no_grad():
        detections = model(input_imgs)
        detections = non_max_suppression(detections, 0.8, 0.4)[0]
        
    image = np.array(Image.fromarray(imagedata))
    detections = rescale_boxes(detections, 416, image.shape[:2])
    
    for x1, y1, x2, y2, conf, cls_conf, cls_pred in detections:
        x1 = int(x1)
        x2 = int(x2)
        y1 = int(y1)
        y2 = int(y2)
        img2 = image[y1:y2,x1:x2]
        img2 = Image.fromarray(img2)
        img2 = np.array(img2.convert('RGB'), dtype=np.uint8)
        boxes = np.zeros((1,5))
        trans = transforms.Compose([DEFAULT_TRANSFORMS, Resize(416)])
        img2, _ = trans((img2,boxes))
        img2 = img2.unsqueeze(0)
        input_imgs2 = Variable(img2.type(Tensor))
        with torch.no_grad():
            detections2 = model2(input_imgs2)
            detections2 = non_max_suppression(detections2, 0.8, 0.4)[0]
            new_lst = []
            if (detections2 == None):
                continue
            for i in detections2:
                if (i[5] > 0.8):
                    new_lst.append([float(i[0]),float(i[1]),float(i[2]),float(i[3]),float(i[4]),float(i[5]),int(i[6])])
            new_lst = sorted(new_lst, key=lambda x: (x[0],x[1]))
            final_str = ""
            for i in new_lst:
                final_str += classes[i[6]]
            result.append(final_str)
    print(result)
    return result
        
prev_time = time.time()
generate_plate_num(model, model2, image1)
current_time = time.time()
inference_time = datetime.timedelta(seconds=current_time - prev_time)
print(inference_time)
prev_time = time.time()
generate_plate_num(model, model2, image2)
current_time = time.time()
inference_time = datetime.timedelta(seconds=current_time - prev_time)
print(inference_time)
prev_time = time.time()
generate_plate_num(model, model2, image3)
current_time = time.time()
inference_time = datetime.timedelta(seconds=current_time - prev_time)
print(inference_time)
prev_time = time.time()
generate_plate_num(model, model2, image4)
current_time = time.time()
inference_time = datetime.timedelta(seconds=current_time - prev_time)
print(inference_time)
