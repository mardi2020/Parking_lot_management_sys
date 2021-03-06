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

from PyQt5.QtWidgets import *
from PyQt5 import uic, QtCore
from PyQt5.QtGui import QPixmap
from client import *


form_class = uic.loadUiType('untitled_.ui')[0]

# 모델 불러오기 (Darknet)
model = Darknet("config/yolov3-custom.cfg", img_size=416) # 번호판 탐지
model2 = Darknet("config/yolov3-custom2.cfg", img_size=416) # 숫자 탐지

# 모델 파라미터 불러오기
model.load_state_dict(torch.load("checkpoints_plate/yolov3_ckpt_70.pth"))
model2.load_state_dict(torch.load("checkpoints_number/yolov3_ckpt_99.pth"))
model.eval()
model2.eval()

hangul = { 'ga':"가",'na':"나", 'da':"다", 'ra':"라", 'ma':"마", 'ba':"바", 'sa':"사", 'a':"아", 'ja':"자", 'geo':"조",
           'neo':"너", 'deo':"더", 'reo':"러", 'meo':"머", 'beo':"버", 'seo':"서", 'eo':"어", 'jeo':"저",
           'go':"고", 'no':'노', 'do':'도', 'ro':'로', 'mo':'모', 'bo':'보', 'so':'소', 'o':'오', 'jo':'조',
           'gu':'구', 'nu':'누', 'du':'두', 'ru':'루', 'mu':'무', 'bu':'부', 'su':'수', 'u':'우', 'ju':"주",
           'ha':"하", 'heo':"허", 'ho':"호"}


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

class WindowClass(QMainWindow, form_class) :
    def __init__(self):
        super().__init__()
        self.setupUi(self)

        # 버튼 연결
        self.findfile_btn.clicked.connect(self.find_slot)
        self.settleup_btn.clicked.connect(self.settleup_slot)
        self.parking_btn.clicked.connect(self.parking_slot)
        self.pushButton.clicked.connect(self.close_slot)
        self.pushButton_2.clicked.connect(self.Info_slot)
        self.App = Application()

    def close_slot(self) :
        sys.exit()

    def find_slot(self) :
        fname, _ = QFileDialog.getOpenFileName(self, 'Open file', './', 'Image files (*.jpg *.gif, *jfif)')

        if fname and os.path.isfile(fname):
            print(fname)
            pixmap = QPixmap(fname)
            pixmap =  pixmap.scaled(pixmap.width() // 3, pixmap.height() // 3, QtCore.Qt.KeepAspectRatio)
            self.label_6.setPixmap(pixmap)

            # 사진 경로를 모델로 전송
            imagedata = cv2.imread(fname)
            plate_number = generate_plate_num(model, model2, imagedata)
            print(plate_number[0])
            pln = plate_number[0]
            h = ""
            for i in range(2, len(pln)):
                if not pln[i].isdigit() :
                    h += pln[i]
            pln = pln.replace(h, hangul[h]+' ')
            print(pln)
            self.recog_carnum.setText(pln)

            return pln
        
    def settleup_slot(self):
        #carnum = find_slot()
        #self.recog_carnum.setText(carnum)
        carnum = self.recog_carnum.text()
        self.App.SettleupExpense(carnum)
        exp = self.App.Expense()
        self.expense.setText(str(exp))
        print("settle up")

    def parking_slot(self):
        loc = self.want_parking.text()
        print(loc)
        carnum = self.recog_carnum.text()
        #App = Application()
        self.App.AddCar(loc, carnum)
        print("parking")

    def Info_slot(self) :
        space = self.App.RequestPLinfo()
        s = ""
        for i in range(len(space)) :
            if space[i] == 0 :
                s += str(i+1)
                s += ", "
        s += "번 자리"
        self.empty.setText(s)

        #self.parking_slot()


app = QApplication([])
win = WindowClass()
win.show()
app.exec_()
