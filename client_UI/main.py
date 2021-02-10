import sys
import os
from PyQt5.QtWidgets import *
from PyQt5 import uic, QtCore
from PyQt5.QtGui import QPixmap
from client import *

form_class = uic.loadUiType('untitled_.ui')[0]

class WindowClass(QMainWindow, form_class) :
    def __init__(self):
        super().__init__()
        self.setupUi(self)

        # 버튼 연결
        self.findfile_btn.clicked.connect(self.find_slot)
        self.settleup_btn.clicked.connect(self.settleup_slot)
        self.parking_btn.clicked.connect(self.parking_slot)

    def find_slot(self) :
        fname, _ = QFileDialog.getOpenFileName(self, 'Open file', './', 'Image files (*.jpg *.gif, *jfif)')

        if fname and os.path.isfile(fname):
            print(fname)
            pixmap = QPixmap(fname)
            pixmap =  pixmap.scaled(pixmap.width() // 3, pixmap.height() // 3, QtCore.Qt.KeepAspectRatio)
            self.label_6.setPixmap(pixmap)

        # 사진파일을 서버로 전송
        App = Application()
        #App.SendToServer(fname)
        App.Send2Server(fname)

    def settleup_slot(self):
        self.recog_carnum.setText("11가 1111")
        App = Application()
        App.SettleupExpense()
        exp = App.Expense()
        self.expense.setText(str(exp))
        print("settle up")

    def parking_slot(self):
        print("parking")


app = QApplication([])
win = WindowClass()
win.show()
app.exec_()